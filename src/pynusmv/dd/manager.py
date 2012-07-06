from ..utils.wrap import PointerWrapper
    

class DDManager(PointerWrapper):
    """
    Python class for BDD Manager.
    """
    
    def __init__(self, ptr):
        """
        Create a new manager with ptr.
        
        ptr -- the pointer to the NuSMV DdManager.
        """
        super().__init__(ptr)