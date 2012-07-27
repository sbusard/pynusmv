"""
ARCTL evaluation functions.
"""

from pynusmv.dd.bdd import BDD
from pynusmv.mc.mc import eval_simple_expression

from .ast import (Atom, Not, And, Or, Implies, Iff, 
                  AaF, AaG, AaX, AaU, AaW,
                  EaF, EaG, EaX, EaU, EaW)

def eval(fsm, spec):
    """Return a BDD representing the set of states of fsm satisfying spec."""
    
    if type(spec) is Atom:
        return eval_simple_expression(fsm, spec.value)
        
    elif type(spec) is Not:
        return ~eval(fsm, spec.child)
        
    elif type(spec) is And:
        return eval(fsm, spec.left) & eval(fsm, spec.right)
        
    elif type(spec) is Or:
        return eval(fsm, spec.left) | eval(fsm, spec.right)
        
    elif type(spec) is Implies:
        return (~eval(fsm, spec.left)) | eval(fsm, spec.right)
        
    elif type(spec) is Iff:
        l = eval(fsm, spec.left)
        r = eval(fsm, spec.right)
        return (l & r) | ((~l) & (~r))
        
    elif type(spec) is AaF:
        return aaf(fsm, eval(fsm, spec.action), eval(fsm, spec.child))
        
    elif type(spec) is AaG:
        return aag(fsm, eval(fsm, spec.action), eval(fsm, spec.child))
        
    elif type(spec) is AaX:
        return aax(fsm, eval(fsm, spec.action), eval(fsm, spec.child))
        
    elif type(spec) is AaU:
        return aau(fsm, eval(fsm, spec.action),
                   eval(fsm, spec.left),
                   eval(fsm, spec.right))
        
    elif type(spec) is AaW:
        return aaw(fsm, eval(fsm, spec.action),
                   eval(fsm, spec.left),
                   eval(fsm, spec.right))
                   
    elif type(spec) is EaF:
        return eaf(fsm, eval(fsm, spec.action), eval(fsm, spec.child))
        
    elif type(spec) is EaG:
        return eag(fsm, eval(fsm, spec.action), eval(fsm, spec.child))
        
    elif type(spec) is EaX:
        return eax(fsm, eval(fsm, spec.action), eval(fsm, spec.child))
        
    elif type(spec) is EaU:
        return eau(fsm, eval(fsm, spec.action),
                   eval(fsm, spec.left),
                   eval(fsm, spec.right))
        
    elif type(spec) is EaW:
        return eaw(fsm, eval(fsm, spec.action),
                   eval(fsm, spec.left),
                   eval(fsm, spec.right))
                   
    else:
        # TODO Generate error
        print("[ERROR] ARCTL eval: unrecognized specification type", spec)
        return None
        
        
def _fp(funct, start):
    """
    Return the fixpoint of funct, as a BDD, starting with start BDD.
    
    μZ.f(Z) least fixpoint is implemented with _fp(funct, false).
    νZ.f(Z) greatest fixpoint is implemented with _fp(funct, true).
    """
    
    acc = start
    frontier = funct(acc) - acc
    while not frontier.is_false():
        acc = acc + frontier
        frontier = funct(acc) - acc
    return acc
    
    
def _ex(fsm, alpha, phi):
    """_ex(a, p) is pre of p through transitions satisfying a"""
    return fsm.pre(phi, alpha)
    
    
def _eu(fsm, alpha, phi, psi):
    """_eu(a, p, q) = muZ. (q | (p & _ex(a, Z)))"""
    return _fp(lambda Z: (psi | (phi & _ex(fsm, alpha, Z))),
               BDD.false(fsm.bddEnc.DDmanager))
    
    
def _eg(fsm, alpha, phi):
    """_eg(a, p) = nuZ. (p & _ex(a, Z))"""
    return _fp(lambda Z: (phi & _ex(fsm, alpha, Z)),
               BDD.true(fsm.bddEnc.DDmanager))
    
    
def eax(fsm, alpha, phi):
    """eax(a, p) = _ex(a, p)"""
    return _ex(fsm, alpha, phi)
    
    
def aax(fsm, alpha, phi):
    """aax(a, p) = _ex(a, true) & ~_ex(a, ~p)"""
    return (_ex(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager)) &
          (~_ex(fsm, alpha, (~phi))))
    
    
def eau(fsm, alpha, phi, psi):
    """eau(a, p, q) = _eu(a, p, q)"""
    return _eu(fsm, alpha, phi, psi)
    
    
def aau(fsm, alpha, phi, psi):
    """aau(a, p, q) = ~_eu(a, ~q, ~q & ~_ex(a, true)) & ~_eg(a, ~q)"""
    return ((~_eu(fsm, alpha, ~psi, (~psi) &
           (~_ex(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager))))) &
           (~_eg(fsm, alpha, (~psi))))
    

def eaf(fsm, alpha, phi):
    """eaf(a, p) = _eu(a, true, p)"""
    return _eu(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager), phi)
    
    
def aaf(fsm, alpha, phi):
    """aaf(a, p) = ~_eu(a, ~p, ~p & ~_ex(a, true)) & ~_eg(a, ~p)"""
    return (~_eu(fsm, alpha, (~phi),
                 (~phi) & (~_ex(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager)))) &
                  (~_eg(fsm, alpha, (~phi))))
    
    
def eag(fsm, alpha, phi):
    """eag(a, p) = _eu(a, p, p & ~_ex(a, true)) | _eg(a, p)"""
    return _eu(fsm, alpha, phi, (phi &
               (~_ex(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager)))) |
                _eg(fsm, alpha, phi))
    
    
def aag(fsm, alpha, phi):
    """aag(a, p) = ~_eu(a, true, ~p)"""
    return ~_eu(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager), ~phi)

  
def eaw(fsm, alpha, phi, psi):
    """eaw(a, p, q) = ???"""
    pass # TODO Find equivalence and apply accordingly
    
    
def aaw(fsm, alpha, phi, psi):
    """aaw(a, p, q) = ???"""
    pass # TODO Find equivalence and apply accordingly