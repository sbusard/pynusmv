from pynusmv.fsm.bddFsm import BddFsm
from pynusmv.dd.bdd import BDD

from pynusmv.nusmv.enc.bdd import bdd as nsbddenc
from pynusmv.nusmv.dd import dd as nsdd
from pynusmv.nusmv.trans.bdd import bdd as nsbddtrans

from .exception import UnknownTransError

class MMFsm(BddFsm):
    """
    A multimodal BDD FSM.
    
    Such an FSM is a standard NuSMV FSM except that is contains several
    transition relations. Some operations are overridden to take this into
    account, like post or pre operations.
    
    The trans field of this FSM gives access to a basic TRANS, i.e. the TRUE
    BDD.
    """
    
    def __init__(self, ptr, trans = None, freeit = True):
        """Create a new MMFsm with many TRANS."""
        super().__init__(ptr, freeit)
        self._trans = trans and trans or {} 
        
    
    def _abstract_inputs(self, bdd):
        """Abstract away the input variables from bdd."""
        ptr = nsdd.bdd_forsome(self.bddEnc.DDmanager._ptr,
                               bdd._ptr,
                               nsbddenc.BddEnc_get_input_vars_cube(
                                                              self.bddEnc._ptr))
        return BDD(ptr, self.bddEnc.DDmanager, freeit = True)
                   
                   
    def pre(self, states, inputs = None, trans = None):
        """
        Return the pre-image of states in this FSM.
        
        If inputs is not None, it is used as constraints to get pre-states
        that are reachable through these inputs.
        
        If trans is empty or trans is None and this FSM has no TRANS,
        the result is TRUE (modulo this FSM state constraints).
        """
        if trans is None:
            trans = self._trans.keys()
            
        # Apply FSM constraints
        states = states & self.state_constraints
        if inputs is not None:
            inputs = inputs & self.inputs_constraints
        
        # Compute the pre-image
        result = BDD.true(self.bddEnc.DDmanager)
        for tr in trans:
            if tr not in self._trans:
                UnknownTransError(tr + " is an unknown TRANS name.")
            result = result & self._trans[tr].pre_state_input(states, inputs)
        
        # Apply constraints on result
        result = result & self.state_constraints
        
        # Abstract input variables
        return self._abstract_inputs(result)
        
        
    def post(self, states, inputs = None, trans = None):
        """
        Return the post-image of states in this FSM.
        
        If inputs is not None, it is used as constraints to get post-states
        that are reachable through these inputs.
        
        If trans is empty or trans is None and this FSM has no TRANS,
        the result is TRUE (modulo this FSM state constraints).
        """
        if trans is None:
            trans = self._trans.keys()
            
        # Apply FSM constraints
        states = states & self.state_constraints
        if inputs is not None:
            inputs = inputs & self.inputs_constraints
        
        # Compute the post-image            
        result = BDD.true(self.bddEnc.DDmanager)
        for tr in trans:
            if tr not in self._trans:
                UnknownTransError(tr + " is an unknown TRANS name.")
            trpost = self._trans[tr].post_state_input(states, inputs)
            result = result & trpost
        
        # Apply constraints on result
        result = result & self.state_constraints
        
        # Abstract input variables
        ptr = nsdd.bdd_forsome(
                    self.bddEnc.DDmanager._ptr,
                    result._ptr,
                    nsbddenc.BddEnc_get_input_vars_cube(self.bddEnc._ptr))
                    
        return BDD(ptr, result._manager, freeit = True)
                
        
    def get_inputs_between_states(self, current, next, trans = None):
        """
        Return the BDD representing the possible inputs
        between current and next.
        """
        # TODO Reimplement with additional subset of TRANS as optional arg
        # and by taking self._trans into account
        inputs = bddFsm.BddFsm_states_to_states_get_inputs(self._ptr,
                                                           current._ptr,
                                                           next._ptr)
        return Inputs(inputs, self, freeit = True)
