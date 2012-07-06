"""
mc provides some functions of NuSMV dealing with model checking, like CTL
model checking.
"""

from ..nusmv.mc import mc

from ..fsm.fsm import BddFsm
from ..dd.bdd import BDD
from ..node.node import Node
from ..node.listnode import ListNode

def eval_ctl_spec(fsm, spec, context=None):
    """
    Return the BDD representing the set of states of fsm satisfying spec
    in context.
    
    fsm -- a pynusmv.fsm.fsm.BddFsm representing the system.
    spec -- a pynusmv.node.node.Node representing the formula.
    context -- a pynusmv.node.node.Node representing the context of spec.
    """
    enc = fsm.bddEnc
    specbdd = BDD(mc.eval_ctl_spec(fsm._ptr, enc._ptr,
                                   spec._ptr,
                                   context and context._ptr or None),
                  enc.DDmanager)
    return specbdd
    
    
def explainEX(fsm, state, a):
    """
    Explain why state of fsm satisfies EX a.
    
    Explain why state of fsm satisfies EX phi, where a is a BDD representing
    the set of states of fsm satisfying phi.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    a -- a pynusmv.dd.BDD representing the set of states of fsm satisfying phi.
    
    Return (state, inputs, state') where state is the given state,
    state' is a successor of state belonging to a and inputs is a BDD
    representing the inputs to go from state to state' in fsm.
    """
    
    enc = fsm.bddEnc
    manager = enc.DDmanager
    path = ListNode.from_tuple((state.to_node(),))
    nodelist = ListNode(mc.ex_explain(fsm._ptr, enc._ptr, path._ptr, a._ptr))
    
    # nodelist is reversed!
    statep = nodelist.car.to_bdd(manager)
    nodelist = nodelist.cdr
    inputs = nodelist.car.to_bdd(manager)
    state = nodelist.cdr.car.to_bdd(manager)
    
    return (state, inputs, statep)


def explainEU(fsm, state, a, b):
    """
    Explain why state of fsm satisfies E[a U b].
    
    Explain why state of fsm satisfies E[phi U psi],
    where a is a BDD representing the set of states of fsm satisfying phi
    and b is a BDD representing the set of states of fsm satisfying psi.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    a -- a pynusmv.dd.BDD representing the set of states of fsm satisfying phi.
    b -- a pynusmv.dd.BDD representing the set of states of fsm satisfying psi.
    
    Return a tuple t composed of states and inputs, all represented by BDDs,
    such that t[0] is state, t[-1] belongs to b, and every other state of t
    belongs to a. Furthermore, t represents a path in fsm.
    """
    
    enc = fsm.bddEnc
    manager = enc.DDmanager
    path = ListNode.from_tuple((state.to_node(),))
    nodelist = ListNode(mc.eu_explain(fsm._ptr, enc._ptr, path._ptr, a._ptr, b._ptr))
    
    path = []
    for node in nodelist:
        path.insert(0, node.to_bdd(manager))
    
    return tuple(path)
    

def explainEG(fsm, state, a):
    """
    Explain why state of fsm satisfies EG a.
    
    Explain why state of fsm satisfies EG phi,
    where a is a BDD representing the set of states of fsm satisfying phi.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    a -- a pynusmv.dd.BDD representing the set of states of fsm satisfying phi.
    
    Return a (t, (inputs, loop))
    where t is a tuple composed of states and inputs, all represented by BDDs,
    such that t[0] is stateand every other state of t
    belongs to a. Furthermore, t represents a path in fsm.
    loop represents the sstart of the loop contained in t,
    i.e. t[-1] can lead to loop through inputs, and loop is a state of t.
    """
    
    enc = fsm.bddEnc
    manager = enc.DDmanager
    path = ListNode.from_tuple((state.to_node(),))
    nodelist = ListNode(mc.eg_explain(fsm._ptr, enc._ptr, path._ptr, a._ptr))
    
    path = []
    # Discard last state and input, store them as loop indicators
    loopstate = nodelist[0].to_bdd(manager)
    loopinputs = nodelist[1].to_bdd(manager)
    
    # TODO Use slicing (but first implement it)
    nodelist = ListNode(nodelist.cdr.cdr._ptr) # Skip two first elements
    
    for node in nodelist:
        curstate = node.to_bdd(manager)
        path.insert(0, curstate)
        if curstate._ptr == loopstate._ptr:
            loopstate = curstate  
    
    return (tuple(path), (loopinputs, loopstate))