from tlacenode import Tlacenode
from tlacebranch import Tlacebranch

from ...nusmv.parser import parser
from ...nusmv.mc import mc
from ...nusmv.fsm.bdd import bdd as FsmBdd
from ...nusmv.node import node as nsnode

from ...node.node import Node

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
        
    elif spec.type == parser.parser.NOT:
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
        return Tlacenode(state, [newspec], None, None)
        
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
        
    elif spec.type == parser.parser.NOT:
        return countex(fsm, state, spec.car, context)
        
    elif spec.type == parser.OR:
        # TODO Encaplusate these
        enc = fsm.BddEnc
        specbdd = BDD(mc.eval_ctl_spec(fsm.__ptr, enc.__ptr,
                                       spec.car.__ptr,
                                       context.__ptr))
        if state.entailed(specbdd):
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
                    
    elif spec.type == parser.EX or
         spec.type == parser.EF or
         spec.type == parser.EG or
         spec.type == parser.EU or
         spec.type == parser.EW:
        return Tlacenode(state, None,
                         {spec:witness_branch(fsm, state, spec, context)},
                         None)
                    
    elif spec.type == parser.AX or
         spec.type == parser.AF or
         spec.type == parser.AG or
         spec.type == parser.AU or
         spec.type == parser.AW:
        return Tlacenode(state, None, None, [spec])
                        
    else:
        # deal with unmanaged formulas by returning a single
    	# TLACE node. This includes the case of atomic propositions.
    	# All unrecognized operators are considered as atomics
    	return Tlacenode(state, [spec], None, None)
    	
    	
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
        pass # TODO
        
	elif spec.type == parser.EF:
        pass # TODO
        
	elif spec.type == parser.EG:
        pass # TODO
        
	elif spec.type == parser.EU:
        pass # TODO
        
	elif spec.type == parser.EW:
	    pass # TODO
	    
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
    path = Node.node_from_list([state])
    nodelist = mc.ex_explain(fsm.__ptr, enc.__ptr, path.__ptr, a.__ptr)
    state = nodelist.car
    nodelist = nodelist.cdr
    inputs = nodelist.car
    statep = nodelist.cdr.car
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
    pass # TODO
    

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
    pass # TODO
	    
        
        
class NonExistentialSpecError(Exception):
    """
    Exception for given non existential temporal formula.
    """
    pass