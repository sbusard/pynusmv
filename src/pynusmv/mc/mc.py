"""
mc provides some functions of NuSMV dealing with model checking, like CTL
model checking.
"""

from ..nusmv.mc import mc

from ..dd.dd import BDD

def eval_ctl_spec(fsm, spec, context):
    """
    Return the BDD representing the set of states of fsm satisfying spec
    in context.
    
    
    """
    enc = fsm.BddEnc
    specbdd = BDD(mc.eval_ctl_spec(fsm.__ptr, enc.__ptr,
                                   spec.car.__ptr,
                                   context.__ptr))
    return specbdd