from .eval import evalArctl
from .explain import explain_witness, explain_countex

def checkArctl(fsm, spec, witness = None, countex = None):
    """
    Check whether fsm satisfies spec.
    
    fsm -- a BddFsm;
    spec -- an ARCTL spec, i.e. an AST made of .ast module classes.
    witness -- None, or a function of signature (fsm, state, spec) that
               returns a witness for fsm, state |= spec.
    countex -- None, or a function of signature (fsm, state, spec) that
               returns a counter-example for fsm, state |= spec
    
    Return a tuple (sat, witness) where
        sat is True if fsm satisfies spec, False otherwise;
        if sat is True,
            witness is the result of witness(fsm, state, spec)
        otherwise,
            witness is the result of countex(fsm, state, spec)
        where state is a state satisfying or violating spec in fsm.
    If witness or countex is None,
        tools.arctl.explain.explain_witness or
        tools.arctl.explain.explain_countex is used.
    """
    init = fsm.init
    specbdd = evalArctl(fsm, spec)
    
    violating = init & ~specbdd
    if violating.isnot_false():
        state = fsm.pick_one_state(violating)
        return (False, countex and countex(fsm, state, spec) or
                       explain_countex(fsm, state, spec))
        
    else:
        satisfying = init & specbdd
        state = fsm.pick_one_state(satisfying)
        return (True, witness and witness(fsm, state, spec) or
                      explain_witness(fsm, state, spec))