"""
The :mod:`pynusmv.init` module provides functions to initialize and quit NuSMV.

The :func:`init_nusmv` function can be used as a context manager for the `with`
Python statement::

    with init_nusmv():
        ...

.. warning:: :func:`init_nusmv` should be called before any other call to
   pynusmv functions; :func:`deinit_nusmv` should be called after using
   pynusmv.

"""


__all__ = ['init_nusmv', 'deinit_nusmv', 'reset_nusmv']


import weakref
import gc

from .nusmv.cinit import cinit as nscinit
from .nusmv.opt import opt as nsopt
from .nusmv.cmd import cmd as nscmd
from .nusmv.dd import dd as nsdd

from .exception import NuSMVInitError, PyNuSMVError


# Set of pointer wrappers to collect when deiniting NuSMV
__collecting = True
__collector = None


class _PyNuSMVContext(object):

    """
    A PyNuSMV Context allows to initialize and deinitialize PyNuSMV through
    a `with` Python statement.
    """

    def __enter__(self):
        return None

    def __exit__(self, exc_type, exc_value, traceback):
        if exc_type is None:
            deinit_nusmv()


def init_nusmv(collecting=True):
    """
    Initialize NuSMV. Must be called only once before calling
    :func:`deinit_nusmv`.
    
    :param collecting: Whether or not collecting pointer wrappers to free them
                       before deiniting nusmv.
    
    .. warning: Deactivating the collection of pointer wrappers may provoke
                segmentation faults when deiniting nusmv without correctly
                freeing all pointer wrappers in advance.
                On the other hand, collection may blow memory.

    """
    global __collector, __collecting
    if __collector is not None:
        raise NuSMVInitError("Cannot initialize NuSMV twice.")
    else:
        __collecting = collecting
        __collector = []
        nscinit.NuSMVCore_init_data()
        nscinit.NuSMVCore_init(None, 0)  # No addons specified
        nscinit.NuSMVCore_init_cmd_options()

        # Set NuSMV in interactive mode to avoid fast termination when errors
        # nsopt.unset_batch(nsopt.OptsHandler_get_instance())

        # Initialize option commands (set, unset)
        # to be able to set parser_is_lax
        nsopt.init_options_cmd()
        nscmd.Cmd_SecureCommandExecute("set parser_is_lax")

        return _PyNuSMVContext()


def deinit_nusmv(ddinfo=False):
    """
    Quit NuSMV. Must be called only once, after calling :func:`init_nusmv`.

    :param ddinfo: Whether or not display Decision Diagrams statistics.

    """
    # Apply Python garbage collection first, then collect every pointer wrapper
    # that is not yet collected by Python GC
    from . import glob

    # Print statistics on stdout about DDs handled by the main DD manager.
    if ddinfo:
        try:
            manager = glob.prop_database().master.bddFsm.bddEnc.DDmanager
            nsdd.dd_print_stats(manager._ptr, nscinit.cvar.nusmv_stdout)
        except PyNuSMVError:
            pass

    glob._reset_globals()

    global __collector
    if __collector is None:
        raise NuSMVInitError(
            "Cannot deinitialize NuSMV before initialization.")
    else:
        # First garbage collect with Python
        gc.collect()
        # Then garbage collect with PyNuSMV
        for elem in __collector:
            if elem() is not None:
                elem()._free()
        __collector = None
        nscinit.NuSMVCore_quit()


def reset_nusmv():
    """
    Reset NuSMV, i.e. deinit it and init it again. Cannot be called before
    :func:`init_nusmv`.

    """
    deinit_nusmv()
    init_nusmv()


def _register_wrapper(wrapper):
    """
    Register pointer wrapper to PyNuSMV garbage collector. `wrapper` is
    stored to be collected before quitting NuSMV.

    :param wrapper: the pointer wrapper to register
    :type wrapper: :class:`PointerWrapper <pynusmv.utils.PointerWrapper>`

    """
    global __collector, __collecting
    if __collector is None:
        raise NuSMVInitError("Cannot register before initializing NuSMV.")
    else:
        if __collecting:
            __collector.append(weakref.ref(wrapper))
