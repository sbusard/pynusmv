"""
ATLK with partial observability evaluation functions.

These evaluation functions reimplement the algorithms and try to be more
optimal.
"""

import gc

from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression
from pynusmv.utils import fixpoint as fp
from pynusmv.exception import PyNuSMVError

from ..atlkFO.ast import (TrueExp, FalseExp, Init, Reachable,
                          Atom, Not, And, Or, Implies, Iff, 
                          AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                          nK, nE, nD, nC, K, E, D, C,
                          CEF, CEG, CEX, CEU, CEW, CAF, CAG, CAX, CAU, CAW)
                          

from ..atlkFO.eval import (fair_states, ex, eg, eu, nk, ne, nd, nc)


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
                 
    If variant is not in {"SF", "FS", "FSF"}, the standard "SF" way is used.
    """
    
    if semantics != "group":
        raise PyNuSMVError("Optimal evalATLK: unsupported semantics:" +
                           semantics)
    
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
        return ~evalATLK(fsm, spec.child, variant=variant)
        
    elif type(spec) is And:
        return (evalATLK(fsm, spec.left, variant=variant)
                & evalATLK(fsm, spec.right, variant=variant))
        
    elif type(spec) is Or:
        return (evalATLK(fsm, spec.left, variant=variant)
                | evalATLK(fsm, spec.right, variant=variant))
        
    elif type(spec) is Implies:
        # a -> b = ~a | b
        return ((~evalATLK(fsm, spec.left, variant=variant))
                 | evalATLK(fsm, spec.right, variant=variant))
        
    elif type(spec) is Iff:
        # a <-> b = (a & b) | (~a & ~b)
        l = evalATLK(fsm, spec.left, variant=variant)
        r = evalATLK(fsm, spec.right, variant=variant)
        return (l & r) | ((~l) & (~r))
        
    elif type(spec) is EX:
        return ex(fsm, evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is AX:
        # AX p = ~EX ~p
        return ~ex(fsm, ~evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is EG:
        return eg(fsm, evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is AG:
        # AG p = ~EF ~p = ~E[ true U ~p ]
        return ~eu(fsm,
                   BDD.true(fsm.bddEnc.DDmanager),
                   ~evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is EU:
        return eu(fsm, evalATLK(fsm, spec.left, variant=variant),
                       evalATLK(fsm, spec.right, variant=variant))
        
    elif type(spec) is AU:
        # A[p U q] = ~E[~q W ~p & ~q] = ~(E[~q U ~p & ~q] | EG ~q)
        p = evalATLK(fsm, spec.left, variant=variant)
        q = evalATLK(fsm, spec.right, variant=variant)
        equpq = eu(fsm, ~q, ~q & ~p)
        egq = eg(fsm, ~q)
        return ~(equpq | egq)
        
    elif type(spec) is EF:
        # EF p = E[ true U p ]
        return eu(fsm,
                  BDD.true(fsm.bddEnc.DDmanager),
                  evalATLK(fsm, spec.child, variant=variant))    
        
    elif type(spec) is AF:
        # AF p = ~EG ~p
        return ~eg(fsm, ~evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is EW:
        # E[ p W q ] = E[ p U q ] | EG p
        return (eu(fsm, evalATLK(fsm, spec.left, variant=variant),
                        evalATLK(fsm, spec.right, variant=variant)) |
                eg(fsm, evalATLK(fsm, spec.left, variant=variant)))
        
    elif type(spec) is AW:
        # A[p W q] = ~E[~q U ~p & ~q]
        p = evalATLK(fsm, spec.left, variant=variant)
        q = evalATLK(fsm, spec.right, variant=variant)
        return ~eu(fsm, ~q, ~p & ~q)
        
    elif type(spec) is nK:
        return nk(fsm, spec.agent.value, evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is K:
        # K<'a'> p = ~nK<'a'> ~p
        return ~nk(fsm, spec.agent.value, ~evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is nE:
        return ne(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is E:
        # E<g> p = ~nE<g> ~p
        return ~ne(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is nD:
        return nd(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child, variant=variant)) 
        
    elif type(spec) is D:
        # D<g> p = ~nD<g> ~p
        return ~nd(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is nC:
        return nc(fsm,
                  [a.value for a in spec.group],
                  evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is C:
        # C<g> p = ~nC<g> ~p
        return ~nc(fsm,
                   [a.value for a in spec.group],
                   ~evalATLK(fsm, spec.child, variant=variant))
                   
    elif type(spec) in {CEX, CAX, CEG, CAG, CEU, CAU, CEF, CAF, CEW, CAW}:
        if variant == "SF":
            return eval_strat(fsm, spec)
        elif variant == "FS":
            return eval_strat_improved(fsm, spec)
        elif variant == "FSF":
            return eval_strat_FSF(fsm, spec)
        else:
            return eval_strat(fsm, spec)
        
    else:
        # TODO Generate error
        print("[ERROR] evalATLK: unrecognized specification type", spec)
        return None
        
        
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
            if((eqcl - (eqcl & si.forsome(fsm.bddEnc.statesCube | ngamma_cube)))
                .isnot_false()):
                # Split eqcl into non-conflicting subsets
                while eqcl.isnot_false():
                    si = fsm.pick_one_state_inputs(eqcl)
                    ncss = eqcl & si.forsome(fsm.bddEnc.statesCube |ngamma_cube)
                    eqcl = eqcl - ncss

                    yield (common, ncss, strats)
                return
            
            else:
                # Add equivalence class to common
                common = common | eqcl
        
        # strats is false, everything is in common
        yield (common, strats, strats)


def split_eval(fsm, spec, common, rest):
    """
    Split strats into all its non-conflicting greatest subsets and compute
    the states that satisfy spec in strats.
    
    fsm -- the model
    strats -- a BDD representing a set of states/inputs pairs
    spec -- an AST-based ATLK specification with a top strategic operator.
    
    """    
    if (common | rest).is_false():
        return common | rest
    else:
        gamma = {atom.value for atom in spec.group}
        sat = BDD.false(fsm.bddEnc.DDmanager)
        
        for newcommon, splitted, newrest in split_one(fsm, rest, gamma):
            if splitted.is_false():
                # newcommon contains a partial uniform strategy
                # we can extract the winning states
                # from newcommon | common
                
                # first, filter
                winning = (filter_strat(fsm, spec, common | newcommon, "SF")
                           .forsome(fsm.bddEnc.inputsCube))
                
                # then, get equiv sat
                sat = sat | all_equiv_sat(fsm, winning, gamma)
                gc.collect()
            else:
                sat = sat | split_eval(fsm, spec, common | newcommon | splitted,
                                       newrest)
        return sat


def all_equiv_sat(fsm, winning, agents):
    """
    Return the states s of winning such that all states indistinguishable
    from s by agents are in winning.
    
    fsm -- a MAS representing the system;
    winning -- a BDD representing a set of states of fsm;
    agents -- a set of agents.
    
    """
    # Get states for which all states belong to winning
    # wineq is the set of states for which all equiv states are in winning
    nwinning = ~winning & fsm.bddEnc.statesInputsMask
    return ~(fsm.equivalent_states(nwinning &
             fsm.reachable_states, frozenset(agents))) & winning


def eval_strat(fsm, spec):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    spec is a strategic operator <G> pi.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator.
    
    """
    if type(spec) is CAX:
        # [g] X p = ~<g> X ~p
        newspec = CEX(spec.group, Not(spec.child))
        return ~eval_strat(fsm, newspec)
        
    elif type(spec) is CAG:
        # [g] G p = ~<g> F ~p
        newspec = CEF(spec.group, Not(spec.child))
        return ~eval_strat(fsm, newspec)
        
    elif type(spec) is CAU:
        # [g][p U q] = ~<g>[ ~q W ~p & ~q ]
        newspec = CEW(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~eval_strat(fsm, newspec)
        
    elif type(spec) is CAF:
        # [g] F p = ~<g> G ~p
        newspec = CEG(spec.group, Not(spec.child))
        return ~eval_strat(fsm, newspec)
        
    elif type(spec) is CAW:
        # [g][p W q] = ~<g>[~q U ~p & ~q]
        newspec = CEU(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~eval_strat(fsm, newspec)
    
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    
    return split_eval(fsm, spec, BDD.false(fsm.bddEnc.DDmanager),
                      fsm.protocol(agents) & fsm.reachable_states)


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



def eval_strat_improved(fsm, spec, toSplit=None, toKeep=None):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    spec is a strategic operator <G> pi.
    
    Implement the improved algorithm alternating between splitting a conflicting
    equvalence class and filtering winning state/inputs pairs.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator.
    toSplit -- the part of the strategy to split (not necessarily uniform).
    toKeep -- the part of the strategy to keep (uniform).
    
    """
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    gamma = agents
    ngamma_cube = fsm.bddEnc.inputsCube - fsm.inputs_cube_for_agents(gamma)
    
    if not toSplit and not toKeep:
        toSplit = fsm.protocol(agents)
        toKeep = BDD.false(fsm.bddEnc.DDmanager)
    
    if type(spec) is CAX:
        # [g] X p = ~<g> X ~p
        newspec = CEX(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, "FS")

    elif type(spec) is CAG:
        # [g] G p = ~<g> F ~p
        newspec = CEF(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, "FS")

    elif type(spec) is CAU:
        # [g][p U q] = ~<g>[ ~q W ~p & ~q ]
        newspec = CEW(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~evalATLK(fsm, newspec, "FS")

    elif type(spec) is CAF:
        # [g] F p = ~<g> G ~p
        newspec = CEG(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, "FS")

    elif type(spec) is CAW:
        # [g][p W q] = ~<g>[~q U ~p & ~q]
        newspec = CEU(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~evalATLK(fsm, newspec, "FS")
    
    winning = filter_strat(fsm, spec, toSplit | toKeep, variant="FS")
    toSplit = toSplit & winning
    toKeep = toKeep & winning
    
    # Get one conflicting equivalence class
    if winning.is_false(): # no state/inputs pairs are winning => return false
        return winning
    
    # TODO Improve this: if a pair is not conflicting now, it won't be 
    # conflicting anymore. We should keep track of non-conflicting equivalence
    # classes through recursive calls
    
    # Split one conflicting equivalence class
    for common, splitted, rest in split_one(fsm, toSplit, gamma):
        
        if splitted.is_false():
            # No conflicting classes, return states that are winning for all eq
            common = (common | toKeep).forsome(fsm.bddEnc.inputsCube)
            sat = sat | all_equiv_sat(fsm, common, agents)
        
        else:
            sat = sat | eval_strat_improved(fsm, spec,
                                            toSplit=rest,
                                            toKeep=toKeep | common | splitted)
    
    return sat


def filter_strat(fsm, spec, strat=None, variant="SF"):
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
                 
    If variant is not in {"SF", "FS", "FSF"}, the standard "SF" way is used.
    """
    
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    
    # Filtering
    if type(spec) is CEX:
        winning = cex_si(fsm, agents,
                         evalATLK(fsm, spec.child, variant=variant), strat)

    elif type(spec) is CEG:
        winning = ceg_si(fsm, agents, evalATLK(fsm, spec.child, variant=variant), strat)

    elif type(spec) is CEU:
        winning = ceu_si(fsm, agents, evalATLK(fsm, spec.left, variant=variant),
                         evalATLK(fsm, spec.right, variant=variant), strat)

    elif type(spec) is CEF:
        # <g> F p = <g>[true U p]
        winning = ceu_si(fsm, agents, BDD.true(fsm.bddEnc.DDmanager),
                         evalATLK(fsm, spec.child, variant=variant), strat)

    elif type(spec) is CEW:
       winning = cew_si(fsm, agents, evalATLK(fsm, spec.left, variant=variant),
                        evalATLK(fsm, spec.right, variant=variant), strat)
    
    
    return winning & fsm.bddEnc.statesInputsMask & fsm.protocol(agents)
    
    
def eval_strat_FSF(fsm, spec):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    spec is a strategic operator <G> pi.
    
    Implement a variant of the algorithm that
    filters, splits and then filters.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator.
    
    """
    
    if type(spec) is CAX:
        # [g] X p = ~<g> X ~p
        newspec = CEX(spec.group, Not(spec.child))
        return ~eval_strat_FSF(fsm, newspec)
        
    elif type(spec) is CAG:
        # [g] G p = ~<g> F ~p
        newspec = CEF(spec.group, Not(spec.child))
        return ~eval_strat_FSF(fsm, newspec)
        
    elif type(spec) is CAU:
        # [g][p U q] = ~<g>[ ~q W ~p & ~q ]
        newspec = CEW(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~eval_strat_FSF(fsm, newspec)
        
    elif type(spec) is CAF:
        # [g] F p = ~<g> G ~p
        newspec = CEG(spec.group, Not(spec.child))
        return ~eval_strat_FSF(fsm, newspec)
        
    elif type(spec) is CAW:
        # [g][p W q] = ~<g>[~q U ~p & ~q]
        newspec = CEU(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~eval_strat_FSF(fsm, newspec)
        
        
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    
    # First filtering
    winning = filter_strat(fsm, spec, variant="FSF")
    
    if winning.is_false(): # no state/inputs pairs are winning => return false
        return winning
    
    return split_eval(fsm, spec, BDD.false(fsm.bddEnc.DDmanager),
                      winning)