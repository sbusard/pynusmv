"""
AST classes for CTLK formulas representation.
"""

from collections import namedtuple

class Spec:
    """A Spec is represents a CTLK formula."""

# TrueExp, FalseExp, Init, Reachable
TrueExp = namedtuple('TrueExp', [])
TrueExp.__bases__ += (Spec,)
TrueExp.__str__ = lambda self: "True"

FalseExp = namedtuple('FalseExp', [])
FalseExp.__bases__ += (Spec,)
FalseExp.__str__ = lambda self: "False"

Init = namedtuple('Init', [])
Init.__bases__ += (Spec,)
Init.__str__ = lambda self: "Init"

Reachable = namedtuple('Reachable', [])
Reachable.__bases__ += (Spec,)
Reachable.__str__ = lambda self: "Reachable"

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
                                  

# temporal : AF, AG, AX, AU, AW, EF, EG, EX, EU, EW
AF = namedtuple('AF', ['child'])
AF.__bases__ += (Spec,)
AF.__str__ = (lambda self: "A" + "F " + str(self.child))
                            
AG = namedtuple('AG', ['child'])
AG.__bases__ += (Spec,)
AG.__str__ = (lambda self: "A" + "G " + str(self.child))
                           
AX = namedtuple('AX', ['child'])
AX.__bases__ += (Spec,)
AX.__str__ = (lambda self: "A" + "X " + str(self.child))
                           
AU = namedtuple('AU', ['left', 'right'])
AU.__bases__ += (Spec,)
AU.__str__ = (lambda self: "A" + "[" + str(self.left) +
                                 " U " + str(self.right) + "]")
                           
AW = namedtuple('EW', ['left', 'right'])
AW.__bases__ += (Spec,)
AW.__str__ = (lambda self: "A"+ "[" + str(self.left) +
                                " W " + str(self.right) + "]")
 
EF = namedtuple('EF', ['child'])
EF.__bases__ += (Spec,)
EF.__str__ = (lambda self: "E" + "F " + str(self.child))
                           
EG = namedtuple('EG', ['child'])
EG.__bases__ += (Spec,)
EG.__str__ = (lambda self: "E" + "G " + str(self.child))
                           
EX = namedtuple('EX', ['child'])
EX.__bases__ += (Spec,)
EX.__str__ = (lambda self: "E" + "X " + str(self.child))
                           
EU = namedtuple('EU', ['left', 'right'])
EU.__bases__ += (Spec,)
EU.__str__ = (lambda self: "E"+ "[" + str(self.left) +
                                " U " + str(self.right) + "]")
                           
EW = namedtuple('EW', ['left', 'right'])
EW.__bases__ += (Spec,)
EW.__str__ = (lambda self: "E"+ "[" + str(self.left) +
                                " W " + str(self.right) + "]")
                                

# epistemic : nK, nE, nD, nC, K, E, D, C
nK = namedtuple('nK', ['agent', 'child'])
nK.__bases__ += (Spec,)
nK.__str__ = (lambda self: "nK" + "<" + str(self.agent) + ">" + " "
                                + str(self.child))

nE = namedtuple('nE', ['group', 'child'])
nE.__bases__ += (Spec,)
nE.__str__ = (lambda self: "nE" + "<" +
                           ','.join([ag.value for ag in self.group]) +
                           ">" + " " + str(self.child))

nD = namedtuple('nD', ['group', 'child'])
nD.__bases__ += (Spec,)
nD.__str__ = (lambda self: "nD" + "<" +
                           ','.join([ag.value for ag in self.group]) +
                           ">" + " " + str(self.child))

nC = namedtuple('nC', ['group', 'child'])
nC.__bases__ += (Spec,)
nC.__str__ = (lambda self: "nC" + "<" +
                           ','.join([ag.value for ag in self.group]) +
                           ">" + " " + str(self.child))
                                
K = namedtuple('K', ['agent', 'child'])
K.__bases__ += (Spec,)
K.__str__ = (lambda self: "K" + "<" + str(self.agent) + ">" + " "
                              + str(self.child))

E = namedtuple('E', ['group', 'child'])
E.__bases__ += (Spec,)
E.__str__ = (lambda self: "E" + "<" +
                           ','.join([ag.value for ag in self.group]) +
                           ">" + " " + str(self.child))

D = namedtuple('D', ['group', 'child'])
D.__bases__ += (Spec,)
D.__str__ = (lambda self: "D" + "<" +
                           ','.join([ag.value for ag in self.group]) +
                           ">" + " " + str(self.child))

C = namedtuple('C', ['group', 'child'])
C.__bases__ += (Spec,)
C.__str__ = (lambda self: "C" + "<" +
                           ','.join([ag.value for ag in self.group]) +
                           ">" + " " + str(self.child))