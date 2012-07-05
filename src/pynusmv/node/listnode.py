from ..nusmv.node import node as nsnode
from .node import Node

class ListNode(Node):
    """
    A list stored as NuSMV nodes.
    
    The ListNode class implements a NuSMV nodes-based list.
    """
    
    def __init__(self, ptr):
        """
        Create a new node with ptr.
        
        ptr -- the pointer to the NuSMV node.
        """
        super().__init__(ptr)
        
    
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
            return Node(nsnode.car(ptr))
        
        elif type(val) is slice:
            # TODO Implement slicing
            raise NotImplementedError("ListNode slice not implemented")
            
        else:
            raise IndexError("ListNode index wrong type")
        
        
    def __iter__(self):
        for i in range(len(self)):
            yield self[i]
        
    
    def __str__(self):
        """
        Return the string representation of this node.
        
        Call nsnode.sprint_node() to get the string representation.
        """
        return nsnode.sprint_node(self._ptr)
            
            
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
        return ListNode(n)