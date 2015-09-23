import cmd

from pynusmv.mc import eval_simple_expression
from pynusmv.exception import PyNuSMVError
from ..util.nonExitingArgumentParser import (NonExitingArgumentParser,
                                             ArgumentParsingError)
from pynusmv.exception import NuSMVBddPickingError

def choose_one_state(fsm, BDD, bound=10):
    """
    Use an interactive prompt to ask the user to choose a State in BDD.
    
    fsm -- the model;
    BDD -- a BDD representing states of fsm.
    
    Return a new state of BDD or None if no state has been chosen.
    """
    shell = _One_State_Shell(fsm, BDD, bound)
    shell.cmdloop()
    return shell.chosen
    
    
def choose_next_state(fsm, state, bound=10):
    """
    Use an interactive prompt to ask the user
    to choose a successor of state in fsm.
    
    fsm -- the model;
    state -- a state of fsm.
    
    Return a new (inputs, next) pair of fsm such that next is a successor of
    state in fsm through inputs, or (None, None) if no state has been chosen.
    """
    shell = _Next_State_Shell(fsm, state, bound)
    shell.cmdloop()
    return (shell.chosenInputs, shell.chosen)
    
    
class _One_State_Shell(cmd.Cmd):
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
    
    def __init__(self, fsm, bdd, bound=10, prompt=">> "):
        """
        Initialize a one state shell.
        
        fsm -- the FSM of interest;
        bdd -- the original BDD, represents a set of states of fsm;
        bound -- the maximum number of states that can be displayed;
        prompt -- the prompt of the shell.
        """
        super(_One_State_Shell, self).__init__()
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
        states = self.fsm.pick_all_states(bdd)
        if len(states) > self.bound:
            print("Too many states ("+ str(len(states)) + "), add constraints")
        else:
            prev = None
            for state in states:
                self.shown.append(state)
                # Show the state
                header = " State " + str(len(self.shown)) + " "
                print(header.center(40, "-"))
                values = state.get_str_values()
                for var in values:
                    if (prev is not None and
                        prev.get_str_values()[var] != values[var] or
                        prev is None) :
                        print(var, "=", values[var])
                        
                prev = state
        
        
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
            return self._choose(state)
            
    
    def _choose(self, index):
        """
        Choose the state with ID index.
        
        If index is a correct ID, set self.chosen to the state, return True;
        otherwise don't set self.chosen and return False.
        """
        if index < 0 or len(self.shown) <= index:
            print("choose: error: unknown state.")
            return False
        else:
            self.chosen = self.shown[index]
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
        
        
class _Next_State_Shell(_One_State_Shell):
    """
    A shell allowing to choose a successor of a given state in a given FSM.
    
    This shell differs from _One_State_Shell by
        - getting a state of the FSM instead of a BDD;
        - choosing a successor of this state;
        - displaying possible inputs to reach each possible state;
        - chosen item can be a tuple (inputs, state) instead of just a state,
          if there are input variables in the model. In this case, self.chosen
          is the state and self.chosenInputs is the inputs (can be None).
    """
    
    def __init__(self, fsm, state, bound=10, prompt=">> "):
        """
        Initialize a one state shell.
        
        fsm -- the FSM of interest;
        bdd -- the state of interest, belongs to fsm;
        bound -- the maximum number of states that can be displayed;
        prompt -- the prompt of the shell.
        """
        
        self.state = state
        self.chosenInputs = None
        bdd = fsm.post(state)
        super(_Next_State_Shell, self).__init__(fsm, bdd, bound, prompt)
        
        
    def _show_last(self):
        """
        Show the last BDD.
        
        If the size of the last BDD is greater than self.bound,
        show only a message asking for constraints
        (and the size of the BDD in terms of states).
        """
        
        def show_state_or_inputs(item, prev = None):
            values = item.get_str_values()
            for var in values:
                if (prev is not None and
                    prev.get_str_values()[var] != values[var] or
                    prev is None) :
                    print(var, "=", values[var])
        
        # Reset shown states
        self.shown = []
        bdd = self.bdds[-1]
        states = self.fsm.pick_all_states(bdd)
        if len(states) > self.bound:
            print("Too many states ("+ str(len(states)) + "), add constraints")
        else:
            prev = None
            for state in states:
                # Get inputs
                inputs = self.fsm.get_inputs_between_states(self.state, state)
                
                try:
                    inputs = self.fsm.pick_all_inputs(inputs)
                    
                    print(" State ".center(40, "-"))
                    show_state_or_inputs(state, prev)
                    print(" Reachable through ".center(40,"-"))
                    previ = None
                    for inp in inputs:
                        self.shown.append((inp,state))
                        print((" Inputs " + str(len(self.shown)) + " ").
                              center(40, "-"))
                        show_state_or_inputs(inp, previ)
                        previ = inp
                    
                except NuSMVBddPickingError:
                    # Cannot get inputs, so no inputs
                    self.shown.append((None, state))
                    # Show the state
                    header = " State " + str(len(self.shown)) + " "
                    print(header.center(40, "-"))
                    show_state_or_inputs(state, prev)
                        
                prev = state
                
                
    def _choose(self, index):
        """
        Choose the (inputs, state) pair with ID index.
        
        If index is a correct ID, set self.chosen to the state and self.inputs
        to the inputs, return True;
        otherwise don't set self.chosen and self.inputs and return False.
        """
        if index < 0 or len(self.shown) <= index:
            print("choose: error: unknown state.")
            return False
        else:
            self.chosenInputs, self.chosen = self.shown[index]
            return True