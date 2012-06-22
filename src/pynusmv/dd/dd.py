from ..nusmv.dd import dd

class BDD:
    """
    Python class for BDD structure.
    
    The BDD class contains a pointer to a BDD in NuSMV (a bdd_ptr)
    and provides a set of operations on this BDD.
    """
    
    def __init__(self, ptr):
        """
        Create a new BDD with ptr.
        
        ptr -- the pointer to the NuSMV BDD.
        """
        self.__ptr = ptr
        
        
    def entailed(self, bdd):
        """
        Determine whether this BDD is less than or equal to bdd.
        
        bdd -- a BDD.
        
        Return True if this BDD is less than or equal to bdd.
        """
        pass # TODO