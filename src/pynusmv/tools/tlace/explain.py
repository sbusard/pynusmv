from tlacenode import Tlacenode
from tlacebranch import Tlacebranch

from ...nusmv.parser import parser
from ...nusmv.node import node

def explain(fsm, state, spec):
	"""
	Returns a TLACE node explaining why state of fsm violates spec.
	"""
	return countex(fsm, state, spec, None)
	
	
def witness(fsm, state, spec, context):
	"""
	Returns a TLACE node explaining why state of fsm satisfies spec.
	context is necessary for getting states satisfying given properties.
	"""
	pass # TODO
	
	
def countex(fsm, state, spec, context):
	"""
	Returns a TLACE node explaining why state of fsm violates spec.
	context is necessary for getting states satisfying given properties.
	"""
	
	if node.node_get_type(spec) == parser.CONTEXT:
		return countex(fsm, state, node.cdr(spec), node.car(spec))
		
	if node.node_get_type(spec) == parser.FALSEEXP:
		return witness(fsm, state, parser.TRUEEXP, context)
		
	if node.node_get_type(spec) == parser.NOT:
		return witness(fsm, state, node.car(spec), context)