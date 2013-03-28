"""
The :mod:`pynusmv.dd` module provides some BDD-related structures:

* :class:`BDD` represents a BDD.
* :class:`BDDList` represents a list of BDDs.
* :class:`Inputs` represents input variables values,
  i.e. a particular action of the model.
* :class:`State` represents a particular state of the model.
* :class:`DDManager` represents a NuSMV DD manager.

"""


__all__ = ['BDD', 'BDDList', 'Inputs', 'DDManager', 'State']


from .nusmv.dd import dd as nsdd
from .nusmv.node import node as nsnode
from .nusmv.compile.symb_table import symb_table as nssymb_table
from .nusmv.enc.bdd import bdd as nsbddEnc
from .nusmv.utils import utils as nsutils

from .utils import PointerWrapper
from .exception import MissingManagerError


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
    
    Any BDD operation raises a :exc:`MissingManagerError <pynusmv.exception.MissingManagerError>` whenever the manager
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
        super().__init__(ptr, freeit)
        self._manager = dd_manager
    
        
    def _free(self):
        if self._freeit and self._ptr is not None:
            nsdd.bdd_free(self._manager._ptr, self._ptr)
            self._freeit = False
            

    def equal(self, other):
        """
        Determine whether this BDD is equal to `other` or not.
        
        :param other: the BDD to compare
        :type other: :class:`BDD`
        """
        if nsdd.bdd_equal(self._ptr, other._ptr):
            return True
        else:
            return False
        
        
    # ==========================================================================
    # ===== BDD operations =====================================================
    # ==========================================================================
    
    def dup(self):
        """
        Return a copy of this BDD.
        
        """
        # Call to bdd_ptr bdd_dup (bdd_ptr);
        
        return BDD(nsdd.bdd_dup(self._ptr), self._manager, freeit = True)


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
        
        
    def nott(self):
        """
        Compute the complement of this BDD.
        
        """
        # Call to bdd_ptr bdd_not (DdManager *, bdd_ptr);
        
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_not(self._manager._ptr, self._ptr), self._manager,
                   freeit = True)
        
    
    def andd(self, other):
        """
        Compute the conjunction of this BDD and `other`.
        
        :param other: the other BDD
        :type other: :class:`BDD`
        
        """
        # Call to bdd_ptr bdd_and (DdManager *, bdd_ptr, bdd_ptr);
        
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_and(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        

    def orr(self, other):
        """
        Compute the conjunction of this BDD and `other`.
        
        :param other: the other BDD
        :type other: :class:`BDD`
        
        """
        # Call to bdd_ptr bdd_or (DdManager *, bdd_ptr, bdd_ptr);
        
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_or(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        

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
                   self._manager, freeit = True)
        
        
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
                   self._manager, freeit = True)
        

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
                   self._manager, freeit = True)
        
        
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
                   self._manager, freeit = True)
        
        
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
                   self._manager, freeit = True)
        


    # ==========================================================================
    # ===== Built-in BDD operations ============================================
    # ==========================================================================
    
    
    def __lt__(self, other):
        return (self <= other) and not (self == other)
        
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
        return self.orr(other)
        
    def __or__(self, other):
        return self.orr(other)
    
    
    def __mul__(self, other):
        return self.andd(other)

    def __and__(self, other):
        return self.andd(other)    
    
    
    def __sub__(self, other):
        return self & ~(other)
    
    
    
    def __xor__(self, other):
        return self.xor(other)
    
    
    
    def __neg__(self):
        return self.nott()
    
    def __invert__(self):
        return self.nott()
    
    
    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
    @staticmethod
    def true(manager):
        """
        Return the TRUE BDD.
        
        :param manager: the manager of the returned BDD
        :type manager: :class:`DDManager`
        
        """
        # Call to bdd_ptr bdd_true (DdManager *);
        
        return BDD(nsdd.bdd_true(manager._ptr), manager, freeit = True)
        
        
    @staticmethod
    def false(manager):
        """
        Return the FALSE BDD.
        
        :param manager: the manager of the returned BDD
        :type manager: :class:`DDManager`
        
        """
        # Call to bdd_ptr bdd_false (DdManager *);
        
        return BDD(nsdd.bdd_false(manager._ptr), manager, freeit = True)
    

class BDDList(PointerWrapper):
    """
    A BDD list stored as NuSMV nodes.
    
    The BDDList class implements a NuSMV nodes-based BDD list and can be used as 
    any Python list.
    
    """
    # BDDLists are freed when destroyed, as well as the content.
    # When getting elements or tuple from a BDDList, copies of BDDs are made
    # and returned.
    
    
    def __init__(self, ptr, ddmanager = None, freeit = True):
        super().__init__(ptr, freeit)
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
        l = 0
        while ptr:
            l += 1
            ptr = nsnode.cdr(ptr)
        return l
        
    
    def __getitem__(self, val):
        """
        Return the BDD stored at val.
        
        :param val: the index requested OR a slice.
        
        .. note:: cannot access elements with negative indices.
        """
        if type(val) is int:
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
            bdd_ptr =  nsnode.node2bdd(nsnode.car(ptr))
            if bdd_ptr is not None:
                return BDD(nsdd.bdd_dup(bdd_ptr), self._manager,
                           freeit = True)
            else:
                return None
        
        elif type(val) is slice:
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
                yield BDD(nsdd.bdd_dup(bdd_ptr), self._manager, freeit = True)
            else:
                yield None
            ptr = nsnode.cdr(ptr)
            
            
    def to_tuple(self):
        """
        Return a tuple containing all BDDs of self.       
        The returned BDDs are copies of the ones of self.
        """
        l = []
        for elem in self:
            l.append(elem)
        return tuple(l)
            
            
    # ==========================================================================
    # ===== Class methods ======================================================
    # ==========================================================================                                        
    
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
        n = None
        manager = None
        for elem in bddtuple:
            if elem:
                e = nsnode.bdd2node(nsdd.bdd_dup(elem._ptr))
                if manager is None:
                    manager = elem._manager
            else:
                e = elem
            n = nsnode.cons(e, n)
        return BDDList(n, manager, freeit = True)
        

class Inputs(BDD):
    """
    Python class for inputs structure.
    
    An Inputs is a :class:`BDD` representing a single valuation of the inputs
    variables of the model, i.e. an action of the model.
    
    """
    
    def __init__(self, ptr, fsm, freeit = True):
        super().__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    
    def get_str_values(self):
        """
        Return a dictionary of the (variable, value) pairs of these Inputs.
        
        :rtype: a dictionary of pairs of strings.
        
        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for inputs
        layers = nssymb_table.SymbTable_get_class_layer_names(table._ptr, None)
        symbols = nssymb_table.SymbTable_get_layers_i_symbols(table._ptr, layers)

        # Get assign symbols (BddEnc)
        assignList = nsbddEnc.BddEnc_assign_symbols(enc._ptr,self._ptr,
                                                  symbols, 0, None)

        values = {}
        # Traverse the symbols to print variables of the state
        asList_ptr = assignList
        while asList_ptr:
            assignment = nsnode.car(asList_ptr)
            var = nsnode.car(assignment)
            val = nsnode.cdr(assignment)
            values[nsnode.sprint_node(var)] = nsnode.sprint_node(val)
            asList_ptr = nsnode.cdr(asList_ptr)
            
        nsnode.free_list(assignList)
        
        nsutils.NodeList_destroy(symbols)
            
        return values
        

    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
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


class DDManager(PointerWrapper):
    """
    Python class for NuSMV BDD managers.
    
    """
    pass
    
    
class State(BDD):
    """
    Python class for State structure.
    
    A State is a :class:`BDD` representing a single state of the model.
    
    """
    
    def __init__(self, ptr, fsm, freeit = True):
        super().__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    
    def get_str_values(self):
        """
        Return a dictionary of the (variable, value) pairs of this State.
        
        :rtype: a dictionary of pairs of strings.
        
        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for states
        layers = nssymb_table.SymbTable_get_class_layer_names(table._ptr, None)
        symbols = nssymb_table.SymbTable_get_layers_sf_symbols(table._ptr, layers)
        
        # Get assign symbols (BddEnc)
        assignList = nsbddEnc.BddEnc_assign_symbols(enc._ptr,self._ptr,
                                                  symbols, 0, None)

        values = {}
        # Traverse the symbols to print variables of the state
        asList_ptr = assignList
        while asList_ptr:
            assignment = nsnode.car(asList_ptr)
            var = nsnode.car(assignment)
            val = nsnode.cdr(assignment)
            values[nsnode.sprint_node(var)] = nsnode.sprint_node(val)
            asList_ptr = nsnode.cdr(asList_ptr)
            
        nsnode.free_list(assignList)
        
        nsutils.NodeList_destroy(symbols)
            
        return values
        
        
    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
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


class StateInputs(BDD):
    """
    Python class for State and Inputs structure.
    
    A StateInputs is a :class:`BDD` representing a single state/inputs pair
    of the model.
    
    """
    
    def __init__(self, ptr, fsm, freeit = True):
        super().__init__(ptr, fsm.bddEnc.DDmanager, freeit)
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
        symbols = nssymb_table.SymbTable_get_layers_sf_symbols(table._ptr, layers)
        isymbols = nssymb_table.SymbTable_get_layers_i_symbols(table._ptr, layers)
        nsutils.NodeList_concat(symbols, isymbols)
        nsutils.NodeList_destroy(isymbols)
        
        # Get assign symbols (BddEnc)
        assignList = nsbddEnc.BddEnc_assign_symbols(enc._ptr,self._ptr,
                                                  symbols, 0, None)

        values = {}
        # Traverse the symbols to print variables of the state
        asList_ptr = assignList
        while asList_ptr:
            assignment = nsnode.car(asList_ptr)
            var = nsnode.car(assignment)
            val = nsnode.cdr(assignment)
            values[nsnode.sprint_node(var)] = nsnode.sprint_node(val)
            asList_ptr = nsnode.cdr(asList_ptr)
            
        nsnode.free_list(assignList)
        
        nsutils.NodeList_destroy(symbols)
            
        return values