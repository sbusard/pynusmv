from ..nusmv.dd import dd
from ..nusmv.node import node as nsnode
from ..utils.wrap import PointerWrapper

class MissingManagerError(Exception):
    """
    Exception for missing BDD manager.
    """
    pass
    

class BDD(PointerWrapper):
    """
    Python class for BDD structure.
    
    The BDD class contains a pointer to a BDD in NuSMV (a bdd_ptr)
    and provides a set of operations on this BDD.
    """
    
    def __init__(self, ptr, dd_manager=None):
        """
        Create a new BDD with ptr.
        
        ptr -- the pointer to the NuSMV BDD.
        dd_manager -- the DD manager for this BDD.
        """
        super().__init__(ptr)
        self._manager = dd_manager
        
        
    def entailed(self, bdd):
        """
        Determine whether this BDD is less than or equal to bdd.
        
        bdd -- a BDD.
        
        Return True if this BDD is less than or equal to bdd.
        
        Raise a MissingManagerError if no manager is present in this BDD.
        """
        
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_entailed(self._manager, self._ptr, bdd._ptr):
            return True
        else:
            return False
    
    
    # TODO Change this to allow shorter syntax
    #      like not(b and c)
    def compute_and(self, other):
        """Return the BDD representing the intersection of self and other."""
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_and(self._manager, self._ptr, other._ptr),
                   self._manager)
        
            
    def compute_not(self):
        """Return the complement of self."""
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_not(self._manager, self._ptr), self._manager)
          
            
    def is_not_false(self):
        """Return whether self is not false"""
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_isnot_false(self._manager, self._ptr):
            return True
        else:
            return False
            
    
    def to_node(self):
        """Cast this BDD to a node."""
        
        from ..node.node import Node
        
        return Node(nsnode.bdd2node(self._ptr))
        
        
    def to_add(self):
        """Cast self pointer to add pointer. Return the pointer."""
        return dd.bdd_to_add(self._manager, self._ptr)