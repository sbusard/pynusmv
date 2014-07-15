import sys
import argparse

from pynusmv.prop import PropDb
from pynusmv.prop import propTypes
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob
from pynusmv.fsm import BddFsm

from tools.tlace.check import check as check_ctl_spec
from tools.tlace.xml import xml_representation
    
    
def check_and_explain(allargs):
    """
    Check specs on the given NuSMV model and compute TLACEs when needed.
    
    Build the model from a given file, check every CTL spec in it
    and compute and store TLACEs when needed.
    
    allargs -- a sys.args-like arguments list, without script name.
    """
    
    # Parse arguments
    parser = argparse.ArgumentParser(description='CTL model checker '
                                                 'with TLACE generation.')
    # Populate arguments: for now, only the model
    parser.add_argument('model', help='the NuSMV model with specifications')
    args = parser.parse_args(allargs)
    
    # Initialize the model
    fsm = BddFsm.from_filename(args.model)
    propDb = glob.prop_database()
    
    # Check all CTL properties
    for prop in propDb:
        #  Check type
        if prop.type == propTypes['CTL']:
            spec = prop.exprcore
    
            (satisfied, cntex) = check_ctl_spec(fsm, spec)
            # Print the result and the TLACE if any
            print('Specification',str(spec), 'is', str(satisfied),
                  file=sys.stderr)
        
            if not satisfied:
                print(xml_representation(fsm, cntex, spec))
            
            print()


if __name__ == '__main__': 
    # Initialize NuSMV
    init_nusmv()   
    check_and_explain(sys.argv[1:])
    # Quit NuSMV
    deinit_nusmv()