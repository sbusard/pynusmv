__all__ = ['BDD', 'BDDList', 'Inputs', 'DDManager', 'State']

from .nusmv.dd import dd as nsdd
from .utils import PointerWrapper
from .exception import MissingManagerError
     
from .nusmv.node import node as nsnode
from .nusmv.dd import dd as nsdd
     
from .nusmv.compile.symb_table import symb_table
from .nusmv.enc.bdd import bdd as bddEnc
from .nusmv.utils import utils as nsutils

class BDD(PointerWrapper):
    """
    Python class for BDD structure.
    
    The BDD class contains a pointer to a BDD in NuSMV (a bdd_ptr)
    and provides a set of operations on this BDD.
    Thanks to operator overloading, it is possible to write compact expressions
    on BDDs. Available operations:
        a + b, a | b compute bdd_or(a,b)
        a * b, a & b compute bdd_and(a,b)
        ~a, -a, compute bdd_not(a)
        a - b computes (a and not b)
        a ^ b computes bdd_xor(a,b)
        a <= b computes bdd_leq(a,b)
        a < b, a > b, a >= b derive from a <= b
        a == b compares pointers.
    
    A BDD operation raises a MissingManagerError whenever the manager
    of the BDD is None and a manager is needed to perform the operation.
    
    All BDDs are freed by default. Every operation on BDDs that return a new
    BDD uses bdd_dup to ensure that the new BDD wraps a pointer to free.
    """
    
    def __init__(self, ptr, dd_manager=None, freeit = True):
        """
        Create a new BDD with ptr.
        
        ptr -- the pointer to the NuSMV BDD.
        dd_manager -- the DD manager for this BDD.
        """
        super().__init__(ptr, freeit)
        self._manager = dd_manager
    
        
    def _free(self):
        if self._freeit and self._ptr is not None:
            nsdd.bdd_free(self._manager._ptr, self._ptr)
            self._freeit = False
            

    def equal(self, other):
        """Return whether self and other are the same BDD."""
        if nsdd.bdd_equal(self._ptr, other._ptr):
            return True
        else:
            return False
        
        
    # ==========================================================================
    # ===== BDD operations =====================================================
    # ==========================================================================
    
    def dup(self):
        """
        Creates a copy of a BDD node.
    
        The reference count is increased by one unit.
    
        bdd_ptr bdd_dup (bdd_ptr);
        """
        return BDD(nsdd.bdd_dup(self._ptr), self._manager, freeit = True)


    def is_true(self):
        """
        Check if the BDD is true.
        
        int bdd_is_true (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if nsdd.bdd_is_true(self._manager._ptr, self._ptr):
            return True
        else:
            return False
        
    
    def is_false(self):
        """
        Check if the BDD is false.
        
        int bdd_is_false (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if nsdd.bdd_is_false(self._manager._ptr, self._ptr):
            return True
        else:
            return False
        
        
    def isnot_true(self):
        """
        Check if the BDD is not true.
        
        int bdd_isnot_true (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if nsdd.bdd_isnot_true(self._manager._ptr, self._ptr):
            return True
        else:
            return False
        
    
    def isnot_false(self):
        """
        Check if the BDD is not false.
        
        int bdd_isnot_false (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if nsdd.bdd_isnot_false(self._manager._ptr, self._ptr):
            return True
        else:
            return False
    
    
    def entailed(self, other):
        """
        Determines whether self is less than or equal to other.
        
        int bdd_entailed (DdManager * dd, bdd_ptr f, bdd_ptr g);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if nsdd.bdd_entailed(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False
        
        
    def intersected(self, other):
        """
        Determines whether an intersection between self and other is not empty.
        
        int bdd_intersected (DdManager * dd, bdd_ptr f, bdd_ptr g);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if nsdd.bdd_intersected(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False


    def leq(self, other):
        """
        Determines whether self is less than or equal to other.
        
        int bdd_leq (DdManager * dd, bdd_ptr f, bdd_ptr g);
        """
        if self._manager is None:
            raise MissingManagerError()
            
        if nsdd.bdd_leq(self._manager._ptr, self._ptr, other._ptr):
            return True
        else:
            return False
        
        
    def nott(self):
        """
        Applies NOT to the corresponding discriminant of f.
        
        bdd_ptr bdd_not (DdManager *, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_not(self._manager._ptr, self._ptr), self._manager,
                   freeit = True)
        
    
    def andd(self, other):
        """
        Applies AND to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_and (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_and(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        

    def orr(self, other):
        """
        Applies OR to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_or (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_or(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        

    def xor(self, other):
        """
        Applies XOR to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_xor (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_xor(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        
        
    def iff(self, other):
        """
        Applies IFF to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_iff (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_iff(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        

    def imply(self, other):
        """
        Applies IMPLY to the corresponding discriminants of self and other.
        
        bdd_ptr bdd_imply (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_imply(self._manager._ptr, self._ptr, other._ptr),
                   self._manager, freeit = True)
        
        
    def forsome(self, cube):
        """
        Existentially abstracts all the variables in cube from self.
        
        bdd_ptr bdd_forsome (DdManager *, bdd_ptr, bdd_ptr);
        """
        if self._manager is None:
            raise MissingManagerError()
        return BDD(nsdd.bdd_forsome(self._manager._ptr, self._ptr, cube._ptr),
                   self._manager, freeit = True)
        
        
    def forall(self, cube):
        """
        Universally abstracts all the variables in cube from self.
        
        bdd_ptr bdd_forall (DdManager *, bdd_ptr, bdd_ptr);
        """
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
    
    def true(manager):
        """
        Return the TRUE BDD.
        
        manager -- a DDManager.
        
        bdd_ptr bdd_true (DdManager *);
        """
        return BDD(nsdd.bdd_true(manager._ptr), manager, freeit = True)
        
        
    def false(manager):
        """
        Return the FALSE BDD.
        
        bdd_ptr bdd_false (DdManager *);
        """
        return BDD(nsdd.bdd_false(manager._ptr), manager, freeit = True)
    

class BDDList(PointerWrapper):
    """
    A BDD list stored as NuSMV nodes.
    
    The BDDList class implements a NuSMV nodes-based BDD list.
    
    BDDLists are freed when destroyed, as well as the content.
    When getting elements or tuple from a BDDList, copies of BDDs are made
    and returned.
    """
    
    
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
        
        val -- the index requested OR a slice.
        
        Note: cannot access elements with negative indices.
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
    
    def from_tuple(l):
        """
        Create a node-based list from the Python tuple l.
        
        l -- a Python tuple of BDDs.
        
        Return a BDDList n representing the given tuple, using NuSMV nodes.
        The nodes are created using new_node, so no node is stored
        in the NuSMV hash table.
        
        All BDDs are assumed from the same DD manager;
        the created list contains the DD manager of the first non-None BDD.
        If all elements of l are None,
        the manager of the created BDDList is None.
        
        All BDDs are duplicated before stored.
        """
        
        # Reverse tuple before, because we build the list reversely.
        l = l[::-1]
        n = None
        manager = None
        for elem in l:
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
    
    The Inputs class contains a pointer to a BDD in NuSMV (a bdd_ptr)
    representing inputs of an FSM.
    """
    
    def __init__(self, ptr, fsm, freeit = True):
        super().__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    
    def get_str_values(self):
        """
        Return a dictionary of the (variable, value) pairs of these Inputs.
        
        The returned values are strings.
        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for inputs
        layers = symb_table.SymbTable_get_class_layer_names(table._ptr, None)
        symbols = symb_table.SymbTable_get_layers_i_symbols(table._ptr, layers)

        # Get assign symbols (BddEnc)
        assignList = bddEnc.BddEnc_assign_symbols(enc._ptr,self._ptr,
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
    
    def from_bdd(bdd, fsm):
        """Return a new Inputs of fsm from bdd."""
        return Inputs(nsdd.bdd_dup(bdd._ptr), fsm)


class DDManager(PointerWrapper):
    """
    Python class for BDD Manager.
    
    Note: dd managers are never freed. NuSMV takes care of it.
    """
    pass
    
    
class State(BDD):
    """
    Python class for State structure.
    
    The State class contains a pointer to a BDD in NuSMV (a bdd_ptr)
    representing a state of an FSM.
    """
    
    def __init__(self, ptr, fsm, freeit = True):
        super().__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    
    def get_str_values(self):
        """
        Return a dictionary of the (variable, value) pairs of this State.
        
        The returned values are strings.
        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for states
        layers = symb_table.SymbTable_get_class_layer_names(table._ptr, None)
        symbols = symb_table.SymbTable_get_layers_sf_symbols(table._ptr, layers)
        
        # Get assign symbols (BddEnc)
        assignList = bddEnc.BddEnc_assign_symbols(enc._ptr,self._ptr,
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
    
    def from_bdd(bdd, fsm):
        """Return a new State of fsm from bdd."""
        return State(nsdd.bdd_dup(bdd._ptr), fsm)
