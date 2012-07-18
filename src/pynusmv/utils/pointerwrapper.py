class PointerWrapper:
    """Wrapper for a NuSMV pointer."""
    
    def __init__(self, pointer, freeit = False):
        """
        pointer -- the pointer to wrap.
        freeit -- whether the pointer has to be freed when this wrapper
                  is destroyed.
        """
        self._ptr = pointer
        self._freeit = freeit