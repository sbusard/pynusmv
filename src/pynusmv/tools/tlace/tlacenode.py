from ...nusmv.dd import dd

class Tlacenode:
    """
    A Tlacenode is a TLACE node.
    It contains a state, a list of atomic propositions, a list of TLACE branches
    and a list of universal formulas.
    """
    
    def __init__(self, state, atomics=None, branches=None, universals=None):
        """
        Creates a new TLACE node.
        state the state of the TLACE node;
        atomics a list of atomic propositions;
        branches a list of TLACE branches;
        universals a list of universal formulas.
        """
        self._state = dd.bdd_dup(state)
        self._atomics = atomics or []
        self._branches = branches or []
        self._universals = universals or []
    
        
    def __str__(self):
        pass # TODO