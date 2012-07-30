from ..nusmv.fsm.bdd import bdd as bddFsm
from ..nusmv.enc.bdd import bdd as bddEnc
from ..nusmv.cmd import cmd as nscmd

from ..enc.enc import BddEnc
from ..dd.bdd import BDD
from ..dd.state import State
from ..dd.inputs import Inputs
from ..utils.pointerwrapper import PointerWrapper

class BddFsm(PointerWrapper):
    """
    Python class for BddFsm structure.
    
    The BddFsm class contains a pointer to a BddFsm in NuSMV and provides a set
    of operations on this FSM.
    
    BddFsm do not have to be freed.
    """
    
        
    @property
    def bddEnc(self):
        """The BDD encoding of this FSM."""
        return BddEnc(bddFsm.BddFsm_get_bdd_encoding(self._ptr))
        
    
    @property
    def init(self):
        """The BDD of initial states of this FSM."""
        return BDD(bddFsm.BddFsm_get_init(self._ptr), self.bddEnc.DDmanager,
                   freeit = True)
                   
                   
    def pre(self, states, inputs = None):
        """
        Return the pre-image of states in this FSM.
        
        If inputs is not None, it is used as constraints to get pre-states
        that are reachable through these inputs.
        """
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
        if inputs is None:
            return BDD(bddFsm.BddFsm_get_forward_image(self._ptr, states._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        else:
            return BDD(bddFsm.BddFsm_get_constrained_forward_image(
                            self._ptr, states._ptr, inputs._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        
        
    def pick_one_state(self, bdd):
        """Return a BDD representing a state of bdd."""
        state = bddEnc.BddEnc_pick_one_state(self.bddEnc._ptr, bdd._ptr)
        return State(state, self, freeit = True)

    
    def pick_one_inputs(self, bdd):
        """Return a BDD representing a possible inputs of bdd."""
        inputs = bddEnc.BddEnc_pick_one_input(self.bddEnc._ptr, bdd._ptr)
        return Inputs(inputs, self, freeit = True)
        
        
    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
    def from_filename(filename):
        """
        Return the FSM corresponding to the model in filename.
        
        Raise a NuSMVCommandError if something is wrong.
        """
        from ..prop.propDb import PropDb
        ret = nscmd.Cmd_SecureCommandExecute("read_model -i " + filename)
        if ret:
            raise NuSMVCommandError("Cannot read the model " + filename)
        ret = nscmd.Cmd_SecureCommandExecute("go")
        if ret:
            raise NuSMVCommandError("Cannot build the model")
        
        
        propDb = PropDb.get_global_database()
        return propDb.master.bddFsm
        
        
class NuSMVCommandError(Exception):
    """A NuSMV command ended with an error."""
    pass