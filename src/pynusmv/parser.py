"""
The :mod:`pynusmv.parser` module provides functions to parse strings
and return corresponding ASTs. This module includes three types of
functionalities:

* :func:`parse_simple_expression`, :func:`parse_next_expression`,
  :func:`parse_identifier` and :func:`parse_ctl_spec` are direct access to
  NuSMV parser, returning wrappers to NuSMV internal data structures
  representing the language AST.
* :data:`identifier`, :data:`simple_expression`, :data:`constant`,
  :data:`next_expression`, :data:`type_identifier`, :data:`var_section`,
  :data:`ivar_section`, :data:`frozenvar_section`, :data:`define_section`,
  :data:`constants_section`, :data:`assign_constraint`,
  :data:`init_constraint`, :data:`trans_constraint`, :data:`invar_constraint`,
  :data:`fairness_constraint`, :data:`justice_constraint`,
  :data:`compassion_constraint`, :data:`module` and :data:`model` are pyparsing
  parsers parsing the corresponding elements of a NuSMV model (see NuSMV
  documentation for more information on these elements of the language).
* :func:`parseAllString` is a helper function to directly return ASTs for
  strings parsed with pyparsing parsers.

"""


__all__ = [
    'parse_simple_expression',
    'parse_next_expression',
    'parse_identifier',
    'parse_ctl_spec',
    'identifier',
    'complex_identifier',
    'simple_expression',
    'constant',
    'next_expression',
    'type_identifier',
    'var_section',
    'ivar_section',
    'frozenvar_section',
    'define_section',
    'constants_section',
    'assign_constraint',
    'init_constraint',
    'trans_constraint',
    'invar_constraint',
    'fairness_constraint',
    'justice_constraint',
    'compassion_constraint',
    'module',
    'model',
    'parseAllString']


from .exception import NuSMVParsingError, _Error

from .utils import update
from .model import (Identifier, Self, Dot, ArrayAccess, Trueexp, Falseexp,
                    NumberWord, RangeConst,
                    Conversion, WordFunction, Count, Next, Smallinit, Case,
                    Subscript, BitSelection, Set, Not, Concat,
                    Minus, Mult, Div, Mod, Add, Sub, LShift, RShift, Union, In,
                    Equal, NotEqual, Lt, Gt, Le, Ge, And, Or, Xor, Xnor, Ite,
                    Iff, Implies, ArrayExpr,
                    Boolean, Word, Scalar, Range, Array, Modtype,
                    Variables, InputVariables, FrozenVariables, Defines,
                    Assigns, Constants, Trans, Init, Invar, Fairness, Justice,
                    Compassion,
                    Expression)

from .nusmv.parser import parser as nsparser
from .nusmv.node import node as nsnode

from pyparsing import (Word as PWord, Forward, Optional, Literal,
                       OneOrMore, FollowedBy, Suppress, ZeroOrMore,
                       Combine, Group,
                       oneOf, delimitedList, restOfLine,
                       alphas, alphanums, nums,
                       ParserElement)
from collections import OrderedDict
from functools import reduce

ParserElement.enablePackrat()


def parse_simple_expression(expression):
    """
    Parse a simple expression.

    :param string expression: the expression to parse
    :raise: a :exc:`NuSMVParsingError
            <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs

    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.

    """
    node, err = nsparser.ReadSimpExprFromString(expression)
    if err:
        errors = nsparser.Parser_get_syntax_errors_list()
        raise NuSMVParsingError.from_nusmv_errors_list(errors)
    else:
        node = nsnode.car(node)  # Get rid of the top SIMPWFF node
        if node.type is nsparser.CONTEXT and nsnode.car(node) is None:
            # Get rid of the top empty context if any
            return nsnode.cdr(node)
        else:
            return node


def parse_next_expression(expression):
    """
    Parse a "next" expression.

    :param string expression: the expression to parse
    :raise: a :exc:`NuSMVParsingError
            <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs

    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.

    """
    node, err = nsparser.ReadNextExprFromString(expression)
    if err:
        errors = nsparser.Parser_get_syntax_errors_list()
        raise NuSMVParsingError.from_nusmv_errors_list(errors)
    else:
        node = nsnode.car(node)  # Get rid of the top NEXTWFF node
        if node.type is nsparser.CONTEXT and nsnode.car(node) is None:
            # Get rid of the top empty context if any
            return nsnode.cdr(node)
        else:
            return node


def parse_identifier(expression):
    """
    Parse an identifier

    :param string expression: the identifier to parse
    :raise: a :exc:`NuSMVParsingError
            <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs

    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.

    """
    node, err = nsparser.ReadIdentifierExprFromString(expression)
    if err:
        errors = nsparser.Parser_get_syntax_errors_list()
        raise NuSMVParsingError.from_nusmv_errors_list(errors)
    else:
        node = nsnode.car(node)  # Get rid of the top COMPID node
        if node.type is nsparser.CONTEXT and nsnode.car(node) is None:
            # Get rid of the top empty context if any
            return nsnode.cdr(node)
        else:
            return node


def parse_ctl_spec(spec):
    """
    Parse a CTL specification

    :param string spec: the specification to parse
    :raise: a :exc:`NuSMVParsingError <pynusmv.exception.NuSMVParsingError>`
            if a parsing error occurs

    .. warning:: Returned value is a SWIG wrapper for the NuSMV node_ptr.
       It is the responsibility of the caller to manage it.

    """
    node, err = nsparser.ReadCmdFromString("CTLWFF " + spec)
    if err:
        errors = nsparser.Parser_get_syntax_errors_list()
        raise NuSMVParsingError.from_nusmv_errors_list(errors)
    else:
        node = nsnode.car(node)  # Get rid of the top CTLWFF node
        if node.type is nsparser.CONTEXT and nsnode.car(node) is None:
            # Get rid of the top empty context if any
            return nsnode.cdr(node)
        else:
            return node


def parseAllString(parser, string):
    """
    Parse `string` completely with `parser` and set source of the result
    to `string`. `parser` is assumed to return a one-element list when parsing
    `string`.

    :param parser: a pyparsing parser
    :param string: the string to parse
    :type string: :class:`str`
    """
    res = parser.parseString(string, parseAll=True)
    if hasattr(res[0], "source"):
        res[0].source = string
    return res[0]


def _reduce_list_to_expr(list_):
    """
    Reduces l to its token representation.

    :param l: a list of tokens separated by operators, with at least one token.
    """

    _otc = {"*": Mult, "/": Div, "mod": Mod, "+": Add, "-": Sub,
            "<<": LShift, ">>": RShift,
            "=": Equal, "!=": NotEqual, "<": Lt, ">": Gt, "<=": Le, ">=": Ge,
            "|": Or, "xor": Xor, "xnor": Xnor}

    res = list_[0]
    for operator, token in zip(list_[1::2], list_[2::2]):
        res = _otc[operator](res, token)
    return res


# Identifiers
identifier = PWord(alphas + "_", alphanums + "_$#-")
identifier.setParseAction(lambda s, l, t: Identifier(t[0]))

simple_expression = Forward()

# Variable and DEFINE identifiers
_cip = Forward()
_cip <<= Optional("." + Literal("self") + _cip
                  | "." + identifier + _cip
                  | "[" + simple_expression + "]" + _cip)
complex_identifier = ((FollowedBy("self") + Literal("self") + _cip)
                      | (identifier + _cip))


def _handle_ci(tokens):
    """
    Create a complex identifier from the given list of `tokens`.

    :param tokens: a non-empty list of tokens
    """
    def _handle_id(token):
        if token == "self":
            return Self()
        else:
            return token

    if len(tokens) <= 1:
        return _handle_id(tokens[0])
    elif tokens[1] == ".":
        return Dot(_handle_id(tokens[0]), _handle_ci(tokens[2:]))
    else:  # tokens[1] == "["
        return _handle_ci([ArrayAccess(_handle_id(tokens[0]), tokens[2])] +
                          tokens[4:])

complex_identifier.setParseAction(lambda s, l, t: _handle_ci(t))

_define_identifier = complex_identifier
_variable_identifier = complex_identifier


# Integer numbers
_integer_number = Combine(Optional("-") + PWord(nums))
_integer_number.setParseAction(lambda s, l, t: int(t[0]))

# Constants
_boolean_constant = oneOf("TRUE FALSE")
_boolean_constant.setParseAction(lambda s, l, t:
                                 Falseexp() if t[0] == "FALSE" else Trueexp())

_integer_constant = _integer_number
_symbolic_constant = identifier
_range_constant = _integer_number + Suppress("..") + _integer_number
_range_constant.setParseAction(lambda s, l, t: RangeConst(t[0], t[1]))

_word_sign_specifier = oneOf("u s")
_word_base = oneOf("b B o O d D h H")
_word_width = PWord(nums)
_word_value = PWord("0123456789abcdefABCDEF", "0123456789abcdefABCDEF_")
_word_constant = Combine(Literal("0") + Optional(_word_sign_specifier)
                         + _word_base + Optional(_word_width) + "_"
                         + _word_value)
_word_constant.setParseAction(lambda s, l, t: NumberWord(t[0]))

constant = (_word_constant
            # Range constant removed to follow the parser implemented by NuSMV
            # | _range_constant
            | _integer_constant | _boolean_constant | _symbolic_constant)

# Basic expressions
_basic_expr = Forward()
_conversion = (Literal("word1") + Suppress("(") + _basic_expr + Suppress(")")
               | Literal("bool") + Suppress("(") + _basic_expr + Suppress(")")
               | Literal("toint") + Suppress("(") + _basic_expr + Suppress(")")
               | Literal("signed") + Suppress("(") + _basic_expr
               + Suppress(")")
               | Literal("unsigned") + Suppress("(") + _basic_expr
               + Suppress(")"))
_conversion.setParseAction(lambda s, l, t: Conversion(t[0], t[1]))

_word_function = (Literal("extend") + Suppress("(") + _basic_expr + ","
                  + _basic_expr + Suppress(")")
                  | Literal("resize") + Suppress("(") + _basic_expr + ","
                  + _basic_expr + Suppress(")"))
_word_function.setParseAction(lambda s, l, t: WordFunction(t[0], t[1], t[2]))

_count = (Literal("count") + Suppress("(") + delimitedList(_basic_expr)
          + Suppress(")"))
_count.setParseAction(lambda s, l, t: Count(t[1]))

_next = Literal("next") + Suppress("(") + _basic_expr + Suppress(")")
_next.setParseAction(lambda s, l, t: Next(t[1]))

_case_case = _basic_expr + Suppress(":") + _basic_expr + Suppress(";")
_case_body = OneOrMore(_case_case)
_case_body.setParseAction(lambda s, l, t: OrderedDict(zip(t[::2], t[1::2])))
_case = Suppress("case") + _case_body + Suppress("esac")
_case.setParseAction(lambda s, l, t: Case(t[0]))

_base = (complex_identifier ^
         (_conversion
          | _word_function
          | _count
          | _next
          | Suppress("(") + _basic_expr + Suppress(")")
          | _case
          | constant))

_ap = Forward()
_array_subscript = Group(Suppress("[") + _basic_expr + Suppress("]"))

_word_bit_selection = Group(Suppress("[") + _basic_expr + Suppress(":")
                            + _basic_expr + Suppress("]"))

_ap <<= Optional(_array_subscript + _ap | _word_bit_selection + _ap)
_array = _base + _ap


def _handle_array(tokens):
    """
    Create an array from the given list of `tokens`.

    :param tokens: a non-empty list of tokens
    """
    if len(tokens) <= 1:
        return tokens[0]
    elif len(tokens[1]) == 1:
        return _handle_array([Subscript(tokens[0], tokens[1][0])] + tokens[2:])
    else:  # len(tokens[1]) == 2
        return _handle_array([BitSelection(tokens[0], tokens[1][0],
                                           tokens[1][1])] +
                             tokens[2:])


_array.setParseAction(lambda s, l, t: _handle_array(t))

_not = ZeroOrMore("!") + _array
_not.setParseAction(lambda s, l, t: reduce(lambda e, n: Not(e), t[:-1], t[-1]))

_concat = _not + ZeroOrMore(Suppress("::") + _not)
_concat.setParseAction(lambda s, l, t: reduce(lambda e, n: Concat(e, n),
                                              t[0:1] + t[2::2]))

_minus = ZeroOrMore("-") + _concat
_minus.setParseAction(lambda s, l, t: reduce(lambda e, n: Minus(e),
                                             t[:-1], t[-1]))

_mult = _minus + ZeroOrMore(oneOf("* / mod") + _minus)
_mult.setParseAction(lambda s, l, t: _reduce_list_to_expr(t))

_add = _mult + ZeroOrMore(oneOf("+ -") + _mult)
_add.setParseAction(lambda s, l, t: _reduce_list_to_expr(t))

_shift = _add + ZeroOrMore(oneOf("<< >>") + _add)
_shift.setParseAction(lambda s, l, t: _reduce_list_to_expr(t))

_set_set = Suppress("{") + delimitedList(_basic_expr) + Suppress("}")
_set_set.setParseAction(lambda s, l, t: Set(t))
_set = (_range_constant | _shift | _set_set)

_union = _set + ZeroOrMore("union" + _set)
_union.setParseAction(lambda s, l, t: reduce(lambda e, n: Union(e, n),
                                             t[0:1] + t[2::2]))

_in = _union + ZeroOrMore("in" + _union)
_in.setParseAction(lambda s, l, t: reduce(lambda e, n: In(e, n),
                                          t[0:1] + t[2::2]))

_comparison = _in + ZeroOrMore(oneOf("= != < > <= >=") + _in)
_comparison.setParseAction(lambda s, l, t: _reduce_list_to_expr(t))

_and = _comparison + ZeroOrMore("&" + _comparison)
_and.setParseAction(lambda s, l, t: reduce(lambda e, n: And(e, n),
                                           t[0:1] + t[2::2]))

_or = _and + ZeroOrMore(oneOf("| xor xnor") + _and)
_or.setParseAction(lambda s, l, t: _reduce_list_to_expr(t))

_ite = Forward()
_ite <<= _or + Optional("?" + _ite + ":" + _ite)
_ite.setParseAction(lambda s, l, t:
                    t[0] if len(t) <= 1 else Ite(t[0], t[2], t[4]))

_iff = _ite + ZeroOrMore("<->" + _ite)
_iff.setParseAction(lambda s, l, t: reduce(lambda e, n: Iff(e, n),
                                           t[0:1] + t[2::2]))

_implies = Forward()
_implies <<= _iff + ZeroOrMore("->" + _implies)
_implies.setParseAction(lambda s, l, t: reduce(lambda e, n: Implies(n, e),
                                               t[0:1] + t[2::2]))

_basic_expr <<= _implies

simple_expression <<= _basic_expr
next_expression = _basic_expr

# Type specifier
_simple_type_specifier = Forward()

_boolean_type = Literal("boolean")
_boolean_type.setParseAction(lambda s, l, t: Boolean())

_word_type = (Optional(Literal("unsigned") | Literal("signed"))
              + Literal("word") + Suppress("[") + _basic_expr + Suppress("]"))
_word_type.setParseAction(lambda s, l, t: Word(t[1]) if t[0] == "word"
                          else Word(t[2], sign=t[0]))

_enum_type = (Suppress("{")
              + delimitedList(_integer_number | _symbolic_constant)
              + Suppress("}"))
_enum_type.setParseAction(lambda s, l, t: Scalar(t))

_range_type = _shift + Suppress("..") + _shift
_range_type.setParseAction(lambda s, l, t: Range(t[0], t[1]))

_array_type = (Suppress("array") + _shift + Suppress("..") + _shift
               + Suppress("of") + _simple_type_specifier)
_array_type.setParseAction(lambda s, l, t: Array(t[0], t[1], t[2]))

_simple_type_specifier <<= (_boolean_type
                            | _word_type
                            | _enum_type
                            | _array_type
                            | _range_type)

_module_type_specifier = (Optional("process") + identifier
                          + Optional(Suppress("(")
                                     +
                                     Optional(delimitedList(simple_expression))
                                     + Suppress(")")))
_module_type_specifier.setParseAction(
    lambda s, l, t: Modtype(t[1], t[2:], process=True)
    if t[0] == "process"
    else Modtype(t[0], t[1:]))

type_identifier = _simple_type_specifier | _module_type_specifier

# Variables
_var_declaration = identifier + Suppress(":") + type_identifier + Suppress(";")
_var_section_body = OneOrMore(_var_declaration)
_var_section_body.setParseAction(lambda s, l, t:
                                 OrderedDict(zip(t[::2], t[1::2])))
var_section = Suppress("VAR") + _var_section_body
var_section.setParseAction(lambda s, l, t: Variables(t[0]))

_ivar_declaration = (identifier + Suppress(":") + _simple_type_specifier
                     + Suppress(";"))
_ivar_section_body = OneOrMore(_ivar_declaration)
_ivar_section_body.setParseAction(lambda s, l, t:
                                  OrderedDict(zip(t[::2], t[1::2])))
ivar_section = Suppress("IVAR") + _ivar_section_body
ivar_section.setParseAction(lambda s, l, t: InputVariables(t[0]))

_frozenvar_declaration = (identifier + Suppress(":") + _simple_type_specifier
                          + Suppress(";"))
_frozenvar_section_body = OneOrMore(_frozenvar_declaration)
_frozenvar_section_body.setParseAction(lambda s, l, t:
                                       OrderedDict(zip(t[::2], t[1::2])))
frozenvar_section = Suppress("FROZENVAR") + _frozenvar_section_body
frozenvar_section.setParseAction(lambda s, l, t: FrozenVariables(t[0]))

# DEFINE and CONSTANTS
_array_expression = Forward()
_array_expression <<= ( Suppress("[") + Group(delimitedList(next_expression))
                        + Suppress("]")
                      | Suppress("[") + Group(delimitedList(_array_expression))
                        + Suppress("]"))
_define = _array_expression | next_expression
_define_declaration = (identifier + Suppress(":=") + _define + Suppress(";"))

def _handle_define_body(s, l, t):
    b = OrderedDict()
    for identifier, body in zip(t[::2], t[1::2]):
        if not isinstance(body, Expression):
            body = ArrayExpr(body)
        b[identifier] = body
    return b
_define_section_body = OneOrMore(_define_declaration)
_define_section_body.setParseAction(_handle_define_body)
define_section = Suppress("DEFINE") + _define_section_body
define_section.setParseAction(lambda s, l, t: Defines(t[0]))

_constants_section_body = delimitedList(identifier) + Suppress(";")
_constants_section_body.setParseAction(lambda s, l, t: list(t))
constants_section = Suppress("CONSTANTS") + _constants_section_body
constants_section.setParseAction(lambda s, l, t: Constants(list(t)))

# ASSIGN, TRANS, INIT, INVAR, FAIRNESS
_assign_identifier = (Literal("init") + Suppress("(") + complex_identifier
                      + Suppress(")")
                      | Literal("next") + Suppress("(") + complex_identifier
                      + Suppress(")")
                      | complex_identifier)
_assign_identifier.setParseAction(lambda s, l, t:
                                  Smallinit(t[1]) if t[0] == "init"
                                  else Next(t[1]) if t[0] == "next"
                                  else t[0])
_assign = (_assign_identifier + Suppress(":=") + simple_expression
           + Suppress(";"))
_assign_constraint_body = OneOrMore(_assign)
_assign_constraint_body.setParseAction(lambda s, l, t:
                                       OrderedDict(zip(t[::2], t[1::2])))
assign_constraint = Suppress("ASSIGN") + _assign_constraint_body
assign_constraint.setParseAction(lambda s, l, t: Assigns(t[0]))

_trans_constraint_body = next_expression + Optional(Suppress(";"))
_trans_constraint_body.setParseAction(lambda s, l, t: list(t))
trans_constraint = Suppress("TRANS") + _trans_constraint_body
trans_constraint.setParseAction(lambda s, l, t: Trans(list(t)))

_init_constraint_body = simple_expression + Optional(Suppress(";"))
_init_constraint_body.setParseAction(lambda s, l, t: list(t))
init_constraint = Suppress("INIT") + _init_constraint_body
init_constraint.setParseAction(lambda s, l, t: Init(list(t)))

_invar_constraint_body = simple_expression + Optional(Suppress(";"))
_invar_constraint_body.setParseAction(lambda s, l, t: list(t))
invar_constraint = Suppress("INVAR") + _invar_constraint_body
invar_constraint.setParseAction(lambda s, l, t: Invar(list(t)))

_fairness_constraint_body = simple_expression + Optional(Suppress(";"))
_fairness_constraint_body.setParseAction(lambda s, l, t: list(t))
fairness_constraint = Suppress("FAIRNESS") + _fairness_constraint_body
fairness_constraint.setParseAction(lambda s, l, t: Fairness(list(t)))

_justice_constraint_body = simple_expression + Optional(Suppress(";"))
_justice_constraint_body.setParseAction(lambda s, l, t: list(t))
justice_constraint = Suppress("JUSTICE") + _justice_constraint_body
justice_constraint.setParseAction(lambda s, l, t: Justice(list(t)))

_compassion_constraint_body = (Suppress("(")
                               + simple_expression + Suppress(",")
                               + simple_expression + Suppress(")")
                               + Optional(Suppress(";")))
_compassion_constraint_body.setParseAction(lambda s, l, t: [t[0], t[1]])
compassion_constraint = Suppress("COMPASSION") + _compassion_constraint_body
compassion_constraint.setParseAction(lambda s, l, t: Compassion(list(t)))


# Module declaration
_module_element = (var_section
                   | ivar_section
                   | frozenvar_section
                   | define_section
                   | constants_section
                   | assign_constraint
                   | trans_constraint
                   | init_constraint
                   | invar_constraint
                   | fairness_constraint
                   | justice_constraint
                   | compassion_constraint)
_module_args = (Suppress("(") + Optional(delimitedList(identifier)) +
                Suppress(")"))
module = (Suppress("MODULE") + identifier + Group(Optional(_module_args))
          + ZeroOrMore(_module_element))


def _create_module(string, location, tokens):
    """
    Create a module based on the given list of tokens.

    :param string: the string from which the module has been parsed
    :param location: the index of `string` at which the parsed module starts
    :param tokens: the list of sections representing the parsed module

    :rtype: :class:`pynusmv.model.ModuleMetaClass`

    """

    from .model import ModuleMetaClass, Module as ModuleClass

    name = tokens[0]
    args = tokens[1]
    namespace = OrderedDict()
    namespace["NAME"] = name
    namespace["ARGS"] = args
    for section in tokens[2:]:
        if section.name not in namespace:
            namespace[section.name] = section.body
        else:
            update(namespace[section.name], section.body)
    return ModuleMetaClass(str(name), (ModuleClass,), namespace)

module.setParseAction(_create_module)

# Model declaration
comment = ("--" + restOfLine).suppress()
model = Group(OneOrMore(module))
model.ignore(comment)
