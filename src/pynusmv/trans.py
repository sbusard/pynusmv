__all__ = ['BddTrans']

from .utils.pointerwrapper import PointerWrapper
from .dd.bdd import BDD
     
from .nusmv.trans.bdd import bdd as nsbddtrans
from .nusmv.enc.bdd import bdd as nsbddenc

class BddTrans(PointerWrapper):
    """
    Python class for BddTrans structure.
    
    The BddTrans class contains a pointer to a BddTrans in NuSMV,
    a Bdd encoding and a DD manager,
    and provides a set of operations on this transition relation.
    """
    
    def __init__(self, ptr, enc = None, manager = None, freeit = True):
        super().__init__(ptr, freeit)
        self._enc = enc
        self._manager = manager
    
    
    @property
    def monolithic(self):
        """
        BDD representing this transition as one monolithic BDD.
        
        Returned BDD has no DD manager!
        """
        ptr = nsbddtrans.BddTrans_get_monolithic_bdd(self._ptr) 
        return BDD(ptr, self._manager, freeit = True)
        
        
    def pre(self, states, inputs=None):
        """
        Compute the pre-image of states, through inputs if not None.
        """
        nexts = BDD(
            nsbddenc.BddEnc_state_var_to_next_state_var(self._enc._ptr,
                                                            states._ptr),
            states._manager, freeit = True)
            
        if inputs is not None:
            nexts = nexts & inputs
        img = nsbddtrans.BddTrans_get_backward_image_state(
                                                          self._ptr, nexts._ptr)
        return BDD(img, self._manager, freeit = True)
        
    
    def post(self, states, inputs=None):
        """
        Compute the post-image of states, through inputs if not None.
        """
        if inputs is not None:
            states = states & inputs
        img = nsbddtrans.BddTrans_get_forward_image_state(
                                                         self._ptr, states._ptr)
        img = nsbddenc.BddEnc_next_state_var_to_state_var(self._enc._ptr, img)
        return BDD(img, self._manager, freeit = True)