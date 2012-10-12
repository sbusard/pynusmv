import cmd

from pynusmv.mc.mc import eval_simple_expression
from pynusmv.utils.exception import PyNuSMVError
from ..util.nonExitingArgumentParser import (NonExitingArgumentParser,
                                             ArgumentParsingError)

def choose_next_state(fsm, BDD):
    """
    Use an interactive prompt to ask the user to choose a State in BDD.
    
    fsm -- the model;
    BDD -- a BDD representing states of fsm.
    
    Return a new state of BDD or None if no state has been chosen.
    """
    shell = _Next_State_Shell(fsm, BDD)
    shell.cmdloop()
    return shell.chosen
    
    
class _Next_State_Shell(cmd.Cmd):
    """
    A shell allowing to choose state among a given BDD.
    
    This shell has
        the original BDD (to be able to reset)
        the FSM from which the BDD comes
            (to be able to display and pick states)
        a list of (more and more small) BDDs (to be able to backtrack)
        a bound on the number of displayed states 
            (to display only small amounts of states)
            
    This shell starts with displaying
        either the number of states if the bound is reached
        or all the possible states
            
    This shell provides some commands
        reset (remove all the constraints and restart with original BDD)
        backtrack (remove last constraints from the set of constraints)
        constraint (add constraints)
        choose (choose a state in the ones displayed, if some displayed)
        EOF (quit without getting a state)
        an empty line shows again the last bdd
        
    This shell stops when a state is chosen or EOF is entered.
    When this shell stops, self.chosen is filled with the chosen state, or
    with None if no state has been chosen (EOF entered).
    """
    
    def __init__(self, fsm, bdd, bound=10, prompt="> "):
        """
        Initialize a next state shell.
        
        fsm -- the FSM of interest;
        bdd -- the original BDD, represents a set of states of fsm;
        bound -- the maximum number of states that can be displayed;
        prompt -- the prompt of the shell.
        """
        super().__init__()
        self.prompt = prompt
        self.fsm = fsm
        self.original = bdd
        self.bound = bound
        
        self.constraints = []
        self.bdds = [self.original]
        
        self.shown = [] # The list of last shown states
        self.chosen = None # The chosen state
        
        self.parsers = {} # Parsers for commands
        
        
    def _show_last(self):
        """
        Show the last BDD.
        
        If the size of the last BDD is greater than self.bound,
        show only a message asking for constraints
        (and the size of the BDD in terms of states).
        """
        # Reset shown states
        self.shown = []
        bdd = self.bdds[-1]
        count = self.fsm.count_states(bdd)
        if count > self.bound:
            print("Too many states ("+ str(int(count)) + "), add constraints")
        else:
            prev = None
            while(bdd.isnot_false()):
                # Get the state
                s = self.fsm.pick_one_state(bdd)
                
                # Add it to the list of shown states
                self.shown.append(s)
                
                # Show the state
                header = " State " + str(len(self.shown)) + " "
                print(header.center(40,"-"))
                values = s.get_str_values()
                for var in values:
                    if (prev is not None and
                        prev.get_str_values()[var] != values[var] or
                        prev is None) :
                        print(var, "=", values[var])
                        
                prev = s
                bdd -= s
        
        
    def preloop(self):
        self._show_last()
        
    
    # For every command cmd, implement
    #   parse_cmd: create and store the parser for cmd
    #   help_cmd: display the parser help for cmd
    #   do_cmd: actually perform cmd
    
    
    def parse_choose(self):
        """Build and store the parser for the choose command."""
        # choose STATE
        # STATE is an int, corresponding to the chosen state in the previously
        # shown list
        
        if "choose" not in self.parsers:
            parser = NonExitingArgumentParser(
                        "choose",
                        description="Choose a state.",
                        add_help=False)
            parser.add_argument('state', metavar='STATE', type=int,
                                help="the id of the chosen state")
            self.parsers["choose"] = parser
        
        
    def help_choose(self):
        if "choose" not in self.parsers:
            self.parse_choose()
        self.parsers["choose"].print_help()
       
        
    def do_choose(self, arg):
        if "choose" not in self.parsers:
            self.parse_choose()
            
        # Handle arguments parsing errors
        try:
            args = self.parsers["choose"].parse_args(arg.split())
        except ArgumentParsingError as err:
            print(err, end="")
            return False
            
        if len(self.shown) <= 0:
            print("choose: error: no specified states.")
            return False
            
        state = int(args.state) - 1
        if state < 0 or len(self.shown) <= state:
            print("choose: error: unknown state.")
            return False
        else:
            self.chosen = self.shown[state]
            return True
            
            
    def emptyline(self):
        """When entering an empty line, the last BDD is shown."""
        self._show_last()
        
        
    def do_backtrack(self, arg):
        """Undo last applied constraints."""
        if len(self.bdds) <= 1:
            print("backtrack: error: cannot backtrack empty constraints.")
        else:
            self.bdds = self.bdds[:-1]
            self.constraints = self.constraints[:-1]
            self._show_last()
        return False
        
        
    def parse_constraint(self):
        """Build and store the parser for the constraints command."""
        # constraints CONSTRAINTS
        # CONSTRAINTS a simple expression representing constraints
        # on the current set of states, used to restrict it
        
        if "constraint" not in self.parsers:
            parser = NonExitingArgumentParser(
                        "constraint",
                        description="Restrict the current BDD"
                                    " with constraints.",
                        add_help=False)
            parser.add_argument('constraints', metavar='CONSTRAINTS', nargs='?',
                                help="the constraints to add; if ommitted, show current list of applied constraints")
            self.parsers["constraint"] = parser
        
    def help_constraint(self):
        if "constraint" not in self.parsers:
            self.parse_constraint()
        self.parsers["constraint"].print_help()
        
    def do_constraint(self, arg):
        if "constraint" not in self.parsers:
            self.parse_constraint()
            
        # Handle arguments parsing errors
        try:
            if arg.strip() == "":
                arg = []
            else:
                arg = [arg.strip()]
            args = self.parsers["constraint"].parse_args(arg)
        except ArgumentParsingError as err:
            print(err, end="")
            return False
            
        if args.constraints is None:
            if len(self.constraints) <= 0:
                print("Currently no constraint.")
            else:
                print("Current constraints:")
                for const in self.constraints:
                    print(const)
            return False
            
        else:
            try:
                # Parse constraints
                constBDD = eval_simple_expression(self.fsm, args.constraints)
                # Restrict the BDD
                newBdd = self.bdds[-1] & constBDD
                # If the BDD is not empty, add it to the list and show it
                if self.fsm.count_states(newBdd) > 0:
                    self.bdds.append(newBdd)
                    self.constraints.append(args.constraints)
                    self._show_last()
                else:
                    # else backtrack (in fact, do nothing)
                    print("constraint: the constraints are too strong,",
                          "no more states; retry.")
            except PyNuSMVError as err:
                print(err)
                return False
            
        return False
        
        
    def do_reset(self, arg):
        """Reset the choosing process."""
        self.constraints = []
        self.bdds = [self.original]
        self._show_last()


    def do_EOF(self, arg):
        "Quit without choosing a state."
        return True