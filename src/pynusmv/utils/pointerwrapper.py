"""
Superclass wrapper for NuSMV pointers.

Every pointer to a NuSMV structure is wrapped in a PointerWrapper
or in a subclass of PointerWrapper.
Every subclass instance takes a pointer to a NuSMV structure as constructor
parameter.

It is the responsibility of PointerWrapper and its subclasses to free
the wrapped pointer. Some pointers have to be freed like bdd_ptr, but other
do not have to be freed since NuSMV takes care of this; for example, BddFrm_ptr
does not have to be freed.

To ensure that a pointer is freed only once, PyNuSMV ensures that
any pointer is wrapped by only one PointerWrapper (or subclass of it)
if the pointer have to be freed.
"""

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