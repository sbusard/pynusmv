__all__ = ['propTypes', 'Prop', 'PropDb']

from .nusmv.prop import prop as nsprop

from .fsm import BddFsm
from .spec import Spec
from .utils.pointerwrapper import PointerWrapper

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


class PropDb(PointerWrapper):
    """
    Python class for PropDb structure.
    
    The PropDb class contains a pointer to a propDb in NuSMV and provides a set
    of operations on this prop database.
    
    PropDb do not have to be freed.
    """
    
    
    @property
    def master(self):
        """The master property of this database."""
        return Prop(nsprop.PropDb_get_master(self._ptr))
    
    
    def get_prop_at_index(self, index):
        """Return the prop stored at index."""
        return Prop(nsprop.PropDb_get_prop_at_index(self._ptr, index))
    
    
    def get_size(self):
        """Return the size of this database."""
        return nsprop.PropDb_get_size(self._ptr)
        
    
    def __len__(self):
        """Return the length of this propDb."""
        return self.get_size()
        
    
    def __getitem__(self, index):
        """
        Return the indexth property.
        
        Throw a IndexError if index < -len(self) or index >= len(self) 
        """
        if index < -len(self) or index >= len(self):
            raise IndexError("PropDb index out of range")
        if index < 0:
            index = index + len(self)
        return self.get_prop_at_index(index)
        
    
    def __iter__(self):
        """Return an iterator on this propDb."""
        for i in range(len(self)):
            yield self[i]