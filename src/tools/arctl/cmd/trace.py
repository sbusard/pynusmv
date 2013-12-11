"""
ARCTL CLI with trace explanation.
"""

import cmd
import argparse

from pynusmv.init import init_nusmv, deinit_nusmv, reset_nusmv
from pynusmv.fsm import BddFsm
from pynusmv.exception import PyNuSMVError

from tools.arctl.parsing import parseArctl
from tools.arctl.check import checkArctl

from pyparsing import ParseException


def print_bdd(bdd):
    """Print bdd at stdout."""
    for var, val in bdd.get_str_values().items():
        print(var, "=", val)
        
def print_path(path, inputs, loop):
    c = 0
    # print first
    if loop is not None and loop == path[0]:
        print("----- Loop starts here")
    print("----- State", str(c))
    c += 1
    print_bdd(path[0])
    # print path
    for i, s in zip(path[1::2], path[2::2]):
        print("----- Inputs")
        print_bdd(i)
        if loop is not None and loop == s:
            print("----- Loop starts here")
        print("----- State", str(c))
        c += 1
        print_bdd(s)
    # print loop
    if loop is not None:
        print("----- Inputs")
        print_bdd(inputs)
        print("----- State", str(c))
        c += 1
        print_bdd(loop)


class ARCTLshell(cmd.Cmd):

    prompt = "> "
    
    fsm = None
    fsmpath = None
    
    def preloop(self):
        init_nusmv()
        
    def postloop(self):
        deinit_nusmv()
        
        
    def do_read(self, arg):
        """
        Read file as SMV model to get FSM.
        usage: read FILEPATH
        """
        if len(arg) < 1:
            print("[ERROR] read command needs the SMV model path.")
        else:
            if self.fsm is not None:
                reset_nusmv()
            try:
                self.fsm = BddFsm.from_filename(arg)
                self.fsmpath = arg
            except Exception as e:
                print("[ERROR]", e)
            
            
    def do_check(self, arg):
        """
        Check an ARCTL specification on the current FSM.
        usage: check SPEC
        """
        if self.fsm is None:
            print("[ERROR] No FSM read. Use read command before.")
        elif arg == "":
            print("[ERROR] Need a specification.")
        else:
            try:
                spec = parseArctl(arg)[0]
                (res, (wit, (inp, loop))) = checkArctl(self.fsm, spec)
                if res:
                    print("The specification", arg, "is true,",
                          "witnessed by")
                    #print_path(wit, inp, loop)
                        
                else:
                    print("The specification", arg, "is false,",
                          "as shown by")
                    #print_path(wit, inp, loop)
                    
            except PyNuSMVError as e:
                print("[ERROR]", e)
                
            except ParseException as e:
                print("[ERROR]", e)
                
                
    def do_fsm(self, arg):
        """
        Print the path to the currently read FSM.
        usage: fsm
        """
        if self.fsmpath is None:
            print("No FSM.")
        else:
            print(self.fsmpath)
        
            
            
    def do_quit(self, arg):
        """
        Quit this prompt.
        usage: quit
        """
        return True
        
        

if __name__ == "__main__":
    ARCTLshell().cmdloop()