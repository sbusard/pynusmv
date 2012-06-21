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
        loop is None or a node of path indicating the start of a loop.
        """
        self._formula = formula
        self._path = (path, loop)
        
        
    def __str__(self):
        pass # TODO