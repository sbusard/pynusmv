from pynusmv.dd import BDD
from pynusmv.fsm import BddFsm
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser as nsparser
from .exception import UnknownAgentError
from .glob import symb_table
from .bddTrans import BddTrans

class MAS(BddFsm):
    """
    A multi-agent system.
    
    A multi-agent system is an FSM with one temporal trans for the whole model
    and one epistemic trans by agent, labelled with the name of the agent.
    
    epistemic -- a dictionary of agent->the NuSMV node-based TRANS of the agent.
    """
    
    def __init__(self, ptr, variables, inputvars,
                 epistemic = None, freeit = False):
        """
        Create a new MAS.
        
        ptr -- the pointer of the NuSMV FSM
        variables -- a dictionary of agents' variables (agent->Set of vars)
        inputvars -- a dictionary of agents' input variables
                     (agent->Set of input vars)
        epistemic -- a dictionary of epistemic relations (agent->EPISTEMIC)
        freeit -- whether or not free the pointer
        """
        super().__init__(ptr, freeit = freeit)
        self._epistemic = epistemic and epistemic or {}
        self._epistemic_trans = {}
        self._agents_variables = variables
        self._agents_inputvars = inputvars
        
    
    @property
    def agents(self):
        """The list of names of the agents of the system."""
        return set(self._epistemic.keys())
        
    
    @property
    def agents_variables(self):
        """The state variables of each agent (as a dictionary)."""
        return self._agents_variables
        
    
    @property
    def agents_inputvars(self):
        """The input variables of each agent (as a dictionary)."""
        return self._agents_inputvars
        
        
    def _compute_epistemic_trans(self, agents):
        trans = None
        for agent in agents:
            if agent not in self._epistemic:
                raise UnknownAgentError(str(agents) +
                                                " are an unknown agents names.")
            trans = nsnode.find_node(nsparser.AND,
                                     self._epistemic[agent],
                                     trans)
        self._epistemic_trans[agents] = BddTrans.from_trans(symb_table(), trans)
                                              
                                              
    def equivalent_states(self, states, agents):
        """
        Return the BDD representing the set of states epistemically equivalent
        to states through the epistemic relation of agents. agents is a set of agents name.
        
        states -- a BDD of states of self
        agents -- a set of agents names.
                  These agents must be ones of this MAS agents.
        """
            
        # Apply FSM constraints
        states = states & self.state_constraints
        
        # Compute the post-image
        agents = frozenset(agents)
        if agents not in self._epistemic_trans:
            # Compute the BddTrans
            self._compute_epistemic_trans(agents)
        # pre_ or post_ operations can be used since the epistemic relations
        # are equivalence relations and thus are symetric
        result = self._epistemic_trans[agents].pre(states)
        
        # Apply constraints on result
        return result & self.state_constraints
        
        
    def pre_strat(self, states, agents):
        """
        Return the set of states s of this MAS such that there exists values
        of input variables of the agents such that for all values of input
        variables of the other agents, all successors of s belong to states.
        
        states -- a BDD representing a set of states of this MAS;
        agents -- a set of agents names, agents of this MAS.
        """
        pass
        