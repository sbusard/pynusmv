class PointerWrapper:
    """Wrapper for a NuSMV pointer."""
    
    def __init__(self, pointer, freeit = False):
        """
        Initialize a pointer wrapper with pointer.
        
        pointer -- the pointer to wrap.
        freeit -- whether or not the wrapped pointer must be freed when
                  this object is destroyed.
                  Default to False since most pointers do not have to be freed
                  in Python (NuSMV takes care of them).
                  
        It is the responsibility of subclasses to implement the destructor
        and the responsibility of subclasses instantiators to correctly
        set freeit.
        """
        self._ptr = pointer
        self._freeit = freeit