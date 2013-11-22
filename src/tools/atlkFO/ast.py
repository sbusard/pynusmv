"""
AST classes for ATLK formulas representation.
"""

from collections import namedtuple

class Spec:
    """A Spec is represents a ATLK formula."""

# TrueExp, FalseExp, Init, Reachable
class TrueExp(Spec):
    def __str__(self):
        return "True"
        
class FalseExp(Spec):
    def __str__(self):
        return "False"
        
class Init(Spec):
    def __str__(self):
        return "Init"
        
class Reachable(Spec):
    def __str__(self):
        return "Reachable"
        
class Atom(Spec):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return "'" + str(self.value) + "'"

# logical : Not, And, Or, Implies, Iff
class Not(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return "~" + "(" + str(self.child) + ")"

class And(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return "(" + str(self.left) + " & " + str(self.right) + ")"

class Or(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return "(" + str(self.left) + " | " + str(self.right) + ")"

class Implies(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return "(" + str(self.left) + " -> " + str(self.right) + ")"

class Iff(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return "(" + str(self.left) + " <-> " + str(self.right) + ")"
                                  

# temporal : AF, AG, AX, AU, AW, EF, EG, EX, EU, EW
class AF(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "A" + "F " + str(self.child)

class AG(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "A" + "G " + str(self.child)

class AX(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "A" + "X " + str(self.child)

class AU(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return ("A" + "[" + str(self.left) +
                      " U " + str(self.right) + "]")

class AW(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return ("A" + "[" + str(self.left) +
                      " W " + str(self.right) + "]")
class EF(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "E" + "F " + str(self.child)

class EG(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "E" + "G " + str(self.child)

class EX(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "E" + "X " + str(self.child)

class EU(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return ("E" + "[" + str(self.left) +
                      " U " + str(self.right) + "]")

class EW(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return ("E" + "[" + str(self.left) +
                      " W " + str(self.right) + "]")
                                

# epistemic : nK, nE, nD, nC, K, E, D, C
class nK(Spec):
    def __init__(self, agent, child):
        self.agent = agent
        self.child = child
    def __str__(self):
        return  ("nK" + "<" + str(self.agent) + ">" + " " + str(self.child))
        
class nE(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("nE" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
        
class nD(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("nD" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
        
class nC(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("nC" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))

class K(Spec):
    def __init__(self, agent, child):
        self.agent = agent
        self.child = child
    def __str__(self):
        return  ("K" + "<" + str(self.agent) + ">" + " " + str(self.child))
        
class E(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("E" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
        
class D(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("D" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
        
class C(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("C" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))


# strategic : <g>F, <g>G, <g>X, <g>U, <g>W, [g]F, [g]G, [g]X, [g]U, [g]W
class CEF(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+
                "F " + str(self.child))

class CEG(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+
                "G " + str(self.child))

class CEX(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+
                "X " + str(self.child))

class CEU(Spec):
    def __init__(self, group, left, right):
        self.group = group
        self.left = left
        self.right = right
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+ 
                            "[" + str(self.left) +
                            " U " + str(self.right) + "]")

class CEW(Spec):
    def __init__(self, group, left, right):
        self.group = group
        self.left = left
        self.right = right
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+ 
                            "[" + str(self.left) +
                            " W " + str(self.right) + "]")

class CAF(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+
                "F " + str(self.child))

class CAG(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+
                "G " + str(self.child))

class CAX(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+
                "X " + str(self.child))

class CAU(Spec):
    def __init__(self, group, left, right):
        self.group = group
        self.left = left
        self.right = right
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "[" + str(self.left) +
                            " U " + str(self.right) + "]")

class CAW(Spec):
    def __init__(self, group, left, right):
        self.group = group
        self.left = left
        self.right = right
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "[" + str(self.left) +
                            " W " + str(self.right) + "]")