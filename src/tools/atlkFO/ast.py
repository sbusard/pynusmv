"""
AST classes for ATLK formulas representation.
"""

from collections import namedtuple

class Spec:
    """A Spec is represents a ATLK formula."""
    def subformulas(self):
        """Return the set of sub-formulas of this formula, including itself."""
        raise NotImplementedError("Should be implemented by subclasses.")

# TrueExp, FalseExp, Init, Reachable
class TrueExp(Spec):
    def __str__(self):
        return "True"
    def subformulas(self):
        return {self}
        
class FalseExp(Spec):
    def __str__(self):
        return "False"
    def subformulas(self):
        return {self}
        
class Init(Spec):
    def __str__(self):
        return "Init"
    def subformulas(self):
        return {self}
        
class Reachable(Spec):
    def __str__(self):
        return "Reachable"
    def subformulas(self):
        return {self}
        
class Atom(Spec):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return "'" + str(self.value) + "'"
    def subformulas(self):
        return {self}

# logical : Not, And, Or, Implies, Iff
class Not(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return "~" + "(" + str(self.child) + ")"
    def subformulas(self):
        return {self} | self.child.subformulas()

class And(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return "(" + str(self.left) + " & " + str(self.right) + ")"
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class Or(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return "(" + str(self.left) + " | " + str(self.right) + ")"
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class Implies(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return "(" + str(self.left) + " -> " + str(self.right) + ")"
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class Iff(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return "(" + str(self.left) + " <-> " + str(self.right) + ")"
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()
                                  

# temporal : AF, AG, AX, AU, AW, EF, EG, EX, EU, EW
class AF(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "A" + "F " + str(self.child)
    def subformulas(self):
        return {self} | self.child.subformulas()

class AG(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "A" + "G " + str(self.child)
    def subformulas(self):
        return {self} | self.child.subformulas()

class AX(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "A" + "X " + str(self.child)
    def subformulas(self):
        return {self} | self.child.subformulas()

class AU(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return ("A" + "[" + str(self.left) +
                      " U " + str(self.right) + "]")
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class AW(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return ("A" + "[" + str(self.left) +
                      " W " + str(self.right) + "]")
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class EF(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "E" + "F " + str(self.child)
    def subformulas(self):
        return {self} | self.child.subformulas()

class EG(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "E" + "G " + str(self.child)
    def subformulas(self):
        return {self} | self.child.subformulas()

class EX(Spec):
    def __init__(self, child):
        self.child = child
    def __str__(self):
        return  "E" + "X " + str(self.child)
    def subformulas(self):
        return {self} | self.child.subformulas()

class EU(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return ("E" + "[" + str(self.left) +
                      " U " + str(self.right) + "]")
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class EW(Spec):
    def __init__(self, left, right):
        self.left = left
        self.right = right
    def __str__(self):
        return ("E" + "[" + str(self.left) +
                      " W " + str(self.right) + "]")
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()
                                

# epistemic : nK, nE, nD, nC, K, E, D, C
class nK(Spec):
    def __init__(self, agent, child):
        self.agent = agent
        self.child = child
    def __str__(self):
        return  ("nK" + "<" + str(self.agent) + ">" + " " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()
        
class nE(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("nE" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()
        
class nD(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("nD" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()
        
class nC(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("nC" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()

class K(Spec):
    def __init__(self, agent, child):
        self.agent = agent
        self.child = child
    def __str__(self):
        return  ("K" + "<" + str(self.agent) + ">" + " " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()
        
class E(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("E" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()
        
class D(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("D" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()
        
class C(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return  ("C" + "<" + ','.join([ag.value for ag in self.group]) +
                              ">" + " " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()


# strategic : <g>F, <g>G, <g>X, <g>U, <g>W, [g]F, [g]G, [g]X, [g]U, [g]W
class CEF(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+
                "F " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()

class CEG(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+
                "G " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()

class CEX(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+
                "X " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()

class CEU(Spec):
    def __init__(self, group, left, right):
        self.group = group
        self.left = left
        self.right = right
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+ 
                            "[" + str(self.left) +
                            " U " + str(self.right) + "]")
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class CEW(Spec):
    def __init__(self, group, left, right):
        self.group = group
        self.left = left
        self.right = right
    def __str__(self):
        return ("<" +','.join([ag.value for ag in self.group])+ ">"+ 
                            "[" + str(self.left) +
                            " W " + str(self.right) + "]")
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class CAF(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+
                "F " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()

class CAG(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+
                "G " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()

class CAX(Spec):
    def __init__(self, group, child):
        self.group = group
        self.child = child
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+
                "X " + str(self.child))
    def subformulas(self):
        return {self} | self.child.subformulas()

class CAU(Spec):
    def __init__(self, group, left, right):
        self.group = group
        self.left = left
        self.right = right
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "[" + str(self.left) +
                            " U " + str(self.right) + "]")
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()

class CAW(Spec):
    def __init__(self, group, left, right):
        self.group = group
        self.left = left
        self.right = right
    def __str__(self):
        return ("[" +','.join([ag.value for ag in self.group])+ "]"+ 
                            "[" + str(self.left) +
                            " W " + str(self.right) + "]")
    def subformulas(self):
        return {self} | self.left.subformulas() | self.right.subformulas()