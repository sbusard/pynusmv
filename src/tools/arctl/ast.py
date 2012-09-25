"""
AST classes for ARCTL formulas representation.
"""

from collections import namedtuple

# TrueExp, FalseExp
TrueExp = namedtuple('TrueExp', [])
TrueExp.__str__ = lambda self: "True"

FalseExp = namedtuple('FalseExp', [])
FalseExp.__str__ = lambda self: "False"

# Atom
Atom = namedtuple('Atom', ['value'])
Atom.__str__ = lambda self: "'" + str(self.value) + "'"

# logical : Not, And, Or, Implies, Iff
Not = namedtuple('Not', ['child'])
Not.__str__ = lambda self: "~" + "(" + str(self.child) + ")"

And = namedtuple('And', ['left', 'right'])
And.__str__ = lambda self: "(" + str(self.left) + " & " + str(self.right) + ")"

Or = namedtuple('Or', ['left', 'right'])
Or.__str__ = lambda self: "(" + str(self.left) + " | " + str(self.right) + ")"

Implies = namedtuple('Implies', ['left', 'right'])
Implies.__str__ = (lambda self: "(" + str(self.left) + " -> " +
                                      str(self.right) + ")")

Iff = namedtuple('Iff', ['left', 'right'])
Iff.__str__ = (lambda self: "(" + str(self.left) + " <-> " +
                                  str(self.right) + ")")
                                  

# temporal : AaF, AaG, AaX, AaU, AaW, EaF, EaG, EaX, EaU, EaW
AaF = namedtuple('AaF', ['action', 'child'])
AaF.__str__ = (lambda self: "A" + "<" + str(self.action) + ">" +
                            "F " + str(self.child))
                            
AaG = namedtuple('AaG', ['action', 'child'])
AaG.__str__ = (lambda self: "A" + "<" + str(self.action) + ">" +
                            "G " + str(self.child))
                            
AaX = namedtuple('AaX', ['action', 'child'])
AaX.__str__ = (lambda self: "A" + "<" + str(self.action) + ">" +
                            "X " + str(self.child))
                            
AaU = namedtuple('AaU', ['action', 'left', 'right'])
AaU.__str__ = (lambda self: "A" + "<" + str(self.action) + ">" +
                            "[" + str(self.left) + " U " + str(self.right) +
                            "]")
                            
AaW = namedtuple('EaW', ['action', 'left', 'right'])
AaW.__str__ = (lambda self: "A" + "<" + str(self.action) + ">" +
                            "[" + str(self.left) + " W " + str(self.right) +
                            "]")

EaF = namedtuple('EaF', ['action', 'child'])
EaF.__str__ = (lambda self: "E" + "<" + str(self.action) + ">" +
                            "F " + str(self.child))
                            
EaG = namedtuple('EaG', ['action', 'child'])
EaG.__str__ = (lambda self: "E" + "<" + str(self.action) + ">" +
                            "G " + str(self.child))
                            
EaX = namedtuple('EaX', ['action', 'child'])
EaX.__str__ = (lambda self: "E" + "<" + str(self.action) + ">" +
                            "X " + str(self.child))
                            
EaU = namedtuple('EaU', ['action', 'left', 'right'])
EaU.__str__ = (lambda self: "E" + "<" + str(self.action) + ">" +
                            "[" + str(self.left) + " U " + str(self.right) +
                            "]")
                            
EaW = namedtuple('EaW', ['action', 'left', 'right'])
EaW.__str__ = (lambda self: "E" + "<" + str(self.action) + ">" +
                            "[" + str(self.left) + " W " + str(self.right) +
                            "]")