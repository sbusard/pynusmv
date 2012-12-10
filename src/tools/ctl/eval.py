from pynusmv.nusmv.parser import parser
from pynusmv.dd.bdd import BDD
from pynusmv.spec import spec as SPEC
from pynusmv.mc.mc import eval_ctl_spec

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
    return fsm.pre(phi)
    
def eg(fsm, phi):
    return _fp(lambda Z: phi & fsm.pre(Z),
               BDD.true(fsm.bddEnc.DDmanager))
    
def eu(fsm, phi, psi):
    return _fp(lambda Z: psi | (phi & fsm.pre(Z)),
               BDD.false(fsm.bddEnc.DDmanager))
    
def _fp(funct, start):
    old = start
    new = funct(start)
    while old != new:
        old = new
        new = funct(old)
    return old