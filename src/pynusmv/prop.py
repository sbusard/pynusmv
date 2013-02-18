__all__ = ['propTypes', 'Prop', 'PropDb','Spec', 'true', 'false', 'nott',
           'andd', 'orr', 'imply', 'iff', 'ex', 'eg', 'ef', 'eu', 'ew',
           'ax', 'ag', 'af', 'au', 'aw', 'atom']

from .nusmv.prop import prop as nsprop

from .fsm import BddFsm
from .utils import PointerWrapper

from .nusmv.parser import parser as nsparser
from .nusmv.node import node as nsnode
from .nusmv.compile.type_checking import type_checking as nstype_checking
from .nusmv.compile.symb_table import symb_table as nssymb_table
     
from . import parser

from .utils.exception import NuSMVParserError, NuSMVTypeCheckingError

propTypes = {
             'NoType' :      nsprop.Prop_NoType,
             'CTL' :         nsprop.Prop_Ctl,
             'LTL' :         nsprop.Prop_Ltl,
             'PSL' :         nsprop.Prop_Psl,
             'Invariant' :   nsprop.Prop_Invar,
             'Compute' :     nsprop.Prop_Compute,
             'Comparison' :  nsprop.Prop_CompId
            }


class Prop(PointerWrapper):
    """
    Python class for prop structure.
    
    The Prop class contains a pointer to a prop in NuSMV and provides a set
    of operations on this prop.
    
    Prop do not have to be freed since they come from PropDb.
    """
        
    @property
    def type(self):
        """
        The type of this prop.
        
        To compare with pynusmv.nusmv.prop.prop.Prop_X for type X.
        Types are NoType, Ctl, Ltl, Psl, Invar, Compute, CompId
        """
        return nsprop.Prop_get_type(self._ptr)
        
    @property
    def name(self):
        """The name of this prop, as a string."""
        return nsprop.Prop_get_name_as_string(self._ptr)
        
    @property
    def expr(self):
        """The expression of this prop, as a Spec."""
        return Spec(nsprop.Prop_get_expr(self._ptr))
        
    @property
    def exprcore(self):
        """The core expression of this prop, as a Spec."""
        return Spec(nsprop.Prop_get_expr_core(self._ptr))
        
    @property
    def bddFsm(self):
        """The fsm of this prop, into BddFsm format."""
        return BddFsm(nsprop.Prop_get_bdd_fsm(self._ptr))


class PropDb(PointerWrapper):
    """
    Python class for PropDb structure.
    
    The PropDb class contains a pointer to a propDb in NuSMV and provides a set
    of operations on this prop database.
    
    PropDb do not have to be freed.
    """
    
    
    @property
    def master(self):
        """The master property of this database."""
        return Prop(nsprop.PropDb_get_master(self._ptr))
    
    
    def get_prop_at_index(self, index):
        """Return the prop stored at index."""
        return Prop(nsprop.PropDb_get_prop_at_index(self._ptr, index))
    
    
    def get_size(self):
        """Return the size of this database."""
        return nsprop.PropDb_get_size(self._ptr)
        
    
    def __len__(self):
        """Return the length of this propDb."""
        return self.get_size()
        
    
    def __getitem__(self, index):
        """
        Return the indexth property.
        
        Throw a IndexError if index < -len(self) or index >= len(self) 
        """
        if index < -len(self) or index >= len(self):
            raise IndexError("PropDb index out of range")
        if index < 0:
            index = index + len(self)
        return self.get_prop_at_index(index)
        
    
    def __iter__(self):
        """Return an iterator on this propDb."""
        for i in range(len(self)):
            yield self[i]
            

class Spec(PointerWrapper):
    """
    A specification stored as NuSMV nodes.
    
    The Spec class implements a NuSMV nodes-based specification.
    
    No check is made to insure that the node is effectively a specification,
    i.e. the stored pointer is not checked against spec types.
    
    Specs do not have to be freed.
    """
    
    def __init__(self, ptr, freeit = False):
        super().__init__(ptr, freeit = freeit)
        
        
    @property
    def type(self):
        """The type of this spec node."""
        return self._ptr.type
    
    @property
    def car(self):
        """The left Spec-typed child of this node."""
        left = nsnode.car(self._ptr)
        if left:
            return Spec(left, freeit = self._freeit)
        else:
            return None
        
    @property
    def cdr(self):
        """The right Spec-typed child of this node."""
        right = nsnode.cdr(self._ptr)
        if right:
            return Spec(right, freeit = self._freeit)
        else:
            return None
            
    def __str__(self):
        return nsnode.sprint_node(self._ptr)

    def __or__(self, other):
        if other is None:
            raise ValueError()
        return Spec(nsnode.find_node(nsparser.OR, self._ptr, other._ptr))
        
    def __and__(self, other):
        if other is None:
            raise ValueError()
        return Spec(nsnode.find_node(nsparser.AND, self._ptr, other._ptr))
        
    def __invert__(self):
        return Spec(nsnode.find_node(nsparser.NOT, self._ptr, None))
        

def true():
    """Return a new Spec corresponding to TRUE"""
    return Spec(nsnode.find_node(nsparser.TRUEEXP, None, None))
    

def false():
    """Return a new Spec corresponding to FALSE"""
    return Spec(nsnode.find_node(nsparser.FALSEEXP, None, None))
    
    
def nott(spec):
    """Return a new Spec corresponding to NOT spec"""
    if spec is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.NOT, spec._ptr, None))
    

def andd(left, right):
    """Return a new Spec corresponding to left AND right"""
    if left is None or right is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.AND, left._ptr, right._ptr))
    

def orr(left, right):
    """Return a new Spec corresponding to left OR right"""
    if left is None or right is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.OR, left._ptr, right._ptr))
       

def imply(left, right):
    """Return a new Spec corresponding to (left IMPLIES right)"""
    if left is None or right is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.IMPLIES, left._ptr, right._ptr))
    

def iff(left, right):
    """Return a new Spec corresponding to (left IFF right)"""
    if left is None or right is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.IFF, left._ptr, right._ptr))
    
    
def ex(spec):
    """Return a new Spec corresponding to EX spec"""
    if spec is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.EX, spec._ptr, None))
    

def eg(spec):
    """Return a new Spec corresponding to EG spec"""
    if spec is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.EG, spec._ptr, None))
    
    
def ef(spec):
    """Return a new Spec corresponding to EF spec"""
    if spec is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.EF, spec._ptr, None))
    
    
def eu(left, right):
    """Return a new Spec corresponding to EU[left U right]"""
    if left is None or right is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.EU, left._ptr, right._ptr))
    
    
def ew(left, right):
    """Return a new Spec corresponding to EW[left U right]"""
    if left is None or right is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.EW, left._ptr, right._ptr))
    
    
def ax(spec):
    """Return a new Spec corresponding to AX spec"""
    if spec is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.AX, spec._ptr, None))
    

def ag(spec):
    """Return a new Spec corresponding to AG spec"""
    if spec is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.AG, spec._ptr, None))
    
    
def af(spec):
    """Return a new Spec corresponding to AF spec"""
    if spec is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.AF, spec._ptr, None))
    
    
def au(left, right):
    """Return a new Spec corresponding to AU[left U right]"""
    if left is None or right is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.AU, left._ptr, right._ptr))
    
    
def aw(left, right):
    """Return a new Spec corresponding to AW[left U right]"""
    if left is None or right is None:
        raise ValueError()
    return Spec(nsnode.find_node(nsparser.AW, left._ptr, right._ptr))


def atom(strrep):
    """
    Return a new Spec corresponding to the given atom.
    
    Parse strrep and provide a new Spec representing this atom.
    The parsed spec is type checked on the current model. A model needs to be
    read and with variables encoded to be able to type check the atomic
    proposition.
    """
    
    from . import glob
    
    # Parsing
    node = parser.parse_simple_expression(strrep)
    
    # Type checking
    # TODO Prevent printing a message on stderr
    symb_table = glob.bdd_encoding().symbTable
    # TODO Type check only if symb_table is not None? With a Warning?
    type_checker = nssymb_table.SymbTable_get_type_checker(symb_table._ptr)
    expr_type = nstype_checking.TypeChecker_get_expression_type(
                                                       type_checker, node, None)
    if not nssymb_table.SymbType_is_boolean(expr_type):
        raise NuSMVTypeCheckingError(strrep + " is wrongly typed.")   
    
    return Spec(node)