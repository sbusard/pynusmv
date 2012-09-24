"""
CTLK CLI with TLACE explanation.
"""

import cmd
import argparse
from collections import namedtuple
from pyparsing import ParseException

from pynusmv.init.init import init_nusmv, deinit_nusmv, reset_nusmv
from pynusmv.utils.exception import PyNuSMVError

from .. import glob
from ..parsing import parseCTLK
from .check import checkCTLK
from .xml import xml_witness, xml_countex


Model = namedtuple("Model", ("path", "content"))


class ArgumentParsingError(Exception):
    """An error occured while parsing arguments."""
    pass

class NonExitingArgumentParser(argparse.ArgumentParser):
    """An ArgumentParser that does not exit."""
    
    def exit(self, status=0, message=None):
        raise ArgumentParsingError(message)


class CTLK_shell(cmd.Cmd):

    def __init__(self):
        super().__init__()
        self.prompt = "> "
        self.argparsers = {}
        self.fsm = None
        self.model = None
    
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
    #   check : check a spec, show a witness/countex in XML/text
    # use command arguments to choose aternatives if needed
    
    # Catch PyNuSMVError to show a message on the prompt
    
    def do_model(self, arg):
        if "model" not in self.argparsers:
            self.parse_model()
        
        # Handle arguments parsing errors
        try:
            args = self.argparsers["model"].parse_args(self._split(arg))
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
            args = self.argparsers["check"].parse_args(self._split(arg))
        except ArgumentParsingError as err:
            print(err, end="")
            return False
            
        try:
            spec = parseCTLK(args.spec)[0]
            (sat, diag) = checkCTLK(self.fsm, spec)
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
        
        
    def parse_check(self):
        """Build and store the parser for the model command."""
        # check [-d] [-x] SPEC
        # check the specification SPEC and provide a diagnostic
        # -d show the diagnostics
        # -x show the diagnostics as XML instead of text
        
        if "check" not in self.argparsers:
            parser = NonExitingArgumentParser(
                        "check",
                        description="Check a specification.",
                        add_help=False)
            parser.add_argument('spec', metavar='"SPEC"',
                                help="the specification to check")
            parser.add_argument('-d', action='store_true', dest="diagnostic",
                                help="show a diagnostic in XML")
            self.argparsers["check"] = parser
                                
            
            
    def do_reset(self, args):
        """Reset the prompt; forget the read model and restart NuSMV."""
        self.fsm = None
        self.model = None        
        glob.reset_globals()
        reset_nusmv()
            
            
    def do_quit(self, arg):
        """Quit the prompt."""
        return True
        
        
    def _split(self, string, sep=None, escape=None):
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