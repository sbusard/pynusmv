import sys
import argparse
from pynusmv.init import init_nusmv, deinit_nusmv
from ..mas import glob
from ..atlkFO.parsing import parseATLK
from .eval import evalATLK as evalATLK_naive
from .evalGen import evalATLK as evalATLK_gen
from .evalOpt import evalATLK as evalATLK_opt
from .evalPartial import evalATLK as evalATLK_partial
from pyparsing import ParseException
from pynusmv.exception import PyNuSMVError

__implementations = {"naive" : evalATLK_naive,
                     "generator" : evalATLK_gen,
                     "optimized" : evalATLK_opt,
                     "partial" : evalATLK_partial}

def check(mas, spec, variant="SF", implem="naive"):
    """
    Return whether the system satisfies the ATLK specification.
    
    mas -- the system
    spec -- the specification, as a string
    variant -- the variant of the algorithm to evaluate strategic operators;
               must be
               * "SF" for the standard way: splitting in uniform strategies then
                 filtering winning states,
               * "FS" for the alternating way: filtering winning states, then
                 splitting one conflicting equivalence class, then recurse
               * "FSF" for the filter-split-filter way: filtering winning states
                 then splitting all remaining actions into uniform strategies,
                 then filtering final winning states.
    implem -- the particular implementation to use for strategic operators
              evaluation:
              * "naive" the naive implementation;
              * "generator" the generator-based implementation;
              * "optimized" an memory-optimized version;
              * "partial" a version based on partial strategies.
                 
    If variant is not in {"SF", "FS", "FSF"}, the standard "SF" way is used.          
    If implem is not in {"naive", "generator", "optimized", "partial"},
    the standard "naive" way is used.
    
    """
    if implem in __implementations:
        sat = __implementations[implem](mas, spec, variant=variant)
    else:
        sat = evalATLK_naive(mas, spec, variant=variant)
    return (~sat & mas.bddEnc.statesInputsMask & mas.init).is_false()
    
def process(allargs):
    """
    Process specs on the given NuSMV model.
    
    Build the model from a given file and check every ATLK specification
    writen on standard input.
    
    allargs -- a sys.args-like arguments list, without script name.
    """
    
    # Parse arguments
    parser = argparse.ArgumentParser(description='ATLK model checker.')
    # Populate arguments: for now, only the model
    parser.add_argument('model', help='the MAS as an SMV model')
    parser.add_argument('-p', dest='property', help='the property check',
                        default=None)
    parser.add_argument('-v', dest='variant', help='the variant to use',
                        default="SF")
    args = parser.parse_args(allargs)
    
    # Initialize the model
    glob.load_from_file(args.model)
    mas = glob.mas()
    
    # Check given property, if any
    if args.property:
        try:
            spec = parseATLK(args.property)[0]
            satisfied = check(mas, spec, variant=args.variant)
            print('Specification', str(spec), 'is', str(satisfied))
        except ParseException as e:
            print("[ERROR] Cannot parse specification:", str(e))
        except PyNuSMVError as e:
            print("[ERROR]", str(e))
    
    # Otherwise, ask for ATLK properties
    else:
        print("Enter ATLK properties, one by line:")
        for line in sys.stdin:
            try:
                spec = parseATLK(line)[0]
            
                satisfied = check(mas, spec)
                print('Specification', str(spec), 'is', str(satisfied))
            except ParseException as e:
                print("[ERROR] Cannot parse specification:", str(e))
            except PyNuSMVError as e:
                print("[ERROR]", str(e))


if __name__ == '__main__':
    init_nusmv()   
    process(sys.argv[1:])
    deinit_nusmv()