from pynusmv.dd import BDD
from pynusmv.fsm import BddFsm, BddTrans
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.dd import dd as nsdd
from .exception import UnknownAgentError

class MAS(BddFsm):
    """
    A multi-agent system.
    
    A multi-agent system is an FSM with one temporal trans for the whole model
    and one epistemic trans by agent, labelled with the name of the agent.
    
    epistemic -- a dictionary of agent->the NuSMV node-based TRANS of the agent.
    """
    
    def __init__(self, ptr, observed, inputvars, epistemic, groups=None,
                 freeit=False):
        """
        Create a new MAS.
        
        ptr -- the pointer of the NuSMV FSM
        observed -- a dictionary of agents' known variables
                    (agent->Set of vars)
                    representing the variables observed by each agent
        inputvars -- a dictionary of agents' input variables
                     (agent->Set of input vars)
        epistemic -- a dictionary of epistemic relations (agent->EPISTEMIC)
        groups -- if not None, a dictionary of group name -> set of names of
                  the agents of the group; the groups names and the agents
                  names must be present in epistemic
        freeit -- whether or not free the pointer
        """
        super(MAS, self).__init__(ptr, freeit=freeit)
        self._epistemic = epistemic and epistemic or {}
        self._epistemic_trans = {}
        self._agents_observed_variables = observed
        self._agents_inputvars = inputvars
        self._groups = groups if groups is not None else {}
        self._protocols = {}
        
    
    @property
    def agents(self):
        """The list of names of the agents of the system."""
        return set(self._epistemic.keys())
        
    
    @property
    def agents_observed_variables(self):
        """The state variables observed by each agent (as a dictionary)."""
        return self._agents_observed_variables
        
    
    @property
    def agents_inputvars(self):
        """The input variables of each agent (as a dictionary)."""
        return self._agents_inputvars
    
    @property
    def groups(self):
        """The set of groups of the MAS, with corresponding agents."""
        return self._groups
        
        
    def _compute_epistemic_trans(self, agents):
        from .glob import symb_table
        trans = None
        for agent in agents:
            if agent not in self._epistemic:
                raise UnknownAgentError(str(agents) +
                                        " are an unknown agents names.")
            trans = nsnode.find_node(nsparser.AND,
                                     self._epistemic[agent],
                                     trans)
        self._epistemic_trans[frozenset(agents)] = BddTrans.from_trans(
                                                            symb_table(),
                                                            trans)
    
    def equivalent_states(self, states, agents):
        """
        Return the BDD representing the set of states epistemically equivalent
        to states through the epistemic relation of agents. agents is a set of
        agents name.
        
        states -- a BDD of states of self
        agents -- a set of agents names.
                  These agents must be ones of this MAS agents.
        """
            
        # Apply FSM constraints
        states = states & self.state_constraints
        
        # Compute the post-image
        if frozenset(agents) not in self._epistemic_trans:
            # Compute the BddTrans
            self._compute_epistemic_trans(agents)
        # pre_ or post_ operations can be used since the epistemic relations
        # are equivalence relations and thus are symetric
        result = self._epistemic_trans[frozenset(agents)].pre(states)
        
        # Apply constraints on result
        return result & self.state_constraints
        
    
    def protocol(self, agents):
        """
        Return the protocol for the given set of agents.
        
        agents -- a set of agent names
        
        The returned value is a BDD representing state-input pairs, such that
        for every state, the given inputs (for agents only) are the ones
        available in the state, i.e. for a state s, <s,a_gamma> is
        in the returned BDD iff there exist a_ngamma (an action for all other
        agents) and s' such that <s, a_gamma, a_ngamma, s'> is in the transition
        relation.
        
        """
        agents = frozenset(agents)
        if agents not in self._protocols:
            gamma_inputs = [var
                            for agent in agents
                            for var in self.agents_inputvars[agent]]
            gamma_cube = self.bddEnc.cube_for_inputs_vars(gamma_inputs)
            ngamma_cube = self.bddEnc.inputsCube - gamma_cube
            self._protocols[agents] = (self.weak_pre(self.reachable_states).
                                       forsome(ngamma_cube) &
                                       self.reachable_states &
                                       self.bddEnc.statesInputsMask)
                                       
        return self._protocols[agents]
        
        
    def inputs_cube_for_agents(self, agents):
        """
        Return the cube of inputs variables of agents.
        
        agents -- a set of agents names of this MAS.
        
        """
        gamma_inputs = [var
                        for agent in agents
                        for var in self.agents_inputvars[agent]]
        return self.bddEnc.cube_for_inputs_vars(gamma_inputs)
        
    def pre(self, states, inputs=None, subsystem=None):
        """
        Return the pre image of states, through inputs (if any) and in
        subsystem (if any).
        """
        
        if inputs is None:
            inputs = BDD.true(self.bddEnc.DDmanager)
        
        if subsystem is None:
            subsystem = BDD.true(self.bddEnc.DDmanager)
        
        return ((self.weak_pre(states & inputs) &
                 subsystem).forsome(self.bddEnc.inputsCube) &
                 self.bddEnc.statesMask)
    
    def post(self, states, inputs=None, subsystem=None):
        """
        Return the post image of states, through inputs (if any) and in
        subsystem (if any).
        """
        
        if inputs is None:
            inputs = BDD.true(self.bddEnc.DDmanager)
        
        if subsystem is None:
            subsystem = BDD.true(self.bddEnc.DDmanager)
        
        states = states & subsystem
        
        return super(MAS, self).post(states, inputs)
        
        
    def pre_strat(self, states, agents, strat=None):
        """
        Return the set of states s of this MAS such that there exists values
        of input variables of the agents such that for all values of input
        variables of the other agents, all successors of s through these inputs
        belong to states.
        If strat is not None, restrict to strat.
        
        states -- a BDD representing a set of states of this MAS;
                  if states represents a set of state/inputs pairs, inputs
                  are abstracted away;
        agents -- a set of agents names, agents of this MAS;
        strat -- a BDD representing allowed state/inputs pairs, or None.
        
        """
        if not strat:
            strat = BDD.true(self.bddEnc.DDmanager)
            
        gamma_cube = self.inputs_cube_for_agents(agents)
        ngamma_cube = self.bddEnc.inputsCube - gamma_cube
        
        # Abstract away actions of states
        states = states.forsome(self.bddEnc.inputsCube)
        
        nstates = ~states & self.bddEnc.statesInputsMask
        strat = strat & self.bddEnc.statesInputsMask
        
        return (
                ( ~(
                    self.weak_pre(nstates).forsome(ngamma_cube)
                  )
                  & 
                  self.weak_pre(states)
                ).forsome(ngamma_cube)
                &
                strat
                &
                self.bddEnc.statesInputsMask
               ).forsome(self.bddEnc.inputsCube)
                
                
    def pre_nstrat(self, states, agents):
        """
        Return the set of states s of this MAS such that for all values of input
        variables of the agents there exists values of input
        variables of the other agents such that there is a successor of s
        through these inputs that belongs to states.
        
        states -- a BDD representing a set of states of this MAS;
        agents -- a set of agents names, agents of this MAS.
        
        """
        return ~self.pre_strat(~states, agents)
        
    
    def pre_strat_si(self, states, agents, strat=None):
        """
        Return the set of state/inputs pairs <s,i_agents> of this MAS such that
        there exist values of input variables of the agents i_agents,
        all successors of s through i_agents U i_nagents belong to states.
        Restrict to strat if not None.
        
        states -- a BDD representing a set of states of this MAS;
                  if states represents a set of state/inputs pairs, inputs
                  are abstracted away;
        agents -- a set of agents names, agents of this MAS;
        strat -- a BDD representing a set of allowed state/inputs pairs.
        
        """
        if strat is None:
            strat = BDD.true(self.bddEnc.DDmanager)
        
        gamma_cube = self.inputs_cube_for_agents(agents)
        ngamma_cube = self.bddEnc.inputsCube - gamma_cube
        
        # Abstract inputs from states to avoid mixing with the possible actions
        # present in states.
        states = states.forsome(self.bddEnc.inputsCube)
        
        nstates = ~states & self.bddEnc.statesInputsMask
        strat = strat & self.bddEnc.statesInputsMask
                
        return (
                ( ~(
                    self.weak_pre(nstates).forsome(ngamma_cube)
                  )
                  & 
                  self.weak_pre(states)
                ).forsome(ngamma_cube)
                &
                strat
                &
                self.bddEnc.statesInputsMask
               )
    
    def check_free_choice(self):
        """
        Check whether this MAS satisfies the free-choice property, that is,
        in every state, the choices of actions for each agent is not
        constrained by the choices of other agents.
        
        Return the set of moves that are not present in the MAS and should,
        or that are present but should not.
        """
        
        if len(self.agents) <= 0:
            return BDD.false(self.bddEnc.DDmanager)
        
        true = BDD.true(self.bddEnc.DDmanager)
        protocols = {agent: self.protocol({agent}) for agent in self.agents}
        enabled = (self.weak_pre(self.reachable_states) &
                   self.reachable_states & self.bddEnc.statesInputsMask)
        
        for s in self.pick_all_states(self.reachable_states):
            product = self.bddEnc.statesInputsMask
            for agent in self.agents:
                product &= protocols[agent] & s
            if (enabled & s) != product:
                return product.xor(enabled & s)
        return BDD.false(self.bddEnc.DDmanager)
    
    def check_uniform_choice(self):
        """
        Check whether this MAS satisfies the uniform-choice property, that is,
        the action of each agent in two epistemically equivalent states are
        the same.
        
        Return a FALSE BDD if this MAS satisfies the uniform-choice property,
        or a set of moves showing why this MAS does not satisfy it, otherwise.
        """
        
        true = BDD.true(self.bddEnc.DDmanager)
        false = BDD.false(self.bddEnc.DDmanager)
        inputs_cube = self.bddEnc.inputsCube
        states_cube = self.bddEnc.statesCube
        
        if len(self.agents) <= 0:
            return false
        
        for agent in self.agents:
            act_cube = self.inputs_cube_for_agents({agent})
            protocol = self.protocol({agent})
            remaining = protocol
            
            while remaining.isnot_false():
                # One state
                s = self.pick_one_state(remaining.forsome(inputs_cube))
                # The equivalence class of s
                eq = (self.equivalent_states(s, {agent}) &
                      self.reachable_states)
                # The actions for agent in s
                i = (protocol & s).forsome(states_cube)
                
                for sp in self.pick_all_states(eq):
                    ip = (protocol & sp).forsome(states_cube)
                    if ip != i:
                        return (s & i) | (sp & ip)
                
                remaining = remaining - eq
        return false
    
    def check_mas(self):
        """
        Check whether this MAS satisfies the free-choice and uniform-choice
        properties, that is,
        - in every state, the choices of actions for each agent is not
          constrained by the choices of other agents;
        - the action of each agent in two epistemically equivalent states are
          the same.
        """
        return (self.check_free_choice().is_false() and
                self.check_uniform_choice().is_false())
        


class Agent(object):
    """
    An agent has a name, a set of observable variables and a set of actions.
    Observable variables must be NuSMV variables (no define, frozen variable
    or input variable).
    Actions must be NuSMV input variables.
    """
    def __init__(self, name, observables, actions):
        self.name = name
        self.observables = observables
        self.actions = actions

class Group(Agent):
    """
    A group is an agent representing a group of agents.
    """
    def __init__(self, name, *agents):
        self.name = name
        self.agents = agents
        self.observables = set(observable for agent in agents
                                          for observable in agent.observables)
        self.actions = set(action for agent in agents
                                  for action in agent.actions)