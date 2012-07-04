"""
mc provides some functions of NuSMV dealing with model checking, like CTL
model checking.
"""

from ..nusmv.mc import mc

from ..fsm.fsm import BddFsm
from ..dd.bdd import BDD
from ..node.node import Node

def eval_ctl_spec(fsm, spec, context=None):
    """
    Return the BDD representing the set of states of fsm satisfying spec
    in context.
    
    fsm -- a pynusmv.fsm.fsm.BddFsm representing the system.
    spec -- a pynusmv.node.node.Node representing the formula.
    context -- a pynusmv.node.node.Node representing the context of spec.
    """
    enc = fsm.BddEnc
    specbdd = BDD(mc.eval_ctl_spec(fsm._ptr, enc._ptr,
                                   spec._ptr,
                                   context and context._ptr or None),
                  enc.DDmanager)
    return specbdd