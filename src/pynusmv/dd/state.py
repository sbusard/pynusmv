from ..nusmv.compile.symb_table import symb_table
from ..nusmv.enc.bdd import bdd as bddEnc

from .bdd import BDD
from ..node.listnode import ListNode
    

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
        layers = symb_table.SymbTable_get_class_layer_names(table, None)
        symbols = symb_table.SymbTable_get_layers_sf_symbols(table, layers)

        # Get assign symbols (BddEnc)
        assignList = ListNode(bddEnc.BddEnc_assign_symbols(enc._ptr,
                              self._ptr, symbols, 0, None))

        values = {}
        # Traverse the symbols to print variables of the state
        for assignment in assignList:
            var = assignment.car
            val = assignment.cdr
            values[str(var)] = str(val)
            
        return values