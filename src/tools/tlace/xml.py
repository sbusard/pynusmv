import sys
from xml.sax.saxutils import escape

from ..utils import indent

__id_node = 0
def xml_representation(fsm, tlacenode, spec):
    """
    Return the XML representation of tlacenode explaining spec violation by fsm.
    
    Return the XML representation of a TLACE
    starting at tlacenode, explaining why the state of tlacenode,
    belonging to fsm, violates spec.
    
    fsm -- the FSM violating spec.
    tlacenode -- the TLACE node explaining the violation of spec by fsm.
    spec -- the violated specification.
    """
    
    indent.reset()
    
    global __id_node
    __id_node = 0
    
    # Open counterexample
    xmlrepr = (
    """<?xml version="1.0" encoding="UTF-8"?>
<counterexample specification="{spec}">
""".format(spec=escape(str(spec))))
    
    indent.inc()
    
    xmlrepr += xml_node(fsm, tlacenode)
    
    indent.dec()
    
    xmlrepr += """</counterexample>"""
    
    return xmlrepr
    
    
def xml_node(fsm, node):
    """
    Return the XML representation of the given TLACE node.
    
    fsm -- the FSM of the node.
    node -- the TLACE node to represent.
    """
    
    # node tag
    global __id_node
    
    xmlrepr = indent.indent(
    """<node id="{0}">
""".format(__id_node))
    
    __id_node += 1
    
    indent.inc()
    # state node
    xmlrepr += xml_state(fsm, node.state)
    
    # atomics
    for atomic in node.atomics:
        xmlrepr += indent.indent(
        """<atomic specification="{0}" />
""".format(escape(str(atomic))))
    
    # branches
    for branch in node.branches:
        xmlrepr += xml_branch(fsm, branch)
    
    # universals
    for universal in node.universals:
        xmlrepr += indent.indent(
        """<universal specification="{0}" />
""".format(escape(str(universal))))
    
    indent.dec()
    
    xmlrepr += indent.indent("""</node>
""")
    
    return xmlrepr
    
    
def xml_branch(fsm, branch):
    """
    Return the XML representation of the given TLACE branch.
    
    fsm -- the FSM of the node.
    branch -- the TLACE branch to represent.
    """
    
    loop_id = -1
    
    xmlrepr = indent.indent(
    """<existential specification="{0}" explained="true">
""".format(escape(str(branch.specification))))
    
    indent.inc()
    
    for n, i in zip(branch.path[0][::2], branch.path[0][1::2]):
        xmlrepr += xml_node(fsm, n)
        xmlrepr += xml_inputs(fsm, i)
        xmlrepr += xml_combinatorial(fsm, i)
        if branch.path[1] is not None and n == branch.path[1][1]:
            loop_id = __id_node - 1
        
    xmlrepr += xml_node(fsm, branch.path[0][-1])
    
    if branch.path[1] is not None:
        xmlrepr += xml_inputs(fsm, branch.path[1][0])
        xmlrepr += indent.indent(
        """<loop to="{0}" />
""".format(loop_id))
    
    indent.dec()
    xmlrepr += indent.indent(
    """</existential>
""")
    
    return xmlrepr
    
    
def xml_state(fsm, state):
    """
    Return the XML representation of the given state.
    
    fsm -- the FSM of the state.
    state -- a BDD representing a state of fsm.
    """
    
    xmlrepr = indent.indent(
    """<state>
""")
    
    indent.inc()
    
    values = state.get_str_values()
    for var in values:
        xmlrepr += indent.indent(
        """<value variable="{0}">{1}</value>
""".format(escape(var), escape(values[var])))
    
    indent.dec()
    xmlrepr += indent.indent("""</state>
""")
    
    return xmlrepr
    

def xml_inputs(fsm, inputs):
    """
    Return the XML representation of the given inputs.
    
    fsm -- the FSM.
    state -- a BDD representing inputs in fsm.
    """
        
    xmlrepr = indent.indent(
    """<input>
""")
    
    indent.inc()
    
    values = inputs.get_str_values()
    for var in values:
        xmlrepr += indent.indent(
        """<value variable="{0}">{1}</value>
""".format(escape(var), escape(values[var])))
    
    indent.dec()
    xmlrepr += indent.indent("""</input>
""")
    
    return xmlrepr
    
    
def xml_combinatorial(fsm, combinatorial):
    """
    Return the XML representation of the given combinatorial.
    
    fsm -- the FSM.
    state -- a BDD representing inputs in fsm.
    """
        
    xmlrepr = indent.indent(
    """<combinatorial>
""")
    
    indent.inc()
    
    # TODO Find a way to get combinatorial
    
    indent.dec()
    xmlrepr += indent.indent("""</combinatorial>
""")
    
    return xmlrepr