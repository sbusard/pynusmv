from ..nusmv.node import node as nsnode
from ..utils.pointerwrapper import PointerWrapper

class Node(PointerWrapper):
    """
    Python class for node structure.
    
    The Node class contains a pointer to a node in NuSMV and provides a set
    of operations on this node.
    """
        
        
    @property
    def type(self):
        """The type of this node."""
        return self._ptr.type
        
    @property
    def car(self):
        """The left Node-typed child of this node."""
        left = nsnode.car(self._ptr)
        if left:
            return Node(left)
        else:
            return None
        
    @property
    def cdr(self):
        """The right Node-typed child of this node."""
        right = nsnode.cdr(self._ptr)
        if right:
            return Node(right)
        else:
            return None
            
            
    def to_bdd(self, manager = None):
        """Cast this node to a BDD, with manager as DD manager."""
        
        from ..dd.bdd import BDD
        
        return BDD(nsnode.node2bdd(self._ptr), manager)
        
        
    def to_state(self, fsm):
        """Cast this node to a State, with fsm as FSM."""
        
        from ..dd.state import State
        
        return State(nsnode.node2bdd(self._ptr), fsm)
        
        
    def to_inputs(self, fsm):
        """Cast this node to Inputs, with fsm as FSM."""
        
        from ..dd.inputs import Inputs
        
        return Inputs(nsnode.node2bdd(self._ptr), fsm)
        
    
    def __str__(self):
        """
        Return the string representation of this node.
        
        Call nsnode.sprint_node() to get the string representation.
        """
        return nsnode.sprint_node(self._ptr)
            
            
            
    # ==========================================================================
    # ===== Static methods =====================================================
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
                                     left and left._ptr or None,
                                     right and right._ptr or None))
    
    
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
                                    left and left._ptr or None,
                                    right and right._ptr or None))