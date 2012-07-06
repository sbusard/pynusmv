from ..nusmv.enc.bdd import bdd as bddEnc
from ..nusmv.enc.base import base as baseEnc
from ..dd.bdd import BDD
from ..dd.manager import DDManager
from ..utils.wrap import PointerWrapper

class BddEnc(PointerWrapper):
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
        super().__init__(ptr)
        
    @property
    def DDmanager(self):
        """The DD manager of this encoding."""
        return DDManager(bddEnc.BddEnc_get_dd_manager(self._ptr))
        
    
    @property
    def symbTable(self):
        """Return a pointer to the NuSMV symb table of this enc."""
        base_enc = bddEnc.bddenc2baseenc(self._ptr)
        return baseEnc.BaseEnc_get_symb_table(base_enc)
        
        
    def pick_one_state(self, bdd):
        """Return a BDD representing a state of bdd."""
        state = bddEnc.BddEnc_pick_one_state(self._ptr, bdd._ptr)
        return BDD(state, self.DDmanager)