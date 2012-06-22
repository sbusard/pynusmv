from ..nusmv.dd import dd

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