from ..nusmv.compile.symb_table import symb_table
from ..nusmv.enc.bdd import bdd as bddEnc
from ..nusmv.dd import dd as nsdd
from ..nusmv.node import node as nsnode
from ..nusmv.utils import utils as nsutils

from .bdd import BDD
    

class State(BDD):
    """
    Python class for State structure.
    
    The State class contains a pointer to a BDD in NuSMV (a bdd_ptr)
    representing a state of an FSM.
    """
    
    def __init__(self, ptr, fsm, freeit = True):
        super().__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    
    def get_str_values(self):
        """
        Return a dictionary of the (variable, value) pairs of this State.
        
        The returned values are strings.
        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for states
        layers = symb_table.SymbTable_get_class_layer_names(table._ptr, None)
        symbols = symb_table.SymbTable_get_layers_sf_symbols(table._ptr, layers)
        
        # Get assign symbols (BddEnc)
        assignList = bddEnc.BddEnc_assign_symbols(enc._ptr,self._ptr,
                                                  symbols, 0, None)

        values = {}
        # Traverse the symbols to print variables of the state
        asList_ptr = assignList
        while asList_ptr:
            assignment = nsnode.car(asList_ptr)
            var = nsnode.car(assignment)
            val = nsnode.cdr(assignment)
            values[nsnode.sprint_node(var)] = nsnode.sprint_node(val)
            asList_ptr = nsnode.cdr(asList_ptr)
            
        nsnode.free_list(assignList)
        
        nsutils.NodeList_destroy(symbols)
            
        return values
        
        
    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
    def from_bdd(bdd, fsm):
        """Return a new State of fsm from bdd."""
        return State(nsdd.bdd_dup(bdd._ptr), fsm)
