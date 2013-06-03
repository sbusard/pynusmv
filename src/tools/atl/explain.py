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
    pass # TODO
    
    
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
    pass # TODO
    
    
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