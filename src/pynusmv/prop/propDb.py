from ..nusmv.prop import prop as nsprop
from .prop import Prop
from ..utils.pointerwrapper import PointerWrapper

class PropDb(PointerWrapper):
    """
    Python class for PropDb structure.
    
    The PropDb class contains a pointer to a propDb in NuSMV and provides a set
    of operations on this prop database.
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
        
    
        
    # ==========================================================================
    # ===== Class methods ======================================================
    # ==========================================================================    
    
    def get_global_database():
        """Return the global prop database of NuSMV."""
        return PropDb(nsprop.PropPkg_get_prop_database())