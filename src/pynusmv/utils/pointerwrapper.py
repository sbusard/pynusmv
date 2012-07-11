class PointerWrapper:
    """Wrapper for a NuSMV pointer."""
    
    def __init__(self, pointer):
        """pointer -- the pointer to wrap."""
        self._ptr = pointer