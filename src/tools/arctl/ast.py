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

# temporal : AaF, AaG, AaX, AaU, AaW, EaF, EaG, EaX, EaU, EaW
AaF = namedtuple('AaF', ['action', 'child'])
AaG = namedtuple('AaG', ['action', 'child'])
AaX = namedtuple('AaX', ['action', 'child'])
AaU = namedtuple('AaU', ['action', 'left', 'right'])
AaW = namedtuple('AaW', ['action', 'left', 'right'])
EaF = namedtuple('EaF', ['action', 'child'])
EaG = namedtuple('EaG', ['action', 'child'])
EaX = namedtuple('EaX', ['action', 'child'])
EaU = namedtuple('EaU', ['action', 'left', 'right'])
EaW = namedtuple('EaW', ['action', 'left', 'right'])