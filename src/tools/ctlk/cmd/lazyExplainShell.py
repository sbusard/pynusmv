import cmd
import re
from .lockStack import LockStack
from ..ast import Spec
from ..lazyTlace.explain import explain_branch
from ..lazyTlace.tlace import (PartialTlacenode, Tlacebranch,
                               TemporalBranch, EpistemicBranch)

class LazyCTLK_explain_shell(cmd.Cmd):
    """
    A shell for CTLK TLACE explanation.
    """
    
    def __init__(self, fsm, tlace):
        super(LazyCTLK_explain_shell, self).__init__()
        self.prompt = ">> "
        self.tlace = tlace
        self.stack = LockStack()
        self.stack.push(tlace)
        self.fsm = fsm
        
        
    def do_help(self, arg):
        """
        Display help for this shell.
        
        arg argument is ignored.
        """
        
        # Explain the syntax for explanations
        
        # Syntax follows this regex: (\.|[0-9]+)*
        
        # Semantics: show the corresponding object
        # a number selects the corresponding branch/node
        # a dot separates two numbers
        # more dots go up the tree
        
        # reset, empty line, CTRL-d
        
        help = """This prompt allows to explore the TLACE explaining a property.
It starts at the root of the TLACE and allows navigation.

The commands to navigate follow this regular expression (\.|[0-9]+)*
    a dot selects the current node/branch, more dots go up to the parent;
    a number selects the branch/node of the current item;
    
    ex: 0.3.0.1 from the root node selects the second node of the first branch
        of the third node of the first branch of the root node;
        ...1 from a node selects the second branch of the parent of its parent
        branch;
        
In addition, three commands are also provided
    reset: restart the explanation to the root node;
    an empty line: shows the current item;
    CTRL-d: quits the prompt to return to the CLI main prompt.
"""
        print(help)
        
        
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
            elements = []
            for (grp, i) in zip(re.finditer(r"\.|[0-9]+", line),
                                  range(len(line))):
                index = grp.group(0)
                elements.append(index)
                if index == ".":
                    stack.pop()
                else:
                    # Check existence and push corresponding element
                    index = int(index)
                    elem = stack.top()
                    if isinstance(elem, PartialTlacenode):
                        if index < 0 or len(elem.branches) <= index:
                            print("error: index out of range:",
                                  "".join(elements))
                            return False
                        else:
                            if isinstance(elem.branches[index], Spec):
                                elem.branches[index] = explain_branch(
                                        self.fsm, elem.state,
                                        elem.branches[index],
                                        elem.branches[index])
                            stack.push(elem.branches[index])
                    elif isinstance(elem, Tlacebranch):
                        index = index * 2
                        if index < 0 or len(elem.path) <= index:
                            print("error: index out of range",
                                  "".join(elements))
                            return False
                        else:
                            stack.push(elem.path[index])
            
        except IndexError:
            print("error: cannot find parent of root TLACE node")
            return False
        
        # Explain the branch
        try:
            elem = stack.top()
            if type(elem) is PartialTlacenode:
                print("Explaining node")
            elif type(elem) is TemporalBranch:
                print("Explaining temporal branch for", elem.specification)
            elif type(elem) is EpistemicBranch:
                print("Explaining epistemic branch for", elem.specification)
            print("-" * 80)
            self._show(elem)
        except IndexError:
            print("error: reached root TLACE node and beyond; abort")
            return False
            
        # Save the modified stack
        self.stack = stack
        
        
    def do_EOF(self, arg):
        """Quit the explanation prompt (you can hit CTRL-d)."""
        print()
        return True
            
    
    def _show(self, tlace, index=0, prev=None, prefix=None):
        """Show the given TLACE element, a TLACE node or branch."""
        prefix = prefix if prefix else ""
        if type(tlace) is PartialTlacenode:
            # State
            header = " State " + str(index) + " "
            print(header.center(40,"-"))
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
                    if isinstance(branch, Spec):
                        print(prefix + str(i),":", branch)
                    elif isinstance(branch, Tlacebranch):
                        print(prefix + str(i),":", branch.specification)
                for univ in tlace.universals:
                    print(univ)
                    
        elif type(tlace) is TemporalBranch:
            # Loop?
            if tlace.loop is not None and tlace.loop[1] == tlace.path[0]:
                header = " Loop starts here "
                print(header.center(40, "-"))
            self._show(tlace.path[0], prefix=prefix + "0.")
            # Show the states, and the inputs,
            for (os, i, s, ind) in zip(tlace.path[::2], tlace.path[1::2],
                                       tlace.path[2::2],
                                       range(len(tlace.path[2::2]))):
                # Inputs
                header = " Inputs "
                print(header.center(40, "-"))
                values = i.get_str_values()
                for var in values:
                    print(var, "=", values[var])
                # don't forget loops,
                if tlace.loop is not None and tlace.loop[1] == s:
                    header = " Loop starts here "
                    print(header.center(40, "-"))
                self._show(s, ind + 1, os, prefix + str(ind + 1) + ".")
            # Print loop inputs if needed
            if tlace.loop is not None:
                header = " Inputs "
                print(header.center(40, "-"))
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
                if len(ag) > 1:
                    header = " Agents : "
                else:
                    header = " Agent : "
                header += ",".join(ag) + " "
                print(header.center(40, "-"))
                # State
                self._show(s, ind + 1, os, prefix + str(ind + 1) + ".")