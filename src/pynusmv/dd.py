"""
The :mod:`pynusmv.dd` module provides some BDD-related structures:

* :class:`BDD` represents a BDD.
* :class:`BDDList` represents a list of BDDs.
* :class:`State` represents a particular state of the model.
* :class:`Inputs` represents input variables values,
  i.e. a particular action of the model.
* :class:`StateInputs` represents a particular state-inputs pair of the model.
* :class:`Cube` represents a particular cube of variables the model.
* :class:`DDManager` represents a NuSMV DD manager.

It also provides global methods to work on BDD variables reordering: :func:`enable_dynamic_reordering`, :func:`disable_dynamic_reordering`,
:func:`dynamic_reordering_enabled`, :func:`reorder`.

"""


__all__ = ['enable_dynamic_reordering', 'disable_dynamic_reordering',
           'dynamic_reordering_enabled', 'reorder',
           'BDD', 'BDDList', 'State', 'Inputs', 'StateInputs', 'Cube',
           'DDManager']


from .nusmv.dd import dd as nsdd
from .nusmv.node import node as nsnode
from .nusmv.compile.symb_table import symb_table as nssymb_table
from .nusmv.enc.bdd import bdd as nsbddEnc
from .nusmv.utils import utils as nsutils
from .nusmv.cinit import cinit as nscinit
from .nusmv.opt import opt as nsopt

from .utils import PointerWrapper
from .exception import MissingManagerError


def enable_dynamic_reordering(DDmanager=None, method="sift"):
    """
    Enable dynamic reordering of BDD variables under control of `DDmanager`
    with the given `method`.

    :param DDmanager: the concerned DD manager; if None, the global DD manager
                      is used instead.
    :type DDmanager: :class:`DDManager`
    :param method: the method to use for reordering:
                   `sift (default method)`,
                   `random`,
                   `random_pivot`,
                   `sift_converge`,
                   `symmetry_sift`,
                   `symmetry_sift_converge`,
                   `window{2, 3, 4}`,
                   `window{2, 3, 4}_converge`,
                   `group_sift`,
                   `group_sift_converge`,
                   `annealing`,
                   `genetic`,
                   `exact`,
                   `linear`,
                   `linear_converge`,
                   `same` (the previously chosen method)
    :type method: :class:`str`

    :raise: a :exc:`MissingManagerError
            <pynusmv.exception.MissingManagerError>` if the manager is missing

    .. note:: For more information on reordering methods, see NuSMV manual.

    """
    if DDmanager is None:
        DDmanager_ptr = nscinit.cvar.dd_manager
    else:
        DDmanager_ptr = DDmanager._ptr
    if DDmanager_ptr is None:
        raise MissingManagerError("Missing manager")

    method = nsdd.StringConvertToDynOrderType(method)
    if method == nsdd.CUDD_REORDER_NONE:
        method = nsdd.CUDD_REORDER_SIFT
    nsopt.set_dynamic_reorder(nsopt.OptsHandler_get_instance())
    nsdd.dd_autodyn_enable(DDmanager_ptr, method)

def disable_dynamic_reordering(DDmanager=None):
    """
    Disable dynamic reordering of BDD variables under control of `DDmanager`.

    :param DDmanager: the concerned DD manager; if None, the global DD manager
                      is used instead.
    :type DDmanager: :class:`DDManager`

    :raise: a :exc:`MissingManagerError
            <pynusmv.exception.MissingManagerError>` if the manager is missing

    """
    if DDmanager is None:
        DDmanager_ptr = nscinit.cvar.dd_manager
    else:
        DDmanager_ptr = DDmanager._ptr
    if DDmanager_ptr is None:
        raise MissingManagerError("Missing manager")
    nsopt.unset_dynamic_reorder(nsopt.OptsHandler_get_instance())
    nsdd.dd_autodyn_disable(DDmanager_ptr)

def dynamic_reordering_enabled(DDmanager=None):
    """
    Return the dynamic reordering method used if reordering is enabled for BDD
    under control of `DDmanager`, None otherwise.

    :param DDmanager: the concerned DD manager; if None, the global DD manager
                      is used instead.
    :type DDmanager: :class:`DDManager`
    :rtype: None, or a the name of the method used

    :raise: a :exc:`MissingManagerError
            <pynusmv.exception.MissingManagerError>` if the manager is missing

    """
    if DDmanager is None:
        DDmanager_ptr = nscinit.cvar.dd_manager
    else:
        DDmanager_ptr = DDmanager._ptr
    if DDmanager_ptr is None:
        raise MissingManagerError("Missing manager")
    enabled, method = nsdd.reordering_status(DDmanager_ptr)
    return nsdd.DynOrderTypeConvertToString(method) if bool(enabled) else None

def reorder(DDmanager=None, method="sift"):
    """
    Force a reordering of BDD variables under control of `DDmanager`.

    :param DDmanager: the concerned DD manager; if None, the global DD manager
                      is used instead.
    :type DDmanager: :class:`DDManager`
    :param method: the method to use for reordering:
                   `sift (default method)`,
                   `random`,
                   `random_pivot`,
                   `sift_converge`,
                   `symmetry_sift`,
                   `symmetry_sift_converge`,
                   `window{2, 3, 4}`,
                   `window{2, 3, 4}_converge`,
                   `group_sift`,
                   `group_sift_converge`,
                   `annealing`,
                   `genetic`,
                   `exact`,
                   `linear`,
                   `linear_converge`,
                   `same` (the previously chosen method)
    :type method: :class:`str`

    :raise: a :exc:`MissingManagerError
            <pynusmv.exception.MissingManagerError>` if the manager is missing

    .. note:: For more information on reordering methods, see NuSMV manual.

    """
    if DDmanager is None:
        DDmanager_ptr = nscinit.cvar.dd_manager
    else:
        DDmanager_ptr = DDmanager._ptr
    if DDmanager_ptr is None:
        raise MissingManagerError("Missing manager")
    method = nsdd.StringConvertToDynOrderType(method)
    if method == nsdd.CUDD_REORDER_NONE:
        method = nsdd.CUDD_REORDER_SIFT
    nsdd.dd_reorder(DDmanager_ptr, method, nsdd.DEFAULT_MINSIZE)


class BDD(PointerWrapper):

    """
    Python class for BDD structure.

    The BDD represents a BDD in NuSMV and provides a set of operations on this
    BDD.
    Thanks to operator overloading, it is possible to write compact expressions
    on BDDs. The available operations are:

    * ``a + b`` and ``a | b`` compute the disjunction of ``a`` and ``b``
    * ``a * b`` and ``a & b`` compute the conjunction of ``a`` and ``b``
    * ``~a`` and ``-a`` compute the negation of ``a``
    * ``a - b`` computes ``a & ~b``
    * ``a ^ b`` computes the exclusive-OR (XOR) of ``a`` and ``b``
    * ``a == b``, ``a <= b``, ``a < b``, ``a > b`` and ``a >= b`` compare ``a``
      and ``b``

    Any BDD operation raises a :exc:`MissingManagerError
    <pynusmv.exception.MissingManagerError>` whenever the manager
    of the BDD is None and a manager is needed to perform the operation.

    """
#    All BDDs are freed by default. Every operation on BDDs that return a new
#    BDD uses bdd_dup to ensure that the new BDD wraps a pointer to free.

    def __init__(self, ptr, dd_manager=None, freeit=True):
        """
        Create a new BDD with `ptr`.

        :param ptr: the pointer to the NuSMV BDD
        :type ptr: NuSMV ``bdd_ptr``
        :param dd_manager: the DD manager for this BDD
        :type dd_manager: :class:`DDManager`
        :param freeit: whether the pointer must be freed with the BDD, or not.

        """
        assert(ptr is not None)
        super(BDD, self).__init__(ptr, freeit)
        self._manager = dd_manager

    def _free(self):
        if self._freeit and self._ptr is not None:
            nsdd.bdd_free(self._manager._ptr, self._ptr)
            self._freeit = False
    
    def __deepcopy__(self, memo):
        return self.dup()
    
    @property
    def size(self):
        """
        The number of BDD nodes of this BDD.
        """
        if self._manager is None:
            raise MissingManagerError()

        return nsdd.bdd_size(self._manager._ptr, self._ptr)

    def equal(self, other):
        """
        Determine whether this BDD is equal to `other` or not.

        :param other: the BDD to compare
        :type other: :class:`BDD`

        """
        if not isinstance(other, BDD):
            return False
        if nsdd.bdd_equal(self._ptr, other._ptr):
            return True
        else:
            return False

    def __hash__(self):
        """
        Return the hash of this BDD.

        """
        return int(self._ptr)

    # ==========================================================================
    # ===== BDD operations ===================================================
    # ==========================================================================

    def dup(self):
        """
        Return a copy of this BDD.

        """
        # Call to bdd_ptr bdd_dup (bdd_ptr);

        return BDD(nsdd.bdd_dup(self._ptr), self._manager, freeit=True)

    def is_true(self):
        """
        Determine whether this BDD is true or not.

        """
        # Call to int bdd_is_true (DdManager *, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()

        if nsdd.bdd_is_true(self._manager._ptr, self._ptr):
            return True
        else:
            return False

    def is_false(self):
        """
        Determine whether this BDD is false or not.

        """
        # Call to int bdd_is_false (DdManager *, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()

        if nsdd.bdd_is_false(self._manager._ptr, self._ptr):
            return True
        else:
            return False

    def isnot_true(self):
        """
        Determine whether this BDD is not true.

        """
        # Call to int bdd_isnot_true (DdManager *, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()

        if nsdd.bdd_isnot_true(self._manager._ptr, self._ptr):
            return True
        else:
            return False

    def isnot_false(self):
        """
        Determine whether this BDD is not false.

        """
        # int bdd_isnot_false (DdManager *, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()

        if nsdd.bdd_isnot_false(self._manager._ptr, self._ptr):
            return True
        else:
            return False

    def entailed(self, other):
        """
        Determine whether this BDD is included in `other` or not.

        :param other: the BDD to compare
        :type other: :class:`BDD`
        """
        # Call to int bdd_entailed (DdManager * dd, bdd_ptr f, bdd_ptr g);

        if self._manager is None:
            raise MissingManagerError()

        if nsdd.bdd_entailed(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False

    def intersected(self, other):
        """
        Determine whether the intersection between this BDD
        and `other` is not empty.

        :param other: the BDD to compare
        :type other: :class:`BDD`

        """
        # Call to int bdd_intersected (DdManager * dd, bdd_ptr f, bdd_ptr g);

        if self._manager is None:
            raise MissingManagerError()

        if nsdd.bdd_intersected(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False

    def leq(self, other):
        """
        Determine whether this BDD is less than or equal to `other`.

        :param other: the BDD to compare
        :type other: :class:`BDD`

        """
        # int bdd_leq (DdManager * dd, bdd_ptr f, bdd_ptr g);

        if self._manager is None:
            raise MissingManagerError()

        if nsdd.bdd_leq(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False

    def not_(self):
        """
        Compute the complement of this BDD.

        """
        # Call to bdd_ptr bdd_not (DdManager *, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_not(self._manager._ptr, self._ptr), self._manager,
                   freeit=True)

    def and_(self, other):
        """
        Compute the conjunction of this BDD and `other`.

        :param other: the other BDD
        :type other: :class:`BDD`

        """
        # Call to bdd_ptr bdd_and (DdManager *, bdd_ptr, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_and(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit=True)

    def or_(self, other):
        """
        Compute the conjunction of this BDD and `other`.

        :param other: the other BDD
        :type other: :class:`BDD`

        """
        # Call to bdd_ptr bdd_or (DdManager *, bdd_ptr, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_or(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit=True)

    def xor(self, other):
        """
        Compute the exclusive-OR of this BDD and `other`.

        :param other: the other BDD
        :type other: :class:`BDD`

        """
        # Call to bdd_ptr bdd_xor (DdManager *, bdd_ptr, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_xor(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit=True)

    def iff(self, other):
        """
        Compute the IFF operation on this BDD and `other`.

        :param other: the other BDD
        :type other: :class:`BDD`

        """
        # Call to bdd_ptr bdd_iff (DdManager *, bdd_ptr, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_iff(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit=True)

    def imply(self, other):
        """
        Compute the IMPLY operation on this BDD and `other`.

        :param other: the other BDD
        :type other: :class:`BDD`

        """
        # Call to bdd_ptr bdd_imply (DdManager *, bdd_ptr, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_imply(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit=True)

    def diff(self, other):
        return self & ~other
    
    def intersection(self, other):
        return self.and_(other)

    def forsome(self, cube):
        """
        Existentially abstract all the variables in cube from this BDD.

        :param cube: the cube
        :type cube: :class:`BDD`

        """
        # Call to bdd_ptr bdd_forsome (DdManager *, bdd_ptr, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_forsome(self._manager._ptr, self._ptr, cube._ptr),
                   self._manager, freeit=True)

    def forall(self, cube):
        """
        Universally abstract all the variables in cube from this BDD.

        :param cube: the cube
        :type cube: :class:`BDD`

        """
        # Call to bdd_ptr bdd_forall (DdManager *, bdd_ptr, bdd_ptr);

        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_forall(self._manager._ptr, self._ptr, cube._ptr),
                   self._manager, freeit=True)

    def minimize(self, c):
        """
        Restrict this BDD with c, as described in Coudert et al. ICCAD90.

        :param c: the BDD used to restrict this BDD
        :type c: :class:`BDD`

        .. note:: Always returns a BDD not larger than the this BDD.

        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_minimize(self._manager._ptr, self._ptr, c._ptr),
                   self._manager, freeit=True)

    # =========================================================================
    # ===== Built-in BDD operations ==========================================
    # =========================================================================

    def __lt__(self, other):
        return (self <= other) and not self == other

    def __le__(self, other):
        return self.leq(other)

    def __eq__(self, other):
        return self.equal(other)

    def __ne__(self, other):
        return not self.equal(other)

    def __gt__(self, other):
        return other.__lt__(self)

    def __ge__(self, other):
        return other.__le__(self)

    def __add__(self, other):
        return self.or_(other)

    def __or__(self, other):
        return self.or_(other)

    def __mul__(self, other):
        return self.and_(other)

    def __and__(self, other):
        return self.and_(other)

    def __sub__(self, other):
        return self & ~(other)

    def __xor__(self, other):
        return self.xor(other)

    def __neg__(self):
        return self.not_()

    def __invert__(self):
        return self.not_()

    # =========================================================================
    # ===== Static methods ===================================================
    # =========================================================================

    @staticmethod
    def true(manager_or_fsm=None):
        """
        Return the TRUE BDD.

        :param manager_or_fsm: if not `None`, the manager of the returned BDD
                               or the FSM; otherwise, the global FSM is used.
        :type manager_or_fsm: :class:`DDManager` or
                              :class:`BddFsm <pynusmv.fsm.BddFsm>`

        """
        # Call to bdd_ptr bdd_true (DdManager *);
        
        from .fsm import BddFsm
        if manager_or_fsm is None:
            from .glob import prop_database
            manager = prop_database().master.bddFsm.bddEnc.DDmanager
        elif isinstance(manager_or_fsm, BddFsm):
            manager = manager_or_fsm.bddEnc.DDmanager
        else:
            manager = manager_or_fsm

        return BDD(nsdd.bdd_true(manager._ptr), manager, freeit=True)

    @staticmethod
    def false(manager_or_fsm=None):
        """
        Return the FALSE BDD.

        :param manager_or_fsm: if not `None`, the manager of the returned BDD
                               or the FSM; otherwise, the global FSM is used.
        :type manager_or_fsm: :class:`DDManager` or
                              :class:`BddFsm <pynusmv.fsm.BddFsm>`

        """
        # Call to bdd_ptr bdd_false (DdManager *);
        
        from .fsm import BddFsm
        if manager_or_fsm is None:
            from .glob import prop_database
            manager = prop_database().master.bddFsm.bddEnc.DDmanager
        elif isinstance(manager_or_fsm, BddFsm):
            manager = manager_or_fsm.bddEnc.DDmanager
        else:
            manager = manager_or_fsm

        return BDD(nsdd.bdd_false(manager._ptr), manager, freeit=True)


class BDDList(PointerWrapper):

    """
    A BDD list stored as NuSMV nodes.

    The BDDList class implements a NuSMV nodes-based BDD list and can be used
    as any Python list.

    """
    # BDDLists are freed when destroyed, as well as the content.
    # When getting elements or tuple from a BDDList, copies of BDDs are made
    # and returned.

    def __init__(self, ptr, ddmanager=None, freeit=True):
        super(BDDList, self).__init__(ptr, freeit)
        self._manager = ddmanager

    def _free(self):
        if self._freeit and self._ptr is not None:
            # Free content
            ptr = self._ptr
            while ptr:
                # Free BDD
                bdd_ptr = nsnode.node2bdd(nsnode.car(ptr))
                if bdd_ptr is not None:
                    nsdd.bdd_free(self._manager._ptr, bdd_ptr)
                ptr = nsnode.cdr(ptr)

            # Free list
            nsnode.free_list(self._ptr)
            self._freeit = False

    def __len__(self):
        ptr = self._ptr
        length = 0
        while ptr:
            length += 1
            ptr = nsnode.cdr(ptr)
        return length

    def __getitem__(self, val):
        """
        Return the BDD stored at val.

        :param val: the index requested OR a slice.

        .. note:: cannot access elements with negative indices.
        """
        if isinstance(val, int):
            if val < 0:
                raise IndexError("BDDList index out of range")
            ptr = self._ptr
            while val > 0:
                if ptr is None:
                    raise IndexError("BDDList index out of range")
                val -= 1
                ptr = nsnode.cdr(ptr)
            if ptr is None:
                raise IndexError("BDDList index out of range")
            bdd_ptr = nsnode.node2bdd(nsnode.car(ptr))
            if bdd_ptr is not None:
                return BDD(nsdd.bdd_dup(bdd_ptr), self._manager,
                           freeit=True)
            else:
                return None

        elif isinstance(val, slice):
            # TODO Implement slicing
            raise NotImplementedError("BDDList slice not implemented")

        else:
            raise IndexError("BDDList index wrong type")

    def __iter__(self):
        ptr = self._ptr
        while ptr:
            # Yield BDD copy
            bdd_ptr = nsnode.node2bdd(nsnode.car(ptr))
            if bdd_ptr is not None:
                yield BDD(nsdd.bdd_dup(bdd_ptr), self._manager, freeit=True)
            else:
                yield None
            ptr = nsnode.cdr(ptr)

    def to_tuple(self):
        """
        Return a tuple containing all BDDs of self.
        The returned BDDs are copies of the ones of self.
        """
        result = []
        for elem in self:
            result.append(elem)
        return tuple(result)

    # =========================================================================
    # ===== Class methods =====================================================
    # =========================================================================

    @staticmethod
    def from_tuple(bddtuple):
        """
        Create a node-based list from the Python tuple `bddtuple`.

        :param bddtuple: a Python tuple of BDDs

        Return a :class:`BDDList` representing the given tuple,
        using NuSMV nodes.
        All BDDs are assumed from the same DD manager;
        the created list contains the DD manager of the first non-`None` BDD.
        If all elements of `bddtuple` are `None`,
        the manager of the created :class:`BDDList` is `None`.
        """

        # Reverse tuple before, because we build the list reversely.
        bddtuple = bddtuple[::-1]
        nodes = None
        manager = None
        for elem in bddtuple:
            if elem:
                enode = nsnode.bdd2node(nsdd.bdd_dup(elem._ptr))
                if manager is None:
                    manager = elem._manager
            else:
                enode = elem
            nodes = nsnode.cons(enode, nodes)
        return BDDList(nodes, manager, freeit=True)


class State(BDD):

    """
    Python class for State structure.

    A State is a :class:`BDD` representing a single state of the model.

    """

    def __init__(self, ptr, fsm, freeit=True):
        super(State, self).__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    def get_str_values(self, layers=None):
        """
        Return a dictionary of the (variable, value) pairs of this State.

        :param layers: if not `None`, the set of names of the layers from which
                       picking the string values
        :rtype: a dictionary of pairs of strings.

        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for states
        if layers is None:
            layers = nssymb_table.SymbTable_get_class_layer_names(table._ptr,
                                                                  None)
            symbols = nssymb_table.SymbTable_get_layers_sf_symbols(table._ptr,
                                                                   layers)
            layers_array = None
        else:
            layers_array = nsutils.array_alloc_strings(len(layers))
            for i, layer in enumerate(layers):
                nsutils.array_insert_strings(layers_array, i, layer)
            symbols = nssymb_table.SymbTable_get_layers_sf_symbols(
                table._ptr, layers_array)

        # Get assign symbols (BddEnc)
        assign_list = nsbddEnc.BddEnc_assign_symbols(enc._ptr, self._ptr,
                                                     symbols, 0, None)

        values = {}
        # Traverse the symbols to print variables of the state
        assign_list_ptr = assign_list
        while assign_list_ptr:
            assignment = nsnode.car(assign_list_ptr)
            var = nsnode.car(assignment)
            val = nsnode.cdr(assignment)
            values[nsnode.sprint_node(var)] = nsnode.sprint_node(val)
            assign_list_ptr = nsnode.cdr(assign_list_ptr)

        nsnode.free_list(assign_list)
        nsutils.NodeList_destroy(symbols)
        if layers_array:
            nsutils.array_free(layers_array)

        return values

    # =========================================================================
    # ===== Static methods ===================================================
    # =========================================================================

    @staticmethod
    def from_bdd(bdd, fsm):
        """
        Return a new State of fsm from bdd.

        :param bdd: a BDD representing a single state
        :type bdd: :class:`BDD`
        :param fsm: the FSM from which the BDD comes from
        :type fsm: :class:`BddFsm <pynusmv.fsm.BddFsm>`

        """
        return State(nsdd.bdd_dup(bdd._ptr), fsm)


class Inputs(BDD):

    """
    Python class for inputs structure.

    An Inputs is a :class:`BDD` representing a single valuation of the inputs
    variables of the model, i.e. an action of the model.

    """

    def __init__(self, ptr, fsm, freeit=True):
        super(Inputs, self).__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    def get_str_values(self, layers=None):
        """
        Return a dictionary of the (variable, value) pairs of these Inputs.

        :param layers: if not `None`, the set of names of the layers from which
                       picking the string values
        :rtype: a dictionary of pairs of strings.

        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for inputs
        if layers is None:
            layers = nssymb_table.SymbTable_get_class_layer_names(table._ptr,
                                                                  None)
            symbols = nssymb_table.SymbTable_get_layers_i_symbols(table._ptr,
                                                                  layers)
            layers_array = None
        else:
            layers_array = nsutils.array_alloc_strings(len(layers))
            for i, layer in enumerate(layers):
                nsutils.array_insert_strings(layers_array, i, layer)
            symbols = nssymb_table.SymbTable_get_layers_i_symbols(table._ptr,
                                                                  layers_array)

        # Get assign symbols (BddEnc)
        assign_list = nsbddEnc.BddEnc_assign_symbols(enc._ptr, self._ptr,
                                                     symbols, 0, None)

        values = {}
        # Traverse the symbols to print variables of the state
        assign_list_ptr = assign_list
        while assign_list_ptr:
            assignment = nsnode.car(assign_list_ptr)
            var = nsnode.car(assignment)
            val = nsnode.cdr(assignment)
            values[nsnode.sprint_node(var)] = nsnode.sprint_node(val)
            assign_list_ptr = nsnode.cdr(assign_list_ptr)

        nsnode.free_list(assign_list)
        nsutils.NodeList_destroy(symbols)
        if layers_array:
            nsutils.array_free(layers_array)

        return values

    # =========================================================================
    # ===== Static methods ===================================================
    # =========================================================================

    @staticmethod
    def from_bdd(bdd, fsm):
        """
        Return a new Inputs of fsm from bdd.

        :param bdd: a BDD representing a single inputs variables
                    valuation
        :type bdd: :class:`BDD`
        :param fsm: the FSM from which the BDD comes from
        :type fsm: :class:`BddFsm <pynusmv.fsm.BddFsm>`

        """
        return Inputs(nsdd.bdd_dup(bdd._ptr), fsm)


class StateInputs(BDD):

    """
    Python class for State and Inputs structure.

    A StateInputs is a :class:`BDD` representing a single state/inputs pair
    of the model.

    """

    def __init__(self, ptr, fsm, freeit=True):
        super(StateInputs, self).__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    def get_str_values(self):
        """
        Return a dictionary of the (variable, value) pairs of this StateInputs.

        :rtype: a dictionary of pairs of strings.

        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for states
        layers = nssymb_table.SymbTable_get_class_layer_names(table._ptr, None)
        symbols = nssymb_table.SymbTable_get_layers_sf_symbols(
            table._ptr, layers)
        isymbols = nssymb_table.SymbTable_get_layers_i_symbols(
            table._ptr, layers)
        nsutils.NodeList_concat(symbols, isymbols)
        nsutils.NodeList_destroy(isymbols)

        # Get assign symbols (BddEnc)
        assign_list = nsbddEnc.BddEnc_assign_symbols(enc._ptr, self._ptr,
                                                     symbols, 0, None)

        values = {}
        # Traverse the symbols to print variables of the state
        assign_list_ptr = assign_list
        while assign_list_ptr:
            assignment = nsnode.car(assign_list_ptr)
            var = nsnode.car(assignment)
            val = nsnode.cdr(assignment)
            values[nsnode.sprint_node(var)] = nsnode.sprint_node(val)
            assign_list_ptr = nsnode.cdr(assign_list_ptr)

        nsnode.free_list(assign_list)

        nsutils.NodeList_destroy(symbols)

        return values


class Cube(BDD):

    """
    Python class for Cube structure.

    A Cube is a :class:`BDD` representing a BDD cube of the model.

    """

    def diff(self, other):
        """
        Compute the difference between this cube and `other`

        :param other: the other cube
        :type other: :class:`BDD`

        """
        # Call to bdd_ptr bdd_cube_diff (DdManager *, bdd_ptr, bdd_ptr);

        # TODO Check that other is a cube!

        if self._manager is None:
            raise MissingManagerError()
        if isinstance(other, Cube):
            return Cube(nsdd.bdd_cube_diff(self._manager._ptr,
                                           self._ptr,
                                           other._ptr),
                        self._manager, freeit=True)
        else:
            return super(Cube, self).diff(other)

    def intersection(self, other):
        """
        Compute the intersection of this Cube and `other`.

        :param other: the other Cube
        :type other: :class:`BDD`

        """
        # Call to bdd_ptr bdd_cube_intersection (DdManager *, bdd_ptr,
        # bdd_ptr);

        # TODO Check that other is a cube!

        if self._manager is None:
            raise MissingManagerError()
        if isinstance(other, Cube):
            return Cube(nsdd.bdd_cube_intersection(self._manager._ptr,
                                                   self._ptr, other._ptr),
                        self._manager, freeit=True)
        else:
            return super(Cube, self).intersection(other)

    def union(self, other):
        """
        Compute the union of this Cube and `other`.

        :param other: the other Cube
        :type other: :class:`Cube`

        """
        # Call to bdd_ptr bdd_cube_union (DdManager *, bdd_ptr, bdd_ptr);

        # TODO Check that other is a cube!

        if self._manager is None:
            raise MissingManagerError()
        if isinstance(other, Cube):
            return Cube(nsdd.bdd_cube_union(self._manager._ptr,
                                            self._ptr,
                                            other._ptr),
                        self._manager,
                        freeit=True)
        else:
            return super(Cube, self).union(other)

    def __sub__(self, other):
        return self.diff(other)

    def __add__(self, other):
        return self.union(other)

    def __or__(self, other):
        return self.union(other)

    def __mul__(self, other):
        return self.intersection(other)

    def __and__(self, other):
        return self.intersection(other)


class DDManager(PointerWrapper):

    """
    Python class for NuSMV BDD managers.

    """

    @property
    def size(self):
        """
        The number of variables handled by this manager.

        """
        return nsdd.dd_get_size(self._ptr)

    @property
    def reorderings(self):
        """
        Returns the number of times reordering has occurred in this manager.

        """
        return nsdd.dd_get_reorderings(self._ptr)