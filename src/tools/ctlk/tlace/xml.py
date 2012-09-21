import sys

from ...utils import indent
from ..ast import Atom
from .tlace import TemporalBranch, EpistemicBranch

__id_node = 0


def xml_countex(fsm, tlacenode, spec):
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
<tlace specification="{spec}" type="counter-example">
""".format(spec=str(spec)))
    
    indent.inc()
    
    xmlrepr += xml_node(fsm, tlacenode)
    
    indent.dec()
    
    xmlrepr += """</tlace>"""
    
    return xmlrepr


def xml_witness(fsm, tlacenode, spec):
    """
    Return the XML representation of tlacenode explaining spec satisfaction
    by fsm.
    
    Return the XML representation of a TLACE
    starting at tlacenode, explaining why the state of tlacenode,
    belonging to fsm, satisfies spec.
    
    fsm -- the FSM satisfying spec.
    tlacenode -- the TLACE node explaining the satisfaction of spec by fsm.
    spec -- the violated specification.
    """
    
    indent.reset()
    
    global __id_node
    __id_node = 0
    
    # Open counterexample
    xmlrepr = (
    """<?xml version="1.0" encoding="UTF-8"?>
<tlace specification="{spec}" type="witness">
""".format(spec=str(spec)))
    
    indent.inc()
    
    xmlrepr += xml_node(fsm, tlacenode)
    
    indent.dec()
    
    xmlrepr += """</tlace>"""
    
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
""".format(str(atomic)))
    
    # branches
    for branch in node.branches:
        if type(branch) is TemporalBranch:
            xmlrepr += xml_temporal_branch(fsm, branch)
        elif type(branch) is EpistemicBranch:
            xmlrepr += xml_epistemic_branch(fsm, branch)
        else:
            # TODO Raise exception
            print("[ERROR] xml_node: unrecognized branch type.")
    
    # universals
    for universal in node.universals:
        xmlrepr += indent.indent(
        """<universal specification="{0}" />
""".format(str(universal)))
    
    indent.dec()
    
    xmlrepr += indent.indent("""</node>
""")
    
    return xmlrepr


def xml_temporal_branch(fsm, branch):
    """
    Return the XML representation of the given temporal TLACE branch.
    
    fsm -- the FSM of the node.
    branch -- the TLACE branch to represent.
    """
    
    loop_id = -1
    
    xmlrepr = indent.indent(
    """<existential specification="{0}" type="temporal">
""".format(str(branch.specification)))
    
    indent.inc()
    
    for n, i in zip(branch.path[::2], branch.path[1::2]):
        xmlrepr += xml_node(fsm, n)
        xmlrepr += xml_inputs(fsm, i)
        xmlrepr += xml_combinatorial(fsm, i)
        if branch.loop is not None and n == branch.loop[1]:
            loop_id = __id_node - 1
    
    # last node
    xmlrepr += xml_node(fsm, branch.path[-1])
    if branch.loop is not None and branch.path[-1] == branch.loop[1]:
        loop_id = __id_node - 1
    
    if branch.loop is not None:
        xmlrepr += xml_inputs(fsm, branch.loop[0])
        xmlrepr += indent.indent(
        """<loop to="{0}" />
""".format(loop_id))
    
    indent.dec()
    xmlrepr += indent.indent(
    """</existential>
""")
    
    return xmlrepr
    
    
def xml_epistemic_branch(fsm, branch):
    """
    Return the XML representation of the given epistemic TLACE branch.
    
    fsm -- the FSM of the node.
    branch -- the TLACE branch to represent.
    """
    
    xmlrepr = indent.indent(
    """<existential specification="{0}" type="epistemic">
""".format(str(branch.specification)))
    
    indent.inc()
    
    for n, ag in zip(branch.path[::2], branch.path[1::2]):
        xmlrepr += xml_node(fsm, n)
        xmlrepr += xml_agents(fsm, [ag] if type(ag) is str else ag)
        
    xmlrepr += xml_node(fsm, branch.path[-1])
    
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
""".format(var, values[var]))
    
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
""".format(var, values[var]))
    
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
    
    
def xml_agents(fsm, agents):
    """Return the XML representation of the given agents' epistemic relation"""
    
    xmlrepr = indent.indent(
    """<epistemic agents="{}">
""".format(','.join(agents)))
    return xmlrepr