from ..nusmv.parser import parser
from ..nusmv.node import node as nsnode

from .node import Node

class SpecNode(Node):
    """
    A specification stored as NuSMV nodes.
    
    The SpecNode class implements a NuSMV nodes-based specification.
    
    No check is made to insure that the node is effectively a specification,
    i.e. the stored pointer is not checked against spec types.
    
    SpecNodes are created using find_node. No need to free them.
    """
    
    @property
    def car(self):
        """The left SpecNode-typed child of this node."""
        left = nsnode.car(self._ptr)
        if left:
            return SpecNode(left, freeit = False)
        else:
            return None
        
    @property
    def cdr(self):
        """The right SpecNode-typed child of this node."""
        right = nsnode.cdr(self._ptr)
        if right:
            return SpecNode(right, freeit = False)
        else:
            return None

    def __or__(self, other):
        if other is None:
            raise ValueError()
        return SpecNode(nsnode.find_node(parser.OR, self._ptr, other._ptr),
                        freeit = False)
        
    def __and__(self, other):
        if other is None:
            raise ValueError()
        return SpecNode(nsnode.find_node(parser.AND, self._ptr, other._ptr),
                        freeit = False)
        
    def __invert__(self):
        return SpecNode(nsnode.find_node(parser.NOT, self._ptr, None),
                        freeit = False)
        

def true():
    """Return a new SpecNode corresponding to TRUE"""
    return SpecNode(nsnode.find_node(parser.TRUEEXP, None, None),
                    freeit = False)
    

def false():
    """Return a new SpecNode corresponding to FALSE"""
    return SpecNode(nsnode.find_node(parser.FALSEEXP, None, None),
                    freeit = False)
    
    
def nott(spec):
    """Return a new SpecNode corresponding to NOT spec"""
    if spec is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.NOT, spec._ptr, None),
                        freeit = False)
    

def andd(left, right):
    """Return a new SpecNode corresponding to left AND right"""
    if left is None or right is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.AND, left._ptr, right._ptr),
                    freeit = False)
    

def orr(left, right):
    """Return a new SpecNode corresponding to left OR right"""
    if left is None or right is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.OR, left._ptr, right._ptr),
                    freeit = False)
       

def imply(left, right):
    """Return a new SpecNode corresponding to (left IMPLIES right)"""
    if left is None or right is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.IMPLIES, left._ptr, right._ptr),
                    freeit = False)
    

def iff(left, right):
    """Return a new SpecNode corresponding to (left IFF right)"""
    if left is None or right is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.IFF, left._ptr, right._ptr),
                    freeit = False)
    
    
def ex(spec):
    """Return a new SpecNode corresponding to EX spec"""
    if spec is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.EX, spec._ptr, None),
                    freeit = False)
    

def eg(spec):
    """Return a new SpecNode corresponding to EG spec"""
    if spec is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.EG, spec._ptr, None),
                    freeit = False)
    
    
def ef(spec):
    """Return a new SpecNode corresponding to EF spec"""
    if spec is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.EF, spec._ptr, None),
                    freeit = False)
    
    
def eu(left, right):
    """Return a new SpecNode corresponding to EU[left U right]"""
    if left is None or right is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.EU, left._ptr, right._ptr),
                    freeit = False)
    
    
def ew(left, right):
    """Return a new SpecNode corresponding to EW[left U right]"""
    if left is None or right is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.EW, left._ptr, right._ptr),
                    freeit = False)
    
    
def ax(spec):
    """Return a new SpecNode corresponding to AX spec"""
    if spec is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.AX, spec._ptr, None),
                    freeit = False)
    

def ag(spec):
    """Return a new SpecNode corresponding to AG spec"""
    if spec is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.AG, spec._ptr, None),
                    freeit = False)
    
    
def af(spec):
    """Return a new SpecNode corresponding to AF spec"""
    if spec is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.AF, spec._ptr, None),
                    freeit = False)
    
    
def au(left, right):
    """Return a new SpecNode corresponding to AU[left U right]"""
    if left is None or right is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.AU, left._ptr, right._ptr),
                    freeit = False)
    
    
def aw(left, right):
    """Return a new SpecNode corresponding to AW[left U right]"""
    if left is None or right is None:
        raise ValueError()
    return SpecNode(nsnode.find_node(parser.AW, left._ptr, right._ptr),
                    freeit = False)


def atom(strrep):
    """
    Return a new SpecNode corresponding to the given atom.
    
    Parse strrep and provide a new SpecNode representing this atom.
    
    Throw a NuSMVParserError if strrep is not a valid atomic proposition.
    """
    # FIXME NuSMV abruptly exits when strrep is not an atomic proposition
    node, err = parser.ReadSimpExprFromString(strrep)
    if err:
        raise NuSMVParserError(strrep + " is not a valid atomic proposition.")
    return SpecNode(nsnode.car(node), freeit = False)
    
    

class NuSMVParserError(Exception):
    """An error occured while parsing a string with NUSMV."""