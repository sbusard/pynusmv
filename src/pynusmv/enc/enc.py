from ..nusmv.enc.bdd import bdd as bddEnc
from ..dd.dd import BDD

class BddEnc:
    """
    Python class for BddEnc structure.
    
    The BddEnc class contains a pointer to a BddEnc in NuSMV and provides a set
    of operations on this BDD encoding.
    """
    
    def __init__(self, ptr):
        """
        Create a new encoding with ptr.
        
        ptr -- the pointer to the NuSMV BddEnc.
        """
        self.__ptr = ptr
        
    @property
    def DDmanager(self):
        """The DD manager of this encoding."""
        return bddEnc.BddEnc_get_dd_manager(self.__ptr)
        
        
    def pick_one_state(self, bdd):
        """Return a BDD representing a state of bdd."""
        state = bddEnc.BddEnc_pick_one_state(self.__ptr, bdd.__ptr)
        return BDD(state, self.DDmanager)