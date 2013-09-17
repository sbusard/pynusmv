import sys
import argparse
from pynusmv.init import init_nusmv, deinit_nusmv
from ..mas import glob
from .parsing import parseATLK
from .eval import evalATLK
from pyparsing import ParseException
from pynusmv.exception import PyNuSMVError
    
    
def check(mas, spec):
    """
    Return whether the system satisfies the ATLK specification.
    
    mas -- the system
    spec -- the specification, as a string
    
    """
    sat = evalATLK(mas, spec)
    return (~sat & mas.bddEnc.statesInputsMask & mas.init).is_false()  
    
def process(allargs):
    """
    Check specs on the given NuSMV model.
    
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
    args = parser.parse_args(allargs)
    
    # Initialize the model
    glob.load_from_file(args.model)
    mas = glob.mas()
    
    # Check given property, if any
    if args.property:
        try:
            spec = parseATLK(args.property)[0]
            satisfied = check(mas, spec)
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