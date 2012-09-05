from pynusmv.fsm.bddFsm import BddFsm

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
                   
                   
    def pre(self, states, inputs = None):
        """
        Return the pre-image of states in this FSM.
        
        If inputs is not None, it is used as constraints to get pre-states
        that are reachable through these inputs.
        """
        # TODO Reimplement with additional subset of TRANS as optional arg
        # and by taking self._trans into account
        if inputs is None:
            return BDD(bddFsm.BddFsm_get_backward_image(self._ptr, states._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        else:
            return BDD(bddFsm.BddFsm_get_constrained_backward_image(
                            self._ptr, states._ptr, inputs._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        
        
    def post(self, states, inputs = None):
        """
        Return the post-image of states in this FSM.
        
        If inputs is not None, it is used as constraints to get post-states
        that are reachable through these inputs.
        """
        # TODO Reimplement with additional subset of TRANS as optional arg
        # and by taking self._trans into account
        if inputs is None:
            return BDD(bddFsm.BddFsm_get_forward_image(self._ptr, states._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        else:
            return BDD(bddFsm.BddFsm_get_constrained_forward_image(
                            self._ptr, states._ptr, inputs._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        
        
        
    
    def get_inputs_between_states(self, current, next):
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
