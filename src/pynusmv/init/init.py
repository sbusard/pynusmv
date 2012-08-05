"""
Provide functions to initialize and quit NuSMV.

init_nusmv should be called before any other call to pynusmv functions.
deinit_nusmv should be called after using pynusmv.

deinit_nusmv must be called when everything is freed.
This means that deinit_nusmv must be called in a context where there is no
NuSMV wrapper object that would be freed after deinit_nusmv call.
"""
import weakref
import gc
from ..nusmv.cinit import cinit

# Set of pointer wrappers to collect when deiniting NuSMV
__collector = None


def init_nusmv():
    """Initialize NuSMV."""
    global __collector
    if __collector is not None:
        raise NuSMVInitError("Cannot initialize NuSMV twice.")
    else:
        __collector = []
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0) # No addons specified
        cinit.NuSMVCore_init_cmd_options()
    

def deinit_nusmv():
    """Quit NuSMV."""
    global __collector
    if __collector is None:
        raise NuSMVInitError("Cannot deinitialize NuSMV before initialization.")
    else:
        # First garbage collect with Python
        gc.collect
        # Then garbage collect with PyNuSMV
        for elem in __collector:
            if elem() is not None:
                elem()._free()
        __collector = None
        cinit.NuSMVCore_quit()
    
    
def reset_nusmv():
    """Reset NuSMV."""
    deinit_nusmv()
    init_nusmv()
    
    
def register_wrapper(wrapper):
    """Register pointer wrapper to NuSMV garbage collector."""
    global __collector
    if __collector is None:
        raise NuSMVInitError("Cannot register before initializing NuSMV.")
    else:
        __collector.append(weakref.ref(wrapper))
    
    
class NuSMVInitError(Exception):
    """NuSMV initialisation-related exception."""
    pass