from ..nusmv.node import node as nsnode

class Node:
    """
    Python class for node structure.
    
    The Node class contains a pointer to a node in NuSMV and provides a set
    of operations on this node.
    """
    
    def __init__(self, ptr):
        """
        Create a new node with ptr.
        
        ptr -- the pointer to the NuSMV node.
        """
        self.__ptr = ptr
        
    
    @property
    def ptr(self):
        return self.__ptr
        
    @property
    def type(self):
        """The type of this node."""
        return self.__ptr.type
        
    @property
    def car(self):
        """The left Node-typed child of this node."""
        left = nsnode.car(self.__ptr)
        if left:
            return Node(left)
        else:
            return None
        
    @property
    def cdr(self):
        """The right Node-typed child of this node."""
        right = nsnode.cdr(self.__ptr)
        if right:
            return Node(right)
        else:
            return None
            
            
    def to_bdd(self, manager = None):
        """Cast this node to a BDD, with manager as DD manager."""
        
        from ..dd.dd import BDD
        
        return BDD(nsnode.node2bdd(self.__ptr), manager)
            
            
            
    # ==========================================================================
    # ===== Class methods ======================================================
    # ==========================================================================    
    
    def find_node(nodetype, left=None, right=None):
        """
        Create a node and store it in the hash table of NuSMV.
        
        nodetype -- an int for the type of the new node.
        left -- a Node being the left child of the new node
                or None if it has no left child.
        right -- a Node being the right child of the new node
                 or None if it has no right child.
        
        Returns the Node-typed new node.
        """
        return Node(nsnode.find_node(nodetype,
                                     left and left.__ptr or None,
                                     right and right.__ptr or None))
    
    
    def new_node(nodetype, left=None, right=None):
        """
        Create a node but do not store it in the hash table of NuSMV.
        
        nodetype -- an int for the type of the new node.
        left -- a Node being the left child of the new node
                or None if it has no left child.
        right -- a Node being the right child of the new node
                 or None if it has no right child.
        
        Returns the Node-typed new node.
        """
        return Node(nsnode.create_node(nodetype,
                                    left and left.__ptr or None,
                                    right and right.__ptr or None))
                                    
    
    def cons(left=None, right=None):
        """
        Create a new node of type CONS with left and right as car and cdr
        children.
        
        left -- a Node, the left child of the new node.
        right -- a Node, the right child of the new node.
        
        Return the Node corresponding to the newly created node.
        
        Note: the new node is not stored in the NuSMV node hash table.
        """
        return Node(nsnode.cons(left and left.ptr or None,
                                right and right.ptr or None))
                                    
    
    def node_from_list(l):
        """
        Create a LISP-like list from the Python-like list l.
        
        l -- a Python list.
        
        Return a Node n representing the given list,
        i.e. n.car is l[0], n.cdr.car is l[1], etc.
        The nodes are created using new_node, so no node is stored
        in the corresponding NuSMV hash table.
        """
        
        l = l[::-1]
        n = None
        for elem in l:
            n = Node.cons(elem, n)
        return n