from ..nusmv.dd import dd
from ..utils.pointerwrapper import PointerWrapper

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
    
    All BDDs are freed by default.
    """
    
    def __init__(self, ptr, dd_manager=None, freeit = True):
        """
        Create a new BDD with ptr.
        
        ptr -- the pointer to the NuSMV BDD.
        dd_manager -- the DD manager for this BDD.
        """
        super().__init__(ptr, freeit)
        self._manager = dd_manager
        
    
    def __del__(self):
        if self._freeit and self._ptr is not None:
            dd.bdd_free(self._manager._ptr, self._ptr)
        

    def equal(self, other):
        """Return whether self and other are the same BDD."""
        if dd.bdd_equal(self._ptr, other._ptr):
            return True
        else:
            return False
        
        
    # ==========================================================================
    # ===== BDD operations =====================================================
    # ==========================================================================
    
    def dup(self):
        """
        Creates a copy of a BDD node.
    
        The reference count is increased by one unit.
    
        bdd_ptr bdd_dup (bdd_ptr);
        """
        return BDD(dd.bdd_dup(self._ptr), self._manager, freeit = True)
        

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
        return BDD(dd.bdd_not(self._manager._ptr, self._ptr), self._manager,
                   freeit = True)
        
    
    def andd(self, other):
        """
        Applies AND to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_and (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_and(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        

    def orr(self, other):
        """
        Applies OR to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_or (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_or(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        

    def xor(self, other):
        """
        Applies XOR to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_xor (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_xor(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        
        
    def iff(self, other):
        """
        Applies IFF to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_iff (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_iff(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        

    def imply(self, other):
        """
        Applies IMPLY to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_imply (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_imply(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        
        
    def forsome(self, cube):
        """
        Existentially abstracts all the variables in cube from self.
        
        bdd_ptr bdd_forsome (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_forsome(self._manager._ptr, self._ptr, cube._ptr),
                   self._manager, freeit = True)
        
        
    def forall(self, cube):
        """
        Universally abstracts all the variables in cube from self.
        
        bdd_ptr bdd_forall (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(dd.bdd_forall(self._manager._ptr, self._ptr, cube._ptr),
                   self._manager, freeit = True)
        


    # ==========================================================================
    # ===== Built-in BDD operations ============================================
    # ==========================================================================
    
    
    def __lt__(self, other):
        return (self <= other) and not (self == other)
        
    def __le__(self, other):
        return self.leq(other)
        
    def __eq__(self, other):
        return self.equal(other)
        
    def __ne__(self, other):
        return not self.equal(other)
        
    def __gt__(self, other):
        return not (self <= other)
        
    def __ge__(self, other):
        return not (self < other)
    
    
    
    def __add__(self, other):
        return self.orr(other)
        
    def __or__(self, other):
        return self.orr(other)
    
    
    def __mul__(self, other):
        return self.andd(other)

    def __and__(self, other):
        return self.andd(other)    
    
    
    def __sub__(self, other):
        return self & ~(other)
    
    
    
    def __xor__(self, other):
        return self.xor(other)
    
    
    
    def __neg__(self):
        return self.nott()
    
    def __invert__(self):
        return self.nott()
    
    
    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
    def true(manager):
        """
        Return the TRUE BDD.
        
        manager -- a DDManager.
        
        bdd_ptr bdd_true (DdManager *);
        """
        return BDD(dd.bdd_true(manager._ptr), manager)
        
        
    def false(manager):
        """
        Return the FALSE BDD.
        
        bdd_ptr bdd_false (DdManager *);
        """
        return BDD(dd.bdd_false(manager._ptr), manager)
    
