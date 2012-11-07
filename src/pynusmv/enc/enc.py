from ..nusmv.enc.bdd import bdd as bddEnc
from ..nusmv.enc.base import base as baseEnc
from ..dd.manager import DDManager
from ..dd.bdd import BDD
from ..symb_table.symb_table import SymbTable
from ..utils.pointerwrapper import PointerWrapper

class BddEnc(PointerWrapper):
    """
    Python class for BddEnc structure.
    
    The BddEnc class contains a pointer to a BddEnc in NuSMV and provides a set
    of operations on this BDD encoding.
    
    BddEnc do not have to be freed.
    """
        
    @property
    def DDmanager(self):
        """The DD manager of this encoding."""
        return DDManager(bddEnc.BddEnc_get_dd_manager(self._ptr))
        
    
    @property
    def symbTable(self):
        """Return the NuSMV symb table of this enc."""
        base_enc = bddEnc.bddenc2baseenc(self._ptr)
        return SymbTable(baseEnc.BaseEnc_get_symb_table(base_enc))
        
    
    @property    
    def statesMask(self):
        """Return a BDD representing a mask for all state variables."""
        return BDD(bddEnc.BddEnc_get_state_frozen_vars_mask_bdd(self._ptr),
                   self.DDmanager, freeit = True)
                   
    
    @property               
    def inputsMask(self):
        """Return a BDD representing a mask for all inputs variables."""
        return BDD(bddEnc.BddEnc_get_input_vars_mask_bdd(self._ptr),
                   self.DDmanager, freeit = True)