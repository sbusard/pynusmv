from .tlacenode import Tlacenode
from .tlacebranch import Tlacebranch

from ...nusmv.parser import parser
from ...nusmv.mc import mc
from ...nusmv.fsm.bdd import bdd as FsmBdd
from ...nusmv.node import node as nsnode

from ...node.node import Node
from ...mc.mc import eval_ctl_spec
from ...dd.dd import BDD

def explain(fsm, state, spec):
    """
    Return a TLACE node explaining why state of fsm violates spec.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    spec -- a pynusmv.node.node.Node node representing the specification.
    
    Return a tlacenode.Tlacenode explaining why state of fsm violates spec.
    """
    return countex(fsm, state, spec, None)
    
    
def countex(fsm, state, spec, context):
    """
    Return a TLACE node explaining why state of fsm violates spec.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    spec -- a pynusmv.node.node.Node node representing the specification.
    context -- a pynusmv.node.node.Node representing the context of spec in fsm.
    
    Return a tlacenode.Tlacenode explaining why state of fsm violates spec.
    """
    
    if spec.type == parser.CONTEXT:
        return countex(fsm, state, spec.cdr, spec.car)
        
    elif spec.type == parser.FALSEEXP:
        newspec = Node.find_node(parser.TRUEEXP)
        
    elif spec.type == parser.NOT:
        newspec = spec.car
        
    elif spec.type == parser.OR:
        newspec = Node.find_node(parser.AND,
                                 Node.find_node(parser.NOT, spec.car),
                                 Node.find_node(parser.NOT, spec.cdr))
    
    elif spec.type == parser.AND:
        newspec = Node.find_node(parser.OR,
                                 Node.find_node(parser.NOT, spec.car),
                                 Node.find_node(parser.NOT, spec.cdr))
    
    elif spec.type == parser.IMPLIES:
        newspec = Node.find_node(parser.AND,
                                 spec.car,
                                 Node.find_node(parser.NOT, spec.cdr))
                            
    elif spec.type == parser.IFF:
        newspec = Node.find_node(
                    parser.OR,
                    Node.find_node(
                        parser.AND,
                        spec.car,
                        Node.find_node(parser.NOT, spec.cdr)),
                    Node.find_node(
                        parser.AND,
                        Node.find_node(parser.NOT, spec.car),
                        spec.cdr))
                    
    elif spec.type == parser.EX:
        newspec = Node.find_node(parser.AX,
                                 Node.find_node(parser.NOT, spec.car))
                                 
    elif spec.type == parser.EF:
        newspec = Node.find_node(parser.AG,
                                 Node.find_node(parser.NOT, spec.car))
                                 
    elif spec.type == parser.EG:
        newspec = Node.find_node(parser.AF,
                                 Node.find_node(parser.NOT, spec.car))
                                 
    elif spec.type == parser.EU:
        newspec = Node.find_node(
                    parser.AW,
                    Node.find_node(parser.NOT, spec.cdr),
                    Node.find_node(
                        parser.AND,
                        Node.find_node(parser.NOT, spec.car),
                        Node.find_node(parser.NOT, spec.cdr)))
                    
    elif spec.type == parser.EW:
        newspec = Node.find_node(
                    parser.AU,
                    Node.find_node(parser.NOT, spec.cdr),
                    Node.find_node(
                        parser.AND,
                        Node.find_node(parser.NOT, spec.car),
                        Node.find_node(parser.NOT, spec.cdr)))
                    
    elif spec.type == parser.AX:
        newspec = Node.find_node(parser.EX,
                                 Node.find_node(parser.NOT, spec.car))
                                 
    elif spec.type == parser.AF:
        newspec = Node.find_node(parser.EG,
                                 Node.find_node(parser.NOT, spec.car))
                                 
    elif spec.type == parser.AG:
        newspec = Node.find_node(parser.EF,
                                 Node.find_node(parser.NOT, spec.car))
                                 
    elif spec.type == parser.AU:
        newspec = Node.find_node(
                    parser.EW,
                    Node.find_node(parser.NOT, spec.cdr),
                    Node.find_node(
                        parser.AND,
                        Node.find_node(parser.NOT, spec.car),
                        Node.find_node(parser.NOT, spec.cdr)))
                        
    elif spec.type == parser.AW:
        newspec = Node.find_node(parser.EU,
                    Node.find_node(parser.NOT, spec.cdr),
                    Node.find_node(
                        parser.AND,
                        Node.find_node(parser.NOT, spec.car),
                        Node.find_node(parser.NOT, spec.cdr)))
                        
    else:
        if spec.type == parser.NOT:
            newspec = spec.car
        else:
            newspec = Node.find_node(parser.NOT, spec)
        return Tlacenode(state, (newspec,), None, None)
        
    return witness(fsm, state, newspec, context)
    
    
def witness(fsm, state, spec, context):
    """
    Return a TLACE node explaining why state of fsm satisfies spec.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    spec -- a pynusmv.node.node.Node node representing the specification.
    context -- a pynusmv.node.node.Node representing the context of spec in fsm.
    
    Return a tlacenode.Tlacenode explaining why state of fsm satisfies spec.
    """

    if spec.type == parser.CONTEXT:
        return witness(fsm, state, spec.cdr, spec.car)
        
    elif spec.type == parser.TRUEEXP:
        return Tlacenode(state, None, None, None)
        
    elif spec.type == parser.NOT:
        return countex(fsm, state, spec.car, context)
        
    elif spec.type == parser.OR:
        if state.entailed(eval_ctl_spec(fsm, spec.car, context)):
            return witness(fsm, state, spec.car, context)
        else:
            return witness(fsm, state, spec.cdr, context)
    
    elif spec.type == parser.AND:
        n1 = witness(fsm, state, spec.car, context)
        n2 = witness(fsm, state, spec.cdr, context)
        return Tlacenode(state,
                         n1.atomics + n2.atomics,
                         n1.branches + n2.branches,
                         n1.universals + n2.universals)
    
    elif spec.type == parser.IMPLIES:
        newspec = Node.find_node(parser.OR,
                                 Node.find_node(parser.NOT, spec.car),
                                 spec.cdr)
        return witness(fsm, state, newspec, context)
                            
    elif spec.type == parser.IFF:
        newspec = Node.find_node(
                    parser.OR,
                    Node.find_node(
                        parser.AND,
                        spec.car,
                        spec.cdr),
                    Node.find_node(
                        parser.AND,
                        Node.find_node(parser.NOT, spec.car),
                        Node.find_node(parser.NOT, spec.cdr)))
        return witness(fsm, state, newspec, context)
                    
    elif (spec.type == parser.EX or
          spec.type == parser.EF or
          spec.type == parser.EG or
          spec.type == parser.EU or
          spec.type == parser.EW):
        return Tlacenode(state, None,
                         (witness_branch(fsm, state, spec, context),),
                         None)
                    
    elif (spec.type == parser.AX or
          spec.type == parser.AF or
          spec.type == parser.AG or
          spec.type == parser.AU or
          spec.type == parser.AW):
        return Tlacenode(state, None, None, (spec,))
                        
    else:
        # deal with unmanaged formulas by returning a single
        # TLACE node. This includes the case of atomic propositions.
        # All unrecognized operators are considered as atomics
        return Tlacenode(state, (spec,), None, None)
        
        
def witness_branch(fsm, state, spec, context):
    """
    Return a TLACE branch explaining why state of fsm satisfies spec.

    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    spec -- a pynusmv.node.node.Node node representing the specification.
    context -- a pynusmv.node.node.Node representing the context of spec in fsm.

    Return a tlacebranch.Tlacebranch explaining why state of fsm satisfies spec.
    
    Throw a NonExistentialSpecError if spec is not existential.
    """
    
    if spec.type == parser.EX:
        f = eval_ctl_spec(fsm, spec.car, context)
        path = explainEX(fsm, state, f)
        branch = (Tlacenode(path[0]),
                  path[1],
                  witness(fsm, spec.car, path[2], context))
        return Tlacebranch(spec, branch)
        
    elif spec.type == parser.EF:
        newspec = Node.find_node(parser.EU,
                                 Node.find_node(parser.TRUEEXP),
                                 spec.car)
        return witness_branch(fsm, state, newspec, context)
        
    elif spec.type == parser.EG:
        f = eval_ctl_spec(fsm, spec.car, context)
        (path, (inloop, loop)) = explainEG(fsm, state, f)
        
        branch = []
        # intermediate states
        for s, i in zip(path[::2], path[1::2]):
            wit = witness(fsm, s, spec.car, context)
            branch.append(wit)
            branch.append(i)
            # manage the loop
            if s == loop:
                loop = wit
        # last state
        branch.append(witness(fsm, path[-1], spec.car, context))
        
        return Tlacebranch(spec, tuple(branch), (inloop, loop))
        
    elif spec.type == parser.EU:
        f = eval_ctl_spec(fsm, spec.car, context)
        g = eval_ctl_spec(fsm, spec.cdr, context)
        path = explainEU(fsm, state, f, g)
        
        branch = []
        # intermediate states
        for s, i in zip(path[::2], path[1::2]):
            branch.append(witness(fsm, s, spec.car, context))
            branch.append(i)
        # last state
        branch.append(witness(fsm, path[-1], spec.cdr, context))
        
        return Tlacebranch(spec, tuple(branch))
        
    elif spec.type == parser.EW:
        euspec = Node.find_node(EU, spec.car, spec.cdr)
        egspec = Node.find_node(EG, spec.car)
        if state.entailed(eval_ctl_spec(fsm, euspec, context)):
            return witness_branch(fsm, state, euspec, context)
        else:
            return witness_branch(fsm, state, egspec, context)
        
    else:
        # Default case, throw an exception because spec is not existential
        raise NonExistentialSpecError()
        
        

def explainEx(fsm, state, a):
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
    
    enc = fsm.BddEnc
    manager = enc.DDmanager
    path = Node.node_from_list([state])
    nodelist = mc.ex_explain(fsm.__ptr, enc.__ptr, path.__ptr, a.__ptr)
    
    # nodelist is reversed!
    statep = BDD(nodelist.car.__ptr, manager)
    nodelist = nodelist.cdr
    inputs = BDD(nodelist.car.__ptr, manager)
    state = BDD(nodelist.cdr.car.__ptr, manager)
    
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
    
    enc = fsm.BddEnc
    manager = enc.DDmanager
    path = Node.node_from_list([state.to_node()])
    nodelist = Node(mc.eu_explain(fsm.ptr, enc.ptr, path.ptr, a.ptr, b.ptr))
    
    path = []
    while nodelist is not None:
        path.insert(0, nodelist.car.to_bdd(manager))
        nodelist = nodelist.cdr    
    
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
    
    enc = fsm.BddEnc
    manager = enc.DDmanager
    path = Node.node_from_list([state.to_node()])
    nodelist = Node(mc.eg_explain(fsm.ptr, enc.ptr, path.ptr, a.ptr))
    
    path = []
    # Discard last state and input, store them as loop indicators
    loopstate = nodelist.car.to_bdd(manager)
    nodelist = nodelist.cdr
    loopinputs = nodelist.car.to_bdd(manager)
    nodelist = nodelist.cdr
    while nodelist is not None:
        curstate = nodelist.car.to_bdd(manager)
        path.insert(0, curstate)
        if curstate.ptr == loopstate.ptr:
            loopstate = curstate
        nodelist = nodelist.cdr    
    
    return (tuple(path), (loopinputs, loopstate))



class NonExistentialSpecError(Exception):
    """
    Exception for given non existential temporal formula.
    """
    pass
