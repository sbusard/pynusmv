"""
CTLK CLI with TLACE explanation.
"""

import cmd
import argparse
import re
from collections import namedtuple
from pyparsing import ParseException

from pynusmv.init.init import init_nusmv, deinit_nusmv, reset_nusmv
from pynusmv.utils.exception import PyNuSMVError

from .. import glob
from ..parsing import parseCTLK
from .check import checkCTLK
from .xml import xml_witness, xml_countex
from .tlace import Tlacenode, EpistemicBranch, TemporalBranch


Model = namedtuple("Model", ("path", "content"))


class ArgumentParsingError(Exception):
    """An error occured while parsing arguments."""
    pass

class NonExitingArgumentParser(argparse.ArgumentParser):
    """An ArgumentParser that does not exit."""
    
    def exit(self, status=0, message=None):
        raise ArgumentParsingError(message)


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
    #   check : check a spec, show a witness/countex in XML/text
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
            
        try:
            spec = parseCTLK(args.spec)[0]
            (sat, diag) = checkCTLK(self.fsm, spec)
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
        
        
    def parse_check(self):
        """Build and store the parser for the check command."""
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
            
        try:
            # If spec is not None, check and use the last spec
            # Else check spec
            # Then explain spec by entering the explain cmd
            if args.spec:
                spec = parseCTLK(args.spec)[0]
                (sat, diag) = checkCTLK(self.fsm, spec)
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
            shell = CTLK_explain_shell(diag)
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
        
        
class CTLK_explain_shell(cmd.Cmd):
    """
    A shell for CTLK TLACE explanation.
    """
    
    def __init__(self, tlace):
        super().__init__()
        self.prompt = ">> "
        self.tlace = tlace
        self.stack = LockStack()
        self.stack.push(tlace)
        
        
    def do_help(self, arg):
        """
        Display help for this shell.
        
        arg argument is ignored.
        """
        
        pass # TODO Implement
        # Explain the syntax for explanations
        
        # Syntax follows this regex: (\.|[0-9]+)*
        
        # Semantics: show the corresponding object
        # a number selects the corresponding branch/node
        # a dot separates two numbers
        # more dots go up the tree
        
        # reset, empty line, CTRL-d
        
        
    def do_reset(self, arg):
        """Reset the explanation to top TLACE node."""
        self.stack = LockStack()
        self.stack.push(self.tlace)
        self._show(self.stack.top())
        
        
    def emptyline(self):
        """Show current node/branch."""
        self._show(self.stack.top())
        return False
        
    
    def default(self, line):
        """Interpret line as a request for explanation."""
        
        # Pre-work
        self.stack.unlock()
        stack = self.stack.copy() # Copy the stack to restore it if error
        
        # Check syntax: (\.|[0-9]+)*
        if not re.match("^(\.|[0-9]+)*$", line):
            print("error: wrong format")
            return False
        
        # Check existence of requested node and branch
        try:
            elements = self._split_and_keep(line, ".")
            for (index, i) in zip(elements, range(len(elements))):
                if index == ".":
                    stack.pop()
                else:
                    # Check existence and push corresponding element
                    index = int(index)
                    elem = stack.top()
                    if type(elem) is Tlacenode:
                        if index < 0 or len(elem.branches) <= index:
                            print("error: index out of range:",
                                  "".join(elements[:i+1]))
                            return False
                        else:
                            stack.push(elem.branches[index])
                    elif type(elem) in {TemporalBranch, EpistemicBranch}:
                        index = index * 2
                        if index < 0 or len(elem.path) <= index:
                            print("error: index out of range",
                                  "".join(elements[:i+1]))
                            return False
                        else:
                            stack.push(elem.path[index])
            
        except IndexError:
            print("error: cannot find parent of root TLACE node")
            return False
        
        # Explain the branch
        try:
            elem = stack.top()
            if type(elem) is Tlacenode:
                print("Explaining node")
            elif type(elem) is TemporalBranch:
                print("Explaining temporal branch for", elem.specification)
            elif type(elem) is EpistemicBranch:
                print("Explaining epistemic branch for", elem.specification)
            self._show(elem)
        except IndexError:
            print("error: reached root TLACE node and beyond")
            return False
            
        # Save the modified stack
        self.stack = stack
        
        
    def do_EOF(self, arg):
        """Quit the explanation prompt (you can hit CTRL-d)."""
        print()
        return True
        
        
    def _split_and_keep(self, string, sep):
        """Split but keep separators."""
        values = []
        cur = ""
        for c in string:
            if c == sep:
                if cur != "":
                    values.append(cur)
                values.append(c)
                cur = ""
            else:
                cur += c
        if cur != "":
            values.append(cur)
        return values
            
    
    def _show(self, tlace, index=0, prev=None, prefix=None):
        """Show the given TLACE element, a TLACE node or branch."""
        prefix = prefix if prefix else ""
        if type(tlace) is Tlacenode:
            # State
            header = "----- State " + str(index) + " "
            header += "-" * (80 - len(header))
            print(header)
            values = tlace.state.get_str_values()
            for var in values:
                if (prev is not None and
                    prev.state.get_str_values()[var] != values[var] or
                    prev is None) :
                    print(var, "=", values[var])
            
            # Annotations
            if len(tlace.atomics) + len(tlace.branches) + len(tlace.universals):
                header = "--- Annotations"
                print(header)
                for atomic in tlace.atomics:
                    print(atomic)
                for (branch, i) in zip(tlace.branches,
                                                    range(len(tlace.branches))):
                    print(prefix + str(i),":", branch.specification)
                for univ in tlace.universals:
                    print(univ)
                    
        elif type(tlace) is TemporalBranch:
            # Loop?
            if tlace.loop is not None and tlace.loop[1] == tlace.path[0]:
                header = "----- Loop starts here "
                header += "-" * (80 - len(header))
                print(header)
            self._show(tlace.path[0], prefix=prefix + "0.")
            # Show the states, and the inputs,
            for (os, i, s, ind) in zip(tlace.path[::2], tlace.path[1::2],
                                       tlace.path[2::2],
                                       range(len(tlace.path[2::2]))):
                # Inputs
                header = "----- Inputs "
                header += "-" * (80 - len(header))
                print(header)
                values = i.get_str_values()
                for var in values:
                    print(var, "=", values[var])
                # don't forget loops,
                if tlace.loop is not None and tlace.loop[1] == s:
                    header = "----- Loop starts here "
                    header += "-" * (80 - len(header))
                    print(header)
                self._show(s, ind + 1, os, prefix + str(ind + 1) + ".")
            # Print loop inputs if needed
            if tlace.loop is not None:
                header = "----- Inputs "
                header += "-" * (80 - len(header))
                print(header)
                values = tlace.loop[0].get_str_values()
                for var in values:
                    print(var, "=", values[var])
                
        elif type(tlace) is EpistemicBranch:
            # Show the states, and the agents,
            self._show(tlace.path[0])
            for (os, ag, s, ind) in zip(tlace.path[::2], tlace.path[1::2],
                                        tlace.path[2::2],
                                        range(len(tlace.path[2::2]))):
                # Agents
                header = ("----- Agents : " +
                          (",".join(ag) if type(ag) is frozenset else str(ag)) +
                          " ")
                header += "-" * (80 - len(header))
                print(header)
                # State
                self._show(s, ind + 1, os, prefix + str(ind + 1) + ".")

class LockStack():
    """
    A lock stack is a stack with an additional lock.
    
    When popping, the lock is activated, but the top value is actually removed
    only when the lock is already activated. Whenever pushing a value, the lock
    is deactivated.
    This means that to remove the top, you have to pop twice in a raw
    (without pushing between the two pops).
    The stack can be unlocked.
    """
    
    def __init__(self):
        self.__stack = []
        self.__lock = False
        
    def pop(self):
        if len(self.__stack) <= 0:
            raise IndexError("Empty stack")
        if self.__lock:
            self.__stack.pop()
        else:
            self.__lock = True
            
    def push(self, value):
        self.__stack.append(value)
        self.__lock = False
        
    def top(self):
        if len(self.__stack) <= 0:
            raise IndexError("Empty stack")
        return self.__stack[-1]
        
    def isEmpty(self):
        return len(self.__stack) == 0
        
    def all(self):
        return tuple(self.__stack)
        
    def unlock(self):
        self.__lock = False
        
    def copy(self):
        copy = LockStack()
        for e in self.__stack:
            copy.push(e)
        return copy



if __name__ == "__main__":
    CTLK_shell().cmdloop()