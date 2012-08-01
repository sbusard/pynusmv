from .eval import evalArctl

def checkArctl(fsm, spec):
    """
    Check whether fsm satisfies spec.
    
    fsm -- a BddFsm;
    spec -- an ARCTL spec, i.e. an AST made of .ast module classes.
    
    Return True if fsm satisfies spec, False otherwise.
    """
    init = fsm.init
    specbdd = evalArctl(fsm, spec)
    
    violating = init & ~specbdd
    if violating.isnot_false():
        return False
        
    else:
        return True