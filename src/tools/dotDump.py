import sys
import argparse
from colorsys import rgb_to_hsv

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob
from pynusmv.exception import PyNuSMVError

# Choices of representation
# -------------------------
# Initial states are marked with a bold border.
#
# Fair states and actions and colored differently according to the fairness
# constraint.
# FIXME Graphviz does not recognize gradient colors

__fair_attr = None
def fairness_attr(fsm):
    """
    Return a dictionary of fairness constraint -> DOT attributes to use in the
    DOT representation of a model to indicate the different fairness
    constraints.
    
    fsm -- the BddFsm of the model.
    
    """
    global __fair_attr
    if not __fair_attr:
        __fair_attr = dict()
        nbfair = len(fsm.fairness_constraints)
        curid = 0
        for f in fsm.fairness_constraints:
            __fair_attr[f] = " ".join(str(val) for val in 
                                  rgb_to_hsv(curid/nbfair, 1, 1 - curid/nbfair))
            curid += 1
        
    return __fair_attr
    

def node(fsm, state, ids):
    """
    Return the DOT representation of the given state.
    
    fsm -- the BddFsm of the model
    state -- a State
    ids -- a dictionary of the ids of some states. Must contain state.
    
    """
    attr = set()
    
    # Label
    attr.add("label=\"" + '\\n'.join(var+"="+val for var, val
                                     in sorted(state.
                                               get_str_values().items()))
                        + "\"")
    
    # Initial state
    if state <= fsm.init:
        attr.add("penwidth=5")
        
    # Fairness
    fair_styles = set()
    fair_attr = fairness_attr(fsm)
    for f in fsm.fairness_constraints:
        if state <= f:
            fair_styles.add(fair_attr[f])
    if len(fair_styles):
        attr.add("style=filled")
        attr.add("fillcolor=\"" + ":".join(fair_styles) + "\"")
        
    if len(attr):
        attrstr = "[" + ','.join(attr) + "]"
    else:
        attrstr = ""
    return ids[state] + " " + attrstr + ";"
    
    
def edge(fsm, source, inputs, target, ids):
    """
    Return the DOT representation of the given transition.
    
    fsm -- the BddFsm of the model
    source -- the source of the transition
    inputs -- the action of the transition, or None
    target -- the target of the transition
    ids -- a dictionary of the ids of some states.
           Must contain source and target.
    
    """
    attr = set()
    
    if inputs is not None:
        # Label
        attr.add("label=\"" + "\\n".join(var+"="+val for var, val
                                        in sorted(inputs.
                                                  get_str_values().items()))
                            + "\"")
        
        # Fairness
        fair_styles = set()
        fair_attr = fairness_attr(fsm)
        for f in fsm.fairness_constraints:
            if inputs <= f:
                fair_styles.add(fair_attr[f])
        if len(fair_styles):
            attr.add("penwidth=2")
            attr.add("color=\"" + ":".join(fair_styles) + "\"")
    
    if len(attr):
        attrstr = "[" + ','.join(attr) + "]"
    else:
        attrstr = ""
    return ids[source] + "->" + ids[target] + " " + attrstr + ";"
    

def dumpDot(fsm):
    """
    Print a DOT representation of the reachable graph of fsm.
    
    fsm -- the BddFsm of the model.
    
    """
    
    init = fsm.init & fsm.state_constraints & fsm.bddEnc.statesMask
    fair_attr = fairness_attr(fsm)
    states = fsm.pick_all_states(fsm.reachable_states)
    transitions = {(s1,i,s2)
                   for s1 in states
                   for s2 in 
                   fsm.pick_all_states(fsm.post(s1) & fsm.reachable_states)
                   for i in (fsm.pick_all_inputs(
                                         fsm.get_inputs_between_states(s1,s2))
                             if len(fsm.bddEnc.inputsVars) > 0
                             else {None})
                   if s2 <= fsm.post(s1, inputs=i)}
    
    dot = ["digraph {"]
    
    # Add states
    # Generate ids
    ids = dict()
    curid = 1
    for state in states:
        ids[state] = "s" + str(curid)
        curid = curid + 1
    
    # Print states
    for state in states:
        dot.append(node(fsm, state, ids))
      
        
    # Add transitions
    for s1, i, s2 in transitions:
        dot.append(edge(fsm, s1, i, s2, ids))
    
    dot.append("}")
    
    return "\n".join(dot)
    
    
def process(allargs):
    """
    Process program arguments and dump the model.
    Write on standard output the DOT format of the dumped model.
    
    allargs -- a sys.args-like arguments list, without script name.
    
    """
    
    # Parse arguments
    parser = argparse.ArgumentParser(description='SMV model DOT dumper.')
    # Populate arguments: for now, only the model
    parser.add_argument('model', help='the SMV model')
    args = parser.parse_args(allargs)
    
    # Initialize the model
    glob.load_from_file(args.model)
    glob.compute_model()
    fsm = glob.prop_database().master.bddFsm
    
    try:
        print(dumpDot(fsm))
    except PyNuSMVError as e:
        print("[ERROR]", str(e), file=sys.stderr)


if __name__ == '__main__':
    init_nusmv()   
    process(sys.argv[1:])
    deinit_nusmv()