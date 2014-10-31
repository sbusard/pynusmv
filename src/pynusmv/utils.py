"""
The :mod:`pynusmv.utils` module contains some secondary functions and classes
used by PyNuSMV internals.

"""


__all__ = ['PointerWrapper', 'fixpoint', 'update']


from pynusmv.init import _register_wrapper


class PointerWrapper(object):

    """
    Superclass wrapper for NuSMV pointers.

    Every pointer to a NuSMV structure is wrapped in a PointerWrapper
    or in a subclass of PointerWrapper.
    Every subclass instance takes a pointer to a NuSMV structure as constructor
    parameter.

    It is the responsibility of PointerWrapper and its subclasses to free
    the wrapped pointer. Some pointers have to be freed like `bdd_ptr`,
    but other do not have to be freed since NuSMV takes care of this;
    for example, `BddFrm_ptr` does not have to be freed.
    To ensure that a pointer is freed only once, PyNuSMV ensures that
    any pointer is wrapped by only one PointerWrapper (or subclass of it)
    if the pointer have to be freed.

    """

    def __init__(self, pointer, freeit=False):
        """
        Create a new PointerWrapper.

        :param pointer: the pointer to wrap
        :param freeit: whether the pointer has to be freed when this wrapper
                       is destroyed

        """
        self._ptr = pointer
        self._freeit = freeit
        _register_wrapper(self)

    def _free(self):
        """
        Every subclass must implement `_free` if there is something to free.

        """
        pass

    def __del__(self):
        if self._freeit and self._ptr is not None:
            self._free()


class AttributeDict(dict):

    """
    An `AttributeDict` is a dictionary for which elements can be accessed by
    using their keys as attribute names.

    """

    def __init__(self, *args, **kwargs):
        super(AttributeDict, self).__init__(*args, **kwargs)
        self.__dict__ = self


def fixpoint(funct, start):
    """
    Return the fixpoint of `funct`, as a BDD, starting with `start` BDD.

    :rtype: :class:`BDD <pynusmv.dd.BDD>`

    .. note:: mu Z.f(Z) least fixpoint is implemented with
              `fixpoint(funct, false)`.
              nu Z.f(Z) greatest fixpoint is implemented with
              `fixpoint(funct, true)`.

    """

    old = start
    new = funct(start)
    while old != new:
        old = new
        new = funct(old)
    return old


def update(old, new):
    """
    Update `old` with `new`. `old` is assumed to have the `extend` or `update`
    method, and `new` is assumed to be a good argument for the corresponding
    method.

    :param old: the data to update.
    :param new: the date to update with.

    """
    try:
        old.extend(new)
    except AttributeError:
        old.update(new)
