"""
ARCTL explain functions.
"""

from pynusmv.dd.bdd import BDD
from .eval import _ex, eag, _eu, eau, eax

def explain_eax(fsm, state, alpha, phi):
    """
    Explain why state of fsm satisfies E<a>X p.
    
    fsm -- the BddFsm of the model;
    state -- a State of fsm satisfying E<a>X p;
    alpha -- a BDD of fsm representing the inputs satisfying a;
    phi -- a BDD of fsm representing the states satisfying p.
    
    Return a tuple (state, inputs, state') where
        inputs belongs to alpha
        state' belongs to phi
        state' is reachable from state through inputs.
    """
    
    # Get next state: is a next of state and satisfies phi
    nexts = fsm.post(state, alpha)
    next = fsm.pick_one_state(nexts & phi)
    
    # Get inputs: is an input between state and next and satisfies alpha
    inputs = fsm.get_inputs_between_states(state, next)
    inputs = fsm.pick_one_inputs(inputs & alpha)
    
    return (state, inputs, next)
    
    
def explain_eag(fsm, state, alpha, phi):
    """
    Explain why state of fsm satisfies E<a>G p.
    
    fsm -- the BddFsm of the model;
    state -- a State of fsm satisfying E<a>G p;
    alpha -- a BDD of fsm representing the inputs satisfying a;
    phi -- a BDD of fsm representing the states satisfying p.
    
    Return a tuple (path, (inputs, loop)).
    
    If inputs and loop are None, this means that the witness is a full finite
    path and that path[-1] satisfies ~E<'alpha'>X 'TRUE' (dead state). In this
    case, path is a tuple (s0, i1, ..., sn) where sj |= phi for all j,
    ij |= alpha for all j and sj -ij+1-> sj+1 for all j.
    
    Otherwise, path is a tuple (s0, i1, ..., sn)
    where
        s0 = state
        sj -ij+1-> sj+1 for all j : 0 <= j < n
        sn -inputs-> loop
        loop = sj for some j : 0 <= j <= n
        sj belongs to phi for all j : 0 <= j <= n
        ij belongs to alpha for all j : 0 <= j <= n
        inputs belongs to alpha.
    """
    # eag(a, p) = _eu(a, p, p & ~_ex(a, true)) | _eg(a, p)
    
    paext = phi & ~_ex(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager))
    euppaext = _eu(fsm, alpha, phi, paext)
    
    # If state satisfies _eu(a, p, p & ~_ex(a, true)),
    # use explain_eau to extract such a path
    if state <= euppaext:
        return (explain_eau(fsm, alpha, phi, paext), (None, None))
    
    # Otherwise,
    else:    
        # Get allstates = E<alpha>G phi
        allstates = eag(fsm, alpha, phi)
    
        # Start path at s
        path = [state]
        # While path[-1] cannot reach itself through states of eag,
        while (path[-1] &
               eax(fsm,
                   alpha,
                   eau(fsm, alpha, allstates, path[-1]))).is_false():
            # choose a successor of path[-1] in eag and add it in path
            path.append(fsm.post(state, alpha) & allstates)
    
        # At this point, path[-1] can reach itself through states of eag
        # Explain it with explain_eau and explain_eax
        eaus = eau(fsm, alpha, allstates, path[-1])
        first = explain_eax(fsm, path[-1], alpha, eaus)
        second = explain_eau(fsm, first[-1], alpha, allstates, path[-1])
        fs = first + second[1:]
    
        # Store the loop
        inputs = fs[-2]
        loop = fs[-1]
    
        # Return the path and the loop
        return (fs[:-2], (inputs, loop))
    
    
def explain_eau(fsm, state, alpha, phi, psi):
    """
    Explain why state of fsm satisfies E<a>[p U q].
    
    fsm -- the BddFsm of the model;
    state -- a State of fsm satisfying E<a>[p U q];
    alpha -- a BDD of fsm representing the inputs satisfying a;
    phi -- a BDD of fsm representing the states satisfying p;
    psi -- a BDD of fsm representing the states satisfying q.
    
    Return a tuple (s0, i1, ..., sn) where
        s0 = state
        sj -ij+1-> sj+1 for all j : 0 <= j < n
        sj belongs to phi for all j : 0 <= j < n
        sn belongs to psi
        ij belongs to alpha for all j : 0 <= j <= n.
    """
    
    # Compute fixpoint and store intermediate BDDs
    funct = lambda Z: (psi | (phi & _ex(fsm, alpha, Z)))
    old = BDD.false(fsm.bddEnc.DDmanager)
    new = funct(old)
    paths = [new]
    # Stop when reaching state
    # This is ensured since state satisfies E<a>[p U q]
    while (state & new).is_false():
        old = new
        new = funct(old)
        paths.append(new - old)
        
    # paths contains intermediate BDDs
    # paths[i] contains the BDD of all states of phi
    # that can reach a state of psi
    # through states of phi, in i steps
    # restricted to paths of alpha actions
    
    # paths[-1] contains state, skip it
    paths = paths[:-1]
    s = state
    path = [s]
    for states in paths[::-1]:
        sp = fsm.pick_one_state(fsm.post(s, alpha) & states)
        i = fsm.pick_one_inputs(fsm.get_inputs_between_states(s, sp))
        path.append(i)
        path.append(sp)
        s = sp
    
    return tuple(path)