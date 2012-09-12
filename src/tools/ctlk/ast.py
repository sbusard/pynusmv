"""
AST classes for CTLK formulas representation.
"""

from collections import namedtuple

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
                                  

# temporal : AF, AG, AX, AU, AW, EF, EG, EX, EU, EW
AF = namedtuple('AF', ['child'])
AF.__str__ = (lambda self: "A" + "F " + str(self.child))
                            
AG = namedtuple('AG', ['child'])
AG.__str__ = (lambda self: "A" + "G " + str(self.child))
                           
AX = namedtuple('AX', ['child'])
AX.__str__ = (lambda self: "A" + "X " + str(self.child))
                           
AU = namedtuple('AU', ['left', 'right'])
AU.__str__ = (lambda self: "A" + "[" + str(self.left) +
                                 " U " + str(self.right) + "]")
                           
AW = namedtuple('EW', ['left', 'right'])
AW.__str__ = (lambda self: "A"+ "[" + str(self.left) +
                                " W " + str(self.right) + "]")
 
EF = namedtuple('EF', ['child'])
EF.__str__ = (lambda self: "E" + "F " + str(self.child))
                           
EG = namedtuple('EG', ['child'])
EG.__str__ = (lambda self: "E" + "G " + str(self.child))
                           
EX = namedtuple('EX', ['child'])
EX.__str__ = (lambda self: "E" + "X " + str(self.child))
                           
EU = namedtuple('EU', ['left', 'right'])
EU.__str__ = (lambda self: "E"+ "[" + str(self.left) +
                                " U " + str(self.right) + "]")
                           
EW = namedtuple('EW', ['left', 'right'])
EW.__str__ = (lambda self: "E"+ "[" + str(self.left) +
                                " W " + str(self.right) + "]")
                                

# epistemic : nK, nE, nD, nC, K, E, D, C
nK = namedtuple('nK', ['agent', 'child'])
nK.__str__ = (lambda self: "nK" + "<" + str(self.agent) + ">" + " "
                                + str(self.child))

nE = namedtuple('nE', ['group', 'child'])
nE.__str__ = (lambda self: "nE" + "<" + str(self.group) + ">" + " "
                                + str(self.child))

nD = namedtuple('nD', ['group', 'child'])
nD.__str__ = (lambda self: "nD" + "<" + str(self.group) + ">" + " "
                                + str(self.child))

nC = namedtuple('nC', ['group', 'child'])
nC.__str__ = (lambda self: "nC" + "<" + str(self.group) + ">" + " "
                                + str(self.child))
                                
K = namedtuple('K', ['agent', 'child'])
K.__str__ = (lambda self: "K" + "<" + str(self.agent) + ">" + " "
                              + str(self.child))

E = namedtuple('E', ['group', 'child'])
E.__str__ = (lambda self: "E" + "<" + str(self.group) + ">" + " "
                              + str(self.child))

D = namedtuple('D', ['group', 'child'])
D.__str__ = (lambda self: "D" + "<" + str(self.group) + ">" + " "
                              + str(self.child))

C = namedtuple('C', ['group', 'child'])
C.__str__ = (lambda self: "C" + "<" + str(self.group) + ">" + " "
                              + str(self.child))