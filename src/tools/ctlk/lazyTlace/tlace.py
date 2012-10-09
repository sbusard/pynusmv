from ..tlace.tlace import (Tlacenode, Tlacebranch, EpistemicBranch,
                           TemporalBranch)

class PartialTlacenode(Tlacenode):
    """
    A PartialTlacenode is a TLACE node with some potentially unexplained
    branches. This means that some branch may not be expandable and must
    be explained before. Its branches can be TLACE branches (instances of
    (a subclass of) the TlaceBranch class) or specifications, the former
    being an explained branch while the latter representing an unexplained
    branch.
    """
    
    def __init__(self, state, atomics=None, branches=None, universals=None):
        """
        Create a new partial TLACE node.
        
        state -- a State representing the state of the TLACE node
        atomics -- a list of atomic propositions,
                   represented by CTLK AST instances
        branches -- a list of TLACE branches represented by Tlacebranches
        universals -- a list of universal formulas,
                      represented by CTLK AST instances
        """
        self._state = state
        self._atomics = atomics or []
        self._branches = branches or []
        self._universals = universals or []