from pynusmv.dd.bdd import BDD
from pynusmv.fsm.bddFsm import BddFsm
from .exception import UnknownAgentError


class MAS(BddFsm):
    """
    A multi-agent system.
    
    A multi-agent system is an FSM with one temporal trans for the whole model
    and one epistemic trans by agent, labelled with the name of the agent.
    """
    
    def __init__(self, ptr, epistemic = None, freeit = False):
        """
        Create a new MAS.
        
        ptr -- the pointer of the NuSMV FSM
        epistemic -- a dictionary of epistemic relations (agent->EPISTEMIC)
        freeit -- whether or not free the pointer
        """
        super().__init__(ptr, freeit = freeit)
        self._epistemic = epistemic and epistemic or {}
        self._reachable = None
                                              
                                              
    def equivalent_states(self, states, agent):
        """
        Return the BDD representing the set of states epistemically equivalent
        to states through the epistemic relation of agent. agent is the name
        of a single agent.
        
        states -- a BDD of states of self
        agent -- the name of an agent. agent must be one of this MAS agents.
        """
            
        # Apply FSM constraints
        states = states & self.state_constraints
        
        # Compute the post-image
        if agent not in self._epistemic:
            raise UnknownAgentError(ag + " is an unknown agent name.")
        # pre_ or post_ operations can be used since the epistemic relations
        # are equivalence relations and thus are symetric
        result = self._epistemic[agent].pre(states)
        
        # Apply constraints on result
        return result & self.state_constraints