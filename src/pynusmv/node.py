"""
The :mod:`pynusmv.node` module provides classes representing NuSMV internal
nodes, as well as a class :class:`FlatHierarchy` to represent a NuSMV flat
hierarchary.
"""

import re
try:
    from collections.abc import Mapping, Sequence
except ImportError:
    from collections import Mapping, Sequence
from collections import OrderedDict
import string
import random

from .utils import PointerWrapper
from .parser import parse_next_expression, parse_identifier

from .nusmv.compile import compile as nscompile
from .nusmv.node import node as nsnode
from .nusmv.utils import utils as nsutils
from .nusmv.set import set as nsset
from .nusmv.parser import parser as nsparser
from .nusmv.parser.parser import (TRANS, INIT, INVAR, ASSIGN, FAIRNESS,
                                  JUSTICE, COMPASSION, SPEC, LTLSPEC, PSLSPEC,
                                  INVARSPEC, COMPUTE, DEFINE, ISA, GOTO,
                                  CONSTRAINT, MODULE, PROCESS, MODTYPE, LAMBDA,
                                  CONSTANTS, PRED, ATTIME, PREDS_LIST, MIRROR,
                                  SYNTAX_ERROR, FAILURE, CONTEXT, EU, AU, EW,
                                  AW, EBU, ABU, MINU, MAXU, VAR, FROZENVAR,
                                  IVAR, BOOLEAN, ARRAY, SCALAR, CONS, BDD,
                                  SEMI, EQDEF, TWODOTS, FALSEEXP, TRUEEXP,
                                  SELF, CASE, COLON, IFTHENELSE, SIMPWFF,
                                  NEXTWFF, LTLWFF, CTLWFF, COMPWFF, ATOM,
                                  NUMBER, COMMA, IMPLIES, IFF, OR, XOR, XNOR,
                                  AND, NOT, EX, AX, EF, AF, EG, AG, SINCE,
                                  UNTIL, TRIGGERED, RELEASES, EBF, EBG, ABF,
                                  ABG, OP_NEXT, OP_GLOBAL, OP_FUTURE, OP_PREC,
                                  OP_NOTPRECNOT, OP_HISTORICAL, OP_ONCE, EQUAL,
                                  NOTEQUAL, LT, GT, LE, GE, UNION, SETIN, MOD,
                                  PLUS, MINUS, TIMES, DIVIDE, UMINUS, NEXT,
                                  SMALLINIT, DOT, BIT, RANGE, UNSIGNED_WORD,
                                  SIGNED_WORD, INTEGER, REAL,
                                  NUMBER_UNSIGNED_WORD, NUMBER_SIGNED_WORD,
                                  NUMBER_FRAC, NUMBER_REAL, NUMBER_EXP, LSHIFT,
                                  RSHIFT, LROTATE, RROTATE, BIT_SELECTION,
                                  CONCATENATION, CAST_BOOL, CAST_WORD1,
                                  CAST_SIGNED, CAST_UNSIGNED, EXTEND,
                                  WORDARRAY, WAREAD, WAWRITE, UWCONST, SWCONST,
                                  WRESIZE, WSIZEOF, CAST_TOINT, COMPID,
                                  ARRAY_TYPE, ARRAY_DEF, NFUNCTION, COUNT)


def find_hierarchy(node):
    """
    Traverse the hierarchy represented by `node` and transfer it to the node
    hash table.

    :param node: the node
    :type node: a SWIG wrapper for a NuSMV node_ptr
    """
    return nsnode.node_normalize(node)


class Node(PointerWrapper):

    """A generic NuSMV node."""

    def __init__(self, left, right, type_=None):
        """
        Create a new node of `type_` with `left` and `right` as left and right
        branches.

        If `type_` is None, the type is infered from the instantiated class.

        :param left: the left branch
        :type left: :class:`Node` or `None`
        :param right: the right branch
        :type right: :class:`Node` or `None`
        :param type_: the type of the node
        :type type_: `int`

        """

        if type_ is None:
            type_ = class_to_type[type(self)]

        left_ptr = left._ptr if left is not None else left
        right_ptr = right._ptr if right is not None else right

        ptr = nsnode.find_node(type_, left_ptr, right_ptr)
        super(Node, self).__init__(ptr, freeit=False)
        self.type = type_
        self._car = left
        self._cdr = right

    @property
    def car(self):
        """The left branch of this node."""
        if not hasattr(self, "_car"):
            # Create the node
            car = nsnode.car(self._ptr)
            if car is not None:
                self._car = Node.from_ptr(car)
            else:
                self._car = None
        return self._car

    @property
    def cdr(self):
        """The right branch of this node."""
        if not hasattr(self, "_cdr"):
            # Create the node
            cdr = nsnode.cdr(self._ptr)
            if cdr is not None:
                self._cdr = Node.from_ptr(cdr)
            else:
                self._cdr = None
        return self._cdr

    def _free(self):
        if self._freeit and self._ptr is not None:
            nsnode.node_free(self._ptr)
            self._freeit = False

    def __str__(self):
        return nsnode.sprint_node(self._ptr)

    def __eq__(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return nsnode.node_equal(self._ptr, other._ptr) != 0
        else:
            return False

    def __hash__(self):
        return nsnode.node2int(self._ptr)

    def __deepcopy__(self, memo):
        # Does not need to deep copy since these are immutable
        # and pointers would be the same
        return self

    @staticmethod
    def from_ptr(ptr, freeit=False):
        cls = type_to_class[ptr.type]
        new_node = cls.__new__(cls)
        new_node._freeit = freeit
        new_node._ptr = ptr
        new_node.type = ptr.type
        return new_node

    @staticmethod
    def _handle_next_expression(expression):
        """
        Return a node representation of the next `expression`.
        If `expression` is already an Expresssion, return it as it is.
        If `expression` is a string of an integer, it is parsed as a next
        expression.

        :param expression: the expression to handle
        :type expression: :class:`Expression` or :class:`str` or :class:`int`
        """
        if isinstance(expression, int):
            expression = str(expression)
        if isinstance(expression, str):
            parsed = parse_next_expression(expression)
            expression = Node.from_ptr(find_hierarchy(parsed))
            nsnode.free_node(parsed)
        return expression


# -----------------------------------------------------------------------------
# ----- SECTION NODES
# -----------------------------------------------------------------------------


class Module(Node):
    pass


class Section(Node):

    """A generic section."""
    pass


class Trans(Section):
    pass


class Init(Section):
    pass


class Invar(Section):
    pass


class Assign(Section):
    pass


class Fairness(Section):
    pass


class Justice(Section):
    pass


class Compassion(Section):
    pass


class Spec(Section):
    pass


class Ltlspec(Section):
    pass


class Pslspec(Section):
    pass


class Invarspec(Section):
    pass


class Compute(Section):
    pass


class Define(Section):
    pass


class ArrayDef(Node):
    pass


class Isa(Section):
    pass


class Constants(Section):
    pass


class Var(Section):
    pass


class Frozenvar(Section):
    pass


class Ivar(Section):
    pass


# -----------------------------------------------------------------------------
# ----- TYPE NODES
# -----------------------------------------------------------------------------


class Type(Node):

    """A generic type node."""
    pass


class Boolean(Type):

    """The boolean type."""

    def __init__(self):
        super(Boolean, self).__init__(None, None, type_=BOOLEAN)


class UnsignedWord(Type):

    """An unsigned word type."""

    def __init__(self, length):
        super(UnsignedWord, self).__init__(length, None, type_=UNSIGNED_WORD)

    @property
    def length(self):
        return self.car


class Word(UnsignedWord):

    """An unsigned word type."""

    def __init__(self, length):
        super(Word, self).__init__(length)


class SignedWord(Type):

    """A signed word type."""

    def __init__(self, length):
        super(SignedWord, self).__init__(length, None, type_=SIGNED_WORD)

    @property
    def length(self):
        return self.car


class Range(Type):

    """A range type."""

    def __init__(self, start, stop):
        """
        Create a new range from `start` to `stop`.

        :param start: the starting value of the range
        :type start: :class:`Expression` or :class:`str` or :class:`int`
        :param stop: the ending value of the range
        :type stop: :class:`Expression` or :class:`str` or :class:`int`

        .. warning:: NuSMV parser limits the values of `start` and `stop` to
                     be a sub-language of the simple expressions.
                     There is no check made by this constructor to verify
                     that `start` and `stop` belong to this sub-language.
        """
        start = self._handle_next_expression(start)
        stop = self._handle_next_expression(stop)
        super(Range, self).__init__(start, stop, type_=TWODOTS)

    @property
    def start(self):
        return self.car

    @property
    def stop(self):
        return self.cdr


class ArrayType(Type):

    """An array type."""

    def __init__(self, start, stop, elementtype):
        """
        Create a new array type.

        :param start: the starting value of the indices of the array
        :type start: :class:`Expression` or :class:`str` or :class:`int`
        :param stop: the ending value of the indices of the array
        :type stop: :class:`Expression` or :class:`str` or :class:`int`
        :param elementtype: the type of the elements of the array
        :type elementtype: :class:`Type`

        .. warning:: NuSMV parser limits the values of `start` and `stop` to
                     be a sub-language of the simple expressions.
                     There is no check made by this constructor to verify
                     that `start` and `stop` belong to this sub-language.
        """
        super(ArrayType, self).__init__(Range(start, stop), elementtype)

    @property
    def start(self):
        return self.car.car

    @property
    def stop(self):
        return self.car.cdr

    @property
    def elementtype(self):
        return self.cdr


class Scalar(Type):

    """The enumeration type."""

    def __init__(self, values):
        """
        Create a new scalar type with values.

        :param values: a non-empty sequence of complex atoms
        :type values: Sequence
        """
        self._values = tuple(self._handle_type_value(value)
                             for value in values)
        values = tuple(reversed(self._values))

        ptr = None
        for value in values:
            ptr = nsnode.find_node(CONS, value._ptr, ptr)

        ptr = nsnode.find_node(SCALAR, ptr, None)
        super(Node, self).__init__(ptr, freeit=False)
        self.type = SCALAR

    @property
    def values(self):
        """The values of this enumration."""
        if not hasattr(self, "_values"):
            values = []
            cur = nsnode.car(self._ptr)
            while cur is not None:
                values.append(Node.from_ptr(nsnode.car(cur)))
                cur = nsnode.cdr(cur)
            self._values = tuple(reversed(values))
        return self._values

    def _handle_type_value(self, value):
        """
        Handle the given value by returning the corresponding node:
        * if `value` is a :class:`Node`, it is returned as it is.
        * if `value` is a :class:`str`, it is parsed against TRUE, FALSE and
          comple atoms (that is, DOTed atoms)
        * if `value` is a :class:`int`, it is returned as a :class:`Number`.

        :param value: the value to handle
        """
        if isinstance(value, Node):
            return value
        elif isinstance(value, int):
            return Number(value)
        else:  # value should be a string
            if value == "TRUE":
                return Trueexp()
            elif value == "FALSE":
                return Falseexp()
            else:  # Complex atoms
                parsed = parse_identifier(value)
                res = Node.from_ptr(find_hierarchy(parsed))
                nsnode.free_node(parsed)
                return res


class Modtype(Type):

    """A module instantiation type."""

    def __init__(self, name, arguments):
        """
        Create a new module instantiation of module `name` with `arguments`.

        :param name: the name of the module
        :type name: :class:`Atom` or :class:`str`
        :param arguments: the list of arguments of the instance
        :type arguments: a sequence of :class:`Expression` or :class:`str`
        """
        if isinstance(name, str):
            name = Atom(name)
        self._arguments = tuple(self._handle_next_expression(argument)
                                for argument in arguments)
        ptr = self._arguments[0]._ptr
        for argument in self._arguments[1:]:
            ptr = nsnode.find_node(CONS, argument._ptr, ptr)
        ptr = nsnode.find_node(MODTYPE, name._ptr, ptr)
        super(Node, self).__init__(ptr, freeit=False)
        self.type = MODTYPE

    @property
    def name(self):
        return self.car

    @property
    def arguments(self):
        if not hasattr(self, "_arguments"):
            arguments = []
            cur = nsnode.cdr(self._ptr)
            while cur is not None:
                arguments.append(Node.from_ptr(nsnode.car(cur)))
                cur = nsnode.cdr(cur)
            self._arguments = tuple(reversed(arguments))
        return self._arguments

    def __str__(self):
        rep = str(self.name)
        if len(self.arguments) > 0:
            rep += "("
            rep += ", ".join(str(arg) for arg in self.arguments)
            rep += ")"
        return rep


class Process(Type):

    """
    The process type.

    .. warning:: This type is deprecated.
    """
    pass


class Integer(Type):

    """
    The integer number type.

    .. warning:: This node type is not supported by NuSMV.
    """
    pass


class Real(Type):

    """
    The real number type.

    .. warning:: This node type is not supported by NuSMV.
    """
    pass


class Wordarray(Type):

    """
    The word array type.

    .. warning:: This type is not documented in NuSMV documentation.
    """
    pass


# -----------------------------------------------------------------------------
# ----- EXPRESSION NODES
# -----------------------------------------------------------------------------


class Expression(Node):

    """A generic expression node."""

    def __init__(self, left, right, type_=None):
        """
        Create a new expression of `type_` with `left` and `right` branches.
        If left or right is a string or an int, it is parsed as a next
        expression.

        :param left: the left branch
        :type left: :class:`Node` or :class:`str` or :class:`int`
        :param right: the right branch
        :type right: :class:`Node` or :class:`str` or :class:`int`
        :param type_: the type of the expression
        :type type_: :class:`int` or `None`
        """
        left = self._handle_next_expression(left)
        right = self._handle_next_expression(right)
        super(Expression, self).__init__(left, right, type_)

    def __hash__(self):
        return id(self._ptr)

    def in_context(self, context):
        return Context(self, context)

    def array(self, index):
        return Array(self, index)

    def twodots(self, stop):
        return Twodots(self, stop)

    def ifthenelse(self, true, false):
        return Ifthenelse(self, true, false)

    def implies(self, expression):
        return Implies(self, expression)

    def iff(self, expression):
        return Iff(self, expression)

    def or_(self, expression):
        return Or(self, expression)

    def xor(self, expression):
        return Xor(self, expression)

    def xnor(self, expression):
        return Xnor(self, expression)

    def and_(self, expression):
        return And(self, expression)

    def not_(self):
        return Not(self)

    def equal(self, expression):
        return Equal(self, expression)

    def notequal(self, expression):
        return Notequal(self, expression)

    def lt(self, expression):
        return Lt(self, expression)

    def gt(self, expression):
        return Gt(self, expression)

    def le(self, expression):
        return Le(self, expression)

    def ge(self, expression):
        return Ge(self, expression)

    def union(self, expression):
        return Union(self, expression)

    def setin(self, expression):
        return Setin(self, expression)

    def in_(self, expression):
        return Setin(self, expression)

    def mod(self, expression):
        return Mod(self, expression)

    def plus(self, expression):
        return Plus(self, expression)

    def minus(self, expression):
        return Minus(self, expression)

    def times(self, expression):
        return Times(self, expression)

    def divide(self, expression):
        return Divide(self, expression)

    def uminus(self):
        return Uminus(self)

    def next(self):
        return Next(self)

    def dot(self, expression):
        return Dot(self, expression)

    def lshift(self, expression):
        return Lshift(self, expression)

    def rshift(self, expression):
        return Rshift(self, expression)

    def lrotate(self, expression):
        return Lrotate(self, expression)

    def rrotate(self, expression):
        return Rrotate(self, expression)

    def bit_selection(self, start, stop):
        return BitSelection(self, start, stop)

    def concatenation(self, expression):
        return Concatenation(self, expression)

    def concat(self, expression):
        return Concatenation(self, expression)

    def castbool(self):
        return CastBool(self)

    def bool(self):
        return CastBool(self)

    def castword1(self):
        return CastWord1(self)

    def word1(self):
        return CastWord1(self)

    def castsigned(self):
        return CastSigned(self)

    def signed(self):
        return CastSigned(self)

    def castunsigned(self):
        return CastUnsigned(self)

    def unsigned(self):
        return CastUnsigned(self)

    def extend(size):
        return Extend(self, size)

    def waread(self, expression):
        return Waread(self, expression)

    def read(self, expression):
        return Waread(self, expression)

    def wawrite(second, third):
        return Wawrite(self, second, third)

    def write(self, second, third):
        return Wawrite(self, second, third)

    def uwconst(self, expression):
        return Uwcons(self, expression)

    def swconst(self, expression):
        return Swconst(self, expression)

    def wresize(self, size):
        return Wresize(self, size)

    def resize(self, size):
        return Wresize(self, size)

    def wsizeof(self):
        return Wsizeof(self)

    def sizeof(self):
        return Wsizeof(self)

    def casttoint(self):
        return CastToint(self)

    def toint(self):
        return CastToint(self)

    def __lt__(self, other):
        return Lt(self, other)

    def __le__(self, other):
        return Le(self, other)

    def __eq__(self, other):
        return Equal(self, other)

    def __ne__(self, other):
        return Notequal(self, other)

    def __gt__(self, other):
        return Gt(self, other)

    def __ge__(self, other):
        return Ge(self, other)

    def __getattr__(self, name):
        if name not in {"car", "cdr", "_ptr", "name", "this", "_car", "_cdr"}:
            return Dot(self, name)
        else:
            raise AttributeError("Missing {} attribute.".format(name))

    def __getitem__(self, key):
        if isinstance(key, slice):
            start, stop = slice.start, slice.stop
            start = self._handle_next_expression(start)
            stop = self._handle_next_expression(stop)
            return BitSelection(self, start, stop)
        else:
            key = self._handle_next_expression(key)
            return Array(self, key)

    def __add__(self, other):
        return Plus(self, other)

    def __radd__(self, other):
        return Plus(other, self)

    def __sub__(self, other):
        return Minus(self, other)

    def __rsub__(self, other):
        return Minus(other, self)

    def __mul__(self, other):
        return Times(self, other)

    def __rmul__(self, other):
        return Times(other, self)

    def __truediv__(self, other):
        return Divide(self, other)

    def __rtruediv__(self, other):
        return Divide(other, self)

    def __mod__(self, other):
        return Mod(self, other)

    def __rmod__(self, other):
        return Mod(other, self)

    def __lshift__(self, other):
        return Lshift(self, other)

    def __rlshift__(self, other):
        return Lshift(other, self)

    def __rshift__(self, other):
        return Rshift(self, other)

    def __rrshift__(self, other):
        return Rshift(other, self)

    def __and__(self, other):
        return And(self, other)

    def __rand__(self, other):
        return And(other, self)

    def __xor__(self, other):
        return Xor(self, other)

    def __rxor__(self, other):
        return Xor(other, self)

    def __or__(self, other):
        return Or(self, other)

    def __ror__(self, other):
        return Or(other, self)

    def __neg__(self):
        return Uminus(self)

    def __invert__(self):
        return Not(self)

    @staticmethod
    def from_string(expression):
        """
        Parse the string representation of the given expression and return the
        corresponding node.

        :param expression: the string to parse
        :rtype: :class:`Expression` subclass

        """
        expression = str(expression)
        parsed = parse_next_expression(expression)
        expression = Node.from_ptr(find_hierarchy(parsed))
        nsnode.free_node(parsed)
        return expression


class Leaf(Expression):
    pass


class Failure(Leaf):

    """A FAILURE node."""

    def __init__(self, message, kind):
        left = nsnode.find_node(COLON,
                                nsnode.string2node
                                (nsutils.find_string(message)),
                                nsnode.int2node(kind))
        super(Failure, self).__init__(left, nsnode.int2node(0), type_=FAILURE)

    @property
    def message(self):
        nsnode.node2string(self.car.car)

    @property
    def kind(self):
        nsnode.node2int(self.car.cdr)


class Falseexp(Leaf):

    """The FALSE expression."""

    def __init__(self):
        super(Falseexp, self).__init__(None, None, type_=FALSEEXP)


class Trueexp(Leaf):

    """The TRUE expression."""

    def __init__(self):
        super(Trueexp, self).__init__(None, None, type_=TRUEEXP)


class Self(Leaf):

    """The `self` expression."""

    def __init__(self):
        super(Self, self).__init__(None, None, type_=SELF)


class Atom(Leaf):

    """An ATOM node."""

    def __init__(self, name):
        """
        Create an ATOM node with `name`.

        :param name: the name of the atom
        :type name: :class:`str`
        """
        name_ptr = nsnode.string2node(nsutils.find_string(name))
        ptr = nsnode.find_node(ATOM, name_ptr, None)
        super(Node, self).__init__(ptr, freeit=False)
        self.type = ATOM

    @property
    def name(self):
        return nsutils.get_text(nsnode.node2string(nsnode.car(self._ptr)))


class Number(Leaf):

    """A node containing an integer."""

    def __init__(self, value):
        """
        Create a number.

        :param value: the value of the integer
        :type value: :class:`int`
        """
        value_ptr = nsnode.int2node(value)
        ptr = nsnode.find_node(NUMBER, value_ptr, None)
        super(Node, self).__init__(ptr, freeit=False)
        self.type = NUMBER

    @property
    def value(self):
        return nsnode.node2int(nsnode.car(self._ptr))


class NumberUnsignedWord(Leaf):

    """A node containing an unsigned word value."""

    def __init__(self, value):
        """
        Create an unsigned word.

        :param value: the word value
        :type value: :class:`str`
        """
        value_ptr = nsnode.word2node(
            nsutils.WordNumber_from_parsed_string(value, None))
        ptr = nsnode.find_node(NUMBER_UNSIGNED_WORD, value_ptr, None)
        super(Node, self).__init__(ptr, freeit=False)
        self.type = NUMBER_UNSIGNED_WORD

    @property
    def value(self):
        return nsutils.WordNumber_get_parsed_string(
            nsnode.node2word(nsnode.car(self._ptr)))


class NumberSignedWord(Leaf):

    """A node containing a signed word value."""

    def __init__(self, value):
        """
        Create a signed word.

        :param value: the word value
        :type value: :class:`str`
        """
        value_ptr = nsnode.word2node(
            nsutils.WordNumber_from_parsed_string(value, None))
        ptr = nsnode.find_node(NUMBER_SIGNED_WORD, value_ptr, None)
        super(Node, self).__init__(ptr, freeit=False)
        self.type = NUMBER_SIGNED_WORD

    @property
    def value(self):
        return nsutils.WordNumber_get_parsed_string(
            nsnode.node2word(nsnode.car(self._ptr)))


class NumberFrac(Leaf):

    """
    A rational number.

    .. warning:: This node type is not supported by NuSMV.
    """
    pass


class NumberReal(Leaf):

    """
    A real number.

    .. warning:: This node type is not supported by NuSMV.
    """
    pass


class NumberExp(Leaf):

    """
    An exponential-formed number.

    .. warning:: This node type is not supported by NuSMV.
    """
    pass


class Context(Expression):

    """A CONTEXT node."""

    @property
    def context(self):
        return self.car

    @property
    def expression(self):
        return self.cdr


class Array(Expression):

    """An ARRAY node."""
    @property
    def array(self):
        return self.car

    @property
    def index(self):
        return self.cdr


class Twodots(Expression):

    """A range of integers."""
    @property
    def start(self):
        return self.car

    @property
    def stop(self):
        return self.cdr


class Case(Expression):

    """A set of cases."""

    def __init__(self, values):
        """
        Create a set of cases.

        :param values: a non-empty mapping where keys are conditions of the
                       cases and associated values are expressions
        :type values: Mapping or sequence of pairs representing the mapping
        """
        # Create the corresponding NuSMV
        # Keep the mapping (or sequence transformed into mapping)
        # for further use

        if isinstance(values, Mapping):
            values = list(values.items())

        res = Failure("case conditions are not exhaustive",
                      nsutils.FAIRLURE_CASE_NOT_EXHAUSTIVE)._ptr
        for condition, expression in reversed(values):
            condition = self._handle_next_expression(condition)
            expression = self._handle_next_expression(expression)
            res = nsnode.find_node(CASE,
                                   nsnode.find_node(COLON, condition._ptr,
                                                    expression._ptr),
                                   res)
        super(Case, self).__init__(res.car, res.cdr, type_=CASE)
        self._values = OrderedDict(reversed(values))

    @property
    def values(self):
        """
        The mapping values of this Case expression.

        .. warning:: The returned mapping should not be modified. Modifying the
                     returned mapping will not change the actual NuSMV values
                     of this node.
        """
        if not hasattr(self, "_values"):
            self._values = OrderedDict()
            head = self
            while head.cdr.type is not FAILURE:
                self._values[head.car.car] = head.car.cdr
                head = head.cdr
        return self._values


class Ifthenelse(Expression):

    """The `cond ? truebranch : falsebranch` expression."""

    def __init__(self, condition, true, false):
        """
        Create an if-then-else expression with `condition`, `true` and `false`.

        :param condition: the condition of the expression
        :type condition: :class:`Node` or :class:`str`
        :param true: the true branch of the expression
        :type true: :class:`Node` or :class:`str`
        :param false: the false branch of the expression
        :type false: :class:`Node` or :class:`str`
        """
        condition = self._handle_next_expression(condition)
        true = self._handle_next_expression(true)
        false = self._handle_next_expression(false)
        super(Ifthenelse, self).__init__(Colon(condition, true), false,
                                         type_=IFTHENELSE)

    @property
    def condition(self):
        return self.car.car

    @property
    def true(self):
        return self.car.cdr

    @property
    def false(self):
        return self.cdr


class Implies(Expression):
    pass


class Iff(Expression):
    pass


class Or(Expression):
    pass


class Xor(Expression):
    pass


class Xnor(Expression):
    pass


class And(Expression):
    pass


class Not(Expression):

    """A NOT expression."""

    def __init__(self, expression):
        """
        Create a NOT expression with `expression`.

        :param expression: the child of the NOT expression
        :type expression: :class:`Node` or :class:`str` or :class:`int`
        """
        super(Not, self).__init__(expression, None, type_=NOT)

    @property
    def expression(self):
        return self.car


class Equal(Expression):

    def __bool__(self):
        return nsnode.node_equal(self.car._ptr, self.cdr._ptr) != 0
    
    def __nonzero__(self):
        return self.__bool__()


class Notequal(Expression):

    def __bool__(self):
        return nsnode.node_equal(self.car._ptr, self.cdr._ptr) == 0
    
    def __nonzero__(self):
        return self.__bool__()


class Lt(Expression):
    pass


class Gt(Expression):
    pass


class Le(Expression):
    pass


class Ge(Expression):
    pass


class Union(Expression):
    pass


class Setin(Expression):
    pass


class Mod(Expression):
    pass


class Plus(Expression):
    pass


class Minus(Expression):
    pass


class Times(Expression):
    pass


class Divide(Expression):
    pass


class Uminus(Expression):

    """A unitary minus expression."""

    def __init__(self, expression):
        """
        Create a unitary minus expression with `expression`.

        :param expression: the child of the expression
        :type expression: :class:`Node` or :class:`str` or :class:`int`
        """
        super(Uminus, self).__init__(expression, None, type_=UMINUS)

    @property
    def expression(self):
        return self.car


class Next(Expression):

    """A NEXT expression."""

    def __init__(self, expression):
        """
        Create a NEXT expression with `expression`.

        :param expression: the child of the NEXT expression
        :type expression: :class:`Node` or :class:`str` or :class:`int`
        """
        super(Next, self).__init__(expression, None, type_=NEXT)

    @property
    def expression(self):
        return self.car


class Dot(Expression):
    pass


class Lshift(Expression):
    pass


class Rshift(Expression):
    pass


class Lrotate(Expression):
    pass


class Rrotate(Expression):
    pass


class BitSelection(Expression):

    """A Bit selection node."""

    def __init__(self, word, start, stop):
        """
        Create a bit selection of bits from `start` to `stop` in `word`.

        :param word: the word to select from
        :type word: :class:`Node` or class:`str`
        :param start: the starting index
        :type start: :class:`Node` or class:`str` or :class:`int`
        :param stop: the stopping index
        :type stop: :class:`Node` or class:`str` or :class:`int`
        """
        word = self._handle_next_expression(word)
        start = self._handle_next_expression(start)
        stop = self._handle_next_expression(stop)
        super(BitSelection, self).__init__(word, Colon(start, stop),
                                           type_=BIT_SELECTION)

    @property
    def word(self):
        return self.car

    @property
    def start(self):
        return self.cdr.car

    @property
    def stop(self):
        return self.cdr.cdr


class Concatenation(Expression):
    pass


class CastBool(Expression):

    """A boolean casting node."""

    def __init__(self, expression):
        """
        Create a boolean casting of `expression`.

        :param expression: the expression to cast
        :type expression: :class:`Node` or :class:`str` or :class:`int`
        """
        super(CastBool, self).__init__(expression, None, type_=CAST_BOOL)

    @property
    def expression(self):
        return self.car


class CastWord1(Expression):

    """A word-1 casting node."""

    def __init__(self, expression):
        """
        Create a casting of `expression` to word-1.

        :param expression: the expression to cast
        :type expression: :class:`Node` or :class:`str` or :class:`int`
        """
        super(CastWord1, self).__init__(expression, None, type_=CAST_WORD1)

    @property
    def expression(self):
        return self.car


class CastSigned(Expression):

    """A signed number casting node."""

    def __init__(self, expression):
        """
        Create a signed number casting of `expression`.

        :param expression: the expression to cast
        :type expression: :class:`Node` or :class:`str` or :class:`int`
        """
        super(CastSigned, self).__init__(expression, None, type_=CAST_SIGNED)

    @property
    def expression(self):
        return self.car


class CastUnsigned(Expression):

    """An unsigned number casting node."""

    def __init__(self, expression):
        """
        Create an unsigned number casting of `expression`.

        :param expression: the expression to cast
        :type expression: :class:`Node` or :class:`str` or :class:`int`
        """
        super(CastUnsigned).__init__(expression, None, type_=CAST_UNSIGNED)

    @property
    def expression(self):
        return self.car


class Extend(Expression):
    pass


class Waread(Expression):
    pass


class Wawrite(Expression):

    """A WAWRITE node."""

    def __init__(self, first, second, third):
        """
        Create a WAWRITE node with `first`, `second` and `third`.

        :param first: the first element of the WAWRITE node
        :type first: :class:`Node` or :class:`str`
        :param second: the second element of the WAWRITE node
        :type second: :class:`Node` or :class:`str`
        :param third: the third element of the WAWRITE node
        :type third: :class:`Node` or :class:`str`
        """
        super(Wawrite, self).__init__(first,
                                      Expression(second, third, type_=WAWRITE),
                                      type_=WAWRITE)


class Uwconst(Expression):
    pass


class Swconst(Expression):
    pass


class Wresize(Expression):
    pass


class Wsizeof(Expression):

    """A size-of-word node."""

    def __init__(self, expression):
        """
        Create size-of node of `expression`.

        :param expression: the expression
        :type expression: :class:`Node` or :class:`str`
        """
        super(Wsizeof, self).__init__(expression, None, type_=WSIZEOF)

    @property
    def expression(self):
        return self.car


class CastToint(Expression):

    """An integer casting node."""

    def __init__(self, expression):
        """
        Create an integer casting of `expression`.

        :param expression: the expression to cast
        :type expression: :class:`Node` or :class:`str` or :class:`int`
        """
        super(CastToint, self).__init__(expression, None, type_=CAST_TOINT)

    @property
    def expression(self):
        return self.car


class Count(Expression):

    """A set expression."""

    def __init__(self, values):
        """
        Create a new count expression with values.

        :param values: a non-empty sequence of expressions
        :type values: Sequence
        """
        self._values = tuple(self._handle_next_expression(value)
                             for value in values)
        ptr = find_hierarchy(parse_next_expression(
            "count(" + ", ".join(str(value)
                                 for value in self.values) + ")"))
        super(Node, self).__init__(ptr, freeit=False)
        self.type = COUNT

    @property
    def values(self):
        """The values of this count."""
        if not hasattr(self, "_values"):
            values = []
            cur = self._ptr
            while cur is not None:
                values.append(cur.car)
                cur = cur.cdr
            self._values = tuple(values)
        return self._values


class CustomExpression(Expression):

    """A generic custom expression."""
    pass


class Set(CustomExpression):

    """A set expression."""

    def __init__(self, values):
        """
        Create a new set expression with values.

        :param values: a non-empty sequence of expressions
        :type values: Sequence
        """
        self._values = tuple(self._handle_next_expression(value)
                             for value in values)
        ptr = find_hierarchy(parse_next_expression(
            "{" + ", ".join(str(value)
                            for value in self.values) + "}"))
        super(Node, self).__init__(ptr, freeit=False)
        self.type = UNION

    @property
    def values(self):
        """The values of this set."""
        return self._values


class Identifier(CustomExpression):

    """A custom identifier."""

    @staticmethod
    def from_string(identifier):
        """
        Return the node representation of identifier.

        :param :class:`str` identifier: the string representation of an
                                        identifier
        """
        parsed = parse_identifier(identifier)
        ptr = find_hierarchy(parsed)
        nsnode.free_node(parsed)
        return Node.from_ptr(ptr)


# -----------------------------------------------------------------------------
# ----- PROPERTY NODES
# -----------------------------------------------------------------------------


class Property(Expression):
    pass


class Eu(Property):
    pass


class Au(Property):
    pass


class Ew(Property):
    pass


class Aw(Property):
    pass


class Ebu(Property):
    pass


class Abu(Property):
    pass


class Minu(Property):
    pass


class Maxu(Property):
    pass


class Ex(Property):
    pass


class Ax(Property):
    pass


class Ef(Property):
    pass


class Af(Property):
    pass


class Eg(Property):
    pass


class Ag(Property):
    pass


class Since(Property):
    pass


class Until(Property):
    pass


class Triggered(Property):
    pass


class Releases(Property):
    pass


class Ebf(Property):
    pass


class Ebg(Property):
    pass


class Abf(Property):
    pass


class Abg(Property):
    pass


class OpNext(Property):
    pass


class OpGlobal(Property):
    pass


class OpFuture(Property):
    pass


class OpPrec(Property):
    pass


class OpNotprecnot(Property):
    pass


class OpHistorical(Property):
    pass


class OpOnce(Property):
    pass


# -----------------------------------------------------------------------------
# ----- OTHER NODES
# -----------------------------------------------------------------------------

class Cons(Node):
    pass


class Pred(Node):
    pass


class Attime(Node):
    pass


class PredsList(Node):
    pass


class Mirror(Node):
    pass


class SyntaxError_(Node):
    pass


class Simpwff(Node):
    pass


class Nextwff(Node):
    pass


class Ltlwff(Node):
    pass


class Ctlwff(Node):
    pass


class Compwff(Node):
    pass


class Compid(Node):
    pass


class Bdd(Node):
    pass


class Semi(Node):
    pass


class Eqdef(Node):
    pass


class Smallinit(Node):
    pass


class Bit(Node):
    pass


class Nfunction(Node):
    pass


class Goto(Node):
    pass


class Constraint(Node):
    pass


class Lambda(Node):
    pass


class Comma(Node):
    pass


class Colon(Node):
    pass


# -----------------------------------------------------------------------------
# ----- MISC
# -----------------------------------------------------------------------------

# -----------------------------------------------------------------------------
# ----- DECLARATIONS
# -----------------------------------------------------------------------------

class Declaration(Atom):

    """
    A Declaration behaves like an atom, except that it knows which type
    it belongs to. Furthermore, it does not know its name for sure, and
    cannot be printed without giving it a name.

    """

    def __init__(self, type_, section, name=None):
        self.declared_type = type_
        self.section = section
        if name is None:
            name = (section +
                    ''.join(random.choice(string.ascii_lowercase)
                            for _ in range(6)))
            self._anonymous = True
        super(Declaration, self).__init__(name)

    @property
    def name(self):
        """The name of the declared identifier."""
        if self._anonymous:
            raise AttributeError("Unknown declaration name.")
        return self.car

    @name.setter
    def name(self, name):
        """Update the name of the declared identifier."""
        name_ptr = nsnode.string2node(nsutils.find_string(name))
        ptr = nsnode.setcar(self._ptr, name_ptr)
        self._anonymous = False


class DVar(Declaration):

    """A declared VAR."""

    def __init__(self, type_, name=None):
        super(DVar, self).__init__(type_, "VAR", name=name)


class DIVar(Declaration):

    """A declared IVAR."""

    def __init__(self, type_, name=None):
        super(DIVar, self).__init__(type_, "IVAR", name=name)


class DFVar(Declaration):

    """A declared FROZENVAR."""

    def __init__(self, type_, name=None):
        super(DFVar, self).__init__(type_, "FROZENVAR", name=name)


class DDef(Declaration):

    """A declared DEFINE."""

    def __init__(self, type_, name=None):
        super(DDef, self).__init__(type_, "DEFINE", name=name)


type_to_class = {
    TRANS: Trans,
    INIT: Init,
    INVAR: Invar,
    ASSIGN: Assign,
    FAIRNESS: Fairness,
    JUSTICE: Justice,
    COMPASSION: Compassion,
    SPEC: Spec,
    LTLSPEC: Ltlspec,
    PSLSPEC: Pslspec,
    INVARSPEC: Invarspec,
    COMPUTE: Compute,
    DEFINE: Define,
    ISA: Isa,
    GOTO: Goto,
    CONSTRAINT: Constraint,
    MODULE: Module,
    PROCESS: Process,
    MODTYPE: Modtype,
    LAMBDA: Lambda,
    CONSTANTS: Constants,
    PRED: Pred,
    ATTIME: Attime,
    PREDS_LIST: PredsList,
    MIRROR: Mirror,
    SYNTAX_ERROR: SyntaxError_,
    FAILURE: Failure,
    CONTEXT: Context,
    EU: Eu,
    AU: Au,
    EW: Ew,
    AW: Aw,
    EBU: Ebu,
    ABU: Abu,
    MINU: Minu,
    MAXU: Maxu,
    VAR: Var,
    FROZENVAR: Frozenvar,
    IVAR: Ivar,
    BOOLEAN: Boolean,
    ARRAY: Array,
    SCALAR: Scalar,
    CONS: Cons,
    BDD: Bdd,
    SEMI: Semi,
    EQDEF: Eqdef,
    TWODOTS: Twodots,
    FALSEEXP: Falseexp,
    TRUEEXP: Trueexp,
    SELF: Self,
    CASE: Case,
    COLON: Colon,
    IFTHENELSE: Ifthenelse,
    SIMPWFF: Simpwff,
    NEXTWFF: Nextwff,
    LTLWFF: Ltlwff,
    CTLWFF: Ctlwff,
    COMPWFF: Compwff,
    ATOM: Atom,
    NUMBER: Number,
    COMMA: Comma,
    IMPLIES: Implies,
    IFF: Iff,
    OR: Or,
    XOR: Xor,
    XNOR: Xnor,
    AND: And,
    NOT: Not,
    EX: Ex,
    AX: Ax,
    EF: Ef,
    AF: Af,
    EG: Eg,
    AG: Ag,
    SINCE: Since,
    UNTIL: Until,
    TRIGGERED: Triggered,
    RELEASES: Releases,
    EBF: Ebf,
    EBG: Ebg,
    ABF: Abf,
    ABG: Abg,
    OP_NEXT: OpNext,
    OP_GLOBAL: OpGlobal,
    OP_FUTURE: OpFuture,
    OP_PREC: OpPrec,
    OP_NOTPRECNOT: OpNotprecnot,
    OP_HISTORICAL: OpHistorical,
    OP_ONCE: OpOnce,
    EQUAL: Equal,
    NOTEQUAL: Notequal,
    LT: Lt,
    GT: Gt,
    LE: Le,
    GE: Ge,
    UNION: Union,
    SETIN: Setin,
    MOD: Mod,
    PLUS: Plus,
    MINUS: Minus,
    TIMES: Times,
    DIVIDE: Divide,
    UMINUS: Uminus,
    NEXT: Next,
    SMALLINIT: Smallinit,
    DOT: Dot,
    BIT: Bit,
    RANGE: Range,
    UNSIGNED_WORD: UnsignedWord,
    SIGNED_WORD: SignedWord,
    INTEGER: Integer,
    REAL: Real,
    NUMBER_UNSIGNED_WORD: NumberUnsignedWord,
    NUMBER_SIGNED_WORD: NumberSignedWord,
    NUMBER_FRAC: NumberFrac,
    NUMBER_REAL: NumberReal,
    NUMBER_EXP: NumberExp,
    LSHIFT: Lshift,
    RSHIFT: Rshift,
    LROTATE: Lrotate,
    RROTATE: Rrotate,
    BIT_SELECTION: BitSelection,
    CONCATENATION: Concatenation,
    CAST_BOOL: CastBool,
    CAST_WORD1: CastWord1,
    CAST_SIGNED: CastSigned,
    CAST_UNSIGNED: CastUnsigned,
    EXTEND: Extend,
    WORDARRAY: Wordarray,
    WAREAD: Waread,
    WAWRITE: Wawrite,
    UWCONST: Uwconst,
    SWCONST: Swconst,
    WRESIZE: Wresize,
    WSIZEOF: Wsizeof,
    CAST_TOINT: CastToint,
    COMPID: Compid,
    ARRAY_TYPE: ArrayType,
    ARRAY_DEF: ArrayDef,
    NFUNCTION: Nfunction,
    COUNT: Count,
}
class_to_type = {
    Trans: TRANS,
    Init: INIT,
    Invar: INVAR,
    Assign: ASSIGN,
    Fairness: FAIRNESS,
    Justice: JUSTICE,
    Compassion: COMPASSION,
    Spec: SPEC,
    Ltlspec: LTLSPEC,
    Pslspec: PSLSPEC,
    Invarspec: INVARSPEC,
    Compute: COMPUTE,
    Define: DEFINE,
    Isa: ISA,
    Goto: GOTO,
    Constraint: CONSTRAINT,
    Module: MODULE,
    Process: PROCESS,
    Modtype: MODTYPE,
    Lambda: LAMBDA,
    Constants: CONSTANTS,
    Pred: PRED,
    Attime: ATTIME,
    PredsList: PREDS_LIST,
    Mirror: MIRROR,
    SyntaxError_: SYNTAX_ERROR,
    Failure: FAILURE,
    Context: CONTEXT,
    Eu: EU,
    Au: AU,
    Ew: EW,
    Aw: AW,
    Ebu: EBU,
    Abu: ABU,
    Minu: MINU,
    Maxu: MAXU,
    Var: VAR,
    Frozenvar: FROZENVAR,
    Ivar: IVAR,
    Boolean: BOOLEAN,
    Array: ARRAY,
    Scalar: SCALAR,
    Cons: CONS,
    Bdd: BDD,
    Semi: SEMI,
    Eqdef: EQDEF,
    Twodots: TWODOTS,
    Falseexp: FALSEEXP,
    Trueexp: TRUEEXP,
    Self: SELF,
    Case: CASE,
    Colon: COLON,
    Ifthenelse: IFTHENELSE,
    Simpwff: SIMPWFF,
    Nextwff: NEXTWFF,
    Ltlwff: LTLWFF,
    Ctlwff: CTLWFF,
    Compwff: COMPWFF,
    Atom: ATOM,
    Number: NUMBER,
    Comma: COMMA,
    Implies: IMPLIES,
    Iff: IFF,
    Or: OR,
    Xor: XOR,
    Xnor: XNOR,
    And: AND,
    Not: NOT,
    Ex: EX,
    Ax: AX,
    Ef: EF,
    Af: AF,
    Eg: EG,
    Ag: AG,
    Since: SINCE,
    Until: UNTIL,
    Triggered: TRIGGERED,
    Releases: RELEASES,
    Ebf: EBF,
    Ebg: EBG,
    Abf: ABF,
    Abg: ABG,
    OpNext: OP_NEXT,
    OpGlobal: OP_GLOBAL,
    OpFuture: OP_FUTURE,
    OpPrec: OP_PREC,
    OpNotprecnot: OP_NOTPRECNOT,
    OpHistorical: OP_HISTORICAL,
    OpOnce: OP_ONCE,
    Equal: EQUAL,
    Notequal: NOTEQUAL,
    Lt: LT,
    Gt: GT,
    Le: LE,
    Ge: GE,
    Union: UNION,
    Setin: SETIN,
    Mod: MOD,
    Plus: PLUS,
    Minus: MINUS,
    Times: TIMES,
    Divide: DIVIDE,
    Uminus: UMINUS,
    Next: NEXT,
    Smallinit: SMALLINIT,
    Dot: DOT,
    Bit: BIT,
    Range: RANGE,
    UnsignedWord: UNSIGNED_WORD,
    SignedWord: SIGNED_WORD,
    Integer: INTEGER,
    Real: REAL,
    NumberUnsignedWord: NUMBER_UNSIGNED_WORD,
    NumberSignedWord: NUMBER_SIGNED_WORD,
    NumberFrac: NUMBER_FRAC,
    NumberReal: NUMBER_REAL,
    NumberExp: NUMBER_EXP,
    Lshift: LSHIFT,
    Rshift: RSHIFT,
    Lrotate: LROTATE,
    Rrotate: RROTATE,
    BitSelection: BIT_SELECTION,
    Concatenation: CONCATENATION,
    CastBool: CAST_BOOL,
    CastWord1: CAST_WORD1,
    CastSigned: CAST_SIGNED,
    CastUnsigned: CAST_UNSIGNED,
    Extend: EXTEND,
    Wordarray: WORDARRAY,
    Waread: WAREAD,
    Wawrite: WAWRITE,
    Uwconst: UWCONST,
    Swconst: SWCONST,
    Wresize: WRESIZE,
    Wsizeof: WSIZEOF,
    CastToint: CAST_TOINT,
    Compid: COMPID,
    ArrayType: ARRAY_TYPE,
    ArrayDef: ARRAY_DEF,
    Nfunction: NFUNCTION,
    Count: COUNT,
}


class FlatHierarchy(PointerWrapper):

    """
    Python class for flat hiearchy. The flat hierarchy is a NuSMV model where
    all the modules instances are reduced to their variables.

    A FlatHierarchy is used to store information obtained after flattening
    module hierarchy. It stores:

    * the list of TRANS, INIT, INVAR, ASSIGN, SPEC, COMPUTE, LTLSPEC,
      PSLSPEC, INVARSPEC, JUSTICE, COMPASSION,
    * a full list of variables declared in the hierarchy,
    * a hash table associating variables to their assignments and constraints.

    .. note:: There are a few assumptions about the content stored in this
              class:

              1. All expressions are stored in the same order as in the input
                 file (in module body or module instantiation order).
              2. Assigns are stored as a list of pairs (process instance name,
                 assignments in it).
              3. Variable list contains only vars declared in this hierarchy.
              4. The association var->assignments should be for assignments of
                 this hierarchy only.
              5. The association var->constraints (init, trans, invar) should
                 be for constraints of this hierarchy only.

    """
    # flat hierarchies do not have to be freed.

    def __init__(self, ptr, freeit=False):
        """
        Create a new FlatHierarchy.

        :param ptr: the pointer of the NuSMV flat hierarchy
        :param boolean freeit: whether or not free the pointer

        """
        super(FlatHierarchy, self).__init__(ptr, freeit=freeit)

    @property
    def symbTable(self):
        """The symbolic table of the hierarchy."""
        from .fsm import SymbTable
        return SymbTable(nscompile.FlatHierarchy_get_symb_table(self._ptr))

    @property
    def variables(self):
        """
        The set of variables declared in this hierarchy.

        .. warning:: The returned variables must not be altered.
        """
        var_set = nscompile.FlatHierarchy_get_vars(self._ptr)
        ite = nsset.Set_GetFirstIter(var_set)
        variables = []
        while not nsset.Set_IsEndIter(ite):
            variables.append(Node.from_ptr(nsset.Set_GetMember(var_set, ite)))
            ite = nsset.Set_GetNextIter(ite)
        return tuple(variables)

    @property
    def init(self):
        """The INIT section of the flat hierarchy."""
        init = nscompile.FlatHierarchy_get_init(self._ptr)
        return Node.from_ptr(init) if init is not None else None

    @init.setter
    def init(self, new_init):
        ptr = new_init._ptr if new_init is not None else None
        nscompile.FlatHierarchy_set_init(self._ptr, ptr)

    @property
    def invar(self):
        """The INVAR section of the flat hierarchy."""
        invar = nscompile.FlatHierarchy_get_invar(self._ptr)
        return Node.from_ptr(invar) if invar is not None else None

    @invar.setter
    def invar(self, new_invar):
        ptr = new_invar._ptr if new_invar is not None else None
        nscompile.FlatHierarchy_set_invar(self._ptr, ptr)

    @property
    def trans(self):
        """The TRANS section of the flat hierarchy."""
        trans = nscompile.FlatHierarchy_get_trans(self._ptr)
        return Node.from_ptr(trans) if trans is not None else None

    @trans.setter
    def trans(self, new_trans):
        ptr = new_trans._ptr if new_trans is not None else None
        nscompile.FlatHierarchy_set_trans(self._ptr, ptr)

    @property
    def justice(self):
        """The JUSTICE section of the flat hierarchy."""
        justice = nscompile.FlatHierarchy_get_justice(self._ptr)
        return Node.from_ptr(justice) if justice is not None else None

    @justice.setter
    def justice(self, new_justice):
        ptr = new_justice._ptr if new_justice is not None else None
        nscompile.FlatHierarchy_set_justice(self._ptr, ptr)

    @property
    def compassion(self):
        """The COMPASSION section of the flat hierarchy."""
        compassion = nscompile.FlatHierarchy_get_compassion(self._ptr)
        return Node.from_ptr(compassion) if compassion is not None else None

    @compassion.setter
    def compassion(self, new_compassion):
        ptr = new_compassion._ptr if new_compassion is not None else None
        nscompile.FlatHierarchy_set_compassion(self._ptr, ptr)
