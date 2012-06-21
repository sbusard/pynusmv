from tlacenode import Tlacenode
from tlacebranch import Tlacebranch

from ...nusmv.parser import parser

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
    
    
def witness(fsm, state, spec, context):
    """
    Return a TLACE node explaining why state of fsm satisfies spec.
    
    fsm -- the system.
    state -- a BDD representing a state of fsm.
    spec -- a pynusmv.node.node.Node node representing the specification.
    context -- a pynusmv.node.node.Node representing the context of spec in fsm.
    
    Returns a tlacenode.Tlacenode explaining why state of fsm satisfies spec.
    """
    pass # TODO
    
    
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