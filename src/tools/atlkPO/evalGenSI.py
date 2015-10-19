"""
ATLK with partial observability evaluation functions.

These evaluation functions reimplement the algorithms with generators to
decrease the memory consumption of the implementation.
"""

import gc
from functools import reduce

from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression
from pynusmv.utils import fixpoint as fp
from pynusmv.exception import PyNuSMVError

from ..mas.mas import Agent, Group

from ..atlkFO.ast import (TrueExp, FalseExp, Init, Reachable,
                          Atom, Not, And, Or, Implies, Iff, 
                          AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                          nK, nE, nD, nC, K, E, D, C,
                          CEF, CEG, CEX, CEU, CEW, CAF, CAG, CAX, CAU, CAW)
                          

from ..atlkFO.eval import (fair_states, ex, eg, eu, nk, ne, nd, nc)

from . import config


# A dictionary to keep track of number of strategies for improved algorithm
__strategies = {} 
# A dictionary to keep track of number of filterings for improved algorithm
__filterings = {} 


def evalATLK(fsm, spec, variant="SF", semantics="group"):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    
    fsm -- a MAS representing the system
    spec -- an AST-based ATLK specification
    variant -- the variant of the algorithm to evaluate strategic operators;
               must be
               * "SF" for the standard way: splitting in uniform strategies then
                 filtering winning states,
               * "FS" for the alternating way: filtering winning states, then
                 splitting one conflicting equivalence class, then recurse
               * "FSF" for the filter-split-filter way: filtering winning states
                 then splitting all remaining actions into uniform strategies,
                 then filtering final winning states.
    semantics -- the semantics to use for starting point and strategy point
                 equivalence; must be
                 * "group" for the original ATLK_irF semantics considering
                   the group as a single agent (distributed knowledge is used)
                 * "individual" for the original ATL_ir semantics considering
                   the group as individual agents (individual knowledge is
                   used)
    
    If variant is not in {"SF", "FS", "FSF"}, the standard "SF" way is used.
    If semantics is not in {"group", "individual"}, the "group" semantics is
    used.
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
        return ~evalATLK(fsm, spec.child, variant=variant, semantics=semantics)
        
    elif type(spec) is And:
        return (evalATLK(fsm, spec.left, variant=variant, semantics=semantics)
                & evalATLK(fsm, spec.right, variant=variant,
                           semantics=semantics))
        
    elif type(spec) is Or:
        return (evalATLK(fsm, spec.left, variant=variant, semantics=semantics)
                | evalATLK(fsm, spec.right, variant=variant,
                           semantics=semantics))
        
    elif type(spec) is Implies:
        # a -> b = ~a | b
        return ((~evalATLK(fsm, spec.left, variant=variant,
                           semantics=semantics))
                 | evalATLK(fsm, spec.right, variant=variant,
                            semantics=semantics))
        
    elif type(spec) is Iff:
        # a <-> b = (a & b) | (~a & ~b)
        l = evalATLK(fsm, spec.left, variant=variant, semantics=semantics)
        r = evalATLK(fsm, spec.right, variant=variant, semantics=semantics)
        return (l & r) | ((~l) & (~r))
        
    elif type(spec) is EX:
        return ex(fsm, evalATLK(fsm, spec.child, variant=variant,
                                semantics=semantics))
        
    elif type(spec) is AX:
        # AX p = ~EX ~p
        return ~ex(fsm, ~evalATLK(fsm, spec.child, variant=variant,
                                  semantics=semantics))
        
    elif type(spec) is EG:
        return eg(fsm, evalATLK(fsm, spec.child, variant=variant, 
                                semantics=semantics))
        
    elif type(spec) is AG:
        # AG p = ~EF ~p = ~E[ true U ~p ]
        return ~eu(fsm,
                   BDD.true(fsm.bddEnc.DDmanager),
                   ~evalATLK(fsm, spec.child, variant=variant,
                             semantics=semantics))
        
    elif type(spec) is EU:
        return eu(fsm, evalATLK(fsm, spec.left, variant=variant,
                                semantics=semantics),
                       evalATLK(fsm, spec.right, variant=variant, 
                                semantics=semantics))
        
    elif type(spec) is AU:
        # A[p U q] = ~E[~q W ~p & ~q] = ~(E[~q U ~p & ~q] | EG ~q)
        p = evalATLK(fsm, spec.left, variant=variant, semantics=semantics)
        q = evalATLK(fsm, spec.right, variant=variant, semantics=semantics)
        equpq = eu(fsm, ~q, ~q & ~p)
        egq = eg(fsm, ~q)
        return ~(equpq | egq)
        
    elif type(spec) is EF:
        # EF p = E[ true U p ]
        return eu(fsm,
                  BDD.true(fsm.bddEnc.DDmanager),
                  evalATLK(fsm, spec.child, variant=variant,
                           semantics=semantics))
        
    elif type(spec) is AF:
        # AF p = ~EG ~p
        return ~eg(fsm, ~evalATLK(fsm, spec.child, variant=variant, 
                                  semantics=semantics))
        
    elif type(spec) is EW:
        # E[ p W q ] = E[ p U q ] | EG p
        return (eu(fsm, evalATLK(fsm, spec.left, variant=variant,
                                 semantics=semantics),
                        evalATLK(fsm, spec.right, variant=variant,
                                 semantics=semantics)) |
                eg(fsm, evalATLK(fsm, spec.left, variant=variant,
                                 semantics=semantics)))
        
    elif type(spec) is AW:
        # A[p W q] = ~E[~q U ~p & ~q]
        p = evalATLK(fsm, spec.left, variant=variant, semantics=semantics)
        q = evalATLK(fsm, spec.right, variant=variant, semantics=semantics)
        return ~eu(fsm, ~q, ~p & ~q)
        
    elif type(spec) is nK:
        return nk(fsm, spec.agent.value, evalATLK(fsm, spec.child,
                                                  variant=variant,
                                                  semantics=semantics))
        
    elif type(spec) is K:
        # K<'a'> p = ~nK<'a'> ~p
        return ~nk(fsm, spec.agent.value, ~evalATLK(fsm, spec.child,
                                                    variant=variant,
                                                    semantics=semantics))
        
    elif type(spec) is nE:
        return ne(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child, variant=variant,
                           semantics=semantics))
        
    elif type(spec) is E:
        # E<g> p = ~nE<g> ~p
        return ~ne(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child, variant=variant,
                             semantics=semantics))
        
    elif type(spec) is nD:
        return nd(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child, variant=variant,
                           semantics=semantics))
        
    elif type(spec) is D:
        # D<g> p = ~nD<g> ~p
        return ~nd(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child, variant=variant,
                             semantics=semantics))
        
    elif type(spec) is nC:
        return nc(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child, variant=variant,
                           semantics=semantics))
        
    elif type(spec) is C:
        # C<g> p = ~nC<g> ~p
        return ~nc(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child, variant=variant,
                             semantics=semantics))
    
    elif type(spec) is CAX:
        # [g] X p = ~<g> X ~p
        newspec = CEX(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, variant=variant, semantics=semantics)
        
    elif type(spec) is CAG:
        # [g] G p = ~<g> F ~p
        newspec = CEF(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, variant=variant, semantics=semantics)
        
    elif type(spec) is CAU:
        # [g][p U q] = ~<g>[ ~q W ~p & ~q ]
        newspec = CEW(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~evalATLK(fsm, newspec, variant=variant, semantics=semantics)
        
    elif type(spec) is CAF:
        # [g] F p = ~<g> G ~p
        newspec = CEG(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, variant=variant, semantics=semantics)
        
    elif type(spec) is CAW:
        # [g][p W q] = ~<g>[~q U ~p & ~q]
        newspec = CEU(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~evalATLK(fsm, newspec, variant=variant, semantics=semantics)
                   
    elif type(spec) in {CEX, CEG, CEU, CEF, CEW}:
        if variant == "SF":
            return eval_strat(fsm, spec, semantics=semantics)
        elif variant == "FS":
            raise PyNuSMVError("Eval Gen SI does not support FS variant")
        elif variant == "FSF":
            return eval_strat_FSF(fsm, spec, semantics=semantics)
        else:
            return eval_strat(fsm, spec, semantics=semantics)
        
    else:
        # TODO Generate error
        print("[ERROR] evalATLK: unrecognized specification type", spec)
        return None



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
    Return the set of state/inputs pairs of strat satisfying <agents>[phi U psi]
    under full observability in strat.
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
    Return the set of state/inputs pairs of strat satisfying <agents>[phi W psi]
    under full observability in strat.
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


def get_equiv_class(fsm, gamma, state):
    """
    Return the equivalence class for gamma state belongs to. The distributed
    knowledge based equivalence fo gamma containing state is returned.
    
    fsm -- the model
    gamma -- a set of agents of fsm
    """
    return fsm.equivalent_states(state, gamma) & fsm.reachable_states


def is_conflicting(fsm, eqclass, gamma):
    """
    Return whether the given equivalence class for gamma is conflicting or not.
    
    fsm -- the model
    eqclass -- a BDD representing a set of states/inputs pairs
    gamma -- a set of agents of fsm
    
    eqclass must be a distributed knowledge based equivalence class.
    
    """
    si = fsm.pick_one_state_inputs(eqclass)
    ngamma_cube = fsm.bddEnc.inputsCube - fsm.inputs_cube_for_agents(gamma)
    return (eqclass - (eqclass & si.forsome(fsm.bddEnc.statesCube |
                                            ngamma_cube))).isnot_false()


def split_conflicting(fsm, eqclass, gamma):
    """
    Split the given equivalence class for gamma into non conflicting
    sub-strategies.
    
    fsm -- the model
    eqclass -- a BDD representing a set of states/inputs pairs
    gamma -- a set of agents of fsm
    
    eqclass must be a distributed knowledge based equivalence class.
    
    Generate all splitted non conflicting sub-strategies.
    
    """
    ngamma_cube = fsm.bddEnc.inputsCube - fsm.inputs_cube_for_agents(gamma)
    # Split eqclass into non-conflicting subsets
    while eqclass.isnot_false():
        si = fsm.pick_one_state_inputs(eqclass)
        ncss = eqclass & si.forsome(fsm.bddEnc.statesCube | ngamma_cube)
        eqclass = eqclass - ncss
    
        yield ncss


def split_one(fsm, strats, gamma):
    """
    Split one equivalence class of strats and return triples composed of
    the common non-conflicting part already encountered,
    a split and the rest of strats to split.
    
    fsm -- the model
    strats -- a BDD representing a set of states/inputs pairs
    gamma -- a set of agents of fsm
    
    Return a generator of all triples of common parts, splits
    and rest of strats.
    
    """
    if strats.is_false():
        yield (strats, strats, strats)
        return
        
    else:
        common = BDD.false(fsm.bddEnc.DDmanager)
        
        while strats.isnot_false():
            # Get one equivalence class
            si = fsm.pick_one_state_inputs(strats)
            s = si.forsome(fsm.bddEnc.inputsCube)
            eqs = get_equiv_class(fsm, gamma, s)
            eqcl = strats & eqs
            
            # Remove it from strats
            strats = strats - eqcl
            
            # The current equivalence class is conflicting
            if is_conflicting(fsm, eqcl, gamma):
                for non_conflicting in split_conflicting(fsm, eqcl, gamma):
                    yield (common, non_conflicting, strats)
                return
            
            else:
                # Add equivalence class to common
                common = common | eqcl
        
        # strats is false, everything is in common
        yield (common, strats, strats)


def split(fsm, strats, gamma, semantics="group"):
    """
    Split strats into all its non-conflicting greatest subsets.
    
    fsm -- the model
    strats -- a BDD representing a set of states/inputs pairs
    gamma -- a set of agents of fsm
    semantics -- the semantic to use for splitting (group or individual)
    
    Return a generator of all non-conflicting greatest subsets of strats.
    
    """
    if semantics == "individual":
        if len(gamma) <= 0:
            yield strats
        else:
            agent = next(iter(gamma))
            gamma = gamma - {agent}
            for strat in split(fsm, strats, {agent}, semantics="group"):
                for sstrat in split(fsm, strat, gamma,
                                    semantics="individual"):
                    yield sstrat
    else:
        if strats.is_false():
            yield strats
        else:
            # semantics should be group
            # Split one equivalence class
            for common, splitted, rest in split_one(fsm, strats, gamma):
                for strat in split(fsm, rest, gamma):
                    yield (common | strat | splitted)


def filter_strat(fsm, spec, strat=None, variant="SF", semantics="group"):
    """
    Returns the subset SA of strat (or the whole system if strat is None),
    state/action pairs of fsm, such that there is a strategy to satisfy spec
    in fsm.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator;
            the operator is CEX, CEG, CEF, CEU or CEW.
    strat -- the subset of the system to consider.
    variant -- the variant of the algorithm to evaluate strategic operators;
               must be
               * "SF" for the standard way: splitting in uniform strategies then
                 filtering winning states,
               * "FS" for the alternating way: filtering winning states, then
                 splitting one conflicting equivalence class, then recurse
               * "FSF" for the filter-split-filter way: filtering winning states
                 then splitting all remaining actions into uniform strategies,
                 then filtering final winning states.
    semantics -- the semantics to use for starting point and strategy point
                 equivalence; must be
                 * "group" for the original ATLK_irF semantics considering
                   the group as a single agent (distributed knowledge is used)
                 * "individual" for the original ATL_ir semantics considering
                   the group as individual agents (individual knowledge is
                   used)
                 
    If variant is not in {"SF", "FS", "FSF"}, the standard "SF" way is used.
    """
    
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    
    # Filtering
    if type(spec) is CEX:
        winning = cex_si(fsm, agents,
                         evalATLK(fsm, spec.child, variant=variant,
                                  semantics=semantics), strat)

    elif type(spec) is CEG:
        winning = ceg_si(fsm, agents, evalATLK(fsm, spec.child,
                                               variant=variant,
                                               semantics=semantics), strat)

    elif type(spec) is CEU:
        winning = ceu_si(fsm, agents, evalATLK(fsm, spec.left,
                                               variant=variant,
                                               semantics=semantics),
                         evalATLK(fsm, spec.right, variant=variant,
                                  semantics=semantics), strat)

    elif type(spec) is CEF:
        # <g> F p = <g>[true U p]
        winning = ceu_si(fsm, agents, BDD.true(fsm.bddEnc.DDmanager),
                         evalATLK(fsm, spec.child, variant=variant,
                                  semantics=semantics), strat)

    elif type(spec) is CEW:
       winning = cew_si(fsm, agents, evalATLK(fsm, spec.left, variant=variant,
                                              semantics=semantics),
                        evalATLK(fsm, spec.right, variant=variant,
                                 semantics=semantics), strat)
    
    
    return winning & fsm.bddEnc.statesInputsMask & fsm.protocol(agents)


def all_equiv_sat(fsm, winning, agents, semantics="group"):
    """
    Return the states s of winning such that all states indistinguishable
    from s by agents are in winning.
    
    fsm -- a MAS representing the system;
    winning -- a BDD representing a set of states of fsm;
    agents -- a set of agents;
    semantics -- the semantics to use (group or individual); if not
                 "individual", "group" semantics is used.
    
    """
    if semantics == "individual":
        equiv_sat = BDD.true(fsm.bddEnc.DDmanager)
        for agent in agents:
            equiv_sat &= all_equiv_sat(fsm, winning, {agent},
                                       semantics="group")
        return equiv_sat
    else:
        # Get states for which all states belong to winning
        # wineq is the set of states for which all equiv states are in winning
        nwinning = ~winning & fsm.bddEnc.statesInputsMask
        return ~(fsm.equivalent_states(nwinning &
                 fsm.reachable_states, frozenset(agents))) & winning


def agents_in_group(fsm, group):
    """
    Return the set of agents in the given group in fsm. If group is not a group
    of fsm, group is returned alone.
    
    This procedure recursively searches for all basic agents in the groups
    possibly composing group.
    
    fsm -- the model;
    group -- a group or agent name of the model.
    """
    
    if group in fsm.groups:
        agents = set()
        for agent in fsm.groups[group]:
            if agent in fsm.groups:
                agents |= agents_in_group(fsm, agent)
            else:
                agents.add(agent)
    else:
        agents = {group}
    return agents


def eval_strat(fsm, spec, semantics="group"):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    spec is a strategic operator <G> pi.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator.
    semantics -- the semantics to use (group or individual)
    
    """
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    
    if semantics == "individual":
        agents = reduce(lambda a, b: a | b,
                        (agents_in_group(fsm, group) for group in agents))
    
    # Restrict protocol to reachable states to avoid splitting useless
    # equivalence classes
    protocol = fsm.protocol(agents) & fsm.reachable_states
    strats = split(fsm, protocol, agents, semantics=semantics)
    nbstrats = 0
    
    if config.debug:
        moves_count = fsm.count_states_inputs(protocol)
        print("Eval strategies (SF): {} move{} in protocol"
              .format(moves_count, "s" if moves_count > 1 else ""))
    
    for strat in strats:
        nbstrats += 1
        winning = (filter_strat(fsm, spec, strat, variant="SF").
                    forsome(fsm.bddEnc.inputsCube))
        sat = sat | all_equiv_sat(fsm, winning, agents, semantics=semantics)
        
        # ----- Garbage collection -------------------------------------
        if (config.garbage.type == "each" or
            (config.garbage.type == "step"
                and nbstrats % config.garbage.step == 0)):
            gc.collect()
        
        if config.debug and nbstrats % 1000 == 0:
            print("Eval strategies (SF): {} strateg{} checked so far"
                  .format(nbstrats, "ies" if nbstrats > 1 else "y"))
    
    # DEBUG Print number of strategies
    if config.debug:
        print("Eval_strat: {} strategies".format(nbstrats))
    
    return sat
    
    
def eval_strat_FSF(fsm, spec, semantics="group"):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    spec is a strategic operator <G> pi.
    
    Implement a variant of the algorithm that
    filters, splits and then filters.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator.
    semantics -- the semantics to use (group or individual).
    
    """
    
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    
    if semantics == "individual":
        agents = reduce(lambda a, b: a | b,
                        (agents_in_group(fsm, group) for group in agents))
    
    protocol = fsm.protocol(agents) & fsm.reachable_states
    
    if config.debug:
        moves_count = fsm.count_states_inputs(protocol)
        print("Eval strategies (FSF): {} move{} in protocol"
              .format(moves_count, "s" if moves_count > 1 else ""))
    
    # First filtering
    winning = filter_strat(fsm, spec, strat=protocol, variant="FSF",
                           semantics=semantics)
    
    if config.debug:
        moves_count = fsm.count_states_inputs(winning)
        print("Eval strategies (FSF): {} move{} after filtering"
              .format(moves_count, "s" if moves_count > 1 else ""))
    
    if winning.is_false(): # no state/inputs pairs are winning => return false
        return winning
    
    # Splitting the strategies
    nbstrats = 0
    for strat in split(fsm, winning, agents, semantics=semantics):
        nbstrats += 1
        
        if config.debug and nbstrats % 1000 == 0:
            print("Eval strategies (FSF): {} strateg{} checked so far"
                  .format(nbstrats, "ies" if nbstrats > 1 else "y"))
        
        # Second filtering
        winning = filter_strat(fsm, spec, strat, variant="FSF",
                               semantics=semantics)
        winning = winning.forsome(fsm.bddEnc.inputsCube)
        sat = sat | all_equiv_sat(fsm, winning, agents, semantics=semantics)
        
        # Collect to avoid memory overflow
        if (config.garbage.type == "each" or
            (config.garbage.type == "step"
             and nbstrats % config.garbage.step == 0)):
            gc.collect()
    
    if config.debug:
        print("Eval strategies (FSF): {} strateg{} checked"
              .format(nbstrats, "ies" if nbstrats > 1 else "y"))
    
    return sat