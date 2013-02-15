from pynusmv.mc import eval_ctl_spec
from .explain import explain

def check(fsm, spec):
    """
    Check whether fsm satisfies spec or not.
    
    Return (True, None) if spec is satisfied,
    and (False, cntex) otherwise,
    where cntex is TLACE node explaining the violation.
    """
    
    initbdd = fsm.init
    specbdd = eval_ctl_spec(fsm, spec)
    
    # Get violating states
    violating = initbdd & ~specbdd
    
    # If some initial states are not in specbdd, the spec if violated
    if violating.isnot_false():
        # Compute a counter-example
        state = fsm.pick_one_state(violating)
        return (False, explain(fsm, state, spec))
        
    # Otherwise, it is satisfied
    else:
        return (True, None)
