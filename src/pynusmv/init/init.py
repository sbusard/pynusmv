"""
Provide functions to initialize and quit NuSMV.

init_nusmv should be called before any other call to pynusmv functions.
deinit_nusmv should be called after using pynusmv.
"""
import weakref
import gc
from ..nusmv.cinit import cinit as nscinit
from ..nusmv.opt import opt as nsopt
from ..nusmv.cmd import cmd as nscmd

from ..utils.exception import NuSMVInitError

# Set of pointer wrappers to collect when deiniting NuSMV
__collector = None


def init_nusmv():
    """
    Initialize NuSMV.
    
    Must be called only once before calling deinit_nusmv.
    """
    global __collector
    if __collector is not None:
        raise NuSMVInitError("Cannot initialize NuSMV twice.")
    else:
        __collector = []
        nscinit.NuSMVCore_init_data()
        nscinit.NuSMVCore_init(None, 0) # No addons specified
        nscinit.NuSMVCore_init_cmd_options()
        
        # Set NuSMV in interactive mode to avoid fast termination when errors
        nsopt.unset_batch(nsopt.OptsHandler_get_instance())
        
        # Initialize option commands (set, unset)
        # to be able to set parser_is_lax
        nsopt.init_options_cmd()
        nscmd.Cmd_SecureCommandExecute("set parser_is_lax")    
    

def deinit_nusmv():
    """
    Quit NuSMV.
    
    Must be called only once, after calling init_nusmv.
    
    Apply Python garbage collection first, then collect every pointer wrapper
    that is not yet collected by Python GC.
    """
    
    from ..globals.globals import Globals
    Globals.reset_globals()    
    
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
        nscinit.NuSMVCore_quit()
    
    
def reset_nusmv():
    """
    Reset NuSMV, i.e. deinit it and init it again.
    
    Cannot be called before init_nusmv.
    """
    deinit_nusmv()
    init_nusmv()
    
    
def register_wrapper(wrapper):
    """Register pointer wrapper to NuSMV garbage collector."""
    global __collector
    if __collector is None:
        raise NuSMVInitError("Cannot register before initializing NuSMV.")
    else:
        __collector.append(weakref.ref(wrapper))