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
        
    def __eq__(self, other):
        if isinstance(other, Node):
            return self.__ptr == other.__ptr
        else:
            return False
            
    def eq(self, other):
        return self.__ptr == other.__ptr
        
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
        return Node(nsnode.new_node(nodetype,
                                    left and left.__ptr or None,
                                    right and right.__ptr or None))