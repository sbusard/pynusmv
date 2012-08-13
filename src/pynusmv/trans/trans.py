from ..utils.pointerwrapper import PointerWrapper
from ..dd.bdd import BDD

from ..nusmv.trans.bdd import bdd as nsbddtrans

class BddTrans(PointerWrapper):
    """
    Python class for BddTrans structure.
    
    The BddTrans class contains a pointer to a BddTrans in NuSMV
    and a DD manager,
    and provides a set of operations on this transition relation.
    """
    
    def __init__(self, ptr, manager = None, freeit = True):
        super().__init__(ptr, freeit)
        self._manager = manager
    
    
    @property
    def monolithic(self):
        """
        BDD representing this transition as one monolithic BDD.
        
        Returned BDD has no DD manager!
        """
        ptr = nsbddtrans.BddTrans_get_monolithic_bdd(self._ptr) 
        return BDD(ptr, self._manager, freeit = True)    