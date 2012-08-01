"""
ARCTL explain functions.
"""

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
    
    Return a tuple (path, (inputs, loop)) where path is a tuple (s0, i1, .., sn)
    where
        s0 = state
        sj -ij+1-> sj+1 for all j : 0 <= j < n
        sn -inputs-> loop
        loop = sj for some j : 0 <= j <= n
        sj belongs to phi for all j : 0 <= j <= n
        ij belongs to alpha for all j : 0 <= j <= n
        inputs belongs to alpha.
    """
    pass # TODO
    
    
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
    pass # TODO