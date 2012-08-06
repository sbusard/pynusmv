class Tlacebranch:
    """
    A Tlacebranch is a TLACE branch.
    It contains an existential formula and a TLACE path,
    represented by a tuple of a list of TLACE nodes
    and a possibly None looping node.
    """
    
    def __init__(self, formula, path, loop=None):
        """
        Create a new TLACE branch.
        
        formula -- an existential temporal formula;
        path -- a list of TLACE nodes;
        loop -- None or a (inputs, node) pair where node is a node of path
                indicating the start of a loop.
        """
        self.__formula = formula
        self.__path = (path, loop)
        
        
    def __str__(self):
        pass # TODO
        
    
    @property
    def specification(self):
        """The spec of this branch."""
        return self.__formula
        
    @property
    def path(self):
        """The path of this branch. A (path, (inputs, loop)) tuple, or None."""
        return self.__path