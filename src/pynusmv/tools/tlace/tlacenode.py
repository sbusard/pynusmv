class Tlacenode:
    """
    A Tlacenode is a TLACE node.
    
    It contains a state, a list of atomic propositions, a list of TLACE branches
    and a list of universal formulas.
    """
    
    def __init__(self, state, atomics=None, branches=None, universals=None):
        """
        Create a new TLACE node.
        
        state -- a BDD representing the state of the TLACE node
        atomics -- a list of atomic propositions represented by Nodes
        branches -- a list of TLACE branches represented by Tlacebranches
        universals -- a list of universal formulas, represented by Nodes
        """
        self._state = state
        self._atomics = atomics or []
        self._branches = branches or []
        self._universals = universals or []
    
        
    def __str__(self):
        pass # TODO