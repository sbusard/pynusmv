"""
The :mod:`pynusmv.model` module provides a way to define NuSMV modules in
Python: the :class:`Module` class represents a generic NuSMV module, and must be
subclassed to define specific NuSMV modules.

"""

import inspect
import sys
import collections

__all__ = ['Module']


class ModuleMetaClass(type):
    """
    The meta class for modules, allowing modules to be printed.
    
    A module can define:
    * a `NAME` member, used as the name of the module; if absent, the name of
      the class is used.
    * a `ARGS` member, used as the list of arguments of the module; if ARGS is
      not defined, the module is declared without arguments.
    * a set of members with name of sections of a standard NuSMV module:
      VAR, IVAR, FROZENVAR, DEFINE, CONSTANTS, ASSIGN, TRANS, INIT, INVAR,
      FAIRNESS, JUSTICE, COMPASSION.
    
    The string representation of the module is then its NuSMV code.
    
    """
    
    @classmethod
    def __prepare__(metacls, name, bases, **keywords):
        return collections.OrderedDict()
    
    def __new__(cls, name, bases, namespace, **keywords):
        result = type.__new__(cls, name, bases, dict(namespace))
        result.members = tuple(namespace)
        return result
    
    # The list of module members that will be printed
    _MODULE_SECTIONS = {"VAR", "IVAR", "FROZENVAR", "DEFINE", "CONSTANTS",
                        "ASSIGN", "TRANS", "INIT", "INVAR", "FAIRNESS",
                        "JUSTICE", "COMPASSION"}
    
    def _trim(cls, string, indentation=""):
        """
        Reformat `string` (:class:`str`) with the following rules:
        
        * tabulations are converted into spaces;
        * leading and trailing empty lines are removed;
        * every line is indented with its relative indentation to the least
          indented non-empty line;
        * the whole content is finally indented with `indentation`
          (:class:`str`).
        
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
    
    def _representation(cls, section, body, indentation=""):
        """
        Return the representation of `section` with `body`. Section name is
        indented with `indentation`, section body with two `indentations`.
        `section`should be one of the following, and he format of `body` should
        satisfy the following rules.
        
        CONSTANTS section
            If `body` is a string (:class:`str`), it is used as
            the body of the constants declaration. Otherwise, `body` must be a
            sequence (other than string) and it is used as the defined
            constants.

        VAR, IVAR, FROZENVAR, DEFINE, ASSIGN sections
            If `body` is a string (:class:`str`), it is used as the body of the
            declaration. If it is a dictionary (:class:`dict`), keys are used as
            names of variables (or input variables, define, etc.) and values as
            bodies of the declaration. Otherwise, `body` must be a sequence
            other than a string and a dictionary, and each element is treated
            separately, and joined with line returns to build the body of the
            section:

            * if the element is a string (:class:`str`), it is kept as it is;
            * otherwise, the element must be a sequence other than a string, the
              first element is used as the name of the variable (or input
              variable, define, etc.) and the second one as the body of the
              declaration.

        TRANS, INIT, INVAR, FAIRNESS, JUSTICE, COMPASSION sections
            If `body` is a string (:class:`str`), it is used as the body of the
            section. Otherwise, it must be a sequence other than a string, the
            string representation of the elements of the sequence are declared
            as different sections.
        
        """
        separators = {"VAR": " : ", "IVAR": " : ", "FROZENVAR": " : ",
                      "DEFINE": " := ", "ASSIGN": " := "}
        
        if section == "CONSTANTS":
            if isinstance(body, str):
                body = cls._trim(body)
                strbodies = [body + ("" if body.endswith(";") else ";")]
            else: # body must be a sequence of declared constants
                strbodies = [", ".join(body) + ";"]
        
        elif section in {"VAR", "IVAR", "FROZENVAR", "DEFINE", "ASSIGN"}:
            if isinstance(body, str):
                body = cls._trim(body)
                strbodies = [body + ("" if body.endswith(";") else ";")]
            elif isinstance(body, dict):
                strbodies = ["\n".join(str(key) + separators[section]
                                       + str(value) + ";"
                                       for key, value in body.items())]
            else: # body must be a sequence
                strbodies = []
                for declaration in body:
                    if isinstance(declaration, str):
                        strbodies.append(declaration
                                  + ("" if declaration.endswith(";") else ";"))
                    else: # declaration must be a sequence of >= two elements
                        key, value = declaration[:2]
                        strbodies.append(str(key) + separators[section]
                                         + str(value) + ";")
                strbodies = ["\n".join(strbodies)]
        
        else: # section is TRANS, INIT, INVAR, FAIRNESS, JUSTICE, COMPASSION
            if isinstance(body, str):
                strbodies = [cls._trim(body)]
            else: # body must be a sequence
                strbodies = [str(expr) for expr in body]
        
        result = []
        for strbody in strbodies:
            result.append((indentation + section + "\n"
                           + cls._trim(strbody, indentation * 2)))
        return "\n".join(result)
    
    def __str__(cls):        
        indentation = " " * 4
        try:
            name = cls.NAME
        except AttributeError:
            name = cls.__name__
        try:
            args = "(" + ", ".join(cls.ARGS) + ")"
        except AttributeError:
            args = ""
        
        representation = ["MODULE " + name + args]
        for section in [member for member in cls.members
                               if member in cls._MODULE_SECTIONS]:
            representation.append(cls._representation(section,
                                                      cls.__dict__[section],
                                                      indentation))
        return "\n".join(representation)


class Module(object, metaclass=ModuleMetaClass):
    """
    A generic module.
    
    To create a new module, the user must subclass the :class:`Module` class and
    add class attributes with names corresponding to sections of NuSMV module
    definitions: `VAR`, `IVAR`, `FROZENVAR`, `DEFINE`, `CONSTANTS`, `ASSIGN`,
    `TRANS`, `INIT`, `INVAR`, `FAIRNESS`, `JUSTICE`, `COMPASSION`.
    
    In addition to these attributes, the `ARGS` and `NAME` attributes can be 
    defined. If `NAME` is defined, it overrides module name for the NuSMV module
    name. If `ARGS` is defined, it must be a sequence object where each
    element's string representation is an argument of the module.
    
    Treatment of the section depends of the type of the section and the value
    of the corresponding attribute.
    
    CONSTANTS section
        If the value of the section is a string (:class:`str`), it is used as
        the body of the constants declaration. Otherwise, the value must be a
        sequence and it is used as the defined constants.
    
    VAR, IVAR, FROZENVAR, DEFINE, ASSIGN sections
        If the value of the section is a string (:class:`str`), it is used as
        the body of the declaration. If it is a dictionary (:class:`dict`), keys
        are used as names of variables (or input variables, define, etc.) and
        values as bodies of the declaration. Otherwise, the value must be a
        sequence, and each element is treated  separately, and joined with line
        returns to build the body of the section:
        
        * if the element is a string (:class:`str`), it is kept as it is;
        * otherwise, the element must be a sequence, and the first element is
          used as the name of the variable (or input variable, define, etc.) and
          the second one as the body of the declaration.

    TRANS, INIT, INVAR, FAIRNESS, JUSTICE, COMPASSION sections
        If the value of the section is a string (:class:`str`), it is used as
        the body of the section. Otherwise, it must be a sequence and the
        string representation of the elements of the sequence are declared
        as different sections.
    
    
    For example, the class ::
    
        class TwoCounter(Module):
            NAME = "twoCounter"
            ARGS = ["run"]
            VAR = {"c1": "0..2",
                   "c2": "0..2"}
            INIT = ["c1 = 0",
                    "c2 = 0"]
            TRANS = \"\"\"
                    next(c1) = run ? c1+1 mod 2 : c1 &
                    next(c2) = run ? c2+1 mod 2 : c2
                    \"\"\"
    
    defines the module ::
    
        MODULE twoCounter(run)
            VAR
                c1 : 0..2;
                c2 : 0..2;
            INIT
                c1 = 0
            INIT
                c2 = 0
            TRANS
                next(c1) = run ? c1+1 mod 2 : c1 &
                next(c2) = run ? c2+1 mod 2 : c2
    
    """
    
    pass


class Model:
    """
    A NuSMV model. It is composed of a set of NuSMV modules.
    
    """
    def __init__(self, *modules):
        self.modules = modules
    
    def __str__(self):
        return "\n".join(str(module) for module in self.modules)