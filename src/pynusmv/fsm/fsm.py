from ..nusmv.fsm.bdd import bdd as bddFsm
from ..nusmv.enc.bdd import bdd as bddEnc
from ..nusmv.cmd import cmd as nscmd

from ..enc.enc import BddEnc
from ..dd.bdd import BDD
from ..dd.state import State
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
        
        
    def pick_one_state(self, bdd):
        """Return a BDD representing a state of bdd."""
        state = bddEnc.BddEnc_pick_one_state(self.bddEnc._ptr, bdd._ptr)
        return State(state, self, freeit = True)
        
        
    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
    def from_filename(filename):
        """Return the FSM corresponding to the model in filename."""
        from ..prop.propDb import PropDb
        nscmd.Cmd_SecureCommandExecute("read_model -i " + filename)
        nscmd.Cmd_SecureCommandExecute("go")
        
        propDb = PropDb.get_global_database()
        return propDb.master.bddFsm