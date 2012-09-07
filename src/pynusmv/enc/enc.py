from ..nusmv.enc.bdd import bdd as bddEnc
from ..nusmv.enc.base import base as baseEnc
from ..dd.manager import DDManager
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