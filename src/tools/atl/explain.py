from pynusmv.dd import BDD

from .eval import ceg, ceu, cew

class Explanation():
    """
    Explanations consistute a graph explaining why a particular state satisfies
    an ATL property.
    
    An Explanation is composed of state and a list of successors reachable
    through inputs.
    """
    def __init__(self, state, successors=None):
        """
        state -- the state of the explanation node;
        successors -- a set of (inputs, successor) pairs where successors
                      are also explanations.
        """
        self.state = state
        self.successors = successors if successors else set()
        
    
    def dot(self):
        """
        Return a dot representation (as a text) of this explanation.
        """
        # Get all states, keep them and mark them
        ids = {}
        curid = 0
        extract = {self}
        while len(extract) > 0:
            expl = extract.pop()
            ids[expl] = "s" + str(curid)
            curid += 1
            for (action, succ) in expl.successors:
                if succ not in ids:
                    extract.add(succ)
                
        dot = "digraph {"
        
        # Add states to the dot representation
        for expl in ids:
            dot += (ids[expl] + " " + "[label=\"" +
                    '\\n'.join(var + "=" + val for var, val in
                                            expl.state.get_str_values().items()) +
                    "\"]" + ";\n")
        
        # For each state, add each transition to the representation
        for expl in ids:
            for action, succ in expl.successors:
                dot += (ids[expl] + "->" + ids[succ] + " " +
                        "[label=\"" + "\\n".join(var + "=" + val for var, val in 
                                        action.get_str_values().items()) + "\"]"
                        + ";\n")
        
        dot += "}"
        
        return dot
    

def explain_cex(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies <agents> X phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> X phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies <agents> X phi, we have to exhibit an action
    # of agents such that all reached states are in phi.
    # We choose to execute this action, that is, exhibit the state, and
    # all the reached states through the action.
    
    # Cubes
    gamma_cube = fsm.inputs_cube_for_agents(agents)
    ngamma_cube = fsm.bddEnc.inputsCube - gamma_cube
    
    # Get the winning actions, for all state
    si = fsm.pre_strat_si(phi, agents)
    # The winning actions from state are the ones belonging to its protocol
    # and to si
    actions = (si & (fsm.protocol(agents) & state) &
               fsm.bddEnc.inputsMask).forsome(fsm.bddEnc.statesCube)
    fullaction = fsm.pick_one_inputs(actions)
    ag_action = fullaction.forsome(ngamma_cube)
    
    
    # Build the graph
    successors = set()
    for successor in fsm.pick_all_states(fsm.post(state, ag_action) &
                                         fsm.bddEnc.statesMask):
        nextexpl = Explanation(successor)
        allactions = fsm.get_inputs_between_states(state, successor) & ag_action 
        for action in fsm.pick_all_inputs(allactions & fsm.bddEnc.inputsMask):
            successors.add((action, nextexpl))
    return Explanation(state, successors)
    
    
def explain_ceu(fsm, state, agents, phi, psi, states=None):
    """
    Explain why state of fsm satisfies <agents> phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> phi U psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi;
    states -- a dictionary of state->explanation pairs.
    """
    # <agents> phi U psi = mu Y. psi | (phi & fsm.pre_strat(Y, agents))
    
    # To show that state satisfies <agents> phi U psi
    #   if state in psi, return Explanation(state) because state is its own
    #   explanation
    #   otherwise, compute the fixpoint, until state is in the last set
    #   then choose one action always leading to states of the previous set
    #   (it is possible because state is added because it can go one step in
    #   the right direction), and recursively explain why the found states
    #   satisfy <agents> phi U psi.
    
    # states is used to store already explained states and create a directed
    # acyclic graph instead of the pure execution tree, to avoid a blow up
    # in terms of number of states.
    
    if states is None:
        states = {}
    
    if state in states:
        return states[state]
    
    if state <= psi:
        new = Explanation(state)
        states[state] = new
        return new
        
    else:
        f = lambda Y : psi | (phi & fsm.pre_strat(Y, agents))
        old = BDD.false(fsm.bddEnc.DDmanager)
        new = psi
        while not state <= new:
            old = new
            new = f(old)
        
        expl = explain_cex(fsm, state, agents, old)
        
        successors = set()
        for action, succ in expl.successors:
            successors.add((action,
                            explain_ceu(fsm, succ.state, agents,
                                        phi, psi, states)))
        new = Explanation(state, successors)
        states[state] = new
        return new
    
    
def explain_ceg(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies <agents> G phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> G phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies <agents> G phi
    # we just have to explain why state satisfies <agents>X<agents>G phi
    # and iterate until we reach the fixpoint.
    # By keeping track of already visited states,
    # we know that we will finally extract the full sub-system of phi states.
    
    # Get CEG(phi)
    cegphi = ceg(fsm, agents, phi)
    
    # Get all states and transitions
    extract = {state}
    states = set()
    transitions = set()
    while len(extract) > 0:
        cur = extract.pop()
        states.add(cur)
        expl = explain_cex(fsm, cur, agents, cegphi)
        for act, succ in expl.successors:
            transitions.add((cur, act, succ.state))
            if succ.state not in states:
                extract.add(succ.state)
    
    # Build the graph
    nodes = {}
    for s in states:
        nodes[s] = Explanation(s)
    for cur, act, succ in transitions:
        nodes[cur].successors.add((act, nodes[succ]))
    
    return nodes[state]
    
    
def explain_cew(fsm, state, agents, phi, psi):
    """
    Explain why state of fsm satisfies <agents> phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> phi W psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    # To show that state satisfies <agents> phi W psi
    # either it satisfies psi and we are done,
    # or we have to explain why state satisfies <agents>X<agents>phi W psi
    # and iterate until we reach the fixpoint or stop at psi.
    # By keeping track of already visited states,
    # we know that we will finally extract the full sub-system.
    
    # Get CEW(phi,psi)
    cewpp = cew(fsm, agents, phi, psi)
    
    # Get all states and transitions
    extract = {state}
    states = set()
    transitions = set()
    while len(extract) > 0:
        cur = extract.pop()
        states.add(cur)
        if not cur <= psi:
            expl = explain_cex(fsm, cur, agents, cewpp)
            for act, succ in expl.successors:
                transitions.add((cur, act, succ.state))
                if succ.state not in states:
                    extract.add(succ.state)
    
    # Build the graph
    nodes = {}
    for s in states:
        nodes[s] = Explanation(s)
    for cur, act, succ in transitions:
        nodes[cur].successors.add((act, nodes[succ]))
    
    return nodes[state]
    
    
def explain_cax(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies [agents] X phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] X phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies [agents] X phi, we have to show that
    # for each action of agents, there is a completion by other agents such that
    # the reached state satisfies phi.
    # That is, we have to take all actions and of agents and for each of them
    # exhibit a successor satisfying phi.
    
    # Cubes
    gamma_cube = fsm.inputs_cube_for_agents(agents)
    ngamma_cube = fsm.bddEnc.inputsCube - gamma_cube
    
    # Get all different actions of agents
    ag_actions = set()
    for action in fsm.pick_all_inputs((fsm.protocol(agents) &
                                         state).forsome(fsm.bddEnc.statesCube)):
        ag_actions.add(action.forsome(ngamma_cube))
    
    # Get successors
    successors = set()
    for ag_action in ag_actions:
        # Choose one successor through ag_action
        succ = fsm.pick_one_state(fsm.post(state, ag_action) & phi &
                                  fsm.bddEnc.statesMask)
        # Get the full action
        act = fsm.pick_one_inputs(fsm.get_inputs_between_states(state, succ) &
                                  ag_action & fsm.bddEnc.inputsMask)
        successors.add((act, Explanation(succ)))
    
    return Explanation(state, successors)    
    
    
def explain_cau(fsm, state, agents, phi, psi, states=None):
    """
    Explain why state of fsm satisfies [agents] phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] phi U psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi;
    states -- a dictionary of state->explanation pairs.
    """
    # [agents] phi U psi = mu Y. psi | (phi & fsm.pre_nstrat(Y, agents))
    
    # To show that state satisfies [agents] phi U psi
    #   if state in psi, return Explanation(state) because state is its own
    #   explanation
    #   otherwise, compute the fixpoint, until state is in the last set
    #   then show that all actions always lead to states of the previous set
    #   (it is possible because state is added because it cannot avoid to go
    #   one step in the right direction), and recursively explain why the found
    #   states satisfy [agents] phi U psi.
    
    # states is used to store already explained states to create a directed
    # acyclic graph instead of the pure execution tree.
    
    if states is None:
        states = {}
        
    if state in states:
        return states[state]
    
    if state <= psi:
        new = Explanation(state)
        states[state] = new
        return new
        
    else:
        f = lambda Y : psi | (phi & fsm.pre_nstrat(Y, agents))
        old = BDD.false(fsm.bddEnc.DDmanager)
        new = psi
        while not state <= new:
            old = new
            new = f(old)
        
        expl = explain_cax(fsm, state, agents, old)
        
        successors = set()
        for action, succ in expl.successors:
            successors.add((action,
                            explain_cau(fsm, succ.state, agents,
                                        phi, psi, states)))
        new = Explanation(state, successors)
        states[state] = new
        return new
    
    
def explain_cag(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies [agents] G phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] G phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies [agents] G phi
    # we just have to explain why state satisfies [agents]X[agents]G phi
    # and iterate until we reach the fixpoint.
    # By keeping track of already visited states,
    # we know that we will finally extract the full sub-system of phi states.
    
    # Get CAG(phi)
    cagphi = ~ceu(fsm, agents, BDD.true(fsm.bddEnc.DDmanager), ~phi)
    
    # Get all states and transitions
    extract = {state}
    states = set()
    transitions = set()
    while len(extract) > 0:
        cur = extract.pop()
        states.add(cur)
        expl = explain_cax(fsm, cur, agents, cagphi)
        for act, succ in expl.successors:
            transitions.add((cur, act, succ.state))
            if succ.state not in states:
                extract.add(succ.state)
    
    # Build the graph
    nodes = {}
    for s in states:
        nodes[s] = Explanation(s)
    for cur, act, succ in transitions:
        nodes[cur].successors.add((act, nodes[succ]))
    
    return nodes[state]
    
    
def explain_caw(fsm, state, agents, phi, psi):
    """
    Explain why state of fsm satisfies [agents] phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] phi W psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    # To show that state satisfies [agents] phi W psi
    # either it satisfies psi and we are done,
    # or we have to explain why state satisfies [agents]X[agents]phi W psi
    # and iterate until we reach the fixpoint or stop at psi.
    # By keeping track of already visited states,
    # we know that we will finally extract the full sub-system.
    
    # Get CEW(phi,psi)
    cawpp = ~ceu(fsm, agents, ~psi, ~psi & ~phi)
    
    # Get all states and transitions
    extract = {state}
    states = set()
    transitions = set()
    while len(extract) > 0:
        cur = extract.pop()
        states.add(cur)
        if not cur <= psi:
            expl = explain_cax(fsm, cur, agents, cawpp)
            for act, succ in expl.successors:
                transitions.add((cur, act, succ.state))
                if succ.state not in states:
                    extract.add(succ.state)
    
    # Build the graph
    nodes = {}
    for s in states:
        nodes[s] = Explanation(s)
    for cur, act, succ in transitions:
        nodes[cur].successors.add((act, nodes[succ]))
    
    return nodes[state]