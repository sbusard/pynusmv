from ..eval import evalCTLK
from .explain import explain_witness, explain_countex

def checkCTLK(fsm, spec):
    """
    Check whether fsm satisfies spec.
    
    fsm -- a MAS
    spec -- a CTLK specification
    
    Return a tuple (sat, diag) where
        sat is True iff fsm satisfies spec, it is False otherwise;
        diag is a partial TLACE explaining why fsm satisfies or violates spec.
    """
    
    sat = evalCTLK(fsm, spec)
    violating = fsm.init & ~sat
    if violating.isnot_false():
        return (False,
                explain_countex(fsm, fsm.pick_one_state(violating), spec))
    else:
        return (True,
                explain_witness(fsm, fsm.pick_one_state(fsm.init), spec))