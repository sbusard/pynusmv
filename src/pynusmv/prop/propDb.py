from ..nusmv.prop import prop as nsprop
from .prop import Prop

class PropDb:
    """
    Python class for PropDb structure.
    
    The PropDb class contains a pointer to a propDb in NuSMV and provides a set
    of operations on this prop database.
    """
    
    def __init__(self, ptr):
        """
        Create a new prop with ptr.
        
        ptr -- the pointer to the NuSMV propDb.
        """
        self.__ptr = ptr
        
    # TODO act like a list
    
    @property
    def master(self):
        """The master property of this database."""
        return Prop(nsprop.PropDb_get_master(self.__ptr))
        
    
    def get_prop_at_index(self, index):
        """Return the prop stored at index."""
        return Prop(nsprop.PropDb_get_prop_at_index(self.__ptr, index))
        
    
        
    # ==========================================================================
    # ===== Class methods ======================================================
    # ==========================================================================    
    
    def get_global_database():
        """Return the global prop database of NuSMV."""
        return PropDb(nsprop.PropPkg_get_prop_database())