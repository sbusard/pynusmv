from pynusmv.dd import BDD

from .eval import ex, eg, eu, fair_states
from tools.explanation.explanation import Explanation


def explain_one_succ(fsm, state, sat):
    """
    Explain why state can reach sat in one step.
    The returned value is a tuple (action, successor)
    where successor is a successor of state through action belonging to sat.
    
    fsm -- the fsm;
    state -- a state of fsm able to reach sat in one step;
    sat -- a BDD representing a set of states of fsm.    
    """
    next = fsm.pick_one_state(fsm.post(state) & sat)
    inputs = fsm.pick_one_inputs(fsm.get_inputs_between_states(state, next))
    return (inputs, next)
    

def explain_all_succ(fsm, state, sat):
    """
    Give all the successors of state belonging to sat.
    The returned value is a set of tuples (action, successor)
    where successor is a successor of state through action belonging to sat.
    
    fsm -- the fsm;
    state -- a state of fsm able to reach sat in one step;
    sat -- a BDD representing a set of states of fsm.    
    """
    successors = set()
    for next in fsm.pick_all_states(fsm.post(state) & sat):
        inputs = fsm.pick_one_inputs(fsm.get_inputs_between_states(state, next))
        successors.add((inputs, next))
    return successors
    

def explain_ex(fsm, state, phi):
    """
    Explain why state of fsm satisfies EX phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying EX phi;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies EX phi, we have to exhibit a successor
    # of state belonging to phi that is reachable and fair.
    
    # EX phi = fsm.pre(phi & fair)
    
    # We don't explain why the extracted successor is fair.
    
    phi = phi & fair_states(fsm) & fsm.reachable_states
    
    # Get successor of state belonging to phi
    inputs, next = explain_one_succ(fsm, state, phi)
    
    return Explanation(state, {(inputs, Explanation(next))})
    
    
def explain_eu(fsm, state, phi, psi):
    """
    Explain why state of fsm satisfies E phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying E phi U psi;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    # E phi U psi = mu Z . (psi & fair) | (phi & fsm.pre(Z))
    
    # To show that state satisfies E phi U psi
    # we have to show a path of successors satisfying phi,
    # ending in a fair state satisfying psi.
    
    # If state satisfies psi, stop;
    # otherwise, compute the fixpoint until state is in the last set,
    # then show that state can reach the previous set,
    # and recursively explain why the reached state satisfies E phi U psi.
    
    # We don't explain why the psi-states are fair.
    
    if state <= psi:
        return Explanation(state)
        
    else:
        # Compute the fixpoint until state is reached (backward)
        fair = fair_states(fsm)
        f = lambda Y : (psi & fair) | (phi & fsm.pre(Y))
        old = BDD.false(fsm.bddEnc.DDmanager)
        new = psi
        while not state <= new:
            old = new
            new = f(old)
            
        # If state <= new, (and it was not the case in the previous iteration)
        # old does not contain state but a successor of state.
        
        # explain that state can reach old
        inputs, next = explain_one_succ(fsm, state, old)
        successors = {(inputs, explain_eu(fsm, next, phi, psi))}
        
        return Explanation(state, successors)
    
    
def explain_eg(fsm, state, phi):
    """
    Explain why state of fsm satisfies EG phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying EG phi;
    phi -- the set of states of fsm satifying phi.
    """
    # To explain why state satisfies EG phi,
    # we have to show a path ending with a loop going through all fairness
    # constraints
    
    # If no fairness constraints are provided,
    # model checking sums up to
    # EG p = nu Z. p & Pre(Z)
    # In this case, just explain by extending EX EG p until a former state is
    # encountered
    if len(fsm.fairness_contraints) <= 0:
        states = BDD.false(fsm.bddEnc.DDmanager)
        orig_expl = expl = Explanation(state)
        eg_phi = eg(fsm, phi)
        while not state <= states:
            states = states | state
            if (fsm.post(state) & states).isnot_false():
                # There is a successor of state that we already saw.
                # Use this one
                action, state = explain_one_succ(fsm, state, states)
            else:
                # There is no successor of state in states, guess a successor
                action, state = explain_one_succ(fsm, state, eg_phi)
            nextexpl = Explanation(state)
            expl.successors = {(action, nextexpl)}
            expl = nextexpl
        # state <= states
        # this means that state lead to a previous state in the path
        # we must explain it
        cur = orig_expl
        while cur.state != state:
            cur = cur.successors[0][1]
        action = fsm.pick_one_inputs(
                                fsm.get_inputs_between_states(state, cur.state))
        expl.successors = {(action, cur)}
        return orig_expl
    
    # Otherwise, there are fairness constraints
    else:
        pass # TODO
    
    
def explain_ew(fsm, state, phi, psi):
    """
    Explain why state of fsm satisfies E phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying E phi W psi;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    if state <= eg(fsm, phi):
        return explain_eg(fsm, state, phi)
    else: # state <= eu(fsm, phi, psi)
        return explain_eu(fsm, state, phi, psi)
    
    
def explain_ax(fsm, state, phi):
    """
    Explain why state of fsm satisfies AX phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying AX phi;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies AX phi, we have to show that all successors
    # satisfy phi or are not fair.
    
    # AX phi = ~fsm.pre((~phi & fair))
    
    # We don't explain why the extracted successors are not fair (when it is
    # the case).
    
    # phi or not fair state
    phi = (phi | ~fair_states(fsm)) & fsm.reachable_states
    
    # Get successor of state belonging to phi
    all_succ = explain_all_succ(fsm, state, phi)
    
    # Build successors
    successors = set()
    for inputs, next in all_succ:
        succ.add((inputs, Explanation(next)))
    
    return Explanation(state, succ)
    
    
def explain_au(fsm, state, phi, psi, states=None):
    """
    Explain why state of fsm satisfies A phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying A phi U psi;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi;
    states -- a dictionary of state->explanation pairs.
    """
    pass # TODO
    
    
def explain_ag(fsm, state, phi):
    """
    Explain why state of fsm satisfies AG phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying AG phi;
    phi -- the set of states of fsm satifying phi.
    """
    pass # TODO
    
    
def explain_aw(fsm, state, phi, psi):
    """
    Explain why state of fsm satisfies A phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying A phi W psi;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    pass # TODO
    
    
def explain_fair(fsm, state):
    """
    Explain why state of fsm is a fair state.
    
    fsm -- the fsm;
    state -- a fair state of fsm.
    """
    return explain_eg(fsm, state, BDD.true(fsm.bddEnc.DDmanager))
    
    
def explain_not_fair(fsm, state):
    """
    Explain why state of fsm is not a fair state.
    
    fsm -- the fsm;
    state -- a non-fair state of fsm.
    """
    pass # TODO