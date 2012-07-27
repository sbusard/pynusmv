"""
AST classes for ARCTL formulas representation.
"""

from collections import namedtuple

# Atom
Atom = namedtuple('Atom', ['value'])

# logical : Not, And, Or, Implies, Iff
Not = namedtuple('Not', ['child'])
And = namedtuple('And', ['left', 'right'])
Or = namedtuple('Or', ['left', 'right'])
Implies = namedtuple('Implies', ['left', 'right'])
Iff = namedtuple('Iff', ['left', 'right'])

# temporal : A, E
A = namedtuple('A', ['action', 'path'])
E = namedtuple('E', ['action', 'path'])

# path : F, G, X, U, W
F = namedtuple('F', ['child'])
G = namedtuple('G', ['child'])
X = namedtuple('X', ['child'])
U = namedtuple('U', ['left', 'right'])
W = namedtuple('W', ['left', 'right'])