"""
CTLK CLI with TLACE explanation.
"""

import cmd
import argparse
from collections import namedtuple
from pyparsing import ParseException

from pynusmv.init.init import init_nusmv, deinit_nusmv, reset_nusmv
from pynusmv.utils.exception import PyNuSMVError

from .lazyExplainShell import LazyCTLK_explain_shell
from .explainShell import CTLK_explain_shell

from ..util.nonExitingArgumentParser import (NonExitingArgumentParser,
                                             ArgumentParsingError)
from .. import glob
from ..parsing import parseCTLK
from ..tlace.check import checkCTLK
from ..tlace.xml import xml_witness, xml_countex
from ..lazyTlace.check import checkCTLK as lazyCheckCTLK


Model = namedtuple("Model", ("path", "content"))


class CTLK_shell(cmd.Cmd):
    """
    A shell for CTLK TLACE usage.
    """

    def __init__(self):
        super().__init__()
        self.prompt = "> "
        self.argparsers = {}
        self.fsm = None
        self.model = None
        self.last = None
    
    def preloop(self):
        init_nusmv()
        
    def postloop(self):
        deinit_nusmv()
        
    # For each command cmd
    #   implement parse_cmd : create, store, return the arg parser
    #   implement do_cmd : gets args with , then do the job
    #   implement help_cmd : print arg parser usage
    
    # Commands list
    #   read : read a new model; error if a model is read
    #   fsm : print the path to the read model, or the read model
    #   reset : reset NuSMV, set read model to None
    #   check : check a spec
    #   explain : explain why a spec violated/satisfied
    # use command arguments to choose aternatives if needed
    
    # Catch PyNuSMVError to show a message on the prompt
    
    def do_model(self, arg):
        if "model" not in self.argparsers:
            self.parse_model()
        
        # Handle arguments parsing errors
        try:
            args = self.argparsers["model"].parse_args(self._split_escaped(arg))
        except ArgumentParsingError as err:
            print(err, end="")
            return False
        
        # Error if
        #   showing path or content when nothing is read
        #   reading a file when one is already read
        
        if (args.content or args.path is None) and self.model is None:
            print("No read model.")
            return False
        
        if args.path is not None and self.model is not None:
            print("model: error: a model is already read from", self.model.path)
            return False
            
        if args.path is None:
            print("read model:", self.model.path)
            
        if args.content:
            print(self.model.content)
            
        if args.path is not None:
            try:
                f = open(args.path, 'r')
                content = f.read()
                f.close()
                glob.load_from_file(args.path)
                self.fsm = glob.mas()
                self.model = Model(args.path, content)
            except PyNuSMVError as err:
                print("model: error:", err)
            except IOError as err:
                print("model: error:", err)
        

    def help_model(self):
        if "model" not in self.argparsers:
            self.parse_model()
        self.argparsers["model"].print_help()
        
    
    def parse_model(self):
        """Build and store the parser for the model command."""
        # model [-c] [PATH]
        # read the model at PATH
        # if PATH is omitted, show the last read model
        # if -c is given, show the content of the read model
        
        if "model" not in self.argparsers:
            parser = NonExitingArgumentParser(
                        "model",
                        description="Read a model.",
                        add_help=False)
            parser.add_argument('path', metavar='PATH',
                                nargs='?', help="the file of the model;"
                                " if omitted, show the current model path")
            parser.add_argument('-c', action='store_true', dest="content",
                                help="show the current model content")
            self.argparsers["model"] = parser
            
            
            
    def do_check(self, arg):
        if "check" not in self.argparsers:
            self.parse_check()
        
        # Error if try to check when no fsm is read
        if self.fsm is None:
            print("check: error: no read model.")
            return False
        
        # Handle arguments parsing error
        try:
            args = self.argparsers["check"].parse_args(self._split_escaped(arg))
        except ArgumentParsingError as err:
            print(err, end="")
            return False
              
        # Check -d and -l options
        if args.diagnostic and args.lazy:
            print("check: error: cannot display a diagnostics if lazy.")
            return False
            
        try:
            spec = parseCTLK(args.spec)[0]
            if not args.lazy:
                (sat, diag) = checkCTLK(self.fsm, spec)
            else:
                (sat, diag) = lazyCheckCTLK(self.fsm, spec)
                
            # Store the result
            self.last = (spec, sat, diag)
            
            print("The formula", '"' + str(spec) + '"', "is {}."
                                            .format("true" if sat else "false"))
            xml = xml_witness if sat else xml_countex
            if args.diagnostic:
                print(xml(self.fsm, diag, spec))
                
        except ParseException as err:
            print("check: error:", err)
        except PyNuSMVError as err:
            print("check: error:", err)
            
        
    def help_check(self):
        if "check" not in self.argparsers:
            self.parse_check()
        self.argparsers["check"].print_help()
        
        # Print CTLK grammar
        print()
        print("""
Grammar for CTLK properties

phi         := atom | logical | temporal | epistemic

logical     := '~'phi | '('phi')' | phi '&' phi | phi '|' phi |
               phi '->' phi | phi '<->' phi
               
temporal    := 'EX' phi | 'EF' phi | 'EG' phi |
               'E[' phi 'U' phi ']' | 'E[' phi 'W' phi ']' |
               'AX' phi | 'AF' phi | 'AG' phi |
               'A[' phi 'U' phi ']' | 'A[' phi 'W' phi ']'
               
epistemic   := 'nK<' agent '>' phi | 'nE<' group '>' phi |
               'nD<' group '>' phi | 'nC<' group '> phi |
               'K<' agent '>' phi | 'E<' group '>' phi |
               'D<' group '>' phi | 'C<' group '> phi

atom being any string surrounded by single quotes (');
agent being an atom and group a comma-separated list of agents.""")
        
        
    def parse_check(self):
        """Build and store the parser for the check command."""
        # check SPEC
        # check the specification SPEC and provide a diagnostic
        
        if "check" not in self.argparsers:
            parser = NonExitingArgumentParser(
                        "check",
                        description="Check a specification.",
                        add_help=False)
            parser.add_argument('spec', metavar='"SPEC"',
                                help="the specification to check")
            parser.add_argument('-d', action='store_true', dest="diagnostic",
                                help="show a diagnostic in XML")
            parser.add_argument('-l', action='store_true', dest="lazy",
                                help="produce a lazy explanation" +
                                     " (cannot be used with -d)")
            self.argparsers["check"] = parser
            
            
    def do_explain(self, arg):
        if "explain" not in self.argparsers:
            self.parse_explain()
        
        # Error if try to check when no fsm is read
        if self.fsm is None:
            print("explain: error: no read model.")
            return False
        
        # Handle arguments parsing error
        try:
            args = (self.argparsers["explain"].
                                           parse_args(self._split_escaped(arg)))
        except ArgumentParsingError as err:
            print(err, end="")
            return False
            
        # Print a warning if lazy and no given specification
        if args.lazy and not args.spec:
            print("explain: warning:",
                  "lazy is ignored since no specification is given.")
            
        try:
            # If spec is not None, check and use the last spec
            # Else check spec
            # Then explain spec by entering the explain cmd
            if args.spec:
                spec = parseCTLK(args.spec)[0]
                if not args.lazy:
                    (sat, diag) = checkCTLK(self.fsm, spec)
                else:
                    (sat, diag) = lazyCheckCTLK(self.fsm, spec)
                self.last = (spec, sat, diag)
                
            if self.last is None:
                print("explain: error: no specification to check.")
                return False
                
            spec = self.last[0]
            sat = self.last[1]
            diag = self.last[2]
            print("The formula", '"' + str(spec) + '"', "is {}."
                                            .format("true" if sat else "false"))
            
            # Start a CTLK explain shell
            if not args.lazy:
                shell = CTLK_explain_shell(diag)
            else:
                shell = LazyCTLK_explain_shell(self.fsm, diag)
            shell.cmdloop()
        except ParseException as err:
            print("check: error:", err)
        except PyNuSMVError as err:
            print("check: error:", err)
            
        
    def help_explain(self):
        if "explain" not in self.argparsers:
            self.parse_explain()
        self.argparsers["explain"].print_help()
        
        
    def parse_explain(self):
        """Build and store the parser for the explain command."""
        # explain ["SPEC"]
        # check the specification SPEC and provide a diagnostic
        # if SPEC is not given, explains the last checked specification
        
        if "explain" not in self.argparsers:
            parser = NonExitingArgumentParser(
                        "explain",
                        description="Check a specification and explain it"
                                    " with an interacive shell.",
                        add_help=False)
            parser.add_argument('spec', metavar='"SPEC"', nargs="?",
                                help="the specification to check;"
                                     " if omitted, explain the last checked "
                                     "specification")
            parser.add_argument('-l', action='store_true', dest="lazy",
                                help="explain with a lazy explanation" +
                                     " (ignored if no given specification)")
            self.argparsers["explain"] = parser
                                
            
            
    def do_reset(self, args):
        """Reset the prompt; forget the read model and restart NuSMV."""
        self.fsm = None
        self.model = None
        self.last = None      
        glob.reset_globals()
        reset_nusmv()
    
    
    def do_EOF(self, arg):
        """Quit the prompt (you can hit CTRL-d)."""
        return True
            
            
    def do_quit(self, arg):
        """Quit the prompt."""
        return True
        
        
    def _split_escaped(self, string, sep=None, escape=None):
        """
        Split string with each occurrence of sep,
        except if this occurrence is between two occurrences of escape.
        """
        sep = sep if sep else [" "]
        escape = escape if escape else '"'
        
        res = []
        cur = ""
        esc = False
        for char in string:
            if char == escape:
                esc = not esc
            elif char in sep and not esc:
                if cur != "":
                    res.append(cur)
                    cur = ""
            else:
                cur += char
        if cur != "":
            res.append(cur)
        return res



if __name__ == "__main__":
    CTLK_shell().cmdloop()