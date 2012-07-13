from ..nusmv.node import node as nsnode
from ..nusmv.dd import dd as nsdd
from ..utils.pointerwrapper import PointerWrapper

class Node(PointerWrapper):
    """
    Python class for node structure.
    
    The Node class contains a pointer to a node in NuSMV and provides a set
    of operations on this node.
    
    Nodes created with find_node do not have to be freed.
    Nodes created with new_node do have to be freed.
    We disallow nodes with mixed freeit/not freeit flags.
    """
    
    def __del__(self):
        if self._freeit:
            nsnode.free_node(self._ptr)
            
        
    @property
    def type(self):
        """The type of this node."""
        return self._ptr.type
        
    @property
    def car(self):
        """The left Node-typed child of this node."""
        left = nsnode.car(self._ptr)
        if left:
            return Node(left, freeit = self._freeit)
        else:
            return None
        
    @property
    def cdr(self):
        """The right Node-typed child of this node."""
        right = nsnode.cdr(self._ptr)
        if right:
            return Node(right, freeit = self._freeit)
        else:
            return None
            
            
    def to_bdd(self, manager = None):
        """
        Cast this node to a BDD, with manager as DD manager.
        
        The returned BDD is copied, such that it is the responsibility of
        the creator of the node to free its content.
        """
        
        from ..dd.bdd import BDD
        
        return BDD(nsdd.bdd_dup(nsnode.node2bdd(self._ptr)), manager,
                   freeit = True)
        
        
    def to_state(self, fsm):
        """
        Cast this node to a State, with fsm as FSM.
        
        The returned BDD is copied, such that it is the responsibility of
        the creator of the node to free its content.
        """
        
        from ..dd.state import State
        
        return State(nsdd.bdd_dup(nsnode.node2bdd(self._ptr)), fsm,
                     freeit = True)
        
        
    def to_inputs(self, fsm):
        """
        Cast this node to Inputs, with fsm as FSM.
        
        The returned BDD is copied, such that it is the responsibility of
        the creator of the node to free its content.
        """
        
        from ..dd.inputs import Inputs
        
        return Inputs(nsdd.bdd_dup(nsnode.node2bdd(self._ptr)), fsm,
                      freeit = True)
        
    
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
        
        Throw a MixingFreeingError if left or right do have to be freeed.
        """
        if (left and left._freeit) or (right and right._freeit):
            raise MixingFreeingError()
        return Node(nsnode.find_node(nodetype,
                                     left and left._ptr or None,
                                     right and right._ptr or None),
                    freeit = False)
    
    
    def new_node(nodetype, left=None, right=None):
        """
        Create a node but do not store it in the hash table of NuSMV.
        
        nodetype -- an int for the type of the new node.
        left -- a Node being the left child of the new node
                or None if it has no left child.
        right -- a Node being the right child of the new node
                 or None if it has no right child.
        
        Returns the Node-typed new node.

        Throw a MixingFreeingError if left or right do not have to be freeed.
        """
        if (left and not left._freeit) or (right and not right._freeit):
            raise MixingFreeingError()
        return Node(nsnode.create_node(nodetype,
                                    left and left._ptr or None,
                                    right and right._ptr or None),
                    freeit = True)
                    
                    
class MixingFreeingError(Exception):
    """Cannot create a node from mixed freeing flag nodes."""