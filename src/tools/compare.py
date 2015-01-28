from pynusmv.init import *
from pynusmv.glob import *
import sys
import argparse

"""
Compare two NuSMV model and print differences.
"""

def states_transitions(fsm):
    """
    Given a BddFsm, return the set of states and transitions of the reachable
    state-space of the fsm.
    
    The returned transitions are triplets s, i, s' telling that s leads to
    s' through i.
    If the model has no input variables, i is None.
    """
    states = fsm.pick_all_states(fsm.reachable_states)
    transitions = {(frozenset(s1.get_str_values().items()),
                    frozenset(i.get_str_values().items())
                    if i is not None else frozenset({None}),
                    frozenset(s2.get_str_values().items())
                   )
                   for s1 in states
                   for s2 in states
                   for i in (fsm.pick_all_inputs(
                                         fsm.get_inputs_between_states(s1,s2))
                             if len(fsm.bddEnc.inputsVars) > 0
                             else {None})}
    states = {frozenset(s.get_str_values().items()) for s in states}
    return states, transitions

def compare(model1, model2, comparisons=None):
    """
    Load and retrieve reachable state-space of model1 and model2 and print
    differences between the two.
    
    The compared aspects are the following:
        - states: which states are reachable in one model but not in the other.
        - common: which transitions are in one model but not in the other,
                  considering only states that are in both models.
        - transitions: which transitions (with input variables) are possible
                       in one model but not in the other.
    If comparisons is None, all statistics are printed; otherwise, only the
    ones present in comparisons are printed.
    
    model1 and model2 must be loadable by PyNuSMV.
    """
    
    if comparisons is None:
        comparisons = {"states", "common", "transitions"}
    
    init_nusmv()
    load(model1)
    compute_model()
    fsm = prop_database().master.bddFsm
    s1, t1 = states_transitions(fsm)
    reset_nusmv()
    load(model2)
    compute_model()
    fsm = prop_database().master.bddFsm
    s2, t2 = states_transitions(fsm)
    deinit_nusmv()
    
    ct12 = {(s, i, sp) for s, i, sp in t1 if s in s2}
    ct21 = {(s, i, sp) for s, i, sp in t2 if s in s1}
    
    if "states" in comparisons:
        print("=" * 79)
        print("FIRST  HAS {} REACHABLE STATES".format(len(s1)))
        print("SECOND HAS {} REACHABLE STATES".format(len(s2)))
        
        if len(s1 - s2) > 0:
            states = ["\n".join(str(var) + " = " + str(val)
                                for var, val in sorted(s))
                      for s in s1 - s2]
            
            title = ("STATES IN FIRST BUT NOT IN SECOND ({})".
                     format(len(states)))
            print("=" * 79)
            print(title)
            print("=" * 79)
            print(("\n" + "-" * 79 + "\n").join(states))
        
        if len(s2 - s1) > 0:
            states = ["\n".join(str(var) + " = " + str(val)
                                for var, val in sorted(s))
                      for s in s2 - s1]
            
            title = ("STATES IN SECOND BUT NOT IN FIRST ({})".
                     format(len(states)))
            print("=" * 79)
            print(title)
            print("=" * 79)
            print(("\n" + "-" * 79 + "\n").join(states))
    
    if "common" in comparisons:
        print("=" * 79)
        print("FIRST  HAS {} REACHABLE TRANSITIONS FROM COMMON STATES".format(
              len(ct12)))
        print("SECOND HAS {} REACHABLE TRANSITIONS FROM COMMON STATES".format(
              len(ct21)))
        
        if len(ct12 - ct21) > 0:
            transitions = ["\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(s)) +
                           (("\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(i)))
                            if None not in i else "") +
                           "\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(sp))
                      for s, i, sp in ct12 - ct21]
            
            title = ("TRANSITIONS IN FIRST BUT NOT IN SECOND ({})".
                     format(len(transitions)))
            print("=" * 79)
            print(title)
            print("=" * 79)
            print(("\n" + "-" * 79 + "\n").join(transitions))
        
        if len(ct21 - ct12) > 0:
            transitions = ["\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(s)) +
                           (("\n" + "-" * 79 + "\n" +
                            ("\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(i))))
                            if None not in i else "") +
                           "\n" + "-" * 79 + "\n" + 
                           "\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(sp))
                      for s, i, sp in ct21 - ct12]
            
            title = ("TRANSITIONS IN SECOND BUT NOT IN FIRST ({})".
                     format(len(transitions)))
            print("=" * 79)
            print(title)
            print("=" * 79)
            print(("\n" + ("-" * 79 + "\n") * 2).join(transitions))
    
    if "transitions" in comparisons:
        print("=" * 79)
        print("FIRST  HAS {} REACHABLE TRANSITIONS".format(len(t1)))
        print("SECOND HAS {} REACHABLE TRANSITIONS".format(len(t2)))
        
        if len(t1 - t2) > 0:
            transitions = ["\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(s)) +
                           (("\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(i)))
                            if None not in i else "") +
                           "\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(sp))
                      for s, i, sp in t1 - t2]
            
            title = ("TRANSITIONS IN FIRST BUT NOT IN SECOND".
                     format(len(transitions)))
            print("=" * 79)
            print(title)
            print("=" * 79)
            print(("\n" + "-" * 79 + "\n").join(transitions))
        
        if len(t2 - t1) > 0:
            transitions = ["\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(s)) +
                           (("\n" + "-" * 79 + "\n" +
                            ("\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(i))))
                            if None not in i else "") +
                           "\n" + "-" * 79 + "\n" + 
                           "\n".join(str(var) + " = " + str(val)
                                     for var, val in sorted(sp))
                      for s, i, sp in t2 - t1]
            
            title = ("TRANSITIONS IN SECOND BUT NOT IN FIRST ({})".
                     format(len(transitions)))
            print("=" * 79)
            print(title)
            print("=" * 79)
            print(("\n" + ("-" * 79 + "\n") * 2).join(transitions))
        
if __name__ == "__main__":
    # Parse arguments
    parser = argparse.ArgumentParser(description='SMV model comparator.')
    # Populate arguments:
    # first model, second model
    # -s show states
    # -t show transitions
    # -c show transitions from common states
    parser.add_argument('first', help='the first SMV model')
    parser.add_argument('second', help='the second SMV model')
    parser.add_argument('-s', action='store_true',
                        help="Show states comparison")
    parser.add_argument('-t', action='store_true',
                        help="Show transitions comparison")
    parser.add_argument('-c', action='store_true',
                        help="Show transitions from common states comparison")
    args = parser.parse_args(sys.argv[1:])
    
    comparisons = set()
    if args.s:
        comparisons.add("states")
    if args.t:
        comparisons.add("transitions")
    if args.c:
        comparisons.add("common")
    if len(comparisons) <= 0:
        comparisons = None
    
    
    compare(args.first, args.second, comparisons)