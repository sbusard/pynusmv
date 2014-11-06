"""
ATLK with full observability evaluation functions.
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
                   
    elif type(spec) is CEX:
        # <g> X p = ~[g] X ~p
        #return ~cax(fsm, {atom.value for atom in spec.group},
        #                 ~evalATLK(fsm, spec.child))
        return cex(fsm, {atom.value for atom in spec.group},
                   evalATLK(fsm, spec.child))
        
    elif type(spec) is CAX:
        return cax(fsm, {atom.value for atom in spec.group},
                        evalATLK(fsm, spec.child))
        
    elif type(spec) is CEG:
        # <g> G p = ~[g] F ~p
        #return ~cau(fsm, {atom.value for atom in spec.group},
        #            BDD.true(fsm.bddEnc.DDmanager),
        #            ~evalATLK(fsm, spec.child))
        return cew(fsm, {atom.value for atom in spec.group},
                   evalATLK(fsm, spec.child), BDD.false(fsm.bddEnc.DDmanager))
        
    elif type(spec) is CAG:
        return cag(fsm, {atom.value for atom in spec.group},
                        evalATLK(fsm, spec.child))
        
    elif type(spec) is CEU:
        # <g> p U q = ~[g][ ~q W ~p & ~q ]
        #return ~caw(fsm, {atom.value for atom in spec.group},
        #            ~evalATLK(fsm, spec.right),
        #            ~evalATLK(fsm, spec.right) & ~evalATLK(fsm, spec.left))
        return ceu(fsm, {atom.value for atom in spec.group},
                   evalATLK(fsm, spec.left), evalATLK(fsm, spec.right))
        
    elif type(spec) is CAU:
        return cau(fsm, {atom.value for atom in spec.group},
                        evalATLK(fsm, spec.left),
                        evalATLK(fsm, spec.right))
        
    elif type(spec) is CEF:
        # <g> F p = ~[g] G ~p
        #return ~cag(fsm, {atom.value for atom in spec.group},
        #                 ~evalATLK(fsm, spec.child))
        return ceu(fsm, {atom.value for atom in spec.group},
                   BDD.true(fsm.bddEnc.DDmanager), evalATLK(fsm, spec.child))
        
    elif type(spec) is CAF:
        # [g] F p = [g][true U p]
        return cau(fsm, {atom.value for atom in spec.group},
                        BDD.true(fsm.bddEnc.DDmanager),
                        evalATLK(fsm, spec.child))
        
    elif type(spec) is CEW:
        # <g>[p W q] = ~[g][~q U ~p & ~q]
        #return ~cau(fsm, {atom.value for atom in spec.group},
        #                 ~evalATLK(fsm, spec.right),
        #                 ~evalATLK(fsm, spec.right) & ~evalATLK(fsm, spec.left))
        return cew(fsm, {atom.value for atom in spec.group},
                   evalATLK(fsm, spec.left), evalATLK(fsm, spec.right))
        
    elif type(spec) is CAW:
        return caw(fsm, {atom.value for atom in spec.group},
                        evalATLK(fsm, spec.left),
                        evalATLK(fsm, spec.right))
        
    else:
        # TODO Generate error
        print("[ERROR] evalATLK: unrecognized specification type", spec)
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
    
    phi = phi.forsome(fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
    return fsm.pre(phi & fair_states(fsm))
    
    
def eg(fsm, phi):
    """
    Return the set of states of fsm satisfying EG phi.
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    #def inner(Z):
    #    res = Z
    #    for f in fsm.fairness_constraints:
    #        res = res & fp(lambda Y : (Z & f) | (phi & fsm.weak_pre(Y)),
    #                             BDD.false(fsm.bddEnc.DDmanager))
    #    return phi & fsm.weak_pre(res)
    #    
    #r = fp(inner, BDD.true(fsm.bddEnc.DDmanager))
    #return r.forsome(fsm.bddEnc.inputsCube)
    
    phi = phi.forsome(fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
    
    if len(fsm.fairness_constraints) == 0:
        return fp(lambda Z : phi & fsm.pre(Z),
                  BDD.true(fsm.bddEnc.DDmanager)).forsome(fsm.bddEnc.inputsCube)
    else:
        def inner(Z):
            res = phi
            for f in fsm.fairness_constraints:
                res = res & fsm.pre(fp(lambda Y : (Z & f) |
                                       (phi & fsm.pre(Y)),
                                       BDD.false(fsm.bddEnc.DDmanager)))
            return res
        return (fp(inner, BDD.true(fsm.bddEnc.DDmanager))
                .forsome(fsm.bddEnc.inputsCube))
    
    
def eu(fsm, phi, psi):
    """
    Return the set of states of fsm satisfying E[ phi U psi ].
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    
    phi = phi.forsome(fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
    psi = psi.forsome(fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
    return fp(lambda X : (psi & fair_states(fsm) & fsm.reachable_states) |
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
    # nC<g> p = mu Z. p | nE<g> Z
    #return fp(lambda Z: (phi | ne(fsm, group, Z)),
    #          BDD.false(fsm.bddEnc.DDmanager))
    # nC<g> p = mu Z. nE<g> (p | Z)
    return fp(lambda Z: ne(fsm, group, (Z | phi)),
              BDD.false(fsm.bddEnc.DDmanager))
              
              
def cax(fsm, agents, phi):
    """
    Return the set of states of fsm satisfying [agents] X phi.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    return fsm.pre_nstrat(phi & fair_gamma_states(fsm, agents), agents)
    

def cau(fsm, agents, phi, psi):
    """
    Return the set of states of fsm satisfying [agents][phi U psi].
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    return fp(lambda Y : (psi & fair_gamma_states(fsm, agents)) |
                         (phi & fsm.pre_nstrat(Y, agents)),
              BDD.false(fsm.bddEnc.DDmanager))
    

def caw(fsm, agents, phi, psi):
    """
    Return the set of states of fsm satisfying [agents][phi W psi].
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    if len(fsm.fairness_constraints) == 0:
        return fp(lambda Z : psi | (phi & fsm.pre_nstrat(Z, agents)),
                  BDD.true(fsm.bddEnc.DDmanager))
    else:
        def inner(Z):
            res = phi
            for f in fsm.fairness_constraints:
                res = res & fsm.pre_nstrat(fp(lambda Y :
                                             (psi &fair_gamma_states(fsm,
                                                                     agents)) |
                                             (Z & f) |
                                             (phi & fsm.pre_nstrat(Y, agents)),
                                             BDD.false(fsm.bddEnc.DDmanager)),
                                           agents)
            return (psi &fair_gamma_states(fsm, agents)) | res
        return fp(inner, BDD.true(fsm.bddEnc.DDmanager))
    
    
def cag(fsm, agents, phi):
    """
    Return the set of states of fsm satisfying [agents] G phi.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    if len(fsm.fairness_constraints) == 0:
        return fp(lambda Z : phi & fsm.pre_nstrat(Z, agents),
                  BDD.true(fsm.bddEnc.DDmanager))
    else:
        def inner(Z):
            res = phi
            for f in fsm.fairness_constraints:
                res = res & fsm.pre_nstrat(fp(lambda Y : (Z & f) |
                                             (phi & fsm.pre_nstrat(Y, agents)),
                                             BDD.false(fsm.bddEnc.DDmanager))
                                           , agents)
            return res
        return fp(inner, BDD.true(fsm.bddEnc.DDmanager))
    
    
__fair_gamma_states = {}
def fair_gamma_states(fsm, agents):
    """
    Return the set of states in which agents cannot avoid a fair path.
    
    fsm -- the model
    agents -- a list of agents names
    """
    return cag(fsm, agents, BDD.true(fsm.bddEnc.DDmanager))
    
    # WARNING !!! The following code performs a segmentation fault
    # The problem seems to come from the condition
    #global _fair_gamma_states
    #agents = frozenset(agents)
    #if agents not in _fair_gamma_states:
    #    _fair_gamma_states[agents] = cag(fsm, agents,
    #                                     BDD.true(fsm.bddEnc.DDmanager))
    #return _fair_gamma_states[agents]
    
    
def cex(fsm, agents, phi):
    """
    Return the set of states of fsm satisfying <agents> X phi.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    """
    #phi = phi & fsm.bddEnc.statesInputsMask
    
    return fsm.pre_strat(phi | nfair_gamma_states(fsm, agents), agents)
    

def ceu(fsm, agents, phi, psi):
    """
    Return the set of states of fsm satisfying <agents>[phi U psi].
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    #phi = phi & fsm.bddEnc.statesInputsMask
    #psi = psi & fsm.bddEnc.statesInputsMask
    
    if len(fsm.fairness_constraints) == 0:
        return fp(lambda Z : psi | (phi & fsm.pre_strat(Z, agents)),
                  BDD.false(fsm.bddEnc.DDmanager))
    else:
        nfair = nfair_gamma_states(fsm, agents)
        def inner(Z):
            res = psi
            for f in fsm.fairness_constraints:
                nf = ~f #& fsm.bddEnc.statesMask
                res = res | fsm.pre_strat(fp(lambda Y :
                                             (phi | psi | nfair) &
                                              (Z | nf) &
                                              (psi | fsm.pre_strat(Y, agents)),
                                              BDD.true(fsm.bddEnc.DDmanager)),
                                              agents)
            return (psi | phi | nfair) & res
        return fp(inner, BDD.false(fsm.bddEnc.DDmanager))
    

def cew(fsm, agents, phi, psi):
    """
    Return the set of states of fsm satisfying [agents][phi W psi].
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    """
    #phi = phi & fsm.bddEnc.statesInputsMask
    #psi = psi & fsm.bddEnc.statesInputsMask
    
    nfair = nfair_gamma_states(fsm, agents)
    
    return fp(lambda Y : (psi | phi | nfair) &
                         (psi | fsm.pre_strat(Y, agents)),
              BDD.true(fsm.bddEnc.DDmanager))
    
    
__nfair_gamma_states = {}
def nfair_gamma_states(fsm, agents):
    """
    Return the set of states in which agents cann avoid fair paths.
    
    fsm -- the model
    agents -- a list of agents names
    """
    # NFair_Gamma = not([Gamma] G True) = <Gamma> F False
    agents = frozenset(agents)
    if agents not in __nfair_gamma_states:
        if len(fsm.fairness_constraints) == 0:
            __nfair_gamma_states[agents] = BDD.false(fsm.bddEnc.DDmanager)
        else:
            def inner(Z):
                res = BDD.false(fsm.bddEnc.DDmanager)
                for f in fsm.fairness_constraints:
                    nf = ~f #& fsm.bddEnc.statesMask
                    res = res | fsm.pre_strat(fp(lambda Y :
                                                 (Z | nf) &
                                                 fsm.pre_strat(Y, agents),
                                                BDD.true(fsm.bddEnc.DDmanager)),
                                              agents)
                return res
            __nfair_gamma_states[agents] = fp(inner, 
                                              BDD.false(fsm.bddEnc.DDmanager))
    return __nfair_gamma_states[agents]
