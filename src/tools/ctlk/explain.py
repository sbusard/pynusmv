"""
Extract simple path from a model to explain particular CTLK operators.
"""

from pynusmv.dd import BDD

from .eval import eg, ex, eu, nk, ne, nc, nd

def explain_ex(fsm, state, p):
    """
    Return a path explaining why state of fsm satisfies EX phi.
    
    fsm -- a MAS
    state -- a State of fsm satisfying EX phi
    p -- a BDD representing the set of states of fsm satisfying the property phi
    
    Return a triple (s, i, s') where
        s is state,
        s' is a state of p that is a successor of s,
        i is one possible input between s and s' in fsm
    The returned value represents a two-states path in fsm starting at state
    and ending with a successor of state from p.
    """
    nexts = fsm.post(state)
    sp = fsm.pick_one_state(nexts & p)
    return (state, 
            fsm.pick_one_inputs(fsm.get_inputs_between_states(state, sp)),
            sp)
    

def explain_eg(fsm, state, p):
    """
    Return a path explaining why state of fsm satisfies EG phi.
    
    fsm -- a MAS
    state -- a State of fsm satisfying EG phi
    p -- a BDD representing the set of states of fsm satisfying the property phi
    
    Return a tuple (path, loop) where
        path is a tuple (s_0,..., i_n, s_n) where
            s_0 is state
            s_j belongs to p forall j
            i_j is a possible input between s_j-1 and s_j in fsm
        loop is a tuple (i_l, s_l) where
            s_l belongs to path
            i_l is a possible input between s_n of path and s_l.
    The returned value represents a looping path in fsm composed of states of p
    and starting at state.
    """
    
    allstates = eg(fsm, p)

    # Start path at s
    path = [state]
    # While path[-1] cannot reach itself through states of allstates,
    while (path[-1] & ex(fsm, eu(fsm, allstates, path[-1]))).is_false():
        # choose a successor of path[-1] in allstates and add it in path
        succ = fsm.pick_one_state(fsm.post(path[-1]) & allstates)
        path.append(fsm.pick_one_inputs(
                                fsm.get_inputs_between_states(path[-1], succ)))
        path.append(succ)

    # At this point, path[-1] can reach itself through states of allstates
    # Explain it with explain_eu and explain_ex
    eus = eu(fsm, allstates, path[-1])
    first = explain_ex(fsm, path[-1], eus)
    second = explain_eu(fsm, first[-1], allstates, path[-1])
    fs = tuple(path) + first[1:] + second[1:]

    # Store the loop
    inputs = fs[-2]
    loop = fs[-1]

    # Return the path and the loop
    return (fs[:-2], (inputs, loop))
    

def explain_eu(fsm, state, p, q):
    """
    Return a path explaining why state of fsm satisfies E[ phi U psi ].
    
    fsm -- a MAS
    state -- a State of fsm satisfying E[ phi U psi ]
    p -- a BDD representing the set of states of fsm satisfying the property phi
    q -- a BDD representing the set of states of fsm satisfying the property psi
    
    Return a tuple (s_0, ..., i_n, s_n) where
        s_0 is state
        s_j belongs to p forall j : 0 <= j < n
        s_n belongs to q
        i_j is a possible input between s_j-1 and s_j in fsm.
    The returned value represents a finite path in fsm, ending with a state of q
    with intermediate states of p and starting at state.
    """
    
    # Compute fixpoint and store intermediate BDDs
    funct = lambda Z: (q | (p & fsm.pre(Z)))
    old = BDD.false(fsm.bddEnc.DDmanager)
    new = funct(old)
    paths = [new]
    # Stop when reaching state
    # This is ensured since state satisfies E[ phi U psi ]
    while (state & new).is_false():
        old = new
        new = funct(old)
        paths.append(new - old)
        
    # paths contains intermediate BDDs
    # paths[i] contains the BDD of all states of phi
    # that can reach a state of psi
    # through states of phi, in i steps
    
    # paths[-1] contains state, skip it
    paths = paths[:-1]
    s = state
    path = [s]
    for states in paths[::-1]:
        sp = fsm.pick_one_state(fsm.post(s) & states)
        i = fsm.pick_one_inputs(fsm.get_inputs_between_states(s, sp))
        path.append(i)
        path.append(sp)
        s = sp
    
    return tuple(path)
    

def explain_nk(fsm, state, agent, p):
    """
    Return a couple of states explaining why state satisfies nK<agent> phi.
    
    fsm -- a MAS
    state -- a State of fsm satisfying nK<agent> phi
    agent -- the name of an agent of fsm
    p -- a BDD representing the set of states of fsm satisfying the property phi
    
    Return a tuple (s, ag, s') where
        s is state
        s' belongs to p
        s' is reachable
        s' is equivalent to s for agent in fsm
        ag is {{agent}}.
    The returned value explains why state satisfies nK<agent> phi in fsm
    by showing a state equivalent to state for agent that satisfies phi.
    """
    equiv_states = nk(fsm, agent, state) & fsm.reachable_states
    sp = fsm.pick_one_state(equiv_states & p)
    return (state, frozenset({agent}), sp)
    

def explain_ne(fsm, state, group, p):
    """
    Return a couple of states explaining why state satisfies nE<group> phi.
    
    fsm -- a MAS
    state -- a State of fsm satisfying nE<group> phi
    group -- a list of names of agents of fsm
    p -- a BDD representing the set of states of fsm satisfying the property phi
    
    Return a tuple (s, agent, s') where
        s is state
        s' belongs to p
        s' is equivalent to s for some agent of group in fsm
        agent is the agent of group for which s and s' are equivalent.
    The returned value explains why state satisfies nE<group> phi in fsm
    by showing a state equivalent to state for some agents in group
    that satisfies phi.
    """
    for agent in group:
        nks = nk(fsm, agent, p)
        if state <= nks:
            return explain_nk(fsm, state, agent, p)
    # If this is reached, state |/= nE<group> phi
    

def explain_nd(fsm, state, group, p):
    """
    Return a couple of states explaining why state satisfies nD<group> phi.
    
    fsm -- a MAS
    state -- a State of fsm satisfying nD<group> phi
    group -- a list of names of agents of fsm
    p -- a BDD representing the set of states of fsm satisfying the property phi
    
    Return a tuple (s, group, s') where
        s is state
        s' belongs to p
        s' is distributively equivalent to s for group in fsm.
    The returned value explains why state satisfies nD<group> phi in fsm
    by showing a state group-distributively equivalent to state
    that satisfies phi.
    """
    equiv_states = nd(fsm, group, state) & fsm.reachable_states
    sp = fsm.pick_one_state(equiv_states & p)
    return (state, frozenset(group), sp)
    

def explain_nc(fsm, state, group, p):
    """
    Return a path group's agents equivalent states
    explaining why state satisfies nC<group> phi.
    
    fsm -- a MAS
    state -- a State of fsm satisfying nC<group> phi
    group -- a list of names of agents of fsm
    p -- a BDD representing the set of states of fsm satisfying the property phi
    
    Return a tuple (s_0, ag_1, ..., ag_n, s_n) where
        s_0 is state
        s_n belongs to p
        s_j is equivalent to s_j+1 for some agent in group in fsm,
            forall j : 0 <= j < n
        ag_j+1 is the name of the agent of group
            for which s_j and s_j+1 are equivalent, forall j : 0 <= j < n
    The returned value explains why state satisfies nC<group> phi in fsm
    by showing path in the knowledge of agents of group
    to a state that satisfies phi.
    """
    
    # Compute fixpoint and store intermediate BDDs
    funct = lambda Z: ne(fsm, group, (Z | p))
    old = BDD.false(fsm.bddEnc.DDmanager)
    new = funct(old)
    paths = [new]
    # Stop when reaching state
    # This is ensured since state satisfies nC<group> phi
    while (state & new).is_false():
        old = new
        new = funct(old)
        paths.append(new - old)
    
    # paths[-1] contains state, skip it
    paths = paths[:-1]
    s = state
    path = [s]
    for states in paths[::-1]:
        (olds, ag, s) = explain_ne(fsm, s, group, states)
        path.append(ag)
        path.append(s)
    # Still need to show why the last one satisfies nE<group> phi
    (olds, ag, s) = explain_ne(fsm, path[-1], group, p)
    path.append(ag)
    path.append(s)
    
    return tuple(path)
    
    
def explain_reachable(fsm, state):
    """
    Return a reversed path explaining why state of fsm is reachable.
    
    fsm -- a MAS
    state -- a reachable State of fsm
    
    Return a tuple (s_0, ..., i_n, s_n) where
        s_0 is state
        s_n belongs is an initial state in fsm
        i_j is a possible input between s_j-1 and s_j in fsm
            forall j : 0 < j <= n.
    The returned value represents a finite reversed path in fsm,
    ending with an initial state and starting at state.
    """
    
    # Compute fixpoint and store intermediate BDDs
    funct = lambda Z: (fsm.init | fsm.post(Z))
    old = BDD.false(fsm.bddEnc.DDmanager)
    new = funct(old)
    paths = [new]
    # Stop when reaching state
    # This is ensured since state is reachable
    while (state & new).is_false():
        old = new
        new = funct(old)
        paths.append(new - old)
        
    # paths contains intermediate BDDs
    # paths[i] contains the BDD of all reachable states
    # that can be reached from an initial state in i steps
    
    # paths[-1] contains state, skip it
    paths = paths[:-1]
    s = state
    path = [s]
    for states in paths[::-1]:
        sp = fsm.pick_one_state(fsm.pre(s) & states)
        i = fsm.pick_one_inputs(fsm.get_inputs_between_states(sp, s))
        path.append(i)
        path.append(sp)
        s = sp
    
    return tuple(path)