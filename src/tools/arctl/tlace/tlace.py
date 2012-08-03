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
        self.__state = state
        self.__atomics = atomics or tuple()
        self.__branches = branches or tuple()
        self.__universals = universals or tuple()
    
    @property
    def state(self):
        """state node"""
        return self.__state
        
    @property
    def atomics(self):
        """atomic annotations of this node"""
        return self.__atomics
        
    @property
    def branches(self):
        """branches of this node"""
        return self.__branches
        
    @property
    def universals(self):
        """universal annotations of this node"""
        return self.__universals
        
    def __str__(self):
        pass # TODO

class Tlacebranch:
    """
    A Tlacebranch is a TLACE branch.
    It contains an existential formula and a TLACE path,
    represented by a tuple of a list of TLACE nodes
    and a possibly None looping node.
    """
    
    def __init__(self, formula, path, loop=None):
        """
        Creates a new TLACE branch.
        formula is an existential temporal formula;
        path is a list of TLACE nodes;
        loop is None or a (inputs, node) pair where node is a node of path
        indicating the start of a loop.
        """
        self.__formula = formula
        self.__path = (path, loop)        
    
    @property
    def specification(self):
        """The spec of this branch."""
        return self.__formula
        
    @property
    def path(self):
        """The path of this branch. A (path, (inputs, loop)) tuple."""
        return self.__path
        
    def __str__(self):
        pass # TODO