from .eval import evalArctl
from .explain import explain_witness
from .explain import explain_countex

def checkArctl(fsm, spec):
    """
    Check whether fsm satisfies spec.
    
    fsm -- a BddFsm;
    spec -- an ARCTL spec, i.e. an AST made of .ast module classes.
    
    Return a tuple (result, path) where
        result is True if fsm satisfies spec, False otherwise;
        path is a witness if result is True, a counter-example otherwise.
    """
    init = fsm.init
    specbdd = evalArctl(fsm, spec)
    
    violating = init & ~specbdd
    if violating.isnot_false():
        state = fsm.pick_one_state(violating)
        return (False, explain_countex(fsm, state, spec))
        
    else:
        satisfying = init & specbdd
        state = fsm.pick_one_state(satisfying)
        return (True, explain_witness(fsm, state, spec))