from ..nusmv.node import node as nsnode
from ..nusmv.dd import dd as nsdd

from ..utils import PointerWrapper
from .bdd import BDD

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