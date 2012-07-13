from ..nusmv.node import node as nsnode
from .node import Node

class ListNode(Node):
    """
    A list stored as NuSMV nodes.
    
    The ListNode class implements a NuSMV nodes-based list.
    
    ListNodes are created with new_node (cons). They have to be freed.
    Content of the list is not freed. It is the responsibility of the user
    to free content.
    """
    
    def free(self):
        nsnode.free_list(self._ptr)
    
    def __del__(self):
        if self._freeit:
            nsnode.free_list(self._ptr)
    
    
    def __len__(self):
        ptr = self._ptr
        l = 0
        while ptr:
            l += 1
            ptr = nsnode.cdr(ptr)
        return l
        
    
    def __getitem__(self, val):
        """
        Return the Node stored at val.
        
        val -- the index requested OR a slice.
        
        Returned node is set to not be freed.
        """
        if type(val) is int:
            if val < 0:
                raise IndexError("ListNode index out of range")
            ptr = self._ptr
            while val > 0:
                if ptr is None:
                    raise IndexError("ListNode index out of range")
                val -= 1
                ptr = nsnode.cdr(ptr)
            if ptr is None:
                    raise IndexError("ListNode index out of range")
            return Node(nsnode.car(ptr), freeit = False)
        
        elif type(val) is slice:
            # TODO Implement slicing
            raise NotImplementedError("ListNode slice not implemented")
            
        else:
            raise IndexError("ListNode index wrong type")
        
        
    def __iter__(self):
        for i in range(len(self)):
            yield self[i]
            
            
    def dup(self):
        return ListNode(nsnode.copy_list(self._ptr), freeit = self._freeit)
        
    
    @property
    def cdr(self):
        """The right ListNode-typed child of this listnode."""
        right = nsnode.cdr(self._ptr)
        if right:
            return ListNode(right, freeit = self._freeit)
        else:
            return None
            
            
    # ==========================================================================
    # ===== Class methods ======================================================
    # ==========================================================================                                        
    
    def from_tuple(l):
        """
        Create a LISP-like list from the Python-like tuple l.
        
        l -- a Python tuple of Nodes.
        
        Return a ListNode n representing the given tuple, using NuSMV nodes.
        The nodes are created using new_node, so no node is stored
        in the corresponding NuSMV hash table.
        """
        
        l = l[::-1]
        n = None
        for elem in l:
            n = nsnode.cons(elem and elem._ptr or None, n)
        return ListNode(n, freeit = True)