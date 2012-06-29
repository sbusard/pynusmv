import sys

class NuSMVCommandError(Exception):
    """An error occured during NuSMV command execution."""
    pass
    
    
def check_and_explain(allargs):
    """
    Check specs on the given NuSMV model and compute TLACEs when needed.
    
    Build the model from a given file, check every CTL spec in it
    and compute and store TLACEs when needed.
    
    allargs -- a sys.args-like arguments list.
    """
    
    # Parse arguments
    parser = argparse.ArgumentParser(description='CTL model checker '
                                                 'with TLACE generation.')
    # TODO Populate arguments: for now, only the model
    args = parser.parse_args(allargs)
    
    # Initialize NuSMV
    # TODO Check that everything is OK
    cinit.NuSMVCore_init_data()
    cinit.NuSMVCore_init(None, 0)
    
    
    # TODO Initialize the model
    
    # TODO Get all CTL properties
    
    # TODO Check them
        
    
    # Quit NuSMV
    # TODO Check that everything is OK
    cinit.NuSMVCore_quit()
    
    
if __name__ == '__main__':
    check_and_explain(sys.argv)