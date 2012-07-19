from ..nusmv.compile.symb_table import symb_table
from ..nusmv.enc.bdd import bdd as bddEnc
from ..nusmv.dd import dd as nsdd
from ..nusmv.node import node as nsnode
from ..nusmv.utils import utils as nsutils

from .bdd import BDD
    

class Inputs(BDD):
    """
    Python class for inputs and combinatorial structure.
    
    The Inputs class contains a pointer to a BDD in NuSMV (a bdd_ptr)
    representing inputs of an FSM.
    """
    
    def __init__(self, ptr, fsm, freeit = True):
        super().__init__(ptr, fsm.bddEnc.DDmanager, freeit)
        self._fsm = fsm

    
    def get_str_values(self):
        """
        Return a dictionary of the (variable, value) pairs of these Inputs.
        
        The returned values are strings.
        """
        enc = self._fsm.bddEnc
        # Get symb table from enc (BaseEnc)
        table = enc.symbTable

        # Get symbols (SymbTable) for inputs
        layers = symb_table.SymbTable_get_class_layer_names(table, None)
        symbols = symb_table.SymbTable_get_layers_i_symbols(table, layers)

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
        """Return a new Inputs of fsm from bdd."""
        return Inputs(nsdd.bdd_dup(bdd._ptr), fsm)
