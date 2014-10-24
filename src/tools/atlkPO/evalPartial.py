"""
ATLK with partial observability evaluation functions.
Partial strategies implementation.
"""

from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression
from pynusmv.utils import fixpoint as fp

from ..atlkFO.ast import (TrueExp, FalseExp, Init, Reachable,
                          Atom, Not, And, Or, Implies, Iff, 
                          AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                          nK, nE, nD, nC, K, E, D, C,
                          CEF, CEG, CEX, CEU, CEW, CAF, CAG, CAX, CAU, CAW)

from ..atlkFO.eval import (fair_states, ex, eg, eu, nk, ne, nd, nc)

from . import config

import gc

# A dictionary to keep track of number of strategies for improved algorithm
__strategies = {} 
# A dictionary to keep track of number of filterings for improved algorithm
__filterings = {} 
# A dictionary to keep track of number of ignorings for improved algorithm
__ignorings = {}

class StrategyFound(Exception):
    """Exception used to tell that a strategy has been found for all states."""
    def __init__(self, satisfied):
        self.satisfied = satisfied


def evalATLK(fsm, spec, states=None, variant="SF"):
    """
    Return the BDD representing the subset of "states" of fsm satisfying spec.
    
    fsm -- a MAS representing the system
    spec -- an AST-based ATLK specification
    states -- a BDD-based set of states of fsm (if None, the initial states)
    variant -- the variant of the algorithm to evaluate strategic operators;
               must be
               * "SF" for the standard way: splitting in uniform strategies
                 then filtering winning states,
               * "FS" for the alternating way: filtering winning states, then
                 splitting one conflicting equivalence class, then recurse
               * "FSF" for the filter-split-filter way: filtering winning
                 states then splitting all remaining actions into uniform
                 strategies, then filtering final winning states.
                 
    If variant is not in {"SF", "FS", "FSF"}, the standard "SF" way is used.
    """
    
    if not states:
        states = fsm.init
    
    if type(spec) is TrueExp:
        return BDD.true(fsm.bddEnc.DDmanager) & states
        
    elif type(spec) is FalseExp:
        return BDD.false(fsm.bddEnc.DDmanager) & states
        
    elif type(spec) is Init:
        return fsm.init & states
        
    elif type(spec) is Reachable:
        return fsm.reachable_states & states
    
    elif type(spec) is Atom:
        return eval_simple_expression(fsm, spec.value) & states
        
    elif type(spec) is Not:
        return states - evalATLK(fsm, spec.child, states, variant=variant)
        
    elif type(spec) is And:
        # TODO We can be smarter by evaluating the second member
        # only on states satisfying the first member
        return (evalATLK(fsm, spec.left, states, variant=variant)
                & evalATLK(fsm, spec.right, states, variant=variant))
        
    elif type(spec) is Or:
        # TODO We can be smarter by evaluating the second member
        # only on states not satisfying the first member
        return (evalATLK(fsm, spec.left, states, variant=variant)
                | evalATLK(fsm, spec.right, states, variant=variant))
        
    elif type(spec) is Implies:
        # TODO We can be smarter by evaluating the second member
        # only on states satisfying the first member
        # p -> q = ~p | q
        p = spec.left
        q = spec.right
        newspec = Or(Not(p), q)
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is Iff:
        # p <-> q = (p & q) | (~p & ~q)
        p = spec.left
        q = spec.right
        newspec = Or(And(p, q), And(Not(p), Not(q)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is EX:
        phi = evalATLK(fsm, spec.child, fsm.post(states), variant=variant)
        return ex(fsm, phi) & states
        
    elif type(spec) is AX:
        # AX p = ~EX ~p
        newspec = Not(EX(Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is EG:
        phi = evalATLK(fsm, spec.child, reach(fsm, states), variant=variant)
        return eg(fsm, phi) & states
        
    elif type(spec) is AG:
        # AG p = ~EF ~p (= ~E[ true U ~p ])
        newspec = Not(EF(Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is EU:
        phi1 = evalATLK(fsm, spec.left, reach(fsm, states), variant=variant)
        phi2 = evalATLK(fsm, spec.right, reach(fsm, states), variant=variant)
        return eu(fsm, phi1, phi2) & states
        
    elif type(spec) is AU:
        # A[p U q] = ~E[~q W ~p & ~q] = ~(E[~q U ~p & ~q] | EG ~q)
        p = spec.left
        q = spec.right
        newspec = Not(Or(EU(Not(q), And(Not(p), Not(q))), EG(Not(q))))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is EF:
        # EF p = E[ true U p ]
        newspec = EU(TrueExp(), spec.child)
        return evalATLK(fsm, newspec, states, variant=variant)  
        
    elif type(spec) is AF:
        # AF p = ~EG ~p
        newspec = Not(EG(Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is EW:
        # E[ p W q ] = E[ p U q ] | EG p
        p = spec.left
        q = spec.right
        newspec = Or(EU(p, q), EG(p))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is AW:
        # A[p W q] = ~E[~q U ~p & ~q]
        p = spec.left
        q = spec.right
        newspec = Not(EU(Not(q), And(Not(p), Not(q))))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is nK:
        agent = frozenset({spec.agent.value})
        phi = evalATLK(fsm, spec.child,
                       fsm.equivalent_states(states, agent),
                       variant=variant)
        return states & nk(fsm, spec.agent.value, phi)
        
    elif type(spec) is K:
        # K<'a'> p = ~nK<'a'> ~p
        newspec = Not(nK(spec.agent, Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is nE:
        agents = frozenset([a.value for a in spec.group])
        phi = Eequiv(fsm, states, agents)
        return states & ne(fsm, agents, phi)
        
    elif type(spec) is E:
        # E<g> p = ~nE<g> ~p
        newspec = Not(nE(spec.group, Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is nD:
        agents = frozenset([a.value for a in spec.group])
        phi = Dequiv(fsm, states, agents)
        return states & nd(fsm, agents, phi)
        
    elif type(spec) is D:
        # D<g> p = ~nD<g> ~p
        newspec = Not(nD(spec.group, Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is nC:
        agents = frozenset([a.value for a in spec.group])
        phi = Cequiv(fsm, states, agents)
        return states & nc(fsm, agents, phi)
        
    elif type(spec) is C:
        # C<g> p = ~nC<g> ~p
        newspec = Not(nC(spec.group, Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is CAX:
        # [g] X p = ~<g> X ~p
        newspec = Not(CEX(spec.group, Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is CAG:
        # [g] G p = ~<g> F ~p
        newspec = Not(CEF(spec.group, Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is CAU:
        # [g][p U q] = ~<g>[ ~q W ~p & ~q ]
        newspec = Not(CEW(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right))))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is CAF:
        # [g] F p = ~<g> G ~p
        newspec = Not(CEG(spec.group, Not(spec.child)))
        return evalATLK(fsm, newspec, states, variant=variant)
        
    elif type(spec) is CAW:
        # [g][p W q] = ~<g>[~q U ~p & ~q]
        newspec = Not(CEU(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right))))
        return evalATLK(fsm, newspec, states, variant=variant)
                   
    elif type(spec) in {CEX, CEF, CEG, CEW, CEU}:
        if variant == "SF":
            return eval_strat(fsm, spec, states)
        elif variant == "FS":
            return eval_strat_improved(fsm, spec, states)
        elif variant == "FSF":
            print("[WARNING] evalATLK: no FSF evaluation for now",
                  " basic evaluation used instead.")
            #return eval_strat_FSF(fsm, spec, states)
            return eval_strat(fsm, spec, states)
        else:
            return eval_strat(fsm, spec, states)
        
    else:
        # TODO Generate error
        print("[ERROR] evalATLK: unrecognized specification type", spec)
        return None


def reach(fsm, states):
    """
    Return the set of states reachable from states in fsm.
    
    fsm -- a MAS representing the system
    states -- a BDD representing a set of states of fsm.
    """
    return fp(lambda Z: states | fsm.post(Z), BDD.false(fsm.bddEnc.DDmanager))
    

def Eequiv(fsm, states, agents):
    """
    Return the set of fsm that equivalent to states w.r.t. group knowledge of
    agents.
    
    fsm -- a MAS representing the system
    states -- a BDD representing a set of states of fsm
    agents -- a set of agents names of fsm.
    """
    result = BDD.false(fsm.bddEnc.DDmanager)
    for agent in group:
        result = result | fsm.equivalent_states(states, frozenset({agent}))
    return result
    

def Dequiv(fsm, states, agents):
    """
    Return the set of fsm that equivalent to states w.r.t. distributed
    knowledge of agents.
    
    fsm -- a MAS representing the system
    states -- a BDD representing a set of states of fsm
    agents -- a set of agents names of fsm.
    """
    return fsm.equivalent_states(states, frozenset({agents}))
    

def Cequiv(fsm, states, agents):
    """
    Return the set of fsm that equivalent to states w.r.t. group knowledge of
    agents.
    
    fsm -- a MAS representing the system
    states -- a BDD representing a set of states of fsm
    agents -- a set of agents names of fsm.
    """
    return fp(lambda Z: Eequiv(fsm, states | Z, agents),
              BDD.false(fsm.bddEnc.DDmanager))

def fair_states_sub(fsm, subsystem=None):
    """
    Return the set of fair states of the subsystem.
    
    fsm -- the model;
    subsystem -- if not None, the subsystem defined as a set of state/inputs
                 pairs.
    """
    
    if len(fsm.fairness_constraints) == 0:
        return BDD.true(fsm.bddEnc.DDmanager)
    else:
        return eg_sub(fsm, BDD.true(fsm.bddEnc.DDmanager), subsystem=subsystem)

def reachable_sub(fsm, init=None, subsystem=None):
    """
    Return the set of states reachable from init in the subsystem.
    
    fsm -- the model;
    init -- if not None, the initial states
            otherwise, the initial states of the model;
    subsystem -- if not None, the subsystem to consider
                 otherwise, the original system.
    """
    
    if init is None:
        init = fsm.init
    if subsystem is None:
        subsystem = BDD.true(fsm.bddEnc.DDmanager)
    
    old = BDD.false(fsm.bddEnc.DDmanager)
    new = init & fsm.bddEnc.statesMask
    while old != new:
        old = new
        new = ((old | fsm.post(old, subsystem=subsystem)) &
               fsm.bddEnc.statesMask)
    return new
    
def ex_sub(fsm, phi, subsystem=None):
    """
    Return the set of states of fsm satisfying EX phi in subsystem.
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    subsystem -- if not None, the subsystem defined as a set of state/inputs
                 pairs.
    """
    
    if subsystem is None:
        subsystem = BDD.true(fsm.bddEnc.DDmanager)
    phi = phi.forsome(fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
    
    return fsm.pre(phi & fair_states_sub(fsm, subsystem=subsystem),
                   subsystem=subsystem)
    
    
def eg_sub(fsm, phi, subsystem=None):
    """
    Return the set of states of fsm satisfying EG phi in subsystem.
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    subsystem -- if not None, the subsystem defined as a set of state/inputs
                 pairs.
    """
    
    if subsystem is None:
        subsystem = BDD.true(fsm.bddEnc.DDmanager)
    
    phi = phi.forsome(fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
    
    if len(fsm.fairness_constraints) == 0:
        return (fp(lambda Z : phi & fsm.pre(Z, subsystem=subsystem),
                   BDD.true(fsm.bddEnc.DDmanager)).
                   forsome(fsm.bddEnc.inputsCube))
    else:
        def inner(Z):
            res = phi
            for f in fsm.fairness_constraints:
                res = res & fsm.pre(fp(lambda Y : (Z & f) |
                                       (phi & fsm.pre(Y, subsystem=subsystem)),
                                       BDD.false(fsm.bddEnc.DDmanager)),
                                    subsystem=subsystem)
            return res
        return (fp(inner, BDD.true(fsm.bddEnc.DDmanager))
                .forsome(fsm.bddEnc.inputsCube))
    
    
def eu_sub(fsm, phi, psi, subsystem=None):
    """
    Return the set of states of fsm satisfying E[ phi U psi ] in subsystem.
    
    fsm -- a MAS representing the system
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    subsystem -- if not None, the subsystem defined as a set of state/inputs
                 pairs.
    """
    
    phi = phi.forsome(fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
    psi = psi.forsome(fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
    return fp(lambda X : (psi & fair_states_sub(fsm, subsystem=subsystem) &
                          fsm.reachable_states) |
                               (phi & ex_sub(fsm, X, subsystem=subsystem)),
                    BDD.false(fsm.bddEnc.DDmanager))


def cex_si(fsm, agents, phi, strat=None):
    """
    Return the set of state/inputs pairs of strat satisfying <agents> X phi
    under full observability in strat.
    If strat is None, strat is considered true.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    strat -- a BDD representing allowed state/inputs pairs, or None
    """
    if not strat:
        strat = BDD.true(fsm.bddEnc.DDmanager)
    
    phi = phi & fsm.bddEnc.statesInputsMask
    
    return fsm.pre_strat_si(phi | nfair_gamma_si(fsm, agents, strat),
                            agents, strat)
    

def ceu_si(fsm, agents, phi, psi, strat=None):
    """
    Return the set of state/inputs pairs of strat satisfying
    <agents>[phi U psi] under full observability in strat.
    If strat is None, strat is considered true.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    strat -- a BDD representing allowed state/inputs pairs, or None
    
    """
    if not strat:
        strat = BDD.true(fsm.bddEnc.DDmanager)
    
    phi = phi & fsm.bddEnc.statesInputsMask & strat
    psi = psi & fsm.bddEnc.statesInputsMask & strat
    
    if len(fsm.fairness_constraints) == 0:
        return fp(lambda Z : psi | (phi & fsm.pre_strat_si(Z, agents, strat)),
                  BDD.false(fsm.bddEnc.DDmanager))
    else:
        nfair = nfair_gamma_si(fsm, agents, strat)
        def inner(Z):
            res = psi
            for f in fsm.fairness_constraints:
                nf = ~f & fsm.bddEnc.statesMask & strat
                res = res | fsm.pre_strat_si(fp(lambda Y :
                                                 (phi | psi | nfair) &
                                                 (Z | nf) &
                                                 (psi |
                                                  fsm.pre_strat_si(Y, agents,
                                                                   strat)
                                                 ),
                                              BDD.true(fsm.bddEnc.DDmanager)),
                                              agents, strat)
            return (psi | phi | nfair) & res
        return fp(inner, BDD.false(fsm.bddEnc.DDmanager))
    

def cew_si(fsm, agents, phi, psi, strat=None):
    """
    Return the set of state/inputs pairs of strat satisfying
    <agents>[phi W psi] under full observability in strat.
    If strat is None, strat is considered true.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    psi -- a BDD representing the set of states of fsm satisfying psi
    strat -- a BDD representing allowed state/inputs pairs, or None
    
    """
    if not strat:
        strat = BDD.true(fsm.bddEnc.DDmanager)
    
    phi = phi & fsm.bddEnc.statesInputsMask & strat
    psi = psi & fsm.bddEnc.statesInputsMask & strat
    
    nfair = nfair_gamma_si(fsm, agents, strat)
    
    return fp(lambda Y : (psi | phi | nfair) &
                         (psi | fsm.pre_strat_si(Y, agents, strat)),
              BDD.true(fsm.bddEnc.DDmanager))
    
    
def ceg_si(fsm, agents, phi, strat=None):
    """
    Return the set of state/inputs pairs of strat satisfying <agents> G phi
    under full observability in strat.
    If strat is None, strat is considered true.
    
    fsm -- a MAS representing the system
    agents -- a list of agents names
    phi -- a BDD representing the set of states of fsm satisfying phi
    strat -- a BDD representing allowed state/inputs pairs, or None
    
    """
    if not strat:
        strat = BDD.true(fsm.bddEnc.DDmanager)
    
    phi = phi & fsm.bddEnc.statesInputsMask & strat
    
    nfair = nfair_gamma_si(fsm, agents, strat)
    
    return fp(lambda Y : (phi | nfair) & fsm.pre_strat_si(Y, agents, strat),
              BDD.true(fsm.bddEnc.DDmanager))


def nfair_gamma_si(fsm, agents, strat=None):
    """
    Return the set of state/inputs pairs of strat
    in which agents can avoid a fair path in strat.
    If strat is None, it is considered true.
    
    fsm -- the model
    agents -- a list of agents names
    strat -- a BDD representing allowed state/inputs pairs, or None
    
    """
    if not strat:
        strat = BDD.true(fsm.bddEnc.DDmanager)
    
    if len(fsm.fairness_constraints) == 0:
        return BDD.false(fsm.bddEnc.DDmanager)
    else:
        def inner(Z):
            res = BDD.false(fsm.bddEnc.DDmanager)
            for f in fsm.fairness_constraints:
                nf = ~f & fsm.bddEnc.statesMask & strat
                res = res | fsm.pre_strat_si(fp(lambda Y :
                                                 (Z | nf) &
                                                 fsm.pre_strat_si(Y, agents,
                                                                  strat),
                                             BDD.true(fsm.bddEnc.DDmanager)),
                                             agents, strat)
            return res
        return fp(inner, BDD.false(fsm.bddEnc.DDmanager))
        
        
def split_reach(fsm, agents, pustrat, subsystem=None):
    """
    Return the set of maximal uniform strategies extending pustrat.
    
    fsm -- a MAS representing the system;
    agents -- a set of agents names;
    pustrat -- a partial uniform strategy represented as BDD-based set of
               moves (state/action pairs);
    subsystem -- the subsystem in which building the strategies;
                 the full system if None.
    
    """
    
    if subsystem is None:
        subsystem = BDD.true(fsm.bddEnc.DDmanager)
    
    new = (fsm.post(pustrat, subsystem) -
           pustrat.forsome(fsm.bddEnc.inputsCube)).forsome(
                                 fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask

    if new.is_false():
        yield pustrat
    
    else:
        for npustrat in split(fsm, new & fsm.protocol(agents), agents,
                              pustrat):
            for strat in split_reach(fsm, agents, pustrat | npustrat, 
                                     subsystem):
                yield strat
        

def split_one(fsm, strats, gamma, pustrat):
    """
    Split one equivalence class of strats and return triples composed of
    the common non-conflicting part already encountered,
    a split and the rest of strats to split.
    Restrict to actions allowed in pustrat, that is, if a state of strats
    is equivalent to one of pustrat, only allows actions given by pustrat.
    
    fsm -- the model
    strats -- a BDD representing a set of states/inputs pairs
    gamma -- a set of agents of fsm
    pustrat -- a partial uniform strategy represented
               by a BDD-based set of moves
    
    Return a generator of all triples of common parts, splits
    and rest of strats.
    
    """
    if strats.is_false():
        yield (strats, strats, strats)
        return
        
    else:
        ngamma_cube = fsm.bddEnc.inputsCube - fsm.inputs_cube_for_agents(gamma)
        common = BDD.false(fsm.bddEnc.DDmanager)
        
        while strats.isnot_false():
            # Get one equivalence class
            si = fsm.pick_one_state_inputs(strats)
            s = si.forsome(fsm.bddEnc.inputsCube)
            eqs = fsm.equivalent_states(s, gamma)
            eqcl = strats & eqs
            
            # Remove it from strats
            strats = strats - eqcl
            
            # The current equivalence class is conflicting
            if((eqcl - (eqcl & si.forsome(fsm.bddEnc.statesCube |
                                          ngamma_cube)))
                .isnot_false()):
                # Split eqcl into non-conflicting subsets
                while eqcl.isnot_false():
                    si = fsm.pick_one_state_inputs(eqcl)
                    
                    ncss = eqcl & si.forsome(fsm.bddEnc.statesCube | 
                                             ngamma_cube)
                    
                    eqcl = eqcl - ncss
                    
                    # Take pustrat into account
                    # eq is the set of states equivalent to ncss states
                    eq = fsm.equivalent_states(
                                     ncss.forsome(fsm.bddEnc.inputsCube),
                                     gamma)
                    if (eq & pustrat).isnot_false():
                        # Some states are equivalent in ncss and pustrat
                        
                        # Check that the strategy is compatible
                        lstrat = eq & pustrat # the strategy in pustrat
                        action = lstrat.forsome(fsm.bddEnc.statesCube
                                                | ngamma_cube) # The action                       
                        # The strategy is compatible if ncss proposes the same
                        # action as lstrat
                        if (action & ncss).isnot_false():
                            yield (common, ncss, strats)
                        # Otherwise the strategy is incompatible, discard it
                    
                    # No new state is equivalent to a pustrat one
                    else:
                        yield (common, ncss, strats)
                return
            
            else:
                # Add equivalence class to common
                common = common | eqcl
        
        # strats is false, everything is in common
        yield (common, strats, strats) 
        
        
def split(fsm, strats, gamma, pustrat=None):
    """
    Split strats into all its non-conflicting greatest subsets.
    Restrict to actions allowed in pustrat, that is, if a state of strats
    is equivalent to one of pustrat, only allows actions given by pustrat.
    
    fsm -- the model
    strats -- a BDD representing a set of states/inputs pairs
    gamma -- a set of agents of fsm
    pustrat -- a partial uniform strategy represented
               by a BDD-based set of moves
    
    Return a generator of all non-conflicting greatest subsets of strats.
    
    """
    if not pustrat:
        pustrat = BDD.false(fsm.bddEnc.DDmanager)
    
    strats = strats & fsm.bddEnc.statesInputsMask
    pustrat = pustrat & fsm.bddEnc.statesInputsMask
    
    if strats.is_false():
        yield strats
    else:
        # Split one equivalence class
        for common, splitted, rest in split_one(fsm, strats, gamma, pustrat):
            for strat in split(fsm, rest, gamma, pustrat):
                yield (common | strat | splitted)


def all_equiv_sat(fsm, winning, agents):
    """
    Return the states s of winning such that all states
    indistinguishable from s by agents are in winning.
    
    fsm -- a MAS representing the system;
    winning -- a BDD representing a set of states of fsm;
    agents -- a set of agents.
    
    """
    # Get states for which all states belong to winning
    # wineq is the set of states for which all equiv states are in winning
    nwinning = ~winning & fsm.bddEnc.statesInputsMask
    return ~(fsm.equivalent_states(nwinning &
             fsm.reachable_states, frozenset(agents))) & winning
             
             
def filter_strat(fsm, spec, states, strat=None, variant="SF"):
    """
    Returns the subset SA of strat (or the whole system if strat is None),
    state/action pairs of fsm, such that there is a strategy to satisfy spec
    in fsm.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator;
            the operator is CEX, CEG, CEF, CEU or CEW.
    states -- the set of states for which the filtering matters;
    strat -- the subset of the system to consider.
    variant -- the variant of the algorithm to evaluate strategic operators;
               must be
               * "SF" for the standard way: splitting in uniform strategies
                 then filtering winning states,
               * "FS" for the alternating way: filtering winning states, then
                 splitting one conflicting equivalence class, then recurse
               * "FSF" for the filter-split-filter way: filtering winning
                 states then splitting all remaining actions into uniform
                 strategies, then filtering final winning states.
                 
    If variant is not in {"SF", "FS", "FSF"}, the standard "SF" way is used.
    """
    if strat is None:
        strat = BDD.true(fsm.bddEnc.DDmanager)
    
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    
    # Filtering
    if type(spec) is CEX:
        phi = evalATLK(fsm, spec.child, fsm.post(states & strat), 
                       variant=variant)
        winning = cex_si(fsm, agents, phi, strat)

    elif type(spec) is CEG:
        phi = evalATLK(fsm, spec.child, strat.forsome(fsm.bddEnc.inputsCube), 
                       variant=variant)
        winning = ceg_si(fsm, agents, phi, strat)

    elif type(spec) is CEU:
        phi1 = evalATLK(fsm, spec.left, strat.forsome(fsm.bddEnc.inputsCube), 
                       variant=variant)
        phi2 = evalATLK(fsm, spec.right, strat.forsome(fsm.bddEnc.inputsCube), 
                       variant=variant)
        winning = ceu_si(fsm, agents, phi1, phi2, strat)

    elif type(spec) is CEF:
        # <g> F p = <g>[true U p]
        phi = evalATLK(fsm, spec.child, strat.forsome(fsm.bddEnc.inputsCube), 
                       variant=variant)
        winning = ceu_si(fsm, agents, BDD.true(fsm.bddEnc.DDmanager),
                         phi, strat)

    elif type(spec) is CEW:
        phi1 = evalATLK(fsm, spec.left, strat.forsome(fsm.bddEnc.inputsCube), 
                       variant=variant)
        phi2 = evalATLK(fsm, spec.right, strat.forsome(fsm.bddEnc.inputsCube), 
                       variant=variant)
        winning = cew_si(fsm, agents, phi1, phi2, strat)
    
    return winning & fsm.bddEnc.statesInputsMask & fsm.protocol(agents)


def eval_strat(fsm, spec, states):
    """
    Return the BDD representing the subset of states satisfying spec.
    spec is a strategic operator <G> pi.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic (<g>)
            operator;
    states -- a BDD representing a set of states of fsm.
    
    """
    
    agents = {atom.value for atom in spec.group}
    
    if config.debug:
        print("Evaluating partial strategies for ", spec)
    
    # Extend with equivalent states
    states = (fsm.equivalent_states(states, agents) &
              fsm.reachable_states)
    
    # Pre-filtering out losing states and actions
    if config.partial.filtering:
        subsystem = filter_strat(fsm, spec, states, variant="SF")
    else:
        subsystem = fsm.protocol(agents)
    # if filtering is enabled, subsystem is the part of the system in which
    # states can win
    
    # if filtering is enabled, we can remove, from states, the set of states
    # for which not all equivalent states are in subsystem. Indeed, if a state
    # is not in subsystem, there is not strategy in this state to win,
    # thus no uniform strategy to win, thus its entire equivalence class
    # cannot win
    states = (states & all_equiv_sat(fsm,
                                     subsystem.forsome(fsm.bddEnc.inputsCube), 
                                     agents))
    
    sat = BDD.false(fsm.bddEnc.DDmanager)
    
    all_states = states
    
    nbstrats = 0
    
    while all_states.isnot_false():
        
        # ----- Separation of state space -------------------------------------
        
        # Put a subset of all_states in states and remove it from all_states
        if config.partial.separation.type is None:
            states = all_states
            
        if config.partial.separation.type == "random":
            state = fsm.pick_one_state(all_states)
            states = (fsm.equivalent_states(state, agents) &
                      fsm.reachable_states & all_states)
                      
        if config.partial.separation.type == "reach":
            reached = fsm.init
            while (reached & all_states).is_false():
                # Should not loop infinitely since we only are interested in
                # reachable states
                # WARNING We cannot restrict to subsystem to get the post-image
                # since we are not sure that all_states are reachable in the
                # subsystem anymore
                reached = reached | fsm.post(reached)
            state = fsm.pick_one_state(reached & all_states)
            states = (fsm.equivalent_states(state, agents) &
                      fsm.reachable_states & all_states)
        
        # ---------------------------------------------------------------------
        
        
        # Remove states from all_states
        all_states = all_states - states
    
        orig_states = states
        remaining = states
    
        while remaining.isnot_false():
            # Extend states with equivalent ones
            states = (fsm.equivalent_states(remaining, agents) &
                      fsm.reachable_states)
        
            remaining_size = fsm.count_states(remaining)
            
            # Go through all strategies
            for strat in (strat
                          for pustrat in split(fsm, states & subsystem, agents)
                          for strat
                          in split_reach(fsm, agents, pustrat,subsystem)):            
                # Check the strategy
                nbstrats += 1
                winning = (filter_strat(fsm, spec, states, strat, variant="SF").
                           forsome(fsm.bddEnc.inputsCube))
                old_sat = sat
                sat = sat | (all_equiv_sat(fsm, winning, agents) & orig_states)
            
            
                # ----- EARLY TERMINATION -------------------------------------
            
                # Early termination if sat contains all requested states
                if config.partial.early.type == "full" and orig_states <= sat:
                    remaining = BDD.false(fsm.bddEnc.DDmanager)
                    break
            
                # Early termination of sat grows
                if config.partial.early.type == "partial" and old_sat < sat:
                    if config.debug:
                        print("Partial strategies: sat grows ({} strateg{})"
                              .format(nbstrats, "ies" if nbstrats > 1 else "y"))
                          
                    remaining = remaining - sat
                    break
            
                # Early termination if remaining states decrease enough
                if config.partial.early.type == "threshold":
                    remaining = remaining - sat
                    rem_count = fsm.count_states(remaining)
                    if (rem_count <= 
                        remaining_size * config.partial.early.threshold):
                        if config.debug:
                            print("Partial strategies:",
                                  "remaining states decrease:",
                                  "{} => {}".format(remaining_size, rem_count))
                              
                        remaining_size = rem_count
                        break
            
                # -------------------------------------------------------------
                
                
                if config.debug and nbstrats % 1000 == 0:
                    print("Partial strategies: {} strateg{} checked so far"
                          .format(nbstrats, "ies" if nbstrats > 1 else "y"))
                
                
                # ----- Garbage collection ------------------------------------
                if (config.garbage.type == "each" or
                    (config.garbage.type == "step"
                        and nbstrats % config.garbage.step == 0)):
                    gc.collect()
        
            else:
                # All strategies have been checked, the remaining states do not
                # satisfy the specification
                remaining = BDD.false(fsm.bddEnc.DDmanager)
        
        
        # Remove from remaining subsets to check the ones we already have the
        # truth value
        all_states = all_states - sat
    
    # DEBUG Print number of strategies
    if config.debug:
        print("Partial strategies: {} strateg{} generated"
              .format(nbstrats, "ies" if nbstrats > 1 else "y"))
    
    return sat


# -----------------------------------------------------------------------------
# ----- DEPRECATED ------------------------------------------------------------
# -----------------------------------------------------------------------------

def eval_strat_improved_old(fsm, spec, states):
    """
    Return the BDD representing the subset of states satisfying spec.
    spec is a strategic operator <G> pi.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic (<g>) operator;
    states -- a BDD representing a set of states of fsm.
    """
    
    if config.debug:
        print("Evaluating partial strategies for ", spec)
        
    __strategies[spec] = 0
    __filterings[spec] = 0
    __ignorings[spec] = 0
    
    agents = {atom.value for atom in spec.group}
    gamma = agents
    ngamma_cube = fsm.bddEnc.inputsCube - fsm.inputs_cube_for_agents(gamma)
    
    all_states = states
    sat = BDD.false(fsm.bddEnc.DDmanager)
    
    while all_states.isnot_false():
        
        if config.debug:
            print("Starting with a new subset of states.")
        
        # ----- Separation of state space -------------------------------------
        
        # Put a subset of all_states in states and remove it from all_states
        if config.partial.separation.type is None:
            states = all_states
            
        if config.partial.separation.type == "random":
            state = fsm.pick_one_state(all_states)
            states = (fsm.equivalent_states(state, agents) &
                      fsm.reachable_states & all_states)
                      
        if config.partial.separation.type == "reach":
            reached = fsm.init
            while (reached & all_states).is_false():
                # Should not loop infinitely since we only are interested in
                # reachable states
                # WARNING We cannot restrict to subsystem to get the post-image
                # since we are not sure that all_states are reachable in the
                # subsystem anymore
                reached = reached | fsm.post(reached)
            state = fsm.pick_one_state(reached & all_states)
            states = (fsm.equivalent_states(state, agents) &
                      fsm.reachable_states & all_states)
        
        # ---------------------------------------------------------------------
        
        
        # Remove states from all_states
        all_states = all_states - states
    
        try:
            sat = sat | eval_strat_recur(fsm, spec, states)
        except StrategyFound as e:
            sat = sat | e.satisfied
        
        all_states = all_states - sat
    
    if config.debug:
        print("{} strategies, {} filterings, {} ignorings computed."
              .format(__strategies[spec], __filterings[spec],
                      __ignorings[spec]))
                      
    return sat
    


def eval_strat_recur(fsm, spec, states, toSplit=None, toKeep=None):
    """
    Return the BDD representing the subset of states satisfying spec.
    spec is a strategic operator <G> pi.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic (<g>)
            operator;
    states -- a BDD representing a set of states of fsm;
    toSplit -- the part of the strategy to split (not necessarily uniform).
    toKeep -- the part of the strategy to keep (uniform).
    
    """
    
    global __filterings, __strategies, __ignorings
    
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    gamma = agents
    ngamma_cube = fsm.bddEnc.inputsCube - fsm.inputs_cube_for_agents(gamma)
    
    if not toSplit and not toKeep:
        # Limit the strategies to consider to the states reachable from states
        toSplit = fsm.protocol(agents) & reach(fsm, states)
        toKeep = BDD.false(fsm.bddEnc.DDmanager)
    
    orig_states = states
    states = (fsm.equivalent_states(states, agents) & fsm.reachable_states)
    
    winning = filter_strat(fsm, spec, states, strat=toSplit | toKeep,
                           variant="FS")
    __filterings[spec] += 1
    
    toSplit = toSplit & winning
    toKeep = toKeep & winning
    
    # Get one conflicting equivalence class
    if winning.is_false(): # no state/inputs pairs are winning => return false
        return winning
    
    
    # Ignore if no state can be winning, that is, there are no full
    # equivalence class of states in winning
    losing = states - winning.forsome(fsm.bddEnc.inputsCube)
    if (states - fsm.equivalent_states(losing, agents)).is_false():
        if config.debug:
            print("Ignoring strategies.")
        __ignorings[spec] += 1
        return BDD.false(fsm.bddEnc.DDmanager)
    
    
    # Split one conflicting equivalence class
    for common, splitted, rest in split_one(fsm, toSplit, gamma,
                                            toSplit | toKeep):
        
        if splitted.is_false():
            # No conflicting classes, return states that are winning for all eq
            common = (common | toKeep).forsome(fsm.bddEnc.inputsCube)
            sat = sat | all_equiv_sat(fsm, common, agents)
            global __strategies
            __strategies[spec] += 1
            
            # Early termination if sat contains all requested states
            if config.partial.early.type == "full" and orig_states <= sat:
                if config.debug:
                    print("Full strategy found.")
                raise StrategyFound(sat)
            
            # Collect to avoid memory overflow
            if (config.garbage.type == "each" or config.garbage.type == "step"
                and __strategies[spec] % config.garbage.step == 0):
                
                gc.collect()
        
        else:
            sat = sat | eval_strat_recur(fsm, spec, states,
                                         toSplit=rest,
                                         toKeep=toKeep | common | splitted)
    
    return sat

# -----------------------------------------------------------------------------
# ----- END OF DEPRECATED -----------------------------------------------------
# -----------------------------------------------------------------------------


def eval_univ(fsm, spec, states, subsystem=None, variant="FS"):
    """
    Given spec a universal CTL formula, return the subset of states satisfying
    spec in the given subsystem.
    
    fsm -- the model;
    spec -- a universal CTL formula;
    states -- a subset of the states of the model;
    subsystem -- if not None, the subsystem in which evaluate the formula;
    variant -- the variant to evaluate sub-formulas.
    """
    if subsystem is None:
        subsystem = BDD.true(fsm.bddEnc.DDmanager)
    
    interesting = subsystem.forsome(fsm.bddEnc.inputsCube)
    
    if type(spec) is AX:
        phi = evalATLK(fsm, spec.child,
                       fsm.post(states, subsystem=subsystem),
                       variant=variant)
        winning = ~ex_sub(fsm, ~phi, subsystem)
    
    elif type(spec) is AG:
        phi = evalATLK(fsm, spec.child,
                       interesting,
                       variant=variant)
        winning = ~eu_sub(fsm, BDD.true(fsm.bddEnc.DDmanager), ~phi, subsystem)
    
    elif type(spec) is AU:
        phi1 = evalATLK(fsm, spec.left,
                        interesting,
                        variant=variant)
        phi2 = evalATLK(fsm, spec.right,
                        interesting,
                        variant=variant)
        winning = ~(eu_sub(fsm, ~phi2, ~phi1 & ~phi2, subsystem) |
                    eg_sub(fsm, ~phi2, subsystem))
    
    elif type(spec) is AF:
        phi = evalATLK(fsm, spec.child,
                       interesting,
                       variant=variant)
        winning = ~eg_sub(fsm, ~phi, subsystem)
    
    elif type(spec) is AW:
        phi1 = evalATLK(fsm, spec.left,
                        interesting,
                       variant=variant)
        phi2 = evalATLK(fsm, spec.right,
                        interesting,
                        variant=variant)
        winning = ~eu_sub(fsm, ~phi2, ~phi1 & ~phi2, subsystem)
    
    return winning & states & fsm.bddEnc.statesMask
    

def strat_to_univ(spec):
    """
    Return the universal formula sharing the same path operator
    as the strategic formula spec.
    
    spec -- a formula with a strategic top operator
    """
    if type(spec) is CEX:
        return AX(spec.child)
    elif type(spec) is CEF:
        return AF(spec.child)
    elif type(spec) is CEG:
        return AG(spec.child)
    elif type(spec) is CEU:
        return AU(spec.left, spec.right)
    elif type(spec) is CEW:
        return AW(spec.left, spec.right)

def eval_strat_improved(fsm, spec, states):
    """
    Return the BDD representing the subset of states satisfying spec.
    spec is a strategic operator <G> pi.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic (<g>)
            operator;
    states -- a BDD representing a set of states of fsm.
    """
    
    assert(len(config.partial.alternate.type & {"univ", "strat"}) > 0)
    
    __strategies[spec] = 0
    __filterings[spec] = 0
    
    gamma = {atom.value for atom in spec.group}
    sat = BDD.false(fsm.bddEnc.DDmanager)
    
    states = fsm.equivalent_states(states, gamma) & fsm.reachable_states
    remaining = states
    
    for pstrat in split(fsm, states & fsm.protocol(gamma), gamma):
        newsat = eval_strat_alternate(fsm, spec, remaining, pstrat)
        sat = sat | newsat
        remaining = remaining - sat
        if sat == states:
            break # Early termination
    
    if config.debug:
        nbs = __strategies[spec]
        nbf = __filterings[spec]
        print("{} strateg{}, {} filtering{} computed."
              .format(nbs, "y" if nbs <= 1 else "ies",
                      nbf, "" if nbf <= 1 else "s"))
    
    return sat

def eval_strat_alternate(fsm, spec, states, pstrat):
    """
    Return the BDD representing the subset of states s' such that there exists
    a uniform maximal partial strategy extending pstrat that is winning for
    spec = <gamma> psi in [s']_gamma.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic (<g>)
            operator;
    states -- a BDD representing a set of states of fsm;
    pstrat -- a uniform partial strategy reachable from states.
    """
    
    # TODO Change such that it works to only filter with universal or strategic
    # evaluation
    
    gamma = {atom.value for atom in spec.group}
    fullpstrat = (pstrat |
                  (fsm.protocol(gamma) -
                   pstrat.forsome(fsm.bddEnc.inputsCube)))
    
    # Get losing states
    if "strat" in config.partial.alternate.type:
        fullpstrat = fullpstrat & reachable_sub(fsm, states, fullpstrat)
        
        notlosing = filter_strat(fsm, spec, states, fullpstrat, variant="FS")
        notlosing = notlosing.forsome(fsm.bddEnc.inputsCube)
        notlosing = notlosing & states
        lose = states - all_equiv_sat(fsm, notlosing, gamma)
        
        states = states - lose
    else:
        lose = BDD.false(fsm.bddEnc.DDmanager)
    
    # Get winning states
    if "univ" in config.partial.alternate.type:
        fullpstrat = fullpstrat & reachable_sub(fsm, states, fullpstrat)
        
        swin = eval_univ(fsm, strat_to_univ(spec), states, fullpstrat,
                           variant="FS")
        win = all_equiv_sat(fsm, swin, gamma) & states
        
        states = states - win
    else:
        win = BDD.false(fsm.bddEnc.DDmanager)
    
    __filterings[spec] += 1
    
    # Check whether there are still states to decide
    if states.is_false():
        __strategies[spec] += 1
        
        # Collect to avoid memory overflow
        if (config.garbage.type == "each" or config.garbage.type == "step"
            and __strategies[spec] % config.garbage.step == 0):
            
            gc.collect()
        
        return win
    
    # Expand the partial strategy to decide for the remaining states
    else:
        # Expand the partial strategy: get new reachable states
        new = (fsm.post(pstrat) - 
               pstrat.forsome(fsm.bddEnc.inputsCube)).forsome(
               fsm.bddEnc.inputsCube) & fsm.bddEnc.statesMask
        if new.is_false():
            # No new states, pstrat is already maximal
            # Winning states in win if win has some meaning
            # otherwise lose has some meaning and winning states are the others
            if "univ" in config.partial.alternate.type:
                return win
            else:
                return states - lose
        
        remaining = states
        
        # Split the new moves
        for npstrat in split(fsm, new & fsm.protocol(gamma), gamma, pstrat):
            newwin = eval_strat_alternate(fsm, spec, remaining,
                                          pstrat | npstrat)
            win = win | newwin
            remaining = remaining - win
            if win == states:
                # Early termination when we know the truth value of all states
                break
        return win

# -----------------------------------------------------------------------------
# CACHING FUNCTIONS AND MANIPULATIONS
# -----------------------------------------------------------------------------

__evalATLK_cache = {}
__orig_evalATLK = evalATLK
def __cached_evalATLK(fsm, spec, states=None, variant="SF"):
    if config.partial.caching:
        if states is None:
            states = fsm.init
        
        if (fsm, spec) in __evalATLK_cache.keys():
            sat, unsat = __evalATLK_cache[(fsm, spec)]
        else:
            false = BDD.false(fsm.bddEnc.DDmanager)
            sat, unsat = false, false

        remaining = states - (sat + unsat)
    
        if remaining.isnot_false():
            remsat = __orig_evalATLK(fsm, spec, remaining, variant)
            remunsat = remaining - remsat
            __evalATLK_cache[(fsm, spec)] = (sat + remsat, unsat + remunsat)
        else:
            remsat = BDD.false(fsm.bddEnc.DDmanager)
    
        return sat + remsat
    
    else:
        return __orig_evalATLK(fsm, spec, states, variant)

evalATLK = __cached_evalATLK