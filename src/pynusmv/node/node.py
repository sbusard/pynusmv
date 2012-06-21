from ..nusmv.node import node as nsnode

class Node:
	"""
	Python class for node structure.
	
	The Node class contains a pointer to a node in NuSMV and provides a set
	of operations on this node.
	"""
	
	def __init__(ptr):
		"""
		Create a new node with ptr.
		
		ptr -- the pointer to the NuSMV node.
		"""
		self.__ptr = ptr	
		
	@property
	def type(self):
	    """The type of this node."""
		nsnode.node_get_type(self.__ptr)
		
	@property
	def car(self):
	    """The left Node-typed child of this node."""
	    Node(nsnode.car(self.__ptr))
	    
	@property
	def cdr(self):
	    """The right Node-typed child of this node."""
	    Node(nsnode.cdr(self.__ptr))
	    
	    
	    
    @classmethod
    def find_node(type, left, right):
        """
        Create a node and store it in the hash table of NuSMV.
        
        type -- an int for the type of the new node.
        left -- a Node being the left child of the new node.
        right -- a Node being the right child of the new node.
        
        Returns the new node.
        """
        return Node(nsnode.find_node(type, left.__ptr, right.__ptr))
        
    @classmethod
    def new_node(type, left, right):
        """
        Create a node but do not store it in the hash table of NuSMV.
        
        type -- an int for the type of the new node.
        left -- a Node being the left child of the new node.
        right -- a Node being the right child of the new node.
        
        Returns the new node.
        """
        return Node(nsnode.new_node(type, left.__ptr, right.__ptr))