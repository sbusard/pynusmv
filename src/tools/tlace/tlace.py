import sys
import argparse

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd

from pynusmv.prop.propDb import PropDb
from pynusmv.prop.prop import propTypes

from tools.tlace.check import check as check_ctl_spec
from tools.tlace.xml import xml_representation


class NuSMVCommandError(Exception):
    """A NuSMV command ended with an error."""
    pass
    
    
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
    
    # Initialize NuSMV
    cinit.NuSMVCore_init_data()
    cinit.NuSMVCore_init(None, 0)
    
    
    # Initialize the model
    status = cmd.Cmd_SecureCommandExecute("read_model -i " + args.model)
    if status is not 0:
        raise NuSMVCommandError('Cannot read model ' + args.model)
    status = cmd.Cmd_SecureCommandExecute("go")
    if status is not 0:
        raise NuSMVCommandError('Cannot build the model.')
    
    # Get the FSM
    propDb = PropDb.get_global_database()
    master = propDb.master
    fsm = propDb.master.bddfsm
    
    # Check all CTL properties
    for prop in propDb:
        # TODO Check type
        if prop.type == propTypes['CTL']:
            spec = prop.exprcore
    
            (satisfied, cntex) = check_ctl_spec(fsm, spec)
            # Print the result and the TLACE if any
            print('Specification',str(spec), 'is', str(satisfied))
        
            if not satisfied:
                print(xml_representation(fsm, cntex, spec))
            
            print()
            
        
    
    # Quit NuSMV
    status = cinit.NuSMVCore_quit()
    if status != 0:
        sys.exit(1)
    

if __name__ == '__main__':    
    check_and_explain(sys.argv[1:])