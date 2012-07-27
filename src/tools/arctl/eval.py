"""
ARCTL evaluation functions.
"""

def eval(fsm, spec):
    """Return a BDD representing the set of states of fsm satisfying spec."""
    pass # TODO
    
    
def _ex(fsm, alpha, phi):
    """ex(a, p) is pre of p through transitions satisfying a"""
    pass # TODO
    
    
def _eu(fsm, alpha, phi, psi):
    """eu(a, p, q) = muZ. (q | (p & ex(a, Z)))"""
    pass # TODO
    
    
def _eg(fsm, alpha, phi):
    """eg(a, p) = nuZ. (p & ex(a, Z))"""
    pass # TODO
    
    
def eax(fsm, alpha, phi):
    """eax(a, p) = ex(a, p)"""
    pass # TODO
    
    
def aax(fsm, alpha, phi):
    """aax(a, p) = ex(a, true) & ~ex(a, ~p)"""
    pass # TODO
    
    
def eau(fsm, alpha, phi, psi):
    """eau(a, p, q) = eu(a, p, q)"""
    pass # TODO
    
    
def aau(fsm, alpha, phi, psi):
    """aau(a, p, q) = ~eu(a, ~q, ~q & (~ex(a, true))) & ~eg(a, ~q)"""
    pass # TODO
    

def eaf(fsm, alpha, phi):
    """eaf(a, p) = eu(a, true, p)"""
    pass # TODO
    
    
def aaf(fsm, alpha, phi):
    """aaf(a, p) = ~eu(a, ~p, ~p & ~ex(a, true)) & ~eg(a, ~p)"""
    pass # TODO
    
    
def eag(fsm, alpha, phi):
    """eag(a, p) = eu(a, p, p & ~ex(a, true)) | eg(a, p)"""
    pass # TODO
    
    
def aag(fsm, alpha, phi):
    """aag(a, p) = ~eu(a, true, ~p)"""
    pass # TODO