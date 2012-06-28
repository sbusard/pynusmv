from ..nusmv.dd import dd
from ..nusmv.node import node as nsnode

class MissingManagerError(Exception):
    """
    Exception for missing BDD manager.
    """
    pass
    

class BDD:
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
        self.__ptr = ptr
        self.__manager = dd_manager
        
        
    @property
    def ptr(self):
        return self.__ptr
        
        
    def entailed(self, bdd):
        """
        Determine whether this BDD is less than or equal to bdd.
        
        bdd -- a BDD.
        
        Return True if this BDD is less than or equal to bdd.
        
        Raise a MissingManagerError if no manager is present in this BDD.
        """
        
        if self.__manager is None:
            raise MissingManagerError()
            
        if dd.bdd_entailed(self.__manager, self.__ptr, bdd.__ptr):
            return True
        else:
            return False
    
    
    # TODO Change this to allow shorter syntax
    #      like not(b and c)
    def compute_and(self, other):
        """Return the BDD representing the intersection of self and other."""
        if self.__manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_and(self.__manager, self.__ptr, other.__ptr),
                   self.__manager)
        
            
    def compute_not(self):
        """Return the complement of self."""
        if self.__manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_not(self.__manager, self.__ptr), self.__manager)
          
            
    def is_not_false(self):
        """Return whether self is not false"""
        if self.__manager is None:
            raise MissingManagerError()
            
        if dd.bdd_isnot_false(self.__manager, self.__ptr):
            return True
        else:
            return False
            
    
    def to_node(self):
        """Cast this BDD to a node."""
        
        from ..node.node import Node
        
        return Node(nsnode.bdd2node(self.__ptr))