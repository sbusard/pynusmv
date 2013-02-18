from ..nusmv.prop import prop as nsprop

from ..fsm import BddFsm
from ..spec import Spec
from ..utils.pointerwrapper import PointerWrapper

propTypes = {
             'NoType' :      nsprop.Prop_NoType,
             'CTL' :         nsprop.Prop_Ctl,
             'LTL' :         nsprop.Prop_Ltl,
             'PSL' :         nsprop.Prop_Psl,
             'Invariant' :   nsprop.Prop_Invar,
             'Compute' :     nsprop.Prop_Compute,
             'Comparison' :  nsprop.Prop_CompId
            }
            

class Prop(PointerWrapper):
    """
    Python class for prop structure.
    
    The Prop class contains a pointer to a prop in NuSMV and provides a set
    of operations on this prop.
    
    Prop do not have to be freed since they come from PropDb.
    """
        
    @property
    def type(self):
        """
        The type of this prop.
        
        To compare with pynusmv.nusmv.prop.prop.Prop_X for type X.
        Types are NoType, Ctl, Ltl, Psl, Invar, Compute, CompId
        """
        return nsprop.Prop_get_type(self._ptr)
        
    @property
    def name(self):
        """The name of this prop, as a string."""
        return nsprop.Prop_get_name_as_string(self._ptr)
        
    @property
    def expr(self):
        """The expression of this prop, as a Spec."""
        return Spec(nsprop.Prop_get_expr(self._ptr))
        
    @property
    def exprcore(self):
        """The core expression of this prop, as a Spec."""
        return Spec(nsprop.Prop_get_expr_core(self._ptr))
        
    @property
    def bddFsm(self):
        """The fsm of this prop, into BddFsm format."""
        return BddFsm(nsprop.Prop_get_bdd_fsm(self._ptr))