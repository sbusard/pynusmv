from ...utils import indent

from ...nusmv.node import node as nsnode
from ...nusmv.cinit import cinit

__id_node = 0
def print_xml_representation(fsm, tlacenode, spec):
	"""
	Print the XML representation of tlacenode explaining spec violation by fsm.
	
	Print at standard output the XML representation of a TLACE
	starting at	tlacenode, explaining why the state of tlacenode,
	belonging to fsm, violates spec.
	
	fsm -- the FSM violating spec.
	tlacenode -- the TLACE node explaining the violation of spec by fsm.
	spec -- the violated specification.
	"""
	
	indent.reset()
	
	indent.prt('<?xml version="1.0" encoding="UTF-8"?>')
	
	# Open counter-example
	indent.prt('<counterexample specification="', end='')
	nsnode.print_node(cinit.nusmv_stdout, spec.ptr)
	print('">')
	
	__id_node = 0
	
	indent.inc()
	print_xml_node(fsm, tlacenode)
	indent.dec()
	
	indent.ptr('</counterexample>')
	
	
def print_xml_node(fsm, node):
    """
    Print the XML representation of the given TLACE node.
    
    fsm -- the FSM of the node.
    node -- the TLACE node to print.
    """
    
    pass # TODO