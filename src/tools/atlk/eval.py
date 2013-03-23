"""
CTLK evaluation functions.
"""

from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression
from pynusmv.utils import fixpoint as fp

from .ast import (TrueExp, FalseExp, Init, Reachable,
                  Atom, Not, And, Or, Implies, Iff, 
                  AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                  nK, nE, nD, nC, K, E, D, C,
                  CEF, CEG, CEX, CEU, CEW, CAF, CAG, CAX, CAU, CAW)


def evalATLK(fsm, spec):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    
    fsm -- a MAS representing the system
    spec -- an AST-based ATLK specification
    """
    
    if type(spec) is TrueExp:
        return BDD.true(fsm.bddEnc.DDmanager)
        
    elif type(spec) is FalseExp:
        return BDD.false(fsm.bddEnc.DDmanager)
        
    elif type(spec) is Init:
        return fsm.init
        
    elif type(spec) is Reachable:
        return fsm.reachable_states
    
    elif type(spec) is Atom:
        return eval_simple_expression(fsm, spec.value)
        
    elif type(spec) is Not:
        return ~evalATLK(fsm, spec.child)
        
    elif type(spec) is And:
        return evalATLK(fsm, spec.left) & evalATLK(fsm, spec.right)
        
    elif type(spec) is Or:
        return evalATLK(fsm, spec.left) | evalATLK(fsm, spec.right)
        
    elif type(spec) is Implies:
        # a -> b = ~a | b
        return (~evalATLK(fsm, spec.left)) | evalATLK(fsm, spec.right)
        
    elif type(spec) is Iff:
        # a <-> b = (a & b) | (~a & ~b)
        l = evalATLK(fsm, spec.left)
        r = evalATLK(fsm, spec.right)
        return (l & r) | ((~l) & (~r))
        
    elif type(spec) is EX:
        return ex(fsm, evalATLK(fsm, spec.child))
        
    elif type(spec) is AX:
        # AX p = ~EX ~p
        return ~ex(fsm, ~evalATLK(fsm, spec.child))
        
    elif type(spec) is EG:
        return eg(fsm, evalATLK(fsm, spec.child))
        
    elif type(spec) is AG:
        # AG p = ~EF ~p = ~E[ true U ~p ]
        return ~eu(fsm,
                   BDD.true(fsm.bddEnc.DDmanager),
                   ~evalATLK(fsm, spec.child))
        
    elif type(spec) is EU:
        return eu(fsm, evalATLK(fsm, spec.left), evalATLK(fsm, spec.right))
        
    elif type(spec) is AU:
        # A[p U q] = ~E[~q W ~p & ~q] = ~(E[~q U ~p & ~q] | EG ~q)
        p = evalATLK(fsm, spec.left)
        q = evalATLK(fsm, spec.right)
        equpq = eu(fsm, ~q, ~q & ~p)
        egq = eg(fsm, ~q)
        return ~(equpq | egq)
        
    elif type(spec) is EF:
        # EF p = E[ true U p ]
        return eu(fsm,
                  BDD.true(fsm.bddEnc.DDmanager),
                  evalATLK(fsm, spec.child))    
        
    elif type(spec) is AF:
        # AF p = ~EG ~p
        return ~eg(fsm, ~evalATLK(fsm, spec.child))
        
    elif type(spec) is EW:
        # E[ p W q ] = E[ p U q ] | EG p
        return (eu(fsm, evalATLK(fsm, spec.left), evalATLK(fsm, spec.right)) |
                eg(fsm, evalATLK(fsm, spec.left)))
        
    elif type(spec) is AW:
        # A[p W q] = ~E[~q U ~p & ~q]
        p = evalATLK(fsm, spec.left)
        q = evalATLK(fsm, spec.right)
        return ~eu(fsm, ~q, ~p & ~q)
        
    elif type(spec) is nK:
        return nk(fsm, spec.agent.value, evalATLK(fsm, spec.child))
        
    elif type(spec) is K:
        # K<'a'> p = ~nK<'a'> ~p
        return ~nk(fsm, spec.agent.value, ~evalATLK(fsm, spec.child))
        
    elif type(spec) is nE:
        return ne(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child))
        
    elif type(spec) is E:
        # E<g> p = ~nE<g> ~p
        return ~ne(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child))
        
    elif type(spec) is nD:
        return nd(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child)) 
        
    elif type(spec) is D:
        # D<g> p = ~nD<g> ~p
        return ~nd(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child))
        
    elif type(spec) is nC:
        return nc(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child))
        
    elif type(spec) is C:
        # C<g> p = ~nC<g> ~p
        return ~nc(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child))
                   
    # TODO ATL operators
        
    else:
        # TODO Generate error
        print("[ERROR] CTLK evalATLK: unrecognized specification type", spec)
        return None
    
def fair_states(fsm):
    """
    Return the set of fair states of the model.
    
    fsm - the model
    """
    return eg(fsm, BDD.true(fsm.bddEnc.DDmanager))
    
    
def ex(fsm, phi):
    """
    Return the set of states of fsm satisfying EX phi.
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    return fsm.pre(phi & fair_states(fsm))
    
    
def eg(fsm, phi):
    """
    Return the set of states of fsm satisfying EG phi.
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    def inner(Z):
        res = Z
        for f in fsm.fairness_constraints:
            res = res & fixpoint(lambda Y : (Z & f) | (phi & fsm.weak_pre(Y)),
                                 BDD.false(fsm.bddEnc.DDmanager))
        return phi & fsm.weak_pre(res)
        
    r = fixpoint(inner, BDD.true(fsm.bddEnc.DDmanager))
    return r.forsome(fsm.bddEnc.inputsCube)
    
    
def eu(fsm, phi, psi):
    """
    Return the set of states of fsm satisfying E[ phi U psi ].
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    return fixpoint(lambda X : (psi & fair_states(fsm) & fsm.reachable_states) |
                               (phi & ex(fsm, X)),
                    BDD.false(fsm.bddEnc.DDmanager))
    
    
def nk(fsm, agent, phi):
    """
    Return the set of states of fsm satisfying nK<'agent'> phi
    
    fsm -- a MAS representing the system
    agent -- the (str) name of an agent of fsm
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    # Return the set of states that have a successor in agent's knowledge
    # that satisfies phi and is reachable
    # nK<'a'> 'p' = fsm.equivalent_states(phi, agent)
    return fsm.equivalent_states(phi & fsm.reachable_states & fair_states(fsm),
                                 frozenset({agent}))
    

def ne(fsm, group, phi):
    """
    Return the set of states of fsm satisfying nE<group> phi
    
    fsm -- a MAS representing the system
    group -- a non-empty list of (str) names of agents of fsm
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    # A state satisfies nE<g> p iff there is an equivalent state in any relation
    # of any agent in g that satisfies p
    # nE<g> p = \/_{ag in g} nK<ag> p
    result = BDD.false(fsm.bddEnc.DDmanager)
    for agent in group:
        result = result | nk(fsm, agent, phi)
    return result
    
    
def nd(fsm, group, phi):
    """
    Return the set of states of fsm satisfying nD<group> phi
    
    fsm -- a MAS representing the system
    group -- a non-empty list of (str) names of agents of fsm
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    return fsm.equivalent_states(phi & fsm.reachable_states & fair_states(fsm),
                                 frozenset(group))
    
    
def nc(fsm, group, phi):
    """
    Return the set of states of fsm satisfying nC<group> phi
    
    fsm -- a MAS representing the system
    group -- a non-empty list of (str) names of agents of fsm
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    # nC<g> p = μZ. p | nE<g> Z
    #return fp(lambda Z: (phi | ne(fsm, group, Z)),
    #          BDD.false(fsm.bddEnc.DDmanager))
    # nC<g> p = μZ. nE<g> (p | Z)
    return fp(lambda Z: ne(fsm, group, (Z | phi)),
              BDD.false(fsm.bddEnc.DDmanager))