def fixpoint(funct, start):
    """
    Return the fixpoint of funct, as a BDD, starting with start BDD.
    
    μZ.f(Z) least fixpoint is implemented with _fp(funct, false).
    νZ.f(Z) greatest fixpoint is implemented with _fp(funct, true).
    """
    
    old = start
    new = funct(start)
    while old != new:
        old = new
        new = funct(old)
    return old