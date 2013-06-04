"""
ATL evaluation functions.
"""

from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression
from pynusmv.utils import fixpoint as fp

from .ast import (TrueExp, FalseExp,
                  Atom, Not, And, Or, Implies, Iff,
                  CEF, CEG, CEX, CEU, CEW, CAF, CAG, CAX, CAU, CAW)


def evalATL(fsm, spec):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    
    fsm -- a MAS representing the system
    spec -- an AST-based ATL specification
    """
    
    if type(spec) is TrueExp:
        return BDD.true(fsm.bddEnc.DDmanager)
        
    elif type(spec) is FalseExp:
        return BDD.false(fsm.bddEnc.DDmanager)
    
    elif type(spec) is Atom:
        return eval_simple_expression(fsm, spec.value)
        
    elif type(spec) is Not:
        return ~evalATL(fsm, spec.child)
        
    elif type(spec) is And:
        return evalATL(fsm, spec.left) & evalATL(fsm, spec.right)
        
    elif type(spec) is Or:
        return evalATL(fsm, spec.left) | evalATL(fsm, spec.right)
        
    elif type(spec) is Implies:
        # a -> b = ~a | b
        return (~evalATL(fsm, spec.left)) | evalATL(fsm, spec.right)
        
    elif type(spec) is Iff:
        # a <-> b = (a & b) | (~a & ~b)
        l = evalATL(fsm, spec.left)
        r = evalATL(fsm, spec.right)
        return (l & r) | ((~l) & (~r))
                   
    elif type(spec) is CEX:
        return cex(fsm, {atom.value for atom in spec.group},
                   evalATL(fsm, spec.child))
        
    elif type(spec) is CAX:
        # [g] X p = ~<g> X ~p
        return ~cex(fsm, {atom.value for atom in spec.group},
                    ~evalATL(fsm, spec.child))
        
    elif type(spec) is CEG:
        return ceg(fsm, {atom.value for atom in spec.group},
                   evalATL(fsm, spec.child))
        
    elif type(spec) is CAG:
        # [g] G p = ~<g> F ~p
        return ~ceu(fsm, {atom.value for atom in spec.group},
                    BDD.true(fsm.bddEnc.DDmanager),
                    ~evalATL(fsm, spec.child))
        
    elif type(spec) is CEU:
        return ceu(fsm, {atom.value for atom in spec.group},
                   evalATL(fsm, spec.left),
                   evalATL(fsm, spec.right))
        
    elif type(spec) is CAU:
        # [g] p U q = ~<g>[ ~q W ~p & ~q ]
        return ~cew(fsm, {atom.value for atom in spec.group},
                    ~evalATL(fsm, spec.right),
                    ~evalATL(fsm, spec.right) & ~evalATL(fsm, spec.left))
        
    elif type(spec) is CEF:
        # <g> F p = <g>[true U p]
        return ceu(fsm, {atom.value for atom in spec.group},
                   BDD.true(fsm.bddEnc.DDmanager),
                   evalATL(fsm, spec.child))
        
    elif type(spec) is CAF:
        # [g] F p = ~<g> G ~p
        return ~ceg(fsm, {atom.value for atom in spec.group},
                    ~evalATL(fsm, spec.child))
        
    elif type(spec) is CEW:
        return cew(fsm, {atom.value for atom in spec.group},
                   evalATL(fsm, spec.left),
                   evalATL(fsm, spec.right))
        
    elif type(spec) is CAW:
        # [g][p W q] = ~<g>[~q U ~p & ~q]
        return ~ceu(fsm, {atom.value for atom in spec.group},
                    ~evalATL(fsm, spec.right),
                    ~evalATL(fsm, spec.right) & ~evalATL(fsm, spec.left))
        
    else:
        # TODO Generate error
        print("[ERROR] evalATL: unrecognized specification type", spec)
        return None
              
              
def cex(fsm, agents, phi):
    """
    Return the set of states of fsm satisfying <agents> X phi.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    return fsm.pre_strat(phi, agents)
    

def ceu(fsm, agents, phi, psi):
    """
    Return the set of states of fsm satisfying <agents>[phi U psi].
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    return fp(lambda Y : psi | (phi & fsm.pre_strat(Y, agents)),
              BDD.false(fsm.bddEnc.DDmanager))
    

def cew(fsm, agents, phi, psi):
    """
    Return the set of states of fsm satisfying <agents>[phi W psi].
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    return fp(lambda Z : psi | (phi & fsm.pre_strat(Z, agents)),
              BDD.true(fsm.bddEnc.DDmanager))
    
    
def ceg(fsm, agents, phi):
    """
    Return the set of states of fsm satisfying <agents> G phi.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    return fp(lambda Z : phi & fsm.pre_strat(Z, agents),
              BDD.true(fsm.bddEnc.DDmanager))