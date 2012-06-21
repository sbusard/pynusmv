from tlacenode import Tlacenode
from tlacebranch import Tlacebranch

from ...nusmv.parser import parser
from ...nusmv.mc import mc

from ...node.node import Node

def explain(fsm, state, spec):
    """
    Return a TLACE node explaining why state of fsm violates spec.
    
    fsm -- the system.
    state -- a BDD representing a state of fsm.
    spec -- a pynusmv.node.node.Node node representing the specification.
    
    Returns a tlacenode.Tlacenode explaining why state of fsm violates spec.
    """
    return countex(fsm, state, spec, None)
    
    
def countex(fsm, state, spec, context):
    """
    Return a TLACE node explaining why state of fsm violates spec.
    
    fsm -- the system.
    state -- a BDD representing a state of fsm.
    spec -- a pynusmv.node.node.Node node representing the specification.
    context -- a pynusmv.node.node.Node representing the context of spec in fsm.
    
    Returns a tlacenode.Tlacenode explaining why state of fsm violates spec.
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
    
    fsm -- the system.
    state -- a BDD representing a state of fsm.
    spec -- a pynusmv.node.node.Node node representing the specification.
    context -- a pynusmv.node.node.Node representing the context of spec in fsm.
    
    Returns a tlacenode.Tlacenode explaining why state of fsm satisfies spec.
    """

    if spec.type == parser.CONTEXT:
        return witness(fsm, state, spec.cdr, spec.car)
        
    elif spec.type == parser.TRUEEXP:
        return Tlacenode(state, None, None, None)
        
    elif spec.type == parser.parser.NOT:
        return countex(fsm, state, spec.car, context)
        
    elif spec.type == parser.OR:
        pass # TODO
    
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
        pass # TODO
                    
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