"""
Provide functions to initialize and quit NuSMV.

init_nusmv should be called before any other call to pynusmv functions.
deinit_nusmv should be called after using pynusmv.

deinit_nusmv must be called when everything is freed.
This means that deinit_nusmv must be called in a context where there is no
NuSMV wrapper object that would be freed after deinit_nusmv call.
"""

from ..nusmv.cinit import cinit

def init_nusmv():
    """Initialize NuSMV."""
    cinit.NuSMVCore_init_data()
    cinit.NuSMVCore_init(None, 0)
    

def deinit_nusmv():
    """Quit NuSMV."""
    cinit.NuSMVCore_quit()