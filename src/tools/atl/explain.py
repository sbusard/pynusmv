from pynusmv.dd import BDD

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
    actions = si & (fsm.protocol(agents) & state)
    fullaction = fsm.pick_one_inputs(actions)
    ag_action = fullaction.forsome(ngamma_cube)
    
    
    # Build the graph
    successors = set()
    for successor in fsm.pick_all_states(fsm.post(state, ag_action)):
        nextexpl = Explanation(successor)
        allactions = fsm.get_inputs_between_states(state, successor) & ag_action 
        for action in fsm.pick_all_inputs(allactions):
            successors.add((action, nextexpl))
    return Explanation(state, successors)
    
    
def explain_ceu(fsm, state, agents, phi, psi):
    """
    Explain why state of fsm satisfies <agents> phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> phi U psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
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
    
    if state <= psi:
        return Explanation(state)
        
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
                            explain_ceu(fsm, succ.state, agents, phi, psi)))
        return Explanation(state, successors)
    
    
def explain_ceg(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies <agents> G phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> G phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    pass # TODO
    
    
def explain_cew(fsm, state, agents, phi, psi):
    """
    Explain why state of fsm satisfies <agents> phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> phi W psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    pass # TODO
    
    
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
        succ = fsm.pick_one_state(fsm.post(state, ag_action) & phi)
        # Get the full action
        act = fsm.pick_one_inputs(fsm.get_inputs_between_states(state, succ) &
                                  ag_action)
        successors.add((act, Explanation(succ)))
    
    return Explanation(state, successors)    
    
    
def explain_cau(fsm, state, agents, phi, psi):
    """
    Explain why state of fsm satisfies [agents] phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] phi U psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    pass # TODO
    
    
def explain_cag(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies [agents] G phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] G phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    pass # TODO
    
    
def explain_caw(fsm, state, agents, phi, psi):
    """
    Explain why state of fsm satisfies [agents] phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] phi W psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    pass # TODO