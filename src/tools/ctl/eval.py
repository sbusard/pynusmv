from pynusmv.nusmv.parser import parser
from pynusmv.dd.bdd import BDD
from pynusmv.mc import eval_ctl_spec

from pynusmv.nusmv.fsm.bdd import bdd as nsBddFsm

def eval_ctl(fsm, spec, context = None):
    """
    Evaluate spec in fsm.
    
    Return the BDD representing all states of fsm satisfying spec.
    """
    
    if spec.type == parser.CONTEXT:
        return eval_ctl(fsm, spec.cdr, spec.car)
        
    elif spec.type == parser.FALSEEXP:
        return BDD.false(fsm.bddEnc.DDmanager)
        
    elif spec.type == parser.TRUEEXP:
        return BDD.true(fsm.bddEnc.DDmanager)
        
    elif spec.type == parser.NOT:
        return ~eval_ctl(fsm, spec.car, context)
        
    elif spec.type == parser.OR:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        return left | right
    
    elif spec.type == parser.AND:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        return left & right
    
    elif spec.type == parser.IMPLIES:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        return ~left | right
                            
    elif spec.type == parser.IFF:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        return (left & right) | (~left & ~right)
                    
    elif spec.type == parser.EX:
        return ex(fsm, eval_ctl(fsm, spec.car, context))
                                 
    elif spec.type == parser.EF:
        left = BDD.true(fsm.bddEnc.DDmanager)
        right = eval_ctl(fsm, spec.car, context)
        return eu(fsm, left, right)
                                 
    elif spec.type == parser.EG:
        return eg(fsm, eval_ctl(fsm, spec.car, context))
                                 
    elif spec.type == parser.EU:
        return eu(fsm,
                  eval_ctl(fsm, spec.car, context),
                  eval_ctl(fsm, spec.cdr, context))
                    
    elif spec.type == parser.EW:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        return eg(fsm, left) | eu(fsm, left, right)
                    
    elif spec.type == parser.AX:
        left = eval_ctl(fsm, spec.car, context)
        return ~ex(fsm, ~left)        
        
    elif spec.type == parser.AF:
        left = eval_ctl(fsm, spec.car, context)
        return ~eg(fsm, ~left)
                                 
    elif spec.type == parser.AG:
        left = eval_ctl(fsm, spec.car, context)
        true = BDD.true(fsm.bddEnc.DDmanager)
        return ~eu(fsm, true, ~left)
                                 
    elif spec.type == parser.AU:
        # A[p U q] = ¬E[¬q W (¬p & ¬q)] = ¬(E[¬q U (¬p & ¬q)] | EG ¬q)
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        return ~(eu(fsm, ~right, ~left & ~right) | eg(fsm, ~right))
                        
    elif spec.type == parser.AW:
        left = eval_ctl(fsm, spec.car, context)
        right = eval_ctl(fsm, spec.cdr, context)
        return ~eu(fsm, ~right, ~left & ~right)
    
    else:
        return eval_ctl_spec(fsm, spec)
        
        
def ex(fsm, phi):
    phi = phi & fair_states(fsm) & fsm.reachable_states
    return fsm.pre(phi) & fsm.reachable_states
    
    
def eg(fsm, phi):
    if phi.is_true():
        return fair_states(fsm)
    res = BDD.true(fsm.bddEnc.DDmanager)
    old = BDD.false(fsm.bddEnc.DDmanager)
    while res != old:
        old = res
        new = ex(fsm, res)
        res = res & new & phi & fsm.reachable_states
    return res
    
    
def eu(fsm, phi, psi):
    Y = psi & fair_states(fsm) & fsm.reachable_states
    old = Y
    new = Y
    while new.isnot_false():
        old = Y
        Y = Y | (ex(fsm, new) & phi)
        new = Y & ~old  
    return Y
    
    
def fair_states(fsm):
    return BDD(nsBddFsm.BddFsm_get_fair_states(fsm._ptr), fsm.bddEnc.DDmanager)