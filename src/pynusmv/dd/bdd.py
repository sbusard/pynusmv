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
    
    A BDD operation raises a MissingManagerError whenever the manager
    of the BDD is None and a manager is needed to perform the operation.
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
            
        if dd.bdd_entailed(self._manager._ptr, self._ptr, bdd._ptr):
            return True
        else:
            return False
            
    
    def to_node(self):
        """Cast this BDD to a node."""
        
        from ..node.node import Node
        
        return Node(nsnode.bdd2node(self._ptr))
        
        
    # ==========================================================================
    # ===== BDD operations =====================================================
    # ==========================================================================
    
    def dup(self):
        """
        Creates a copy of a BDD node.
    
        The reference count is increased by one unit.
    
        bdd_ptr bdd_dup (bdd_ptr);
        """
        return BDD(dd.bdd_dup(self._ptr), self._manager)
        
        
    def free(self):
        """
        Dereference a BDD node.
        
        If it dies, recursively decreases the reference count of its children.
    
        void bdd_free (DdManager *, bdd_ptr);
        """
        pass # TODO Investigate Python garbage collection then implement and use
    

    def is_true(self):
        """
        Check if the BDD is true.
        
        int bdd_is_true (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_is_true(self._manager._ptr, self._ptr):
            return True
        else:
            return False
        
    
    def is_false(self):
        """
        Check if the BDD is false.
        
        int bdd_is_false (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_is_false(self._manager._ptr, self._ptr):
            return True
        else:
            return False
        
        
    def isnot_true(self):
        """
        Check if the BDD is not true.
        
        int bdd_isnot_true (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_isnot_true(self._manager._ptr, self._ptr):
            return True
        else:
            return False
        
    
    def isnot_false(self):
        """
        Check if the BDD is not false.
        
        int bdd_isnot_false (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_isnot_false(self._manager._ptr, self._ptr):
            return True
        else:
            return False
    
    
    def entailed(self, other):
        """
        Determines whether self is less than or equal to other.
        
        int bdd_entailed (DdManager * dd, bdd_ptr f, bdd_ptr g);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_entailed(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False
        
        
    def intersected(self, other):
        """
        Determines whether an intersection between self and other is not empty.
        
        int bdd_intersected (DdManager * dd, bdd_ptr f, bdd_ptr g);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_intersected(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False


    def leq(self, other):
        """
        Determines whether self is less than or equal to other.
        
        int bdd_leq (DdManager * dd, bdd_ptr f, bdd_ptr g);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if dd.bdd_leq(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False
        
        
    def nott(self):
        """
        Applies NOT to the corresponding discriminant of f.
        
        bdd_ptr bdd_not (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_not(self._manager._ptr, self._ptr), self._manager)
        
    
    def andd(self, other):
        """
        Applies AND to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_and (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_and(self._manager._ptr, self._ptr, other._ptr),
                   self._manager)
        

    def orr(self, other):
        """
        Applies OR to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_or (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_or(self._manager._ptr, self._ptr, other._ptr),
                   self._manager)
        

    def xor(self, other):
        """
        Applies XOR to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_xor (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_xor(self._manager._ptr, self._ptr, other._ptr),
                   self._manager)
        
        
    def iff(self, other):
        """
        Applies IFF to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_iff (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_iff(self._manager._ptr, self._ptr, other._ptr),
                   self._manager)
        

    def imply(self, other):
        """
        Applies IMPLY to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_imply (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_imply(self._manager._ptr, self._ptr, other._ptr),
                   self._manager)
        
        
    def forsome(self, cube):
        """
        Existentially abstracts all the variables in cube from self.
        
        bdd_ptr bdd_forsome (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_forsome(self._manager._ptr, self._ptr, cube._ptr),
                   self._manager)
        
        
    def forall(self, cube):
        """
        Universally abstracts all the variables in cube from self.
        
        bdd_ptr bdd_forall (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_forall(self._manager._ptr, self._ptr, cube._ptr),
                   self._manager)
        
    
    def and_accumulate(self, other):
        """
        Applies AND to self and other and stores the result in self.
        
        void bdd_and_accumulate (DdManager *, bdd_ptr *, bdd_ptr);
        """
        pass # TODO Find a way to give the pointer and implement this
        
        
    def or_accumulate(self, other):
        """
        Applies OR to self and other and stores the result in self.
        
        void bdd_or_accumulate (DdManager *, bdd_ptr *, bdd_ptr);
        """
        pass # TODO Find a way to give the pointer and implement this
        
        
        
    def get_true(manager):
        """
        Return the TRUE BDD.
        
        manager -- a DDManager.
        
        bdd_ptr bdd_true (DdManager *);
        """
        return BDD(dd.bdd_true(manager._ptr), manager._ptr)
        
        
    def get_false():
        """
        Return the FALSE BDD.
        
        bdd_ptr bdd_false (DdManager *);
        """
        return BDD(dd.bdd_false(manager._ptr), manager._ptr)
    
    
    # TODO Implement Python-like syntax for BDDs operations
