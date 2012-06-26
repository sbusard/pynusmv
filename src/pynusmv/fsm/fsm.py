from ..nusmv.fsm.bdd import bdd as bddFsm
from ..enc.enc import BddEnc

class BddFsm:
    """
    Python class for BddFsm structure.
    
    The BddFsm class contains a pointer to a BddFsm in NuSMV and provides a set
    of operations on this FSM.
    """
    
    def __init__(self, ptr):
        """
        Create a new FSM with ptr.
        
        ptr -- the pointer to the NuSMV BddFsm.
        """
        self.__ptr = ptr
        
    @property
    def BddEnc(self):
        """The BDD encoding of this FSM."""
        return BddEnc(bddFsm.BddFsm_get_bdd_encoding(self.__ptr))            