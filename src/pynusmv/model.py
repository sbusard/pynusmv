"""
The :mod:`pynusmv.model` module provides a way to define NuSMV modules in
Python. The module is composed of several classes that fall in five sections:

* :class:`Expression` sub-classes represent elements of expressions of the
  NuSMV modelling language. NuSMV expressions can be defined by combining these
  classes (e.g. ``Add(Identifier("c"), 1)``), by using
  :class:`Expression` methods (e.g. ``Identifier("c").add(1)``) or
  by using built-in operators (e.g. ``Identifier("c") + 1``).
* :class:`Type` sub-classes represent types of NuSMV variables.
* :class:`Section` sub-classes represent the different sections (VAR, IVAR,
  TRANS, etc.) of a NuSMV module.
* :class:`Declaration` sub-classes are used in the declaration of a module
  to allow a more pythonic way of declaring NuSMV variables.
* :class:`Module`: the :class:`Module` class represents a generic NuSMV module,
  and must be subclassed to define specific NuSMV modules. See the
  documentation of the :class:`Module` class to get more information on how
  to declare a NuSMV module with this class.

"""

__all__ = [
    "Comment",
    "Identifier",
    "Self",
    "Dot",
    "ArrayAccess",
    "Trueexp",
    "Falseexp",
    "NumberWord",
    "RangeConst",
    "Conversion",
    "WordFunction",
    "Count",
    "Next",
    "Smallinit",
    "Case",
    "Subscript",
    "BitSelection",
    "Set",
    "Not",
    "Concat",
    "Minus",
    "Mult",
    "Div",
    "Mod",
    "Add",
    "Sub",
    "LShift",
    "RShift",
    "Union",
    "In",
    "Equal",
    "NotEqual",
    "Lt",
    "Gt",
    "Le",
    "Ge",
    "And",
    "Or",
    "Xor",
    "Xnor",
    "Ite",
    "Iff",
    "Implies",
    "ArrayExpr",
    "Boolean",
    "Word",
    "Scalar",
    "Range",
    "Array",
    "Modtype",
    "Variables",
    "InputVariables",
    "FrozenVariables",
    "Defines",
    "Assigns",
    "Constants",
    "Trans",
    "Init",
    "Invar",
    "Fairness",
    "Justice",
    "Compassion",
    "Var",
    "IVar",
    "FVar",
    "Def",
    "Module"]

import sys
from collections import OrderedDict
try:
    from collections.abc import Mapping
except ImportError:
    from collections import Mapping
from copy import deepcopy

from .nusmv.node import node as nsnode

from .utils import update
from .exception import NuSMVModuleError


class Element(object):

    """A parsed element."""
    source = None
    """
    The string the element comes from, if any (that is, if it comes from a
    parser).

    """

    def __init__(self, comments=None):
        """
        Create a new element.

        :param comments: If not None, a list of comments (`str`) attached to
                         the element.
        """
        self.comments = comments if comments is not None else []

    def __str__(self, string=""):
        """
        Return the string representation of this element, augmented with
        comments and the given string.

        :param str string: additional string.

        .. note::

            This function is intended to be used by subclasses; they give their
            own string representation through the `string` parameter and this
            function adds the comments.
        """
        if self.source:
            string = self.source
        if self.comments:
            return _add_comments_to_string(string, self.comments)
        else:
            return string

    def to_node(self):
        """
        Translate the element into a Node instance.
        """
        raise NotImplementedError("Should be implemented by subclasses.")


def Comment(element, string):
    """
    Attach the given comment to the given element.

    :param Element element: the element to attach the comment to.
    :param str string: the comment to attach.
    :return: the element itself.
    """
    element.comments.append(string)
    return element


def _add_comments_to_string(string, comments):
    """
    Return `string` augmented with the given comments.

    :param str string: the string to augment.
    :param list(str) comments: the comments.

    .. note::

        If comments is empty, string is returned unchanged, otherwise, the
        returned value is string followed by a space, followed by comments
        preceded by `-- ` and separated by new lines. The last character of
        the returned value is, in the latter case, a new line.
    """
    if comments:
        return (string +
                " " +
                "\n".join("-- " + comment for comment in comments) +
                "\n")
    else:
        return string


# -----------------------------------------------------------------------------
# ----- EXPRESSIONS
# -----------------------------------------------------------------------------


class Expression(Element):

    """A generic expression."""

    _precedence = 0

    def __init__(self, *args, **kwargs):
        super(Expression, self).__init__(*args, **kwargs)

    def __repr__(self):
        return str(self)

    def __lt__(self, other):
        return self.lt(other)

    def lt(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Lt(self, other)

    def __le__(self, other):
        return self.le(other)

    def le(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Le(self, other)

    def __eq__(self, other):
        return self.eq(other)

    def eq(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Equal(self, other)

    def __ne__(self, other):
        return self.ne(other)

    def ne(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return NotEqual(self, other)

    def __gt__(self, other):
        return self.gt(other)

    def gt(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Gt(self, other)

    def __ge__(self, other):
        return self.ge(other)

    def ge(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Ge(self, other)

    def __add__(self, other):
        return self.add(other)

    def add(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Add(self, other)

    def __sub__(self, other):
        return self.sub(other)
    
    def __rsub__(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Sub(other, self)

    def sub(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Sub(self, other)

    def __mul__(self, other):
        return self.mul(other)

    def mul(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Mult(self, other)

    def __truediv__(self, other):
        return self.div(other)

    def div(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Div(self, other)

    def __mod__(self, other):
        return self.mod(other)

    def mod(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Mod(self, other)

    def __lshift__(self, other):
        return self.lshift(other)

    def lshift(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return LShift(self, other)

    def __rshift__(self, other):
        return self.rshift(other)

    def rshift(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return RShift(self, other)

    def __and__(self, other):
        return self.and_(other)

    def and_(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return And(self, other)

    def __xor__(self, other):
        return self.xor(other)

    def xor(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Xor(self, other)

    def xnor(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Xnor(self, other)

    def __or__(self, other):
        return self.or_(other)

    def or_(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Or(self, other)

    def __neg__(self):
        return self.neg()

    def neg(self):
        return self.minus()

    def minus(self):
        return Minus(self)

    def __invert__(self):
        return self.invert()

    def invert(self):
        return self.not_()

    def not_(self):
        return Not(self)

    def __getitem__(self, key):
        from .parser import parseAllString, next_expression
        if isinstance(key, slice):
            start, stop = slice.start, slice.stop
            if isinstance(start, str):
                start = parseAllString(next_expression, start)
            if isinstance(stop, str):
                stop = parseAllString(next_expression, stop)
            return BitSelection(self, start, stop)
        elif isinstance(key, str):
            key = parseAllString(next_expression, key)
            return Subscript(self, key)
        else:
            return Subscript(self, key)

    def word1(self):
        return Conversion("word1", self)

    def bool(self):
        return Conversion("bool", self)

    def toint(self):
        return Conversion("toint", self)

    def signed(self):
        return Conversion("signed", self)

    def unsigned(self):
        return Conversion("unsigned", self)

    def next(self):
        return Next(self)

    def init(self):
        return Init(self)

    def concat(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Concat(self, other)

    def union(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Union(self, other)

    def in_(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return In(self, other)

    def ite(self, true_expr, false_expr):
        from .parser import parseAllString, next_expression
        if isinstance(true_expr, str):
            true_expr = parseAllString(next_expression, true_expr)
        if isinstance(false_expr, str):
            false_expr = parseAllString(next_expression, false_expr)
        return Ite(self, true_expr, false_expr)

    def iff(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Iff(self, other)

    def implies(self, other):
        from .parser import parseAllString, next_expression
        if isinstance(other, str):
            other = parseAllString(next_expression, other)
        return Implies(self, other)

    def __deepcopy__(self, memo):
        raise NotImplementedError("Should be implemented by subclasses.")

    def to_node(self):
        """
        Translate the element into a Node instance.
        """
        from . import node
        from .parser import parse_next_expression
        parsed = parse_next_expression(str(self))
        res = node.Node.from_ptr(node.find_hierarchy(parsed))
        nsnode.free_node(parsed)
        return res
    
    @classmethod
    def from_string(cls, expr):
        """
        Create a new Expression from the given string `expr`.
        """
        from .parser import parseAllString, next_expression
        return parseAllString(next_expression, expr)


class Identifier(Expression):

    """An identifier."""

    def __init__(self, name, *args, **kwargs):
        super(Identifier, self).__init__(*args, **kwargs)
        self.name = name

    def __str__(self):
        string = str(self.name)
        return super(Identifier, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.name == other.name
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Identifier") + 23 ** 2 * hash(self.name)

    def __getattr__(self, name):
        if name is not "name":
            return Dot(self, Identifier(name))
        else:
            raise AttributeError("'{}' object ha no attribute '{}'"
                                 .format(type(self).__name__, name))

    def __deepcopy__(self, memo):
        return Identifier(self.name)

    def to_node(self):
        return


class Self(Identifier):

    """The `self` identifier."""

    def __init__(self, *args, **kwargs):
        super(Self, self).__init__("self", *args, **kwargs)


class ComplexIdentifier(Expression):

    """A complex identifier."""


class Dot(ComplexIdentifier):

    """Access to a part of a module instance."""

    def __init__(self, instance, element, *args, **kwargs):
        super(Dot, self).__init__(*args, **kwargs)
        self.instance = instance
        self.element = element

    def __str__(self):
        string = str(self.instance) + "." + str(self.element)
        return super(Dot, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.instance._equals(other.instance) and
                    self.element._equals(other.element))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Dot") + 23 ** 2 * hash(self.instance) +
                23 ** 3 * hash(self.element))

    def __deepcopy__(self, memo):
        return Dot(deepcopy(self.instance, memo),
                   deepcopy(self.element, memo))


class ArrayAccess(ComplexIdentifier):

    """Access to an index of an array."""

    def __init__(self, array, index, *args, **kwargs):
        super(ArrayAccess, self).__init__(*args, **kwargs)
        self.array = array
        self.index = index

    def __str__(self):
        string = str(self.array) + "[" + str(self.index) + "]"
        return super(ArrayAccess, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.array._equals(other.array) and
                    self.index._equals(other.index))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("ArrayAccess") + 23 ** 2 * hash(self.array) +
                23 ** 3 * hash(self.index))

    def __deepcopy__(self, memo):
        return ArrayAccess(deepcopy(self.array, memo),
                           deepcopy(self.index, memo))


class Constant(Expression):

    """A generic constant."""


class BooleanConst(Constant):

    """A boolean constant."""

    def __init__(self, value, *args, **kwargs):
        super(BooleanConst, self).__init__(*args, **kwargs)
        self.value = value

    def __str__(self):
        string = self.value
        return super(BooleanConst, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.value == other.value
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("BooleanConst") + 23 ** 2 * hash(self.value)

    def __deepcopy__(self, memo):
        return BooleanConst(deepcopy(self.value, memo))


class Trueexp(BooleanConst):

    """The TRUE constant."""

    def __init__(self, *args, **kwargs):
        super(Trueexp, self).__init__("TRUE", *args, **kwargs)

    def __deepcopy__(self, memo):
        return Trueexp()


class Falseexp(BooleanConst):

    """The FALSE constant."""

    def __init__(self, *args, **kwargs):
        super(Falseexp, self).__init__("FALSE", *args, **kwargs)

    def __deepcopy__(self, memo):
        return Falseexp()


class NumberWord(Constant):

    """A word constant."""

    def __init__(self, value, *args, **kwargs):
        super(NumberWord, self).__init__(*args, **kwargs)
        self.value = value

    def __str__(self):
        string = str(self.value)
        return super(NumberWord, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.value == other.value
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Word") + 23 ** 2 * hash(self.value)

    def __deepcopy__(self, memo):
        return NumberWord(deepcopy(self.value, memo))


class RangeConst(Constant):

    """A range of integers."""

    def __init__(self, start, stop, *args, **kwargs):
        super(RangeConst, self).__init__(*args, **kwargs)
        self.start = start
        self.stop = stop

    def __str__(self):
        string = str(self.start) + " .. " + str(self.stop)
        return super(RangeConst, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.start == other.start and self.stop == other.stop
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("RangeConst")
                + 23 ** 2 * hash(self.start) + 23 ** 3 * hash(self.stop))

    def __deepcopy__(self, memo):
        return RangeConst(deepcopy(self.start, memo),
                          deepcopy(self.stop, memo))


class Function(Expression):

    """A generic function."""


class Conversion(Function):

    """Converting an expression into a specific type."""

    def __init__(self, target_type, value, *args, **kwargs):
        super(Conversion, self).__init__(*args, **kwargs)
        self.target_type = target_type
        self.value = value

    def __str__(self):
        string = str(self.target_type) + "(" + str(self.value) + ")"
        return super(Conversion, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.target_type == other.target_type and
                    self.value._equals(other.value))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Conversion")
                + 23 ** 2 * hash(self.target_type)
                + 23 ** 3 * hash(self.value))

    def __deepcopy__(self, memo):
        return Conversion(deepcopy(self.target_type, memo),
                          deepcopy(self.value, memo))


class WordFunction(Function):

    """A function applied on a word."""

    def __init__(self, function, value, size, *args, **kwargs):
        super(WordFunction, self).__init__(*args, **kwargs)
        self.function = function
        self.value = value
        self.size = size

    def __str__(self):
        string = (str(self.function) +
                  "(" + str(self.value) +
                  ", " +
                  str(self.size) +
                  ")")
        return super(WordFunction, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.function == other.function and
                    self.value._equals(other.value) and
                    self.size._equals(other.size))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("WordFunction")
                + 23 ** 2 * hash(self.function)
                + 23 ** 3 * hash(self.value)
                + 23 ** 4 * hash(self.size))

    def __deepcopy__(self, memo):
        return WordFunction(deepcopy(self.function, memo),
                            deepcopy(self.value, memo),
                            deepcopy(self.size, memo))


class Count(Function):

    """A counting function."""

    def __init__(self, values, *args, **kwargs):
        super(Count, self).__init__(*args, **kwargs)
        self.values = values

    def __str__(self):
        string = ("count((" +
                  "), (".join(str(value) for value in self.values) +
                  "))")
        return super(Count, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            if len(self.values) != len(other.values):
                return False
            for sval, oval in zip(self.values, other.values):
                if not sval._equals(oval):
                    return False
            return True
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Count") + 23 ** 2 * hash(self.values)

    def __deepcopy__(self, memo):
        return Count(deepcopy(self.values, memo))


class Next(Expression):

    """A next expression."""

    def __init__(self, value, *args, **kwargs):
        super(Next, self).__init__(*args, **kwargs)
        self.value = value

    def __str__(self):
        string = "next(" + str(self.value) + ")"
        return super(Next, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.value._equals(other.value)
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Next") + 23 ** 2 * hash(self.value)

    def __deepcopy__(self, memo):
        return Next(deepcopy(self.value, memo))


class Smallinit(Expression):

    """An init() expression."""

    def __init__(self, value, *args, **kwargs):
        super(Smallinit, self).__init__(*args, **kwargs)
        self.value = value

    def __str__(self):
        string = "init(" + str(self.value) + ")"
        return super(Smallinit, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.value._equals(other.value)
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Smallinit") + 23 ** 2 * hash(self.value)

    def __deepcopy__(self, memo):
        return Smallinit(deepcopy(self.value, memo))


class Case(Expression):

    """A case expression."""

    def __init__(self, values, *args, **kwargs):
        super(Case, self).__init__(*args, **kwargs)
        self.values = values

    def __str__(self):
        string = ["\ncase"]
        for cond, body in OrderedDict(self.values).items():
            comments = []
            if hasattr(body, "comments") and body.comments:
                comments = body.comments
                body.comments = []
            
            current = str(cond) + ": " + str(body) + ";"
            string.append(_indent(
                          _add_comments_to_string(current, comments).strip()))
            
            if hasattr(body, "comments"):
                body.comments = comments
        string.append("esac")
        string = "\n".join(string)
        return super(Case, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.values == other.values
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Case") + 23 ** 2 * hash(self.values)

    def __deepcopy__(self, memo):
        return Case(deepcopy(self.values, memo))


class Subscript(Expression):

    """Array subscript."""

    def __init__(self, array, index, *args, **kwargs):
        super(Subscript, self).__init__(*args, **kwargs)
        self.array = array
        self.index = index

    def __str__(self):
        string = str(self.array) + "[" + str(self.index) + "]"
        return super(Subscript, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.array._equals(other.array) and
                    self.index == other.index)
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Subscript") +
                23 ** 2 * hash(self.array) +
                23 ** 3 * hash(self.index))

    def __deepcopy__(self, memo):
        return Subscript(deepcopy(self.array, memo),
                         deepcopy(self.index, memo))


class BitSelection(Expression):

    """Word bit selection."""

    def __init__(self, word, start, stop, *args, **kwargs):
        super(BitSelection, self).__init__(*args, **kwargs)
        self.word = word
        self.start = start
        self.stop = stop

    def __str__(self):
        string = (str(self.word) +
                 "[" +
                 str(self.start) +
                 ":" +
                 str(self.stop) +
                 "]")
        return super(BitSelection, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.word._equals(other.word) and
                    self.start._equals(other.start) and
                    self.stop._equals(other.stop))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("BitSelection")
                + 23 ** 2 * hash(self.word)
                + 23 ** 3 * hash(self.start)
                + 23 ** 4 * hash(self.stop))

    def __deepcopy__(self, memo):
        return BitSelection(deepcopy(self.word, memo),
                            deepcopy(self.start, memo),
                            deepcopy(self.stop, memo))


class Set(Expression):

    """A set."""

    def __init__(self, elements, *args, **kwargs):
        super(Set, self).__init__(*args, **kwargs)
        self.elements = elements

    def __str__(self):
        string = ("{" +
                  ", ".join(str(element) for element in self.elements) +
                  "}")
        return super(Set, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return frozenset(self.elements) == frozenset(other.elements)
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Set") + 23 ** 2 * hash(frozenset(self.elements))

    def __deepcopy__(self, memo):
        return Set(deepcopy(self.elements, memo))


class Operator(Expression):

    """An operator."""

    def _enclose(self, expression):
        """
        Return the string representation of expression,
        enclosed in parentheses if needed.
        """
        if (isinstance(expression, Operator)
                and expression._precedence > self._precedence):
            return "(" + str(expression) + ")"
        return str(expression)


class Not(Operator):

    """A negated (`-`) expression."""

    _precedence = 1

    def __init__(self, value, *args, **kwargs):
        super(Not, self).__init__(*args, **kwargs)
        self.value = value

    def __str__(self):
        string = "! " + self._enclose(self.value)
        return super(Not, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.value._equals(other.value)
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Not") + 23 ** 2 * hash(self.value)

    def __deepcopy__(self, memo):
        return Not(deepcopy(self.value, memo))


class Concat(Operator):

    """A concatenation (`::`) of expressions."""

    _precedence = 2

    def __init__(self, left, right, *args, **kwargs):
        super(Concat, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " :: " + self._enclose(self.right)
        return super(Concat, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Concat") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Concat(deepcopy(self.left, memo),
                      deepcopy(self.right, memo))


class Minus(Operator):

    """Minus (`-`) expression."""

    _precedence = 3

    def __init__(self, value, *args, **kwargs):
        super(Minus, self).__init__(*args, **kwargs)
        self.value = value

    def __str__(self):
        string = "- " + self._enclose(self.value)
        return super(Minus, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return self.value._equals(other.value)
        else:
            return False

    def __hash__(self):
        return 17 + 23 * hash("Minus") + 23 ** 2 * hash(self.value)

    def __deepcopy__(self, memo):
        return Minus(deepcopy(self.value, memo))


class Mult(Operator):

    """A multiplication (`*`) of expressions."""

    _precedence = 4

    def __init__(self, left, right, *args, **kwargs):
        super(Mult, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " * " + self._enclose(self.right)
        return super(Mult, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Mult") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __deepcopy__(self, memo):
        return Mult(deepcopy(self.left, memo),
                    deepcopy(self.right, memo))


class Div(Operator):

    """A division (`/`) of expressions."""

    _precedence = 4

    def __init__(self, left, right, *args, **kwargs):
        super(Div, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " / " + self._enclose(self.right)
        return super(Div, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Div") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Div(deepcopy(self.left, memo),
                   deepcopy(self.right, memo))


class Mod(Operator):

    """A modulo (`%`) of expressions."""

    _precedence = 4

    def __init__(self, left, right, *args, **kwargs):
        super(Mod, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " mod " + self._enclose(self.right)
        return super(Mod, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Mod") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Mod(deepcopy(self.left, memo),
                   deepcopy(self.right, memo))


class Add(Operator):

    """An addition (`+`) of expressions."""

    _precedence = 5

    def __init__(self, left, right, *args, **kwargs):
        super(Add, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " + " + self._enclose(self.right)
        return super(Add, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Add") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __deepcopy__(self, memo):
        return Add(deepcopy(self.left, memo),
                   deepcopy(self.right, memo))


class Sub(Operator):

    """A subtraction (`-`) of expressions."""

    _precedence = 5

    def __init__(self, left, right, *args, **kwargs):
        super(Sub, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " - " + self._enclose(self.right)
        return super(Sub, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Sub") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Sub(deepcopy(self.left, memo),
                   deepcopy(self.right, memo))


class LShift(Operator):

    """A left shift (`<<`) of expressions."""

    _precedence = 6

    def __init__(self, left, right, *args, **kwargs):
        super(LShift, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " << " + self._enclose(self.right)
        return super(LShift, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("LShift") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return LShift(deepcopy(self.left, memo),
                      deepcopy(self.right, memo))


class RShift(Operator):

    """A right shift (`>>`) of expressions."""

    _precedence = 6

    def __init__(self, left, right, *args, **kwargs):
        super(RShift, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " >> " + self._enclose(self.right)
        return super(RShift, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("RShift") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return RShift(deepcopy(self.left, memo),
                      deepcopy(self.right, memo))


class Union(Operator):

    """A union (`union`) of expressions."""

    _precedence = 7

    def __init__(self, left, right, *args, **kwargs):
        super(Union, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = (self._enclose(self.left) +
                  " union " +
                  self._enclose(self.right))
        return super(Union, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Union") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __deepcopy__(self, memo):
        return Union(deepcopy(self.left, memo),
                     deepcopy(self.right, memo))


class In(Operator):

    """The `in` expression."""

    _precedence = 8

    def __init__(self, left, right, *args, **kwargs):
        super(In, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " in " + self._enclose(self.right)
        return super(In, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("In") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return In(deepcopy(self.left, memo),
                  deepcopy(self.right, memo))


class Equal(Operator):

    """The `=` expression."""

    _precedence = 9

    def __init__(self, left, right, *args, **kwargs):
        super(Equal, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " = " + self._enclose(self.right)
        return super(Equal, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Equal") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __bool__(self):
        if (isinstance(self.left, Expression) and
                isinstance(self.right, Expression)):
            return self.left._equals(self.right)
        else:
            return self.left == self.right
    
    def __nonzero__(self):
        return self.__bool__()

    def __deepcopy__(self, memo):
        return Equal(deepcopy(self.left, memo),
                     deepcopy(self.right, memo))


class NotEqual(Operator):

    """The `!=` expression."""

    _precedence = 9

    def __init__(self, left, right, *args, **kwargs):
        super(NotEqual, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " != " + self._enclose(self.right)
        return super(NotEqual, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((not self.left._equals(other.left) or
                     not self.right._equals(other.right)) and
                    (not self.left._equals(other.right) or
                     not self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("NotEqual") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __bool__(self):
        if (isinstance(self.left, Expression) and
                isinstance(self.right, Expression)):
            return not self.left._equals(self.right)
        else:
            return self.left != self.right
    
    def __nonzero__(self):
        return self.__bool__()

    def __deepcopy__(self, memo):
        return NotEqual(deepcopy(self.left, memo),
                        deepcopy(self.right, memo))


class Lt(Operator):

    """The `<` expression."""

    _precedence = 9

    def __init__(self, left, right, *args, **kwargs):
        super(Lt, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " < " + self._enclose(self.right)
        return super(Lt, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Lt") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Lt(deepcopy(self.left, memo),
                  deepcopy(self.right, memo))


class Gt(Operator):

    """The `>` expression."""

    _precedence = 9

    def __init__(self, left, right, *args, **kwargs):
        super(Gt, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " > " + self._enclose(self.right)
        return super(Gt, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Gt") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Gt(deepcopy(self.left, memo),
                  deepcopy(self.right, memo))


class Le(Operator):

    """The `<=` expression."""

    _precedence = 9

    def __init__(self, left, right, *args, **kwargs):
        super(Le, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " <= " + self._enclose(self.right)
        return super(Le, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Le") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Le(deepcopy(self.left, memo),
                  deepcopy(self.right, memo))


class Ge(Operator):

    """The `>=` expression."""

    _precedence = 9

    def __init__(self, left, right, *args, **kwargs):
        super(Ge, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " >= " + self._enclose(self.right)
        return super(Ge, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Ge") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Ge(deepcopy(self.left, memo),
                  deepcopy(self.right, memo))


class And(Operator):

    """The `&` expression."""

    _precedence = 10

    def __init__(self, left, right, *args, **kwargs):
        super(And, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " & " + self._enclose(self.right)
        return super(And, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("And") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __deepcopy__(self, memo):
        return And(deepcopy(self.left, memo),
                   deepcopy(self.right, memo))


class Or(Operator):

    """The `|` expression."""

    _precedence = 11

    def __init__(self, left, right, *args, **kwargs):
        super(Or, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " | " + self._enclose(self.right)
        return super(Or, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Or") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __deepcopy__(self, memo):
        return Or(deepcopy(self.left, memo),
                  deepcopy(self.right, memo))


class Xor(Operator):

    """The `xor` expression."""

    _precedence = 11

    def __init__(self, left, right, *args, **kwargs):
        super(Xor, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " xor " + self._enclose(self.right)
        return super(Xor, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Xor") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __deepcopy__(self, memo):
        return Xor(deepcopy(self.left, memo),
                   deepcopy(self.right, memo))


class Xnor(Operator):

    """The `xnor` expression."""

    _precedence = 11

    def __init__(self, left, right, *args, **kwargs):
        super(Xnor, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = (self._enclose(self.left) +
                  " xnor " +
                  self._enclose(self.right))
        return super(Xnor, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Xnor") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __deepcopy__(self, memo):
        return Xnor(deepcopy(self.left, memo),
                    deepcopy(self.right, memo))


class Ite(Operator):

    """The `? :` expression."""

    _precedence = 12

    def __init__(self, condition, left, right, *args, **kwargs):
        super(Ite, self).__init__(*args, **kwargs)
        self.condition = condition
        self.left = left
        self.right = right

    def __str__(self):
        string = (self._enclose(self.condition) +
                  " ? " +
                  self._enclose(self.left) +
                  " : " +
                  self._enclose(self.right))
        return super(Ite, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.condition._equals(other.condition) and
                    self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Ite") + 23 ** 2 * hash(self.condition)
                + 23 ** 3 * hash(self.left) + 23 ** 4 * hash(self.right))

    def __deepcopy__(self, memo):
        return Ite(deepcopy(self.condition, memo),
                   deepcopy(self.left, memo),
                   deepcopy(self.right, memo))


class Iff(Operator):

    """The `<->` expression."""

    _precedence = 13

    def __init__(self, left, right, *args, **kwargs):
        super(Iff, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " <-> " + self._enclose(self.right)
        return super(Iff, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return ((self.left._equals(other.left) and
                     self.right._equals(other.right)) or
                    (self.left._equals(other.right) and
                     self.right._equals(other.left)))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Iff") + 23 ** 2 * hash(self.left)
                + 23 ** 2 * hash(self.right))

    def __deepcopy__(self, memo):
        return Iff(deepcopy(self.left, memo),
                   deepcopy(self.right, memo))


class Implies(Operator):

    """The `->` expression."""

    _precedence = 14

    def __init__(self, left, right, *args, **kwargs):
        super(Implies, self).__init__(*args, **kwargs)
        self.left = left
        self.right = right

    def __str__(self):
        string = self._enclose(self.left) + " -> " + self._enclose(self.right)
        return super(Implies, self).__str__(string=string)

    def _equals(self, other):
        """Return whether `self` is equals to `other`."""
        if isinstance(self, type(other)):
            return (self.left._equals(other.left) and
                    self.right._equals(other.right))
        else:
            return False

    def __hash__(self):
        return (17 + 23 * hash("Implies") + 23 ** 2 * hash(self.left)
                + 23 ** 3 * hash(self.right))

    def __deepcopy__(self, memo):
        return Implies(deepcopy(self.left, memo),
                       deepcopy(self.right, memo))


class ArrayExpr(Element):

    """An array define expression."""

    def __init__(self, array, *args, **kwargs):
        super(ArrayExpr, self).__init__(*args, **kwargs)
        self.array = array

    def __str__(self):
        string = str(self.array)
        return super(ArrayExpr, self).__str__(string=string)
    
    def __hash__(self):
        return 17 + 23 * hash("ArrayExpr") + 23 ** 2 * hash(self.array)
    
    def __deepcopy__(sef, memo):
        return ArrayExpr(deepcopy(self.array, memo))


# -----------------------------------------------------------------------------
# ----- TYPES
# -----------------------------------------------------------------------------

class Type(Element):

    """A generic type specifier."""

    def __deepcopy__(self, memo):
        raise NotImplementedError("Should be implemented by subclasses.")


class SimpleType(Type):

    """A simple type: boolean, word, enum, range, array."""


class Boolean(SimpleType):

    """A boolean type."""

    def __str__(self):
        string = "boolean"
        return super(Boolean, self).__str__(string=string)

    def __deepcopy__(self, memo):
        return Boolean()


class Word(SimpleType):

    """A word type."""

    def __init__(self, size, sign=None, *args, **kwargs):
        super(Word, self).__init__(*args, **kwargs)
        self.size = size
        self.sign = sign

    def __str__(self):
        string = ((self.sign + " " if self.sign else "") +
                  "word" +"[" + str(self.size) + "]")
        return super(Word, self).__str__(string=string)

    def __deepcopy__(self, memo):
        return Word(deepcopy(self.size, memo),
                    deepcopy(self.sign, memo))


class Scalar(SimpleType):

    """An enumeration type."""

    def __init__(self, values, *args, **kwargs):
        super(Scalar, self).__init__(*args, **kwargs)
        self.values = values

    def __str__(self):
        string = "{" + ", ".join(str(value) for value in self.values) + "}"
        return super(Scalar, self).__str__(string=string)

    def __deepcopy__(self, memo):
        return Scalar(deepcopy(self.values, memo))


class Range(SimpleType):

    """A range type."""

    def __init__(self, start, stop, *args, **kwargs):
        super(Range, self).__init__(*args, **kwargs)
        self.start = start
        self.stop = stop

    def __str__(self):
        string = str(self.start) + " .. " + str(self.stop)
        return super(Range, self).__str__(string=string)

    def __deepcopy__(self, memo):
        return Range(deepcopy(self.start, memo),
                     deepcopy(self.stop, memo))


class Array(SimpleType):

    """An array type."""

    def __init__(self, start, stop, elementtype, *args, **kwargs):
        super(Array, self).__init__(*args, **kwargs)
        self.start = start
        self.stop = stop
        self.elementtype = elementtype

    def __str__(self):
        string = ("array " +
                  str(self.start) +
                  " .. " +
                  str(self.stop) +
                  " of " +
                  str(self.elementtype))
        return super(Array, self).__str__(string=string)

    def __deepcopy__(self, memo):
        return Array(deepcopy(self.start, memo),
                     deepcopy(self.stop, memo),
                     deepcopy(self.elementtype, memo))


class Modtype(Type):

    """A module instantiation."""

    def __init__(self, modulename, arguments, process=False, *args, **kwargs):
        super(Modtype, self).__init__(*args, **kwargs)
        self.process = process
        self.modulename = modulename
        self.args = arguments

    def __str__(self):
        string = (("process " if self.process else "") +
                  str(self.modulename) +
                  "(" +
                  ", ".join(str(arg) for arg in self.args) +
                  ")")
        return super(Modtype, self).__str__(string=string)

    def __deepcopy__(self, memo):
        return Modtype(deepcopy(self.modulename, memo),
                       deepcopy(self.args, memo),
                       process=deepcopy(self.process, memo))


# -----------------------------------------------------------------------------
# ----- SECTIONS
# -----------------------------------------------------------------------------

class Section(Element):

    """A section of a module."""

    def __init__(self, name, body, *args, **kwargs):
        super(Section, self).__init__(*args, **kwargs)
        self.name = name
        self.body = body


class MappingSection(Section):

    """Section based on a mapping of identifier and others."""

    def __init__(self,
                 name,
                 mapping,
                 internal_separator=": ",
                 external_separator=";",
                 indentation=" " * 4,
                 *args,
                 **kwargs):
        """
        :param name: the name of the section.
        :param mapping: the mapping of identifiers to their corresponding
                        expression.
        :param internal_separator: the separator of identifiers and expressions
                                   for printing the section.
        :param external_separator: the separator between pairs of the section.
        :param indentation: the indentation for the printed expressions.
        """
        super(MappingSection, self).__init__(name, mapping, *args, **kwargs)
        self.internal_separator = internal_separator
        self.external_separator = external_separator
        self.indentation = indentation

    def update_body(self, new_values):
        """
        Update the body of this section with `new_values`.

        :param new_values: the new values with which to update the current
                           body.

        """
        self.body.update(new_values)

    def __str__(self):
        rep = [self.name]
        for identifier, expr in self.body.items():
            header = (self.indentation +
                      str(identifier) +
                      self.internal_separator)
            
            comments = []
            if hasattr(expr, "comments") and expr.comments:
                comments = expr.comments
                expr.comments = []
            
            strexpr = _add_comments_to_string(str(expr) +
                                              self.external_separator,
                                              comments).strip()
            body = strexpr.split("\n")
            if len(body) > 1:
                rep.append(header +
                           "\n" +
                           _indent("\n".join(body), self.indentation * 2))
            else:
                rep.append(header + strexpr)
            
            if hasattr(expr, "comments"):
                expr.comments = comments
        string = "\n".join(rep)
        return super(MappingSection, self).__str__(string=string)


class Variables(MappingSection):

    """Declaring variables."""

    def __init__(self, variables, *args, **kwargs):
        super(Variables, self).__init__("VAR", variables, *args, **kwargs)


class InputVariables(MappingSection):

    """Declaring input variables."""

    def __init__(self, ivariables, *args, **kwargs):
        super(InputVariables, self).__init__("IVAR",
                                             ivariables,
                                             *args,
                                             **kwargs)


class FrozenVariables(MappingSection):

    """Declaring frozen variables."""

    def __init__(self, fvariables, *args, **kwargs):
        super(FrozenVariables, self).__init__("FROZENVAR",
                                              fvariables,
                                              *args,
                                              **kwargs)


class Defines(MappingSection):

    """Declaring defines."""

    def __init__(self, defines, *args, **kwargs):
        super(Defines, self).__init__("DEFINE",
                                      defines,
                                      internal_separator=" := ",
                                      *args,
                                      **kwargs)


class Assigns(MappingSection):

    """Declaring assigns."""

    def __init__(self, assigns, *args, **kwargs):
        super(Assigns, self).__init__("ASSIGN",
                                      assigns,
                                      internal_separator=" := ",
                                      *args,
                                      **kwargs)


class ListingSection(Section):

    """A section made of a list of elements."""

    def __init__(self,
                 name,
                 listing,
                 separator="\n",
                 indentation=" " * 4,
                 *args,
                 **kwargs):
        """
        :param name: the name of the section.
        :param listing: a list of expressions.
        :param separator: the separator of expressions for printing the
                          section.
        :param indentation: the indentation for the printed expressions.
        """
        super(ListingSection, self).__init__(name, listing, *args, **kwargs)
        self.separator = separator
        self.indentation = indentation

    def update_body(self, otherbody):
        """
        Update the body of this section with `new_values`.

        :param new_values: the new values with which to update the current
                           body.
        """
        self.body += otherbody

    def __str__(self):
        rep = [self.name]
        for element in self.body:
            body = str(element).split("\n")
            if len(body) > 1:
                rep.append(self.indentation + "\n" +
                           _indent("\n".join(body),
                                   self.indentation * 2))
            else:
                rep.append(self.indentation + str(element))
        string = "\n".join(rep)
        return super(ListingSection, self).__str__(string=string)


class Constants(ListingSection):

    """Declaring constants."""

    def __init__(self, constants, *args, **kwargs):
        super(Constants, self).__init__("CONSTANTS",
                                        constants,
                                        separator=", ",
                                        indentation="",
                                        *args,
                                        **kwargs)


class Trans(ListingSection):

    """A TRANS section."""

    def __init__(self, body, *args, **kwargs):
        super(Trans, self).__init__("TRANS",
                                    body,
                                    separator="\nTRANS\n",
                                    *args,
                                    **kwargs)


class Init(ListingSection):

    """An INIT section."""

    def __init__(self, body, *args, **kwargs):
        super(Init, self).__init__("INIT",
                                   body,
                                   separator="\nINIT\n",
                                   *args,
                                   **kwargs)


class Invar(ListingSection):

    """An INVAR section."""

    def __init__(self, body, *args, **kwargs):
        super(Invar, self).__init__("INVAR",
                                    body,
                                    separator="\nINVAR\n",
                                    *args,
                                    **kwargs)


class Fairness(ListingSection):

    """A FAIRNESS section."""

    def __init__(self, body, *args, **kwargs):
        super(Fairness, self).__init__("FAIRNESS",
                                       body,
                                       separator="\nFAIRNESS\n",
                                       *args,
                                       **kwargs)


class Justice(ListingSection):

    """A Justice section."""

    def __init__(self, body, *args, **kwargs):
        super(Justice, self).__init__("JUSTICE",
                                      body,
                                      separator="\nJUSTICE\n",
                                      *args,
                                      **kwargs)


class Compassion(ListingSection):

    """A COMPASSION section."""

    def __init__(self, body, *args, **kwargs):
        super(Compassion, self).__init__("COMPASSION",
                                         body,
                                         separator="\nCOMPASSION\n",
                                         *args,
                                         **kwargs)


# -----------------------------------------------------------------------------
# ----- DECLARATIONS
# -----------------------------------------------------------------------------

class Declaration(Identifier):

    """
    A Declaration behaves like an identifier, except that it knows which type
    it belongs to. Furthermore, it does not know its name for sure, and
    cannot be printed without giving it a name.

    """

    def __init__(self, type_, section, name=None, *args, **kwargs):
        super(Declaration, self).__init__(name, *args, **kwargs)
        self._name = name
        self.type = type_
        self.section = section

    @property
    def name(self):
        """The name of the declared identifier."""
        if not self._name:
            raise AttributeError("Unknown declaration name.")
        return self._name

    @name.setter
    def name(self, name):
        """Update the name of the declared identifier."""
        self._name = name

    def _equals(self, other):
        if isinstance(other, Identifier):
            return self.name == other.name
        else:
            return False


class Var(Declaration):

    """A declared VAR."""

    def __init__(self, type_, name=None, *args, **kwargs):
        super(Var, self).__init__(type_, "VAR", name=name, *args, **kwargs)

    def __deepcopy__(self, memo):
        return Var(deepcopy(self.type, memo), name=deepcopy(self.name, memo))


class IVar(Declaration):

    """A declared IVAR."""

    def __init__(self, type_, name=None, *args, **kwargs):
        super(IVar, self).__init__(type_, "IVAR", name=name, *args, **kwargs)

    def __deepcopy__(self, memo):
        return IVar(deepcopy(self.type, memo), name=deepcopy(self.name, memo))


class FVar(Declaration):

    """A declared FROZENVAR."""

    def __init__(self, type_, name=None, *args, **kwargs):
        super(FVar, self).__init__(type_,
                                   "FROZENVAR",
                                   name=name,
                                   *args,
                                   **kwargs)

    def __deepcopy__(self, memo):
        return FVar(deepcopy(self.type, memo), name=deepcopy(self.name, memo))


class Def(Declaration):

    """A declared DEFINE."""

    def __init__(self, type_, name=None, *args, **kwargs):
        super(Def, self).__init__(type_, "DEFINE", name=name, *args, **kwargs)

    def __deepcopy__(self, memo):
        return Def(deepcopy(self.type, memo), name=deepcopy(self.name, memo))


# -----------------------------------------------------------------------------
# ----- MODULES
# -----------------------------------------------------------------------------

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
    # * the internal and external separators for mappings,
    #   or the separator for enumerations,
    #   or nothing for bodies,
    #   used for printing the section body
    _sections = {"VAR": ("mapping", (": ", ";")),
                 "IVAR": ("mapping", (": ", ";")),
                 "FROZENVAR": ("mapping", (": ", ";")),
                 "DEFINE": ("mapping", (" := ", ";")),
                 "ASSIGN": ("mapping", (" := ", ";")),
                 "CONSTANTS": ("enumeration", ", "),
                 "TRANS": ("bodies",),
                 "INIT": ("bodies",),
                 "INVAR": ("bodies",),
                 "FAIRNESS": ("bodies",),
                 "JUSTICE": ("bodies",),
                 "COMPASSION": ("bodies",)}

    @classmethod
    def __prepare__(mcs, name, bases, **keywords):
        return OrderedDict()

    def __new__(mcs, name, bases, namespace, **keywords):
        newnamespace = OrderedDict()
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
                if not decl._name:
                    decl.name = member
                if decl.section not in newnamespace:
                    newnamespace[decl.section] = OrderedDict()
                newnamespace[decl.section][decl] = decl.type
                # Keep declarations in module
                newnamespace[member] = namespace[member]
            # Keep intact the other members
            else:
                newnamespace[member] = namespace[member]

        # Add NAME, COMMENT and ARGS if missing
        if "NAME" not in newnamespace:
            newnamespace["NAME"] = name
        if "COMMENT" not in newnamespace:
            newnamespace["COMMENT"] = ""
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
                setattr(cls, name, OrderedDict())
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

        from .parser import (parseAllString, identifier)
        newargs = []
        for arg in args:
            if type(arg) is str:
                arg = parseAllString(identifier, arg)
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

        from .parser import (parseAllString,
                             _var_section_body, identifier, type_identifier,
                             _ivar_section_body, _simple_type_specifier,
                             _frozenvar_section_body, _define_section_body,
                             _assign_constraint_body, _assign_identifier,
                             simple_expression, _constants_section_body,
                             _trans_constraint_body, _init_constraint_body,
                             _invar_constraint_body, _fairness_constraint_body,
                             _justice_constraint_body,
                             _compassion_constraint_body)

        # The list of module sections that are considered
        # each key is the section name
        # each value is a tuple giving:
        # * the type of expected value (mapping, enumeration, bodies)
        # * a list parsers: three for mappings (whole section, key and value),
        #   one for enumerations (whole section) and one for bodies (whole
        #   section)
        _sections_parsers = {
            "VAR": ("mapping",
                    _var_section_body,
                    identifier,
                    type_identifier),
            "IVAR": ("mapping",
                     _ivar_section_body,
                     identifier,
                     _simple_type_specifier),
            "FROZENVAR": ("mapping",
                          _frozenvar_section_body,
                          identifier,
                          _simple_type_specifier),
            "DEFINE": ("mapping",
                       _define_section_body,
                       identifier,
                       _simple_type_specifier),
            "ASSIGN": ("mapping",
                       _assign_constraint_body,
                       _assign_identifier,
                       simple_expression),
            "CONSTANTS": ("enumeration",
                          _constants_section_body),
            "TRANS": ("bodies", _trans_constraint_body),
            "INIT": ("bodies", _init_constraint_body),
            "INVAR": ("bodies", _invar_constraint_body),
            "FAIRNESS": ("bodies", _fairness_constraint_body),
            "JUSTICE": ("bodies", _justice_constraint_body),
            "COMPASSION": ("bodies", _compassion_constraint_body)}

        if section not in mcs._sections:
            raise NuSMVModuleError("Unknown section: {}.".format(section))

        if mcs._sections[section][0] == "mapping":
            section_parser, key_parser, value_parser = (_sections_parsers
                                                        [section][-3:])
            if isinstance(body, Mapping):
                res = OrderedDict()
                for key, value in body.items():
                    if isinstance(key, str):
                        key = parseAllString(key_parser, key)
                    if isinstance(value, str):
                        value = parseAllString(value_parser, value)
                    res[key] = value

                return res

            else:
                if isinstance(body, str):
                    body = [body]

                res = OrderedDict()
                for line in body:
                    if isinstance(line, str):
                        line = parseAllString(section_parser, line)
                        update(res, line)

                    else:
                        # line is an enumeration
                        key, value = line[0:2]
                        if isinstance(key, str):
                            key = parseAllString(key_parser, key)
                        if isinstance(value, str):
                            value = parseAllString(value_parser, value)
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
                    expr = parseAllString(parser, expr)

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
            internal_separator, external_separator = cls._sections[section][1]
            rep = [section]
            for identifier, expr in body.items():
                header = (indentation +
                          str(identifier) +
                          internal_separator)

                comments = []
                if hasattr(expr, "comments") and expr.comments:
                    comments = expr.comments
                    expr.comments = []

                strexpr = _add_comments_to_string(str(expr) +
                                                  external_separator,
                                                  comments).strip()
                body = strexpr.split("\n")
                if len(body) > 1:
                    rep.append(header +
                               "\n" +
                               _indent("\n".join(body), indentation * 2))
                else:
                    rep.append(header + strexpr)

                if hasattr(expr, "comments"):
                    expr.comments = comments
            return _indent("\n".join(rep), indentation)

        elif cls._sections[section][0] == "enumeration":
            # body is an enumeration
            # use separator given in _sections
            separator = cls._sections[section][1]
            rep = [indentation + section]
            for value in body:
                if len(str(value).split("\n")) > 1:
                    rep.append(_indent(str(value),
                                       indentation * 2))
                else:
                    rep.append(str(value))
            return indentation + section + "\n" + separator.join(rep)

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

        representation = []

        # Add comment if it exists
        if cls.COMMENT:
            representation.append("-- " + cls.COMMENT)

        representation.append("MODULE " + str(cls.NAME) + args)
        for section in [member for member in cls.members
                        if member in cls._sections]:
            representation.append(cls._section_str(section,
                                                   cls.__dict__[section],
                                                   indentation))
        string = "\n".join(representation)
        return string

    def copy(cls):
        """
        Return a deep copy of this module.

        Only the members of this module present in `cls.members` are copied.
        If these members accept to be `deepcopy`ed, this method is used to
        copy the member, otherwise the member is not copied and is passed
        to the created module as it is.

        """
        newnamespace = OrderedDict()
        for member in cls.members:
            newnamespace[member] = deepcopy(getattr(cls, member))

        return ModuleMetaClass(cls.NAME, cls.__bases__, newnamespace)


def _with_metaclass(meta, *bases):
    """
    Create a base class with a metaclass.
    
    Copyright (c) 2010-2014 Benjamin Peterson
    This function comes from the six python package:
    https://pypi.python.org/pypi/six
    """
    # This requires a bit of explanation: the basic idea is to make a dummy
    # metaclass for one level of class instantiation that replaces itself with
    # the actual metaclass.
    class metaclass(meta):
        def __new__(cls, name, this_bases, d):
            return meta(name, bases, d)
    return type.__new__(metaclass, 'temporary_class', (), {})

class Module(_with_metaclass(ModuleMetaClass, Modtype)):

    """
    A generic module.

    To create a new module, the user must subclass the :class:`Module` class
    and add class attributes with names corresponding to sections of NuSMV
    module definitions: `VAR`, `IVAR`, `FROZENVAR`, `DEFINE`, `CONSTANTS`,
    `ASSIGN`, `TRANS`, `INIT`, `INVAR`, `FAIRNESS`, `JUSTICE`, `COMPASSION`.

    In addition to these attributes, the `ARGS`, `NAME` and `COMMENT`
    attributes can be defined:
    
    * If `NAME` is defined, it overrides module name for the NuSMV module name.
    * If `ARGS` is defined, it must be a sequence object where each element's
      string representation is an argument of the module.
    * If `COMMENT` is defined, it will be used as the module's header, that is,
      it will be added as a NuSMV comment just before the module declaration.

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
            COMMENT = "Two asynchronous counters"
            NAME = "twoCounter"
            ARGS = ["run"]
            c1 = Range(0, 2)
            VAR = {"c2": "0..2"}
            INIT = [c1 == 0 & "c2 = 0"]
            TRANS = [Next(c1) == Ite("run", (c1 + 1) % 2, c1),
                     "next(c2) = run ? c2+1 mod 2 : c2"]

    defines the module ::

        -- Two asynchronous counters
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

    def __init__(self, *args, **kwargs):
        if "process" in kwargs:
            process = kwargs["process"]
        else:
            process = False
        Modtype.__init__(self,
                         self.__class__.NAME,
                         args,
                         process=process,
                         **kwargs)


def _indent(text, indentation=" " * 4):
    """
    Indent each (stripped non-empty) line of the given text with the given
    indentation. That is, for each line, if it contains non-white characters,
    the line is indented. Otherwise it is kept as it is.

    :param string text: the text to indent.
    :param string indentation: the indentation to use.
    """
    rep = []
    for line in text.split("\n"):
        if line.strip():
            rep.append(indentation + line)
        else:
            rep.append(line)
    return "\n".join(rep)