from pynusmv.nusmv.parser import parser
from pynusmv.dd import BDD
from pynusmv.mc import eval_ctl_spec

from pynusmv.nusmv.fsm.bdd import bdd as nsBddFsm

def check(fsm, spec, context=None):
    """
    Return whether spec in context is satisfied by fsm.
    """
    violating = (fsm.init & ~eval_ctl(fsm, spec, context=context) &
                 fsm.state_constraints)
    return violating.is_false()

def eval_ctl(fsm, spec, context=None):
    """
    Evaluate spec in fsm.
    
    Return the BDD representing all reachable states of fsm satisfying spec.
    """
    
    if spec.type == parser.CONTEXT:
        sat = eval_ctl(fsm, spec.cdr, spec.car)
        
    elif spec.type == parser.FALSEEXP:
        sat = BDD.false(fsm.bddEnc.DDmanager)
        
    elif spec.type == parser.TRUEEXP:
        sat = BDD.true(fsm.bddEnc.DDmanager)
        
    elif spec.type == parser.NOT:
        sat = ~eval_ctl(fsm, spec.car, context)
        
    elif spec.type == parser.OR:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        sat = left | right
    
    elif spec.type == parser.AND:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        sat = left & right
    
    elif spec.type == parser.IMPLIES:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        sat = ~left | right
        
    elif spec.type == parser.IFF:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        sat = (left & right) | (~left & ~right)
        
    elif spec.type == parser.EX:
        sat = ex(fsm, eval_ctl(fsm, spec.car, context))
        
    elif spec.type == parser.EF:
        left = BDD.true(fsm.bddEnc.DDmanager)
        right = eval_ctl(fsm, spec.car, context)
        sat = eu(fsm, left, right)
        
    elif spec.type == parser.EG:
        sat = eg(fsm, eval_ctl(fsm, spec.car, context))
        
    elif spec.type == parser.EU:
        sat = eu(fsm,
                 eval_ctl(fsm, spec.car, context),
                 eval_ctl(fsm, spec.cdr, context))
        
    elif spec.type == parser.EW:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        sat = eg(fsm, left) | eu(fsm, left, right)
        
    elif spec.type == parser.AX:
        left = eval_ctl(fsm, spec.car, context)
        sat = ~ex(fsm, ~left)
        
    elif spec.type == parser.AF:
        left = eval_ctl(fsm, spec.car, context)
        sat = ~eg(fsm, ~left)
        
    elif spec.type == parser.AG:
        left = eval_ctl(fsm, spec.car, context)
        true = BDD.true(fsm.bddEnc.DDmanager)
        sat = ~eu(fsm, true, ~left)
        
    elif spec.type == parser.AU:
        # A[p U q] = !E[!q W (!p & !q)] = !(E[!q U (!p & !q)] | EG !q)
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        sat = ~(eu(fsm, ~right, ~left & ~right) | eg(fsm, ~right))
        
    elif spec.type == parser.AW:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        sat = ~eu(fsm, ~right, ~left & ~right)
    
    else:
        sat = eval_ctl_spec(fsm, spec)
    
    return sat & fsm.reachable_states

def ex(fsm, phi):
    phi = phi & fsm.reachable_states
    result = fsm.pre(phi)
    return result & fsm.reachable_states

def eg(fsm, phi):
    res = BDD.true(fsm.bddEnc.DDmanager)
    old = BDD.false(fsm.bddEnc.DDmanager)
    while res != old:
        old = res
        new = ex(fsm, res)
        res = res & new & phi & fsm.reachable_states
    return res

def eu(fsm, phi, psi):
    Y = psi & fsm.reachable_states
    old = Y
    new = Y
    while new.isnot_false():
        old = Y
        Y = Y | (ex(fsm, new) & phi)
        new = Y & ~old
    return Y