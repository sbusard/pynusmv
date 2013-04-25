import sys
import argparse
from pynusmv.init import init_nusmv, deinit_nusmv
from ..mas import glob
from .parsing import parseATL
from .eval import evalATL
from pyparsing import ParseException
from pynusmv.exception import PyNuSMVError
    
    
def check(allargs):
    """
    Check specs on the given NuSMV model.
    
    Build the model from a given file and check every ATL specification
    writen on standard input.
    
    allargs -- a sys.args-like arguments list, without script name.
    """
    
    # Parse arguments
    parser = argparse.ArgumentParser(description='ATL model checker.')
    # Populate arguments: for now, only the model
    parser.add_argument('model', help='the MAS as an SMV model')
    args = parser.parse_args(allargs)
    
    # Initialize the model
    glob.load_from_file(args.model)
    mas = glob.mas()
    
    # Check all ATLK properties
    print("Enter ATL properties, one by line:")
    for line in sys.stdin:
        try:
            spec = parseATL(line)[0]
            
            sat = evalATL(mas, spec)
            satisfied = (~sat & mas.init).is_false()
            print('Specification', str(spec), 'is', str(satisfied))
        except ParseException as e:
            print("[ERROR] Cannot parse specification:", str(e))
        except PyNuSMVError as e:
            print("[ERROR]", str(e))


if __name__ == '__main__':
    init_nusmv()   
    check(sys.argv[1:])
    deinit_nusmv()