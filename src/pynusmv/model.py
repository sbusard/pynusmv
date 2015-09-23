"""
The :mod:`pynusmv.model` module provides a way to define NuSMV modules in
Python: the :class:`Module` class represents a generic NuSMV module, and must
be subclassed to define specific NuSMV modules. This module also defines the
set of AST elements of the language.

"""

__all__ = ["Module"]

import sys
import re
import collections
from copy import deepcopy

from .utils import update, PointerWrapper
from .exception import NuSMVModuleError
from .parser import parse_identifier, parse_next_expression
from .node import (Modtype, Declaration, find_hierarchy, Boolean, Word,
                   SignedWord, UnsignedWord, ArrayType, Scalar, Node, Range,
                   Expression)

from .nusmv.node import node as nsnode
from .nusmv.utils import utils as nsutils


class ModuleMetaClass(type):

    """
    The meta class for modules, allowing modules to be printed.

    The string representation of the module is its NuSMV code. This
    representation includes:
    * the `NAME` member of the class, used as the name of the module;
      if absent, the name of the class is used.
    * the `ARGS` member, used as the list of arguments of the module;
      if ARGS is not defined, the module is declared without arguments.
    * members named after NuSMV module sections:
      VAR, IVAR, FROZENVAR, DEFINE, CONSTANTS, ASSIGN, TRANS, INIT, INVAR,
      FAIRNESS, JUSTICE, COMPASSION.

    Module sections must satisfy the following pattern:
    * pair-based sections such as VAR, IVAR, FROZENVAR, DEFINE and ASSIGN
      must be mapping objects where keys are identifiers and values are types
      (for VAR, IVAR and FROZENVAR) or expressions (for DEFINE and ASSIGN).
    * list-based sections such as CONSTANTS must be enumerations
      composed of elements of the section.
    * expression-based sections such as TRANS, INIT, INVAR, FAIRNESS, JUSTICE
      and COMPASSION must be enumerations composed of expressions.

    """

    # The list of module sections that are considered
    # each key is the section name
    # each value is a tuple giving:
    # * the type of expected value (mapping, enumeration, bodies)
    # * a format string for mappings, based on key and value,
    #   or the separator for enumerations,
    #   or nothing for bodies,
    #   used for printing the section body
    _sections = {"VAR": ("mapping", "{key}: {value};"),
                 "IVAR": ("mapping", "{key}: {value};"),
                 "FROZENVAR": ("mapping", "{key}: {value};"),
                 "DEFINE": ("mapping", "{key} := {value};"),
                 "ASSIGN": ("mapping", "{key} := {value};"),
                 "CONSTANTS": ("enumeration", ", "),
                 "TRANS": ("bodies",),
                 "INIT": ("bodies",),
                 "INVAR": ("bodies",),
                 "FAIRNESS": ("bodies",),
                 "JUSTICE": ("bodies",),
                 "COMPASSION": ("bodies",)}

    @classmethod
    def __prepare__(mcs, name, bases, **keywords):
        return collections.OrderedDict()

    def __new__(mcs, name, bases, namespace, **keywords):
        newnamespace = collections.OrderedDict()
        for member in namespace:
            # Update sections of namespace
            if member in mcs._sections:
                internal = mcs._section_internal(member, namespace[member])
                if member in newnamespace:
                    update(newnamespace[member], internal)
                else:
                    newnamespace[member] = internal
            # Update declarations of namespace
            elif isinstance(namespace[member], Declaration):
                decl = namespace[member]
                if decl._anonymous:
                    decl.name = member
                if decl.section not in newnamespace:
                    newnamespace[decl.section] = collections.OrderedDict()
                newnamespace[decl.section][decl] = decl.declared_type
                # Keep declarations in module
                newnamespace[member] = namespace[member]
            # Keep intact the other members
            else:
                newnamespace[member] = namespace[member]

        # Add NAME and ARGS if missing
        if "NAME" not in newnamespace:
            newnamespace["NAME"] = name
        if "ARGS" not in newnamespace:
            newnamespace["ARGS"] = []

        # Parse ARGS if necessary
        if len(newnamespace["ARGS"]):
            newnamespace["ARGS"] = mcs._args_internal(newnamespace["ARGS"])

        result = type.__new__(mcs, name, bases, dict(newnamespace))
        result.members = tuple(newnamespace)
        result.source = None
        return result

    def __getattr__(cls, name):
        if name in cls._sections:
            if cls._sections[name][0] == "mapping":
                setattr(cls, name, collections.OrderedDict())
            if cls._sections[name][0] in {"enumeration", "bodies"}:
                setattr(cls, name, [])
            cls.members += (name,)
            return getattr(cls, name)
        else:
            raise AttributeError("'{mcs}' class has no attribute '{name}'"
                                 .format(mcs=type(cls), name=name))

    @classmethod
    def _args_internal(mcs, args):
        """
        Return the internal representation of `args`. Each argument present
        in `args` is parsed as an identifier if it is a string, otherwise it
        is kept as it is.

        :param args: a list of module arguments

        """

        newargs = []
        for arg in args:
            if type(arg) is str:
                parsed = parse_identifier(arg)
                arg = Node.from_ptr(find_hierarchy(parsed))
                nsnode.free_node(parsed)
            newargs.append(arg)
        return newargs

    @classmethod
    def _section_internal(mcs, section, body):
        """
        Return the internal representation of `body` of `section`.

        This representation depends on the type of `section`.

        `section` is a mapping
        ----------------------

        The internal representation is a mapping where keys are identifiers
        and values are expressions or type identifiers.

        * If `body` is a single string, `body` is treated as the whole body of
          the section, and parsed accordingly.
        * If `body` is a mapping, it is copied and treated as the required
          mapping. This means that each key or value is parsed as the part it
          represent, if it is a string, or kept as it is otherwise.
        * If `body` is enumerable, each element is treated separated: either it
          is a string, and parsed as a key-value pair, otherwise it is a couple
          of values, and the first one is treated as a key (and parsed if
          necessary), and the second one as a value (and parsed if necessary).

        `section` is an enumeration
        ---------------------------

        The internal representation is an enumeration containing the different
        elements of the section.

        * If `body` is a single string, it is parsed as the whole body of the
          section.
        * If `body` is an enumerable, each element is parsed as a single
          element if it is a string, or kept as it is otherwise.

        `section` is a list of bodies
        -----------------------------

        The internal representation is an enumeration of expressions.

        * If `body` is a single string, it is parsed as one expression, and
          kept as the single expression of the section.
        * If `body` is an enumeration, each element is parsed as an expression
          if it is a string, or kept as it is otherwise.

        """
        
        def parse_and_populate_next_expression(expression):
            """
            Parse the given expression and return a Node representing it.
            
            :param expression: the expression
            :type expression: :class:`str`
            """
            parsed = parse_next_expression(expression)
            res = Node.from_ptr(find_hierarchy(parsed))
            nsnode.free_node(parsed)
            return res
        
        def parse_and_populate_identifier(identifier):
            """
            Parse the given identifier and return a Node representing it.
            
            :param identifier: the identifier
            :type identifier: :class:`str`
            """
            parsed = parse_identifier(identifier)
            res = Node.from_ptr(find_hierarchy(parsed))
            nsnode.free_node(parsed)
            return res
        
        def parentheses_split(text, separator=","):
            """
            Return the elements of `text` separated by `separator`, ignoring
            everything in parentheses, curly braces and square brackets.

            :param text: the text to split
            :type text: :class:`str`
            :param separator: the separator to split with
            :type separator: :class:`str`
            """
            def content_to_braces(matchobj, content):
                content.append(matchobj.group(0))
                return "{{{}}}".format(len(content)-1)
            replaced = []
            repl = re.sub(r"(\([^)]*\))|(\[[^]]*\])|(\{[^}]*\})",
                          lambda m: content_to_braces(m, replaced),
                          text)
            res = []
            for element in repl.split(separator):
                res.append(element.strip().format(*replaced))
            return res
        
        def parse_constants_body(body):
            """
            Parse the given body as a CONSTANTS section body.
            
            :param body: the body (ending with `;`), assumed to be correctly
                         formatted
            :type body: :class:`str`
            """
            constants = []
            body = body.strip()[:-1]
            for constant in parentheses_split(body):
                constants.append(parse_and_populate_identifier(constant))
            return constants
        
        def parse_assign_identifier(identifier):
            """
            Parse the given identifier as an ASSIGN identifier.
            
            :param identifier: the identifier, assumed to be correctly
                               formatted
            :type body: :class:`str`
            """
            from .node import Node, Next, Smallinit
            
            identifier = identifier.strip()
            if identifier.startswith("next"):
                identifier = identifier[5:]
                return Next(parse_and_populate_identifier(identifier))
            elif identifier.startswith("init"):
                identifier = identifier[5:]
                return Smallinit(parse_and_populate_identifier(identifier))
            else:
                return parse_and_populate_identifier(identifier)
        
        def parse_compassion(body):
            """
            Parse the given body as a COMPASSION section body.
            
            :param body: the body (possibly ending with `;`), assumed to be
                         correctly formatted (two comma-separated expressions)
            :type body: :class:`str`
            """
            from .node import Cons
            
            if body.endwith(";"):
                body = body[:-1]
            first, second = parentheses_split(body)
            return Cons(parse_and_populate_next_expression(first),
                        parse_and_populate_next_expression(second))
        
        def parse_and_populate_type_specifier(specifier):
            """
            Parse the given type specifier.
            
            :param specifier: the specifier
            :type specifier: :class:`str`
            """
            specifier = specifier.strip()
            
            # boolean
            if specifier.startswith("boolean"):
                return Boolean()
            
            # scalar
            elif specifier.startswith("{"):
                specifier = specifier[1:-1]
                return Scalar(tuple(parse_and_populate_identifier(value)
                                    for value in parentheses_split(specifier)))
            
            # word
            elif specifier.startswith("word"):
                return Word(parse_and_populate_next_expression(specifier[5:]))
            
            # signed word
            elif re.match(r"^signed\sword(.*)", specifier):
                match = re.match(r"^signed\sword(.*)", specifier)
                return SignedWord(parse_and_populate_next_expression
                                  (match.group(1)))
            
            # unsigned word
            elif re.match(r"^unsigned\sword(.*)", specifier):
                match = re.match(r"^unsigned\sword(.*)", specifier)
                return UnsignedWord(parse_and_populate_next_expression
                                    (match.group(1)))
            
            # array
            elif specifier.startswith("array"):
                specifier = specifier[5:]
                of_pos = specifier.specifier.find(" of ")
                range_str = specifier[:of_pos]
                array_range = parse_type_specifier(range_str)
                rest = specifier[of_pos + 4:]
                return Array(array_range,
                             parse_and_populate_type_specifier(rest))
            
            # range and modtype
            else:
                # range
                range_split = parentheses_split(specifier, separator="..")
                if len(range_split) == 2:
                    start, stop = range_split
                    return Range(parse_and_populate_next_expression(start),
                                 parse_and_populate_next_expression(stop))
                
                # modtype
                else:
                    name = specifier[:specifier.find("(")]
                    rest = specifier[specifier.find("(") + 1:-1]
                    args = []
                    if len(rest) > 0:
                        for value in parentheses_split(rest):
                            args.append(parse_and_populate_next_expression
                                        (value))
                    return Modtype(parse_and_populate_identifier(name),
                                   tuple(args))
        
        def parse_and_populate_declaration_body(body):
            """
            Parse the given body of a declaration section.
            
            :param body: the body to parse
            :type body: :class:`str`
            """
            body = body.strip()
            # last of body should be ;
            body = body[:-1]
            declarations = collections.OrderedDict()
            for declaration in parentheses_split(body, ";"):
                splitted = parentheses_split(declaration, ":")
                identifier = parse_and_populate_identifier(splitted[0])
                type_ = parse_and_populate_type_specifier(splitted[1])
                declarations[identifier] = type_
            return declarations
        
        def parse_define_body(body):
            """
            Parse the given body of a DEFINE section.
            
            :param body: the body to parse
            :type body: :class:`str`
            """
            body = body.strip()
            # last of body should be ;
            body = body[:-1]
            defines = collections.OrderedDict()
            for define in parentheses_split(body, ";"):
                splitted = parentheses_split(define, ":=")
                identifier = parse_and_populate_identifier(splitted[0])
                expr = parse_and_populate_next_expression(splitted[1])
                defines[identifier] = expr
            return defines
        
        def parse_assign_body(body):
            """
            Parse the given body of an ASSIGN section.
            
            :param body: the body to parse
            :type body: :class:`str`
            """
            body = body.strip()
            # last of body should be ;
            body = body[:-1]
            assigns = collections.OrderedDict()
            for assign in parentheses_split(body, ";"):
                splitted = parentheses_split(assign, ":=")
                identifier = parse_assign_identifier(splitted[0])
                expr = parse_and_populate_next_expression(splitted[1])
                assigns[identifier] = expr
            return assigns
        
        # The list of module sections that are considered
        # each key is the section name
        # each value is a tuple giving:
        # * the type of expected value (mapping, enumeration, bodies)
        # * a list parsers: three for mappings (whole section, key and value),
        #   one for enumerations (whole section) and one for bodies (whole
        #   section)
        _sections_parsers = {
            "VAR": ("mapping",
                    parse_and_populate_declaration_body,
                    parse_and_populate_identifier,
                    parse_and_populate_type_specifier),
            "IVAR": ("mapping",
                     parse_and_populate_declaration_body,
                     parse_and_populate_identifier,
                     parse_and_populate_type_specifier),
            "FROZENVAR": ("mapping",
                          parse_and_populate_declaration_body,
                          parse_and_populate_identifier,
                          parse_and_populate_type_specifier),
            "DEFINE": ("mapping",
                       parse_define_body,
                       parse_and_populate_identifier,
                       parse_and_populate_next_expression),
            "ASSIGN": ("mapping",
                       parse_assign_body,
                       parse_assign_identifier,
                       parse_and_populate_next_expression),
            "CONSTANTS": ("enumeration",
                          parse_constants_body),
            "TRANS": ("bodies", parse_and_populate_next_expression),
            "INIT": ("bodies", parse_and_populate_next_expression),
            "INVAR": ("bodies", parse_and_populate_next_expression),
            "FAIRNESS": ("bodies", parse_and_populate_next_expression),
            "JUSTICE": ("bodies", parse_and_populate_next_expression),
            "COMPASSION": ("bodies", parse_compassion)}

        if section not in mcs._sections:
            raise NuSMVModuleError("Unknown section: {}.".format(section))

        if mcs._sections[section][0] == "mapping":
            section_parser, key_parser, value_parser = (_sections_parsers
                                                        [section][-3:])
            if isinstance(body, collections.abc.Mapping):
                res = collections.OrderedDict()
                for key, value in body.items():
                    if isinstance(key, str):
                        key = key_parser(key)
                    if isinstance(value, str):
                        value = value_parser(value)
                    res[key] = value

                return res

            else:
                if isinstance(body, str):
                    body = [body]

                res = collections.OrderedDict()
                for line in body:
                    if isinstance(line, str):
                        line = section_parser(line)
                        update(res, line)

                    else:
                        # line is an enumeration
                        key, value = line[0:2]
                        if isinstance(key, str):
                            key = key_parser(key)
                        if isinstance(value, str):
                            value = value_parser(value)
                        res[key] = value

                return res

        elif (mcs._sections[section][0] == "enumeration" or
              mcs._sections[section][0] == "bodies"):

            if isinstance(body, str):
                body = [body]

            elif isinstance(body, Expression):
                body = [body]

            # body is a list of expressions
            parser = _sections_parsers[section][-1]

            exprs = []
            for expr in body:
                if isinstance(expr, str):
                    expr = parser(expr)

                update(exprs, [expr])

            return exprs

        else:
            raise NuSMVModuleError("Unknown section type: "
                                   "{} for section {}.".format
                                   (mcs._sections[section][0],
                                    section))

    def _trim(cls, string, indentation=""):
        """
        Reformat `string` (:class:`str`) with the following rules:

        * tabulations are converted into spaces;
        * leading and trailing empty lines are removed;
        * every line is indented with its relative indentation to the least
          indented non-empty line;
        * the whole string is indented with `indentation`.

        """
        if not string:
            return ''
        # Convert tabs to spaces (following the normal Python rules)
        # and split into a list of lines:
        lines = string.expandtabs().splitlines()
        # Determine minimum indentation (first line doesn't count):
        indent = sys.maxsize
        for line in lines[1:]:
            stripped = line.lstrip()
            if stripped:
                indent = min(indent, len(line) - len(stripped))
        # Remove indentation (first line is special):
        trimmed = [lines[0].strip()]
        if indent < sys.maxsize:
            for line in lines[1:]:
                trimmed.append(line[indent:].rstrip())
        # Strip off trailing and leading blank lines:
        while trimmed and not trimmed[-1]:
            trimmed.pop()
        while trimmed and not trimmed[0]:
            trimmed.pop(0)
        # Return a single string:
        return '\n'.join(indentation + line for line in trimmed)

    def _section_str(cls, section, body, indentation=""):
        """
        Return the string representation of `section`, depending on `section`
        value, and including `body`, indented with `indentation`.
        `section` must be a key of `cls._sections`.

        """

        if section not in cls._sections:
            raise NuSMVModuleError("Unknown section: {}.".format(section))

        if len(body) <= 0:
            return ""

        if cls._sections[section][0] == "mapping":
            # body is a mapping
            # use format given in _sections
            strformat = cls._sections[section][1]
            value = (indentation + section + "\n"
                     + "\n".join(((indentation * 2) + strformat)
                                 .format(key=str(name), value=str(expr))
                                 for name, expr in body.items()))
            return value

        elif cls._sections[section][0] == "enumeration":
            # body is an enumeration
            # use separator given in _sections
            separator = cls._sections[section][1]
            return (indentation + section + "\n"
                    + separator.join(str(value) for value in body))

        elif cls._sections[section][0] == "bodies":
            # body is an enumeration of bodies
            # return a set of sections with these bodies
            return "\n".join(indentation + section + "\n"
                             + cls._trim(str(value), indentation * 2)
                             for value in body)

        else:
            raise NuSMVModuleError("Unknown section type: "
                                   "{} for section {}.".format
                                   (cls._sections[section][0],
                                    section))

    def __str__(cls):
        if cls.source:
            return cls.source

        indentation = " " * 4
        args = ("(" + ", ".join(str(arg) for arg in cls.ARGS) + ")"
                if len(cls.ARGS) > 0 else "")

        representation = ["MODULE " + str(cls.NAME) + args]
        for section in [member for member in cls.members
                        if member in cls._sections]:
            representation.append(cls._section_str(section,
                                                   cls.__dict__[section],
                                                   indentation))
        return "\n".join(representation)
    
    def copy(cls):
        """
        Return a deep copy of this module.
        
        Only the members of this module present in `cls.members` are copied.
        If these members accept to be `deepcopy`ed, this method is used to
        copy the member, otherwise the member is not copied and is passed
        to the created module as it is.
        
        """
        newnamespace = collections.OrderedDict()
        for member in cls.members:
            newnamespace[member] = deepcopy(getattr(cls, member))
        
        return ModuleMetaClass(cls.NAME, cls.__bases__, newnamespace)


class Module(Modtype, metaclass=ModuleMetaClass):

    """
    A generic module.

    To create a new module, the user must subclass the :class:`Module` class
    and add class attributes with names corresponding to sections of NuSMV
    module definitions: `VAR`, `IVAR`, `FROZENVAR`, `DEFINE`, `CONSTANTS`,
    `ASSIGN`, `TRANS`, `INIT`, `INVAR`, `FAIRNESS`, `JUSTICE`, `COMPASSION`.

    In addition to these attributes, the `ARGS` and `NAME` attributes can be
    defined. If `NAME` is defined, it overrides module name for the NuSMV
    module name. If `ARGS` is defined, it must be a sequence object where each
    element's string representation is an argument of the module.

    Treatment of the section depends of the type of the section and the value
    of the corresponding attribute.

    CONSTANTS section
        If the value of the section is a string (:class:`str`), it is parsed as
        the body of the constants declaration. Otherwise, the value must be a
        sequence and it is parsed as the defined constants.

    VAR, IVAR, FROZENVAR, DEFINE, ASSIGN sections
        If the value of the section is a string (:class:`str`), it is parsed as
        the body of the declaration. If it is a dictionary (:class:`dict`),
        keys are parsed as names of variables (or input variables, define,
        etc.) if they are strings, or used as they are otherwise, and values
        are parsed as bodies of the declaration (if strings, kept as they are
        otherwise). Otherwise, the value must be a sequence, and each element
        is treated separately:

        * if the element is a string (:class:`str`), it is parsed as a
          declaration;
        * otherwise, the element must be a sequence, and the first element is
          used as the name of the variable (or input variable, define, etc.)
          and parsed if necessary, and the second one as the body of the
          declaration.

    TRANS, INIT, INVAR, FAIRNESS, JUSTICE, COMPASSION sections
        If the value of the section is a string (:class:`str`), it is parsed as
        the body of the section. Otherwise, it must be a sequence and the
        representation (parsed if necessary)  of the elements of the sequence
        are declared as different sections.

    In addition to these sections, the class body can contain instances of
    :class:`pynsumv.model.Declaration`. These ones take the name of the
    corresponding variable, and are added to the corresponding section (`VAR`,
    `IVAR`, `FROZENVAR` or `DEFINE`) when creating the class.


    For example, the class ::

        class TwoCounter(Module):
            NAME = "twoCounter"
            ARGS = ["run"]
            c1 = Range(0, 2)
            VAR = {"c2": "0..2"}
            INIT = [c1 == 0 & "c2 = 0"]
            TRANS = [Next(c1) == Ite("run", (c1 + 1) % 2, c1),
                     "next(c2) = run ? c2+1 mod 2 : c2"]

    defines the module ::

        MODULE twoCounter(run)
            VAR
                c1 : 0..2;
                c2 : 0..2;
            INIT
                c1 = 0 & c2 = 0
            TRANS
                next(c1) = run ? c1+1 mod 2 : c1
            TRANS
                next(c2) = run ? c2+1 mod 2 : c2


    After creation, module sections satisfy the following patterns:

    * pair-based sections such as VAR, IVAR, FROZENVAR, DEFINE and ASSIGN
      are mapping objects (dictionaries) where keys are identifiers and values
      are types (for VAR, IVAR and FROZENVAR) or expressions (for DEFINE and
      ASSIGN).
    * list-based sections such as CONSTANTS are enumerations composed of
      elements of the section.
    * expression-based sections such as TRANS, INIT, INVAR, FAIRNESS, JUSTICE
      and COMPASSION are enumerations composed of expressions.

    """

    def __init__(self, *args):
        super().__init__(self.__class__.NAME, args)
