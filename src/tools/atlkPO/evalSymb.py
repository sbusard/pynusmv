"""
ATLK with partial observability evaluation functions.

These functions are based on the fully symbolic approach of Xuang and van der
Meyden.
"""

import gc
from functools import reduce

from pynusmv.dd import BDD
from pynusmv.fsm import BddTrans
from pynusmv.mc import eval_simple_expression
from pynusmv.utils import fixpoint as fp
from pynusmv import node, glob
from pynusmv.exception import PyNuSMVError

from pynusmv.nusmv.compile.symb_table import symb_table as nssymb_table

from ..atlkFO.ast import (TrueExp, FalseExp, Init, Reachable,
                          Atom, Not, And, Or, Implies, Iff, 
                          AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                          nK, nE, nD, nC, K, E, D, C,
                          CEF, CEG, CEX, CEU, CEW, CAF, CAG, CAX, CAU, CAW)

from ..atlkFO.eval import nk, ne, nc

from . import config


def evalATLK(fsm, spec, variant="SF", semantics="group"):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    
    fsm -- a MAS representing the system
    spec -- an AST-based ATLK specification
    variant -- the variant of the algorithm to evaluate strategic operators;
               must be
               * "SF" for the standard way: encoding strategies for the
                 agents then performing the fully symbolic model checking
               * "FSF" for the filter-split-filter way: filtering winning
                 sub-protocol before encoding it and performing the fully
                 symbolic model checking
                 
    If variant is not in {"SF", "FSF"}, the standard "SF" way is used.
    """
    
    if semantics != "group":
        raise PyNuSMVError("Symbolic evalATLK: unsupported semantics:" +
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
        return ew(fsm, evalATLK(fsm, spec.child, variant=variant),
                       BDD.false(fsm.bddEnc.DDmanager))
        
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
        return ~ew(fsm, ~q, ~p & ~q)
        
    elif type(spec) is EF:
        # EF p = E[ true U p ]
        return eu(fsm,
                  BDD.true(fsm.bddEnc.DDmanager),
                  evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is AF:
        # AF p = ~EG ~p
        return ~ew(fsm, ~evalATLK(fsm, spec.child, variant=variant),
                        BDD.false(fsm.bddEnc.DDmanager))
        
    elif type(spec) is EW:
        # E[ p W q ] = E[ p U q ] | EG p
        return ew(fsm, evalATLK(fsm, spec.left, variant=variant),
                        evalATLK(fsm, spec.right, variant=variant))
        
    elif type(spec) is AW:
        # A[p W q] = ~E[~q U ~p & ~q]
        p = evalATLK(fsm, spec.left, variant=variant)
        q = evalATLK(fsm, spec.right, variant=variant)
        return ~eu(fsm, ~q, ~p & ~q)
        
    elif type(spec) is nK:
        return nk(fsm, spec.agent.value,
                  evalATLK(fsm, spec.child, variant=variant))
        
    elif type(spec) is K:
        # K<'a'> p = ~nK<'a'> ~p
        return ~nk(fsm, spec.agent.value,
                   ~evalATLK(fsm, spec.child, variant=variant))
        
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
    
    elif type(spec) is CAX:
        # [g] X p = ~<g> X ~p
        newspec = CEX(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, variant=variant)
        
    elif type(spec) is CAG:
        # [g] G p = ~<g> F ~p
        newspec = CEF(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, variant=variant)
        
    elif type(spec) is CAU:
        # [g][p U q] = ~<g>[ ~q W ~p & ~q ]
        newspec = CEW(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~evalATLK(fsm, newspec, variant=variant)
        
    elif type(spec) is CAF:
        # [g] F p = ~<g> G ~p
        newspec = CEG(spec.group, Not(spec.child))
        return ~evalATLK(fsm, newspec, variant=variant)
        
    elif type(spec) is CAW:
        # [g][p W q] = ~<g>[~q U ~p & ~q]
        newspec = CEU(spec.group,
                      Not(spec.right),
                      And(Not(spec.left), Not(spec.right)))
        return ~evalATLK(fsm, newspec, variant=variant)
                   
    elif type(spec) in {CEX, CEG, CEU, CEF, CEW}:
        if variant == "SF":
            return eval_strat(fsm, spec)
        elif variant == "FSF":
            return eval_strat_FSF(fsm, spec)
        else:
            return eval_strat(fsm, spec)
        
    else:
        # TODO Generate error
        print("[ERROR] evalATLK: unrecognized specification type", spec)
        return None


def _fair(fsm):
    if len(fsm.fairness_constraints) <= 0:
        return BDD.true(fsm.bddEnc.DDmanager)
    else:
        run = fsm.trans
        # fair = nu Z. /\_fc pre(mu Y. (Z /\ fc) \/ pre(Y))
        def inner(Z):
            res = BDD.true(fsm.bddEnc.DDmanager)
            for fc in fsm.fairness_constraints:
                res = res & run.pre(fp(lambda Y: (Z & fc) | run.pre(Y),
                                       BDD.false(fsm.bddEnc.DDmanager)))
            return res
        return fp(inner, BDD.true(fsm.bddEnc.DDmanager))


def ex(fsm, phi):
    run = fsm.trans
    return run.pre(phi & _fair(fsm))

def eu(fsm, phi, psi):
    run = fsm.trans
    return fp(lambda Y : (psi & _fair(fsm)) | (phi & run.pre(Y)),
              BDD.false(fsm.bddEnc.DDmanager))


def ew(fsm, phi, psi):
    run = fsm.trans
    if len(fsm.fairness_constraints) <= 0:
        return fp(lambda Z: psi | (phi & run.pre(Z)),
                  BDD.true(fsm.bddEnc.DDmanager))
    else:
        def inner(Z):
            res = BDD.true(fsm.bddEnc.DDmanager)
            for f in fsm.fairness_constraints:
                res = res & run.pre(fp(lambda Y: (psi & _fair(fsm)) |
                                                 (Z & f) | phi & run.pre(Y),
                                    BDD.false(fsm.bddEnc.DDmanager)))
            return (res & phi) | (psi & _fair(fsm))
        return fp(inner, BDD.true(fsm.bddEnc.DDmanager))


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
    <agents>[phi U psi]
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


def _ifair(fsm, name):
    """
    Return a mu-calculus translation of fair states of fsm under uniform
    strategy played by name.
    """
    
    jump = fsm.transitions[name + "_jump"]
    equiv = fsm.transitions[name + "_equiv"]
    follow = fsm.transitions[name + "_follow"]
    
    # ifair =
    # nu Z. /\_fc <>_group_follow(mu Y. (Z /\ fc) \/ <>_group_follow(Y))
    def inner(Z):
        # /\_fc <>_group_follow(mu Y. (Z /\ fc) \/ <>_group_follow(Y))
        res = BDD.true(fsm.bddEnc.DDmanager)
        for f in fsm.fairness_constraints:
            res = res & follow.pre(fp(lambda Y: (Z & f) | follow.pre(Y),
                                      BDD.false(fsm.bddEnc.DDmanager)))
        return res
    return fp(inner, BDD.true(fsm.bddEnc.DDmanager))


def cex_symbolic(fsm, name, child):
    jump = fsm.transitions[name + "_jump"]
    equiv = fsm.transitions[name + "_equiv"]
    follow = fsm.transitions[name + "_follow"]
    reachable = fsm.reachable_states
    
    if len(fsm.fairness_constraints) <= 0:
        # <group>i X p =
        # <>_group_jump []_group_equiv (reachable => []_group_follow p)
        return jump.pre(~equiv.pre(~(reachable.imply(~follow.pre(~child)))))
    else:
        # <group>i X fair p =
        # <>_group_jump []_group_equiv
        # (reachable => []_group_follow (p \/ ~ifair))
        return jump.pre(~equiv.pre(~(reachable.imply(
                        ~follow.pre(~(child | ~_ifair(fsm, name)))))))


def ceu_symbolic(fsm, name, left, right):
    jump = fsm.transitions[name + "_jump"]
    equiv = fsm.transitions[name + "_equiv"]
    follow = fsm.transitions[name + "_follow"]
    reachable = fsm.reachable_states
    
    if len(fsm.fairness_constraints) <= 0:
        # <group>i[p U q] =
        # <>_group_jump []_group_equiv
        # (reachable => mu Z . q \/ (p /\ []_group_follow Z))
        return jump.pre(~equiv.pre(~(reachable.imply(
                        fp(lambda Z: right | (left & ~follow.pre(~Z)),
                           BDD.false(fsm.bddEnc.DDmanager))))))
    else:
        ifair = _ifair(fsm, name)
        # <group>i fair [p U q] =
        # <>_group_jump []_group_equiv
        # reachable =>
        # mu Z. (p \/ q \/ ~ifair) /\
        #       (q \/_fc []_group_follow
        #                (nu Y. (Z \/ ~fc) /\
        #                              (p \/ q \/ ~ifair) /\
        #                              (q \/ []_group_follow(Y))))
        def inner(Z):
            # (p \/ q \/ ~ifair) /\
            # (q \/_fc []_group_follow
            #          (nu Y. (Z \/ ~fc) /\
            #                        (p \/ q \/ ~ifair) /\
            #                        (q \/ []_group_follow(Y))))
            res = BDD.false(fsm.bddEnc.DDmanager)
            for f in fsm.fairness_constraints:
                # []_group_follow (nu Y. (Z \/ ~fc) /\
                #                        (p \/ q \/ ~ifair) /\
                #                        (q \/ []_group_follow(Y)))
                res = res | ~follow.pre(
                      ~(fp(lambda Y: ((Z | ~f) & (left | right | ~ifair) &
                                     (right | ~follow.pre(~Y))),
                           BDD.true(fsm.bddEnc.DDmanager))))
            return ((left | right | ~ifair) & (right | res))
        return jump.pre(~equiv.pre(~(reachable.imply(
                        fp(inner, BDD.false(fsm.bddEnc.DDmanager))))))


def cew_symbolic(fsm, name, left, right):
    jump = fsm.transitions[name + "_jump"]
    equiv = fsm.transitions[name + "_equiv"]
    follow = fsm.transitions[name + "_follow"]
    reachable = fsm.reachable_states
    
    if len(fsm.fairness_constraints) <= 0:
        # <group>i[p W q] = 
        # <>_group_jump []_group_equiv
        # (reachable => nu Z . q \/ (p /\ []_group_follow Z))
        return jump.pre(~equiv.pre(~(reachable.imply(
                        fp(lambda Z: right | (left & ~follow.pre(~Z)),
                           BDD.true(fsm.bddEnc.DDmanager))))))
    else:
        # <group>i fair [p W q] =
        # <>_group_jump []_group_equiv
        # reachable =>
        # nu Z. (p \/ q \/ ~ifair) /\ (q \/ []_group_follow(Z))
        return jump.pre(~equiv.pre(~(reachable.imply(
                        fp(lambda Z: ((left | right | ~_ifair(fsm, name)) &
                                      (right | ~follow.pre(~Z))),
                           BDD.true(fsm.bddEnc.DDmanager))))))


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
            if((eqcl - (eqcl & si.forsome(fsm.bddEnc.statesCube| ngamma_cube)))
                .isnot_false()):
                # Split eqcl into non-conflicting subsets
                while eqcl.isnot_false():
                    si = fsm.pick_one_state_inputs(eqcl)
                    ncss = (eqcl &
                            si.forsome(fsm.bddEnc.statesCube |ngamma_cube))
                    eqcl = eqcl - ncss

                    yield (common, ncss, strats)
                return
            
            else:
                # Add equivalence class to common
                common = common | eqcl
        
        # strats is false, everything is in common
        yield (common, strats, strats)


def split(fsm, strats, gamma):
    """
    Split strats into all its non-conflicting greatest subsets.
    
    fsm -- the model
    strats -- a BDD representing a set of states/inputs pairs
    gamma -- a set of agents of fsm
    
    Return a generator of all non-conflicting greatest subsets of strats.
    
    """
    if strats.is_false():
        yield strats
    else:
        # Split one equivalence class
        for common, splitted, rest in split_one(fsm, strats, gamma):
            for strat in split(fsm, rest, gamma):
                yield (common | strat | splitted)


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
               * "SF" for the standard way: splitting in uniform strategies
                 then filtering winning states,
               * "FS" for the alternating way: filtering winning states, then
                 splitting one conflicting equivalence class, then recurse
               * "FSF" for the filter-split-filter way: filtering winning
                 states then splitting all remaining actions into uniform
                 strategies, then filtering final winning states.
                 
    If variant is not in {"SF", "FS", "FSF"}, the standard "SF" way is used.
    """
    
    sat = BDD.false(fsm.bddEnc.DDmanager)
    agents = {atom.value for atom in spec.group}
    
    # Filtering
    if type(spec) is CEX:
        winning = cex_si(fsm, agents,
                         evalATLK(fsm, spec.child, variant=variant), strat)

    elif type(spec) is CEG:
        winning = ceg_si(fsm, agents,
                         evalATLK(fsm, spec.child, variant=variant), strat)

    elif type(spec) is CEU:
        winning = ceu_si(fsm, agents, 
                         evalATLK(fsm, spec.left, variant=variant),
                         evalATLK(fsm, spec.right, variant=variant), strat)

    elif type(spec) is CEF:
        # <g> F p = <g>[true U p]
        winning = ceu_si(fsm, agents, BDD.true(fsm.bddEnc.DDmanager),
                         evalATLK(fsm, spec.child, variant=variant), strat)

    elif type(spec) is CEW:
       winning = cew_si(fsm, agents, evalATLK(fsm, spec.left, variant=variant),
                        evalATLK(fsm, spec.right, variant=variant), strat)
    
    
    return winning & fsm.bddEnc.statesInputsMask & fsm.protocol(agents)


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


def vars_trans_from_protocol(fsm, original_variables, original_trans,
                             strat, observables, actions, name):
    """
    Given the BDD fsm of the original model, the set of original variables
    (as triplets of variable name, variable type, variable kind) and
    the transition relation of the original model,
    return the new variables to declare and new transition relations to define
    to fully encode strat in the model, given that strat define a set of
    strategies based on observables and actions (both strings of NuSMV
    state and inputs variables, respectively).
    
    The returned value is a pair where the first element is an enumerable
    of (variable names (as Nodes), variable type (as SymbType),
    variable kind) triplets and the second element is a dictionary of
    trans-name (as str) -> trans-expression.
    
    name is used to prefix variables representing the strategies.
    """
    
    def _var_to_name(variable):
        """
        Given a variable of the model, return a name that can be used as an
        atom for the variable name.
        """
        return (str(variable).replace(".", "$").replace("[", "$").
                replace("]", "$"))

    def _assign_to_name(assigment):
        """
        Given an assigment of variables (an enumerable of var, val pairs),
        return a string that can be used as an atom for representing the
        assigment.
        """
        return "##".join(_var_to_name(var) + "#" + val
                         for var, val in sorted(((str(var), str(val))
                                                 for var, val in assigment)))
    
    variables = {}
    act_cube = fsm.bddEnc.cube_for_inputs_vars(actions)
    obs_cube = fsm.bddEnc.cube_for_state_vars(observables)
    state_cube = fsm.bddEnc.statesCube
    inputs_cube = fsm.bddEnc.inputsCube
    other_vars_cube = (state_cube) - obs_cube
    other_acts_cube = (inputs_cube) - act_cube
    
    # Get all observations by manipulating the protocol
    # and for each observation
    # get the possible values for agent's actions
    protocol = (strat & fsm.bddEnc.statesInputsMask & fsm.state_constraints &
                fsm.inputs_constraints)
    while protocol.isnot_false():
        si = fsm.pick_one_state_inputs(protocol)
        state = fsm.pick_one_state(si.forsome(fsm.bddEnc.inputsCube))
        
        # Get observation values
        obs_vars = tuple(sorted([(node.Identifier.from_string(var),
                                  node.Expression.from_string(val))
                                 for var, val 
                                 in state.get_str_values().items()
                                 if var in observables]))
        
        variables[obs_vars] = set()
        inputs = ((protocol & state).forsome(fsm.bddEnc.statesCube)
                  & fsm.bddEnc.statesInputsMask
                  & fsm.bddEnc.statesMask
                  & fsm.bddEnc.inputsMask
                  & fsm.state_constraints
                  & fsm.inputs_constraints)
        # Get the possible values for actions
        while inputs.isnot_false():
            i = fsm.pick_one_inputs(inputs)
            
            inputs_vars = tuple(sorted(
                          [(node.Identifier.from_string(var),
                            node.Expression.from_string(val))
                           for var, val in i.get_str_values().items()
                           if var in actions]))
            variables[obs_vars].add(inputs_vars)
            
            inputs = inputs - i.forsome(other_acts_cube)
        
        protocol = (protocol - state.forsome(other_vars_cube))
    
    # Translate observations into state variables
    #   => var#value##var#value...
    # translate values for actions in enums
    #   => ivar#value##ivar#value...
    
    
    new_variables = []
    strategies = set()  # strategies is a set of tuples
                        #   (obs assigment,
                        #    corresponding strategy var,
                        #    corresponding strategy var value,
                        #    actions assigment)
    
    for obs_values in variables:
        var_name = (name + "_" + _assign_to_name(obs_values))
        var_name = node.Identifier.from_string(var_name)
        type_values = []
        for action_values in variables[obs_values]:
            type_value = _assign_to_name(action_values)
            type_values.append(type_value)
            
            strategies.add((obs_values, var_name, type_value, action_values))
            
        var_type = fsm.bddEnc.symbTable._get_type_from_node(
                   node.Scalar(type_values))
        
        # a triplet (variable name (as Nodes),
        #            variable type (as SymbType),
        #            variable kind (as int))
        new_variables.append((var_name,
                              var_type,
                              nssymb_table.SYMBOL_STATE_VAR))
    
    
    # Relations:
    #   (1) one to follow the strategy (name_follow)
    #       =>  original trans
    #           & use the actions in the strategy
    #           & the strategies stays the same
    #   (2) one to consider equivalent states (name_equiv)
    #       =>  observations stay the same
    #           & the strategies stays the same
    #   (3) one to jump from a strategy to another (name_jump)
    #       => all state variables stay the same
    
    new_trans = {}
    
    # name_follow = original_trans & strategy stays the same
    #               & the actions are followed
    
    # original trans
    trans = original_trans
    
    # strategy stays the same: variables are in new_variables
    for var_name, _, _ in new_variables:
        trans &= (var_name.next() == var_name)
    
    # actions are followed:
    #   observations are true
    #   & corresponding strategy var is ivar#value##...
    #   ->
    #   ivar = value & ...
    # strategies is a set of tuples
    #   (obs assigment,
    #    corresponding strategy var,
    #    corresponding strategy var value,
    #    actions assigment)
    for move in strategies:
        obs_assign, strat_var, strat_val, act_assign = move
        
        trans_cond = reduce(lambda e, n: e & n,
                            [var == val for var, val in obs_assign],
                            node.Trueexp())
        trans_cond = trans_cond & (strat_var == strat_val)
        
        trans_cons = reduce(lambda e, n: e & n,
                            [var == val for var, val in act_assign],
                            node.Trueexp())
        trans = trans & trans_cond.implies(trans_cons)
    
    new_trans[name + "_follow"] = trans
    
    
    # name_equiv = observations stay the same
    #              & the strategy stays the same
    trans = reduce(lambda e, n: e & n,
                   [node.Identifier.from_string(str(obs)) ==
                    node.Identifier.from_string(str(obs)).next()
                    for obs in observables], node.Trueexp())
    for var_name, _, _ in new_variables:
        trans &= (var_name.next() == var_name)
    
    new_trans[name + "_equiv"] = trans
    
    
    # name_jump = all state variables stay the same
    trans = []
    for var_name, _, kind in original_variables:
        if kind is nssymb_table.SYMBOL_STATE_VAR:
            trans.append(var_name.next() == var_name)
    trans = reduce(lambda e, n: e & n, trans)
    
    new_trans[name + "_jump"] = trans
    
    return (new_variables, new_trans)

def encode_vars_trans(fsm, name, new_variables, new_trans):
    """
    Encode the new variables in new_variables and the new transition relation
    in new_trans.
    
    The result is
        - the variables in new_variables have been declared and encoded in a
          new layer named name
        - fsm.transitions has been populated with the transition relations of
          new_trans
    """
    if not hasattr(fsm, "transitions"):
        fsm.transitions = {}
    
    symb_table = fsm.bddEnc.symbTable
    new_layer = name
    symb_table.create_layer(new_layer)
    
    # Encode variables
    for var, type_, kind in new_variables:
        if kind is nssymb_table.SYMBOL_STATE_VAR:
            symb_table.declare_state_var(new_layer, var, type_)
        elif kind is nssymb_table.SYMBOL_FROZEN_VAR:
            symb_table.declare_frozen_var(new_layer, var, type_)
        elif kind is nssymb_table.SYMBOL_INPUT_VAR:
            symb_table.declare_input_var(new_layer, var, type_)
    glob.encode_variables_for_layers(layers=[new_layer])
    
    # Create and store transition relations (equiv, jump, follow)
    for trans_name, trans_expr in new_trans.items():
        trans = BddTrans.from_string(symb_table, str(trans_expr))
        fsm.transitions[trans_name] = trans
    
    # Sentinel value for specifying that transitions for group_name have been
    # computed
    fsm.transitions[name] = None

def encode_strat(fsm, strat, observables, actions, name):
    """
    Encode the strategies stored in strat in fsm given by strat with name
    as prefix. The strategies are represented by strat (a set of moves),
    observables (the observable state variables) and actions (the input
    variables representing the actions of the moves).
    
    The result is
        - the variables necessary to encode strat have been declared and
          encoded in a new layer named name
        - fsm.transitions has been populated by three new transition relations:
            1. name_follow is the transition relation encoding the fact that
               the current strategy is followed (that is moves are followed
               and the strategy stays the same)
            2. name_equiv is the transition relation encoding the equivalence
               classes of observables, while conserving the current strategy
            3. name_jump is the transtion relation encoding the jump from
               one strategy to another, keeping the current state intact
    
    Note: we assume that the flat hierarchy of fsm is glob.flat_hierarchy()
    """
    
    if not hasattr(fsm, "transitions"):
        fsm.transitions = {}
    
    flat = glob.flat_hierarchy()
    symb_table = fsm.bddEnc.symbTable
    new_layer = name
    
    # Get original transition relation and original variables
    original_trans = flat.trans
    original_variables = []
    for variable in flat.variables:
        original_variables.append((variable,
                                   symb_table.get_variable_type(variable),
                                   nssymb_table.SymbTable_get_symbol_category
                                   (symb_table._ptr, variable._ptr)))
    
    # Get variables and transition relations
    new_variables, new_trans = vars_trans_from_protocol(fsm,
                               original_variables, original_trans,
                               strat, observables, actions, name)
    
    encode_vars_trans(fsm, name, new_variables, new_trans)


def eval_strat(fsm, spec):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    spec is a strategic operator <G> pi.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator.
    """
    
    agents = {atom.value for atom in spec.group}
    agents_actions = set()
    for agent in agents:
        agents_actions |= fsm.agents_inputvars[agent]
    agents_obs = set()
    for agent in agents:
        agents_obs |= fsm.agents_observed_variables[agent]
    strat = fsm.protocol(agents)
    group_name = "_".join(sorted(agents))
    
    # Create the transition relations for agents if needed
    if not hasattr(fsm, "transitions") or group_name not in fsm.transitions:
        
        if config.debug:
            print("Fully symbolic approach: encoding strategies for {}."
                  .format(group_name))
        
        encode_strat(fsm, strat, agents_obs, agents_actions, group_name)

    # Compute the set of winning states
    if config.debug:
        print("Fully symbolic approach: computing winning states for {}."
              .format(spec))
    
    if type(spec) is CEX:
        win = cex_symbolic(fsm, group_name,
                           evalATLK(fsm, spec.child, variant="SF"))

    elif type(spec) is CEG:
        win = cew_symbolic(fsm, group_name,
                           evalATLK(fsm, spec.child, variant="SF"),
                           BDD.false(fsm.bddEnc.DDmanager))

    elif type(spec) is CEU:
        win = ceu_symbolic(fsm, group_name,
                           evalATLK(fsm, spec.left, variant="SF"),
                           evalATLK(fsm, spec.right, variant="SF"))

    elif type(spec) is CEF:
        win = ceu_symbolic(fsm, group_name,
                           BDD.true(fsm.bddEnc.DDmanager),
                           evalATLK(fsm, spec.child, variant="SF"))

    elif type(spec) is CEW:
        win = cew_symbolic(fsm, group_name,
                           evalATLK(fsm, spec.left, variant="SF"),
                           evalATLK(fsm, spec.right, variant="SF"))
    
    if config.debug:
        print("Fully symbolic approach: winning states for {} computed."
              .format(spec))
    
    return win


def spec_to_group_name(spec):
    """Return a name that can be used as a NuSMV identifier, based on spec."""
    return "spec" + (str(spec)
                     .replace(" ", "_")
                     .replace("&", "and")
                     .replace("|", "or")
                     .replace("<", "")
                     .replace(">", "")
                     .replace("~", "")
                     .replace("'", "")
                     .replace("=", "")
                     .replace("(", "")
                     .replace(")", "")
                     .replace("[", "")
                     .replace("]", "")
                    )
    
def eval_strat_FSF(fsm, spec):
    """
    Return the BDD representing the set of states of fsm satisfying spec.
    spec is a strategic operator <G> pi.
    
    Implement a variant of the algorithm that
    filters, splits and then filters.
    
    fsm -- a MAS representing the system;
    spec -- an AST-based ATLK specification with a top strategic operator.
    
    """
    
    # For every sub-formula we build a new set of transition relations
    # TODO evaluate whether building a set of transition relations per
    # group of agents is better (this means limiting oneself to a set of
    # sub-formulas...)
    
    group_name = spec_to_group_name(spec)
    
    # Create the transition relations for agents if needed
    if not hasattr(fsm, "transitions") or group_name not in fsm.transitions:
        agents = {atom.value for atom in spec.group}

        strat = filter_strat(fsm, spec, strat=fsm.protocol(agents),
                             variant="FSF")

        if strat.is_false():
            return strat
        
        agents_actions = set()
        for agent in agents:
            agents_actions |= fsm.agents_inputvars[agent]
        agents_obs = set()
        for agent in agents:
            agents_obs |= fsm.agents_observed_variables[agent]
        
        encode_strat(fsm, strat, agents_obs, agents_actions, group_name)

    # Compute the set of winning states
    if type(spec) is CEX:
        winning = cex_symbolic(fsm, group_name,
                               evalATLK(fsm, spec.child, variant="FSF"))

    elif type(spec) is CEG:
        winning = cew_symbolic(fsm, group_name,
                               evalATLK(fsm, spec.child, variant="FSF"),
                               BDD.false(fsm.bddEnc.DDmanager))

    elif type(spec) is CEU:
        winning = ceu_symbolic(fsm, group_name,
                               evalATLK(fsm, spec.left, variant="FSF"),
                               evalATLK(fsm, spec.right, variant="FSF"))

    elif type(spec) is CEF:
        winning = ceu_symbolic(fsm, group_name,
                               BDD.true(fsm.bddEnc.DDmanager),
                               evalATLK(fsm, spec.child, variant="FSF"))

    elif type(spec) is CEW:
        winning = cew_symbolic(fsm, group_name,
                               evalATLK(fsm, spec.left, variant="FSF"),
                               evalATLK(fsm, spec.right, variant="FSF"))
    
    winning = (winning.forsome(fsm.bddEnc.inputsCube)
               & fsm.reachable_states)
    return winning