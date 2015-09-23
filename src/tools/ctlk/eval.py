"""
CTLK evaluation functions.
"""

from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression
from pynusmv.utils import fixpoint as fp

from .ast import (TrueExp, FalseExp, Init, Reachable,
                  Atom, Not, And, Or, Implies, Iff, 
                  AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                  nK, nE, nD, nC, K, E, D, C)


def evalCTLK(fsm, spec):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    
    fsm -- a MAS representing the system
    spec -- an AST-based CTLK specification
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
        return ~evalCTLK(fsm, spec.child)
        
    elif type(spec) is And:
        return evalCTLK(fsm, spec.left) & evalCTLK(fsm, spec.right)
        
    elif type(spec) is Or:
        return evalCTLK(fsm, spec.left) | evalCTLK(fsm, spec.right)
        
    elif type(spec) is Implies:
        # a -> b = ~a | b
        return (~evalCTLK(fsm, spec.left)) | evalCTLK(fsm, spec.right)
        
    elif type(spec) is Iff:
        # a <-> b = (a & b) | (~a & ~b)
        l = evalCTLK(fsm, spec.left)
        r = evalCTLK(fsm, spec.right)
        return (l & r) | ((~l) & (~r))
        
    elif type(spec) is EX:
        return ex(fsm, evalCTLK(fsm, spec.child))
        
    elif type(spec) is AX:
        # AX p = ~EX ~p
        return ~ex(fsm, ~evalCTLK(fsm, spec.child))
        
    elif type(spec) is EG:
        return eg(fsm, evalCTLK(fsm, spec.child))
        
    elif type(spec) is AG:
        # AG p = ~EF ~p = ~E[ true U ~p ]
        return ~eu(fsm,
                   BDD.true(fsm.bddEnc.DDmanager),
                   ~evalCTLK(fsm, spec.child))
        
    elif type(spec) is EU:
        return eu(fsm, evalCTLK(fsm, spec.left), evalCTLK(fsm, spec.right))
        
    elif type(spec) is AU:
        # A[p U q] = ~E[~q W ~p & ~q] = ~(E[~q U ~p & ~q] | EG ~q)
        p = evalCTLK(fsm, spec.left)
        q = evalCTLK(fsm, spec.right)
        equpq = eu(fsm, ~q, ~q & ~p)
        egq = eg(fsm, ~q)
        return ~(equpq | egq)
        
    elif type(spec) is EF:
        # EF p = E[ true U p ]
        return eu(fsm,
                  BDD.true(fsm.bddEnc.DDmanager),
                  evalCTLK(fsm, spec.child))    
        
    elif type(spec) is AF:
        # AF p = ~EG ~p
        return ~eg(fsm, ~evalCTLK(fsm, spec.child))
        
    elif type(spec) is EW:
        # E[ p W q ] = E[ p U q ] | EG p
        return (eu(fsm, evalCTLK(fsm, spec.left), evalCTLK(fsm, spec.right)) |
                eg(fsm, evalCTLK(fsm, spec.left)))
        
    elif type(spec) is AW:
        # A[p W q] = ~E[~q U ~p & ~q]
        p = evalCTLK(fsm, spec.left)
        q = evalCTLK(fsm, spec.right)
        return ~eu(fsm, ~q, ~p & ~q)
        
    elif type(spec) is nK:
        return nk(fsm, spec.agent.value, evalCTLK(fsm, spec.child))
        
    elif type(spec) is K:
        # K<'a'> p = ~nK<'a'> ~p
        return ~nk(fsm, spec.agent.value, ~evalCTLK(fsm, spec.child))
        
    elif type(spec) is nE:
        return ne(fsm,
                  [a.value for a in spec.group],
                  evalCTLK(fsm, spec.child))
        
    elif type(spec) is E:
        # E<g> p = ~nE<g> ~p
        return ~ne(fsm,
                   [a.value for a in spec.group],
                   ~evalCTLK(fsm, spec.child))
        
    elif type(spec) is nD:
        return nd(fsm,
                  [a.value for a in spec.group],
                  evalCTLK(fsm, spec.child)) 
        
    elif type(spec) is D:
        # D<g> p = ~nD<g> ~p
        return ~nd(fsm,
                   [a.value for a in spec.group],
                   ~evalCTLK(fsm, spec.child))
        
    elif type(spec) is nC:
        return nc(fsm,
                  [a.value for a in spec.group],
                  evalCTLK(fsm, spec.child))
        
    elif type(spec) is C:
        # C<g> p = ~nC<g> ~p
        return ~nc(fsm,
                   [a.value for a in spec.group],
                   ~evalCTLK(fsm, spec.child))
        
    else:
        # TODO Generate error
        print("[ERROR] CTLK evalCTLK: unrecognized specification type", spec)
        return None
    
    
def ex(fsm, phi):
    """
    Return the set of states of fsm satisfying EX phi.
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    return fsm.pre(phi)
    
    
def eg(fsm, phi):
    """
    Return the set of states of fsm satisfying EG phi.
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    return fp(lambda Z: (phi & fsm.pre(Z)),
               BDD.true(fsm.bddEnc.DDmanager))
    
    
def eu(fsm, phi, psi):
    """
    Return the set of states of fsm satisfying E[ phi U psi ].
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    return fp(lambda Z: (psi | (phi & fsm.pre(Z))),
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
    return fsm.equivalent_states(phi & fsm.reachable_states,
                                 frozenset({agent})) & fsm.reachable_states
    

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
    return (fsm.equivalent_states(phi & fsm.reachable_states, frozenset(group))
            & fsm.reachable_states)
    
    
def nc(fsm, group, phi):
    """
    Return the set of states of fsm satisfying nC<group> phi
    
    fsm -- a MAS representing the system
    group -- a non-empty list of (str) names of agents of fsm
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    # nC<g> p = mu Z. p | nE<g> Z
    #return fp(lambda Z: (phi | ne(fsm, group, Z)),
    #          BDD.false(fsm.bddEnc.DDmanager))
    # nC<g> p = mu Z. nE<g> (p | Z)
    return fp(lambda Z: ne(fsm, group, (Z | phi)),
              BDD.false(fsm.bddEnc.DDmanager))