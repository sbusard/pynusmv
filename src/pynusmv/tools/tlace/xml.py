import sys

from ...utils import indent

from ...nusmv.node import node as nsnode
from ...nusmv.cinit import cinit
from ...nusmv.compile.symb_table import symb_table
from ...nusmv.enc.bdd import bdd as bddEnc

from ...node.node import Node

__id_node = 0
def print_xml_representation(fsm, tlacenode, spec):
    """
    Print the XML representation of tlacenode explaining spec violation by fsm.
    
    Print at standard output the XML representation of a TLACE
    starting at tlacenode, explaining why the state of tlacenode,
    belonging to fsm, violates spec.
    
    fsm -- the FSM violating spec.
    tlacenode -- the TLACE node explaining the violation of spec by fsm.
    spec -- the violated specification.
    """
    
    indent.reset()
    
    indent.prt('<?xml version="1.0" encoding="UTF-8"?>')
    
    # Open counter-example
    indent.prt('<counterexample specification="', end='')
    nsnode.print_node(cinit.get_nusmv_stdout(), spec.ptr)
    print('">')
    
    global __id_node
    __id_node = 0
    
    indent.inc()
    
    print_xml_node(fsm, tlacenode)
    
    indent.dec()
    indent.prt('</counterexample>')
    
    
def print_xml_node(fsm, node):
    """
    Print the XML representation of the given TLACE node.
    
    fsm -- the FSM of the node.
    node -- the TLACE node to print.
    """
    
    # node tag
    global __id_node
    indent.prt('<node id="{0}">'.format(__id_node))
    __id_node += 1
    
    indent.inc()
    # state node
    print_xml_state(fsm, node.state)
    
    # atomics
    for atomic in node.atomics:
        indent.prt('<atomic specification="', end='')
        nsnode.print_node(cinit.get_nusmv_stdout(), atomic.ptr)
        print('" />')
    
    # branches
    for branch in node.branches:
        print_xml_branch(fsm, branch)
    
    # universals
    for universal in node.universals:
        indent.prt('<universal specification="', end='')
        nsnode.print_node(cinit.get_nusmv_stdout(), universal.ptr)
        print('" />')
    
    indent.dec()
    indent.prt('</node>')
    
    
def print_xml_branch(fsm, branch):
    """
    Print the XML representation of the given TLACE branch.
    
    fsm -- the FSM of the node.
    branch -- the TLACE branch to print.
    """
    
    loop_id = -1
    
    indent.prt('<existential specification="', end='')
    nsnode.print_node(cinit.get_nusmv_stdout(), branch.specification.ptr)
    print('">')
    indent.inc()
    
    for n, i in zip(branch.path[0][::2], branch.path[0][1::2]):
        print_xml_node(fsm, n)
        print_xml_inputs(fsm, i)
        if branch.path[1] is not None and n == branch.path[1][1]:
            loop_id = __id_node
        
    print_xml_node(fsm, branch.path[0][-1])
    
    if branch.path[1] is not None:
        print_xml_inputs(fsm, branch.path[1][0])
        indent.prt('<loop to="{0}" />'.format(loop_id))
    
    indent.dec()
    indent.prt('</existential>')
    
    
def print_xml_state(fsm, state):
    """
    Print the XML representation of the given state.
    
    fsm -- the FSM of the state.
    state -- a BDD representing a state of fsm.
    """
    
    indent.prt('<state>')
    indent.inc()
    
    enc = fsm.BddEnc
    # Get symb table from enc (BaseEnc)
    table = enc.symbTable

    # Get symbols (SymbTable) for states
    layers = symb_table.SymbTable_get_class_layer_names(table, None)
    symbols = symb_table.SymbTable_get_layers_sf_symbols(table, layers)
    
    # Get assign symbols (BddEnc)
    assignList = Node(bddEnc.BddEnc_assign_symbols(enc.ptr,
                    state.ptr, symbols, 0, None))
                    
    # Traverse the symbols to print variables of the state
    while assignList is not None:
        assignment = assignList.car
        var = assignment.car
        val = assignment.cdr
        
        indent.prt('<value variable="', end='')
        nsnode.print_node(cinit.get_nusmv_stdout(), var.ptr)
        print('">', end='')
        sys.stdout.flush()
        
        nsnode.print_node(cinit.get_nusmv_stdout(), val.ptr)
        print('</value>')
        
        assignList = assignList.cdr
    
    indent.dec()
    indent.prt('</state>')
    

def print_xml_inputs(fsm, inputs):
    """
    Print the XML representation of the given inputs.
    
    fsm -- the FSM.
    state -- a BDD representing inputs in fsm.
    """
        
    pass # TODO