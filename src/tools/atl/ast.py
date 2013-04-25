"""
AST classes for ATL formulas representation.
"""

from collections import namedtuple

class Spec:
    """A Spec is represents a ATL formula."""

# TrueExp, FalseExp, Init, Reachable
TrueExp = namedtuple('TrueExp', [])
TrueExp.__bases__ += (Spec,)
TrueExp.__str__ = lambda self: "True"

FalseExp = namedtuple('FalseExp', [])
FalseExp.__bases__ += (Spec,)
FalseExp.__str__ = lambda self: "False"

# Atom
Atom = namedtuple('Atom', ['value'])
Atom.__bases__ += (Spec,)
Atom.__str__ = lambda self: "'" + str(self.value) + "'"

# logical : Not, And, Or, Implies, Iff
Not = namedtuple('Not', ['child'])
Not.__bases__ += (Spec,)
Not.__str__ = lambda self: "~" + "(" + str(self.child) + ")"

And = namedtuple('And', ['left', 'right'])
And.__bases__ += (Spec,)
And.__str__ = lambda self: "(" + str(self.left) + " & " + str(self.right) + ")"

Or = namedtuple('Or', ['left', 'right'])
Or.__bases__ += (Spec,)
Or.__str__ = lambda self: "(" + str(self.left) + " | " + str(self.right) + ")"

Implies = namedtuple('Implies', ['left', 'right'])
Implies.__bases__ += (Spec,)
Implies.__str__ = (lambda self: "(" + str(self.left) + " -> " +
                                      str(self.right) + ")")

Iff = namedtuple('Iff', ['left', 'right'])
Iff.__bases__ += (Spec,)
Iff.__str__ = (lambda self: "(" + str(self.left) + " <-> " +
                                  str(self.right) + ")")
                                  

# strategic : <g>F, <g>G, <g>X, <g>U, <g>W, [g]F, [g]G, [g]X, [g]U, [g]W
CEF = namedtuple('CEF', ['group', 'child'])
CEF.__bases__ += (Spec,)
CEF.__str__ = (lambda self: "<" +','.join([ag.value for ag in self.group])+ ">"+
                            "F " + str(self.child))
                            
CEG = namedtuple('CEG', ['group', 'child'])
CEG.__bases__ += (Spec,)
CEG.__str__ = (lambda self: "<" +','.join([ag.value for ag in self.group])+ ">"+
                            "G " + str(self.child))
                           
CEX = namedtuple('CEX', ['group', 'child'])
CEX.__bases__ += (Spec,)
CEX.__str__ = (lambda self: "<" +','.join([ag.value for ag in self.group])+ ">"+ 
                            "X " + str(self.child))
                           
CEU = namedtuple('CEU', ['group', 'left', 'right'])
CEU.__bases__ += (Spec,)
CEU.__str__ = (lambda self: "<" +','.join([ag.value for ag in self.group])+ ">"+ 
                            "[" + str(self.left) +
                            " U " + str(self.right) + "]")
                           
CEW = namedtuple('CEW', ['group', 'left', 'right'])
CEW.__bases__ += (Spec,)
CEW.__str__ = (lambda self: "<" +','.join([ag.value for ag in self.group])+ ">"+ 
                            "[" + str(self.left) +
                            " W " + str(self.right) + "]")
 
CAF = namedtuple('CAF', ['group', 'child'])
CAF.__bases__ += (Spec,)
CAF.__str__ = (lambda self: "[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "F " + str(self.child))
                           
CAG = namedtuple('CAG', ['group', 'child'])
CAG.__bases__ += (Spec,)
CAG.__str__ = (lambda self: "[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "G " + str(self.child))
                           
CAX = namedtuple('CAX', ['group', 'child'])
CAX.__bases__ += (Spec,)
CAX.__str__ = (lambda self: "[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "X " + str(self.child))
                           
CAU = namedtuple('CAU', ['group', 'left', 'right'])
CAU.__bases__ += (Spec,)
CAU.__str__ = (lambda self: "[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "[" + str(self.left) +
                            " U " + str(self.right) + "]")
                           
CAW = namedtuple('CAW', ['group', 'left', 'right'])
CAW.__bases__ += (Spec,)
CAW.__str__ = (lambda self: "[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "[" + str(self.left) +
                            " W " + str(self.right) + "]")