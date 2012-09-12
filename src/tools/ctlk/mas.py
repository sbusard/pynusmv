from pynusmv.dd.bdd import BDD
from tools.multimodal.mmFsm import MMFsm
from .exception import UnknownAgentError


class MAS(MMFsm):
    """
    A multi-agent system.
    
    A multi-agent system is a multi-modal FSM with one temporal trans by agent, 
    labelled with the name of this agent, and one epistemic trans by agent,
    labelled with NAME_knows where NAME is the agent's name.
    """
    
    def __init__(self, ptr, trans = None, epistemic = None, freeit = False):
        """
        Create a new MAS.
        
        ptr -- the pointer of the NuSMV FSM
        trans -- a dictionary of temporal transition relations (agent->TRANS)
        epistemic -- a dictionary of epistemic relations (agent->EPISTEMIC)
        freeit -- whether or not free the pointer
        """
        super().__init__(ptr, trans, freeit = freeit)
        self._epistemic = epistemic and epistemic or {}
        
        
    def pre(self, states, inputs = None):
        """
        Compute the pre-image of states through inputs (if not None),
        using the temporal transition relations of the MAS.
        """
        return super().pre(states, inputs, self._trans.keys())
        
        
    def post(self, states, inputs = None):
        """
        Compute the post-image of states through inputs (if not None),
        using the temporal transition relations of the MAS.
        """
        return super().post(states, inputs, self._trans.keys())
        
        
    def get_inputs_between_states(self, current, next):
        """
        Return the BDD representing the possible inputs
        between current and next.
        """
        return super().get_inputs_between_states(
                                              current, next, self._trans.keys())
                                              
                                              
    def equivalent_states(self, states, agents = None):
        """
        Return the BDD representing the set of states epistemically equivalent
        to states through epistemic relations of agents. If agents is None,
        all the known epistemic relations are used.
        """
        if agents is None:
            agents = self._epistemic.keys()
            
        # Apply FSM constraints
        states = states & self.state_constraints
        
        # Compute the post-image            
        result = BDD.true(self.bddEnc.DDmanager)
        for ag in agents:
            if ag not in self._epistemic:
                raise UnknownAgentError(ag + " is an unknown agent name.")
            agpost = self._epistemic[ag].pre_state_input(states)
            result = result & agpost
        
        # Apply constraints on result
        result = result & self.state_constraints
        
        # Abstract input variables
        return self._abstract_inputs(result)