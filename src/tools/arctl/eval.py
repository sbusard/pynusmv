"""
ARCTL evaluation functions.
"""

from pyparsing import ParseException

from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression

from .parsing import parseArctl

from .ast import (TrueExp, FalseExp,
                  Atom, Not, And, Or, Implies, Iff, 
                  AaF, AaG, AaX, AaU, AaW,
                  EaF, EaG, EaX, EaU, EaW)
                  
                  
def evalArctl_from_string(fsm, spec):
    """
    Parse spec and return a BDD representing the set of states of fsm
    satisfying spec.
    
    Throws a ParseException if spec does not represent an ARCTL formula.
    """
    specs = parseArctl(spec)
    if len(specs) != 1:
        raise ParseException("Multiple parsing results")
    return evalArctl(fsm, specs[0])
    

def evalArctl(fsm, spec):
    """Return a BDD representing the set of states of fsm satisfying spec."""
    
    if type(spec) is TrueExp:
        return BDD.true(fsm.bddEnc.DDmanager)
        
    elif type(spec) is FalseExp:
        return BDD.false(fsm.bddEnc.DDmanager)
    
    elif type(spec) is Atom:
        return eval_simple_expression(fsm, spec.value)
        
    elif type(spec) is Not:
        return ~evalArctl(fsm, spec.child)
        
    elif type(spec) is And:
        return evalArctl(fsm, spec.left) & evalArctl(fsm, spec.right)
        
    elif type(spec) is Or:
        return evalArctl(fsm, spec.left) | evalArctl(fsm, spec.right)
        
    elif type(spec) is Implies:
        return (~evalArctl(fsm, spec.left)) | evalArctl(fsm, spec.right)
        
    elif type(spec) is Iff:
        l = evalArctl(fsm, spec.left)
        r = evalArctl(fsm, spec.right)
        return (l & r) | ((~l) & (~r))
        
    elif type(spec) is AaF:
        return aaf(fsm, evalArctl(fsm, spec.action), evalArctl(fsm, spec.child))
        
    elif type(spec) is AaG:
        return aag(fsm, evalArctl(fsm, spec.action), evalArctl(fsm, spec.child))
        
    elif type(spec) is AaX:
        return aax(fsm, evalArctl(fsm, spec.action), evalArctl(fsm, spec.child))
        
    elif type(spec) is AaU:
        return aau(fsm, evalArctl(fsm, spec.action),
                   evalArctl(fsm, spec.left),
                   evalArctl(fsm, spec.right))
        
    elif type(spec) is AaW:
        return aaw(fsm, evalArctl(fsm, spec.action),
                   evalArctl(fsm, spec.left),
                   evalArctl(fsm, spec.right))
                   
    elif type(spec) is EaF:
        return eaf(fsm, evalArctl(fsm, spec.action), evalArctl(fsm, spec.child))
        
    elif type(spec) is EaG:
        return eag(fsm, evalArctl(fsm, spec.action), evalArctl(fsm, spec.child))
        
    elif type(spec) is EaX:
        return eax(fsm, evalArctl(fsm, spec.action), evalArctl(fsm, spec.child))
        
    elif type(spec) is EaU:
        return eau(fsm, evalArctl(fsm, spec.action),
                   evalArctl(fsm, spec.left),
                   evalArctl(fsm, spec.right))
        
    elif type(spec) is EaW:
        return eaw(fsm, evalArctl(fsm, spec.action),
                   evalArctl(fsm, spec.left),
                   evalArctl(fsm, spec.right))
                   
    else:
        # TODO Generate error
        print("[ERROR] ARCTL evalArctl: unrecognized specification type", spec)
        return None
        
        
def _fp(funct, start):
    """
    Return the fixpoint of funct, as a BDD, starting with start BDD.
    
    mu Z.f(Z) least fixpoint is implemented with _fp(funct, false).
    nu Z.f(Z) greatest fixpoint is implemented with _fp(funct, true).
    """
    
    old = start
    new = funct(start)
    while old != new:
        old = new
        new = funct(old)
    return old
    
    
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
    """aau(a, p, q) = ~_eu(a, ~q, ~q & (~p | ~_ex(a, true))) & ~_eg(a, ~q)"""
    return (
                (
                    ~_eu(
                         fsm,
                         alpha,
                         ~psi, 
                         (~psi)
                         &
                         (
                            (~phi)
                            | 
                            (~_ex(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager)))
                         )
                    )
                )
                &
                (
                    ~_eg(fsm, alpha, (~psi))
                )
           )
    

def eaf(fsm, alpha, phi):
    """eaf(a, p) = _eu(a, true, p)"""
    return _eu(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager), phi)
    
    
def aaf(fsm, alpha, phi):
    """aaf(a, p) = ~_eu(a, ~p, ~p & ~_ex(a, true)) & ~_eg(a, ~p)"""
    true = BDD.true(fsm.bddEnc.DDmanager)
    return (
                ~_eu(fsm,
                     alpha,
                     (~phi),
                     (~phi)
                     &
                     (~_ex(fsm, alpha, true))
                    )
                &
                (~_eg(fsm, alpha, (~phi)))
           )
    
    
def eag(fsm, alpha, phi):
    """eag(a, p) = _eu(a, p, p & ~_ex(a, true)) | _eg(a, p)"""
    return _eu(fsm, alpha, phi, (phi &
               (~_ex(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager)))) |
                _eg(fsm, alpha, phi))
    
    
def aag(fsm, alpha, phi):
    """aag(a, p) = ~_eu(a, true, ~p)"""
    return ~_eu(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager), ~phi)

  
def eaw(fsm, alpha, phi, psi):
    """eaw(a, p, q) = ~aau(a, ~q, ~p & ~q)"""
    # TODO Discuss this equivalence with Charles
    return ~aau(fsm, alpha, ~psi, (~phi) & (~psi))
    
    
def aaw(fsm, alpha, phi, psi):    
    # TODO Discuss this equivalence with Charles
    """aaw(a, p, q) = ~eau(a, ~q, ~p & ~q)"""
    return ~eau(fsm, alpha, ~psi, ~phi & ~psi)