"""
ARCTL CLI with TLACE explanation.
"""

import cmd
import argparse

from pynusmv.init import init_nusmv, deinit_nusmv, reset_nusmv
from pynusmv.fsm import BddFsm
from pynusmv.exception import PyNuSMVError

from tools.arctl.parsing import parseArctl
from tools.arctl.check import checkArctl
from tools.arctl.tlace.explain import explain_witness, explain_countex
from tools.arctl.tlace.xml import xml_representation

from pyparsing import ParseException


class ARCTL_TLACE_shell(cmd.Cmd):

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
                (res, wit) = checkArctl(self.fsm, spec,
                                        explain_witness, explain_countex)
                if res:
                    print("The specification", arg, "is true,",
                          "witnessed by")
                    print(xml_representation(self.fsm, wit, spec))
                        
                else:
                    print("The specification", arg, "is false,",
                          "as shown by")
                    print(xml_representation(self.fsm, wit, spec))
                    
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
    ARCTL_TLACE_shell().cmdloop()