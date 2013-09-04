from pynusmv.dd import BDD

from .eval import ceg, ceu, cew
from tools.explanation.explanation import Explanation

def explain_cex(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies <agents> X phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> X phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies <agents> X phi, we have to exhibit an action
    # of agents such that all reached states are in phi.
    # We choose to execute this action, that is, exhibit the state, and
    # all the reached states through the action.
    
    # Cubes
    gamma_cube = fsm.inputs_cube_for_agents(agents)
    ngamma_cube = fsm.bddEnc.inputsCube - gamma_cube
    
    # Get the winning actions, for all state
    si = fsm.pre_strat_si(phi, agents)
    # The winning actions from state are the ones belonging to its protocol
    # and to si
    actions = (si & (fsm.protocol(agents) & state) &
               fsm.bddEnc.inputsMask).forsome(fsm.bddEnc.statesCube)
    fullaction = fsm.pick_one_inputs(actions)
    ag_action = fullaction.forsome(ngamma_cube)
    
    
    # Build the graph
    successors = set()
    for successor in fsm.pick_all_states(fsm.post(state, ag_action) &
                                         fsm.bddEnc.statesMask):
        nextexpl = Explanation(successor)
        allactions = fsm.get_inputs_between_states(state, successor) & ag_action 
        for action in fsm.pick_all_inputs(allactions & fsm.bddEnc.inputsMask):
            successors.add((action, nextexpl))
    return Explanation(state, successors)
    
    
def explain_ceu(fsm, state, agents, phi, psi, states=None):
    """
    Explain why state of fsm satisfies <agents> phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> phi U psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi;
    states -- a dictionary of state->explanation pairs.
    """
    # <agents> phi U psi = mu Y. psi | (phi & fsm.pre_strat(Y, agents))
    
    # To show that state satisfies <agents> phi U psi
    #   if state in psi, return Explanation(state) because state is its own
    #   explanation
    #   otherwise, compute the fixpoint, until state is in the last set
    #   then choose one action always leading to states of the previous set
    #   (it is possible because state is added because it can go one step in
    #   the right direction), and recursively explain why the found states
    #   satisfy <agents> phi U psi.
    
    # states is used to store already explained states and create a directed
    # acyclic graph instead of the pure execution tree, to avoid a blow up
    # in terms of number of states.
    
    if states is None:
        states = {}
    
    if state in states:
        return states[state]
    
    if state <= psi:
        new = Explanation(state)
        states[state] = new
        return new
        
    else:
        f = lambda Y : psi | (phi & fsm.pre_strat(Y, agents))
        old = BDD.false(fsm.bddEnc.DDmanager)
        new = psi
        while not state <= new:
            old = new
            new = f(old)
        
        expl = explain_cex(fsm, state, agents, old)
        
        successors = set()
        for action, succ in expl.successors:
            successors.add((action,
                            explain_ceu(fsm, succ.state, agents,
                                        phi, psi, states)))
        new = Explanation(state, successors)
        states[state] = new
        return new
    
    
def explain_ceg(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies <agents> G phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> G phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies <agents> G phi
    # we just have to explain why state satisfies <agents>X<agents>G phi
    # and iterate until we reach the fixpoint.
    # By keeping track of already visited states,
    # we know that we will finally extract the full sub-system of phi states.
    
    # Get CEG(phi)
    cegphi = ceg(fsm, agents, phi)
    
    # Get all states and transitions
    extract = {state}
    states = set()
    transitions = set()
    while len(extract) > 0:
        cur = extract.pop()
        states.add(cur)
        expl = explain_cex(fsm, cur, agents, cegphi)
        for act, succ in expl.successors:
            transitions.add((cur, act, succ.state))
            if succ.state not in states:
                extract.add(succ.state)
    
    # Build the graph
    nodes = {}
    for s in states:
        nodes[s] = Explanation(s)
    for cur, act, succ in transitions:
        nodes[cur].successors.add((act, nodes[succ]))
    
    return nodes[state]
    
    
def explain_cew(fsm, state, agents, phi, psi):
    """
    Explain why state of fsm satisfies <agents> phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying <agents> phi W psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    # To show that state satisfies <agents> phi W psi
    # either it satisfies psi and we are done,
    # or we have to explain why state satisfies <agents>X<agents>phi W psi
    # and iterate until we reach the fixpoint or stop at psi.
    # By keeping track of already visited states,
    # we know that we will finally extract the full sub-system.
    
    # Get CEW(phi,psi)
    cewpp = cew(fsm, agents, phi, psi)
    
    # Get all states and transitions
    extract = {state}
    states = set()
    transitions = set()
    while len(extract) > 0:
        cur = extract.pop()
        states.add(cur)
        if not cur <= psi:
            expl = explain_cex(fsm, cur, agents, cewpp)
            for act, succ in expl.successors:
                transitions.add((cur, act, succ.state))
                if succ.state not in states:
                    extract.add(succ.state)
    
    # Build the graph
    nodes = {}
    for s in states:
        nodes[s] = Explanation(s)
    for cur, act, succ in transitions:
        nodes[cur].successors.add((act, nodes[succ]))
    
    return nodes[state]
    
    
def explain_cax(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies [agents] X phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] X phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies [agents] X phi, we have to show that
    # for each action of agents, there is a completion by other agents such that
    # the reached state satisfies phi.
    # That is, we have to take all actions and of agents and for each of them
    # exhibit a successor satisfying phi.
    
    # Cubes
    gamma_cube = fsm.inputs_cube_for_agents(agents)
    ngamma_cube = fsm.bddEnc.inputsCube - gamma_cube
    
    # Get all different actions of agents
    ag_actions = set()
    for action in fsm.pick_all_inputs((fsm.protocol(agents) &
                                         state).forsome(fsm.bddEnc.statesCube)):
        ag_actions.add(action.forsome(ngamma_cube))
    
    # Get successors
    successors = set()
    for ag_action in ag_actions:
        # Choose one successor through ag_action
        succ = fsm.pick_one_state(fsm.post(state, ag_action) & phi &
                                  fsm.bddEnc.statesMask)
        # Get the full action
        act = fsm.pick_one_inputs(fsm.get_inputs_between_states(state, succ) &
                                  ag_action & fsm.bddEnc.inputsMask)
        successors.add((act, Explanation(succ)))
    
    return Explanation(state, successors)    
    
    
def explain_cau(fsm, state, agents, phi, psi, states=None):
    """
    Explain why state of fsm satisfies [agents] phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] phi U psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi;
    states -- a dictionary of state->explanation pairs.
    """
    # [agents] phi U psi = mu Y. psi | (phi & fsm.pre_nstrat(Y, agents))
    
    # To show that state satisfies [agents] phi U psi
    #   if state in psi, return Explanation(state) because state is its own
    #   explanation
    #   otherwise, compute the fixpoint, until state is in the last set
    #   then show that all actions always lead to states of the previous set
    #   (it is possible because state is added because it cannot avoid to go
    #   one step in the right direction), and recursively explain why the found
    #   states satisfy [agents] phi U psi.
    
    # states is used to store already explained states to create a directed
    # acyclic graph instead of the pure execution tree.
    
    if states is None:
        states = {}
        
    if state in states:
        return states[state]
    
    if state <= psi:
        new = Explanation(state)
        states[state] = new
        return new
        
    else:
        f = lambda Y : psi | (phi & fsm.pre_nstrat(Y, agents))
        old = BDD.false(fsm.bddEnc.DDmanager)
        new = psi
        while not state <= new:
            old = new
            new = f(old)
        
        expl = explain_cax(fsm, state, agents, old)
        
        successors = set()
        for action, succ in expl.successors:
            successors.add((action,
                            explain_cau(fsm, succ.state, agents,
                                        phi, psi, states)))
        new = Explanation(state, successors)
        states[state] = new
        return new
    
    
def explain_cag(fsm, state, agents, phi):
    """
    Explain why state of fsm satisfies [agents] G phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] G phi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies [agents] G phi
    # we just have to explain why state satisfies [agents]X[agents]G phi
    # and iterate until we reach the fixpoint.
    # By keeping track of already visited states,
    # we know that we will finally extract the full sub-system of phi states.
    
    # Get CAG(phi)
    cagphi = ~ceu(fsm, agents, BDD.true(fsm.bddEnc.DDmanager), ~phi)
    
    # Get all states and transitions
    extract = {state}
    states = set()
    transitions = set()
    while len(extract) > 0:
        cur = extract.pop()
        states.add(cur)
        expl = explain_cax(fsm, cur, agents, cagphi)
        for act, succ in expl.successors:
            transitions.add((cur, act, succ.state))
            if succ.state not in states:
                extract.add(succ.state)
    
    # Build the graph
    nodes = {}
    for s in states:
        nodes[s] = Explanation(s)
    for cur, act, succ in transitions:
        nodes[cur].successors.add((act, nodes[succ]))
    
    return nodes[state]
    
    
def explain_caw(fsm, state, agents, phi, psi):
    """
    Explain why state of fsm satisfies [agents] phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying [agents] phi W psi;
    agents -- the agents of the specification;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    # To show that state satisfies [agents] phi W psi
    # either it satisfies psi and we are done,
    # or we have to explain why state satisfies [agents]X[agents]phi W psi
    # and iterate until we reach the fixpoint or stop at psi.
    # By keeping track of already visited states,
    # we know that we will finally extract the full sub-system.
    
    # Get CEW(phi,psi)
    cawpp = ~ceu(fsm, agents, ~psi, ~psi & ~phi)
    
    # Get all states and transitions
    extract = {state}
    states = set()
    transitions = set()
    while len(extract) > 0:
        cur = extract.pop()
        states.add(cur)
        if not cur <= psi:
            expl = explain_cax(fsm, cur, agents, cawpp)
            for act, succ in expl.successors:
                transitions.add((cur, act, succ.state))
                if succ.state not in states:
                    extract.add(succ.state)
    
    # Build the graph
    nodes = {}
    for s in states:
        nodes[s] = Explanation(s)
    for cur, act, succ in transitions:
        nodes[cur].successors.add((act, nodes[succ]))
    
    return nodes[state]
    
    
if __name__ == '__main__':
    import argparse, sys
    from pynusmv.init import init_nusmv, deinit_nusmv
    from ..mas import glob
    from .parsing import parseATL
    from .eval import evalATL
    from .check import check
    from pyparsing import ParseException
    from pynusmv.exception import PyNuSMVError
    from .ast import CEF, CEG, CEX, CEU, CEW, CAF, CAG, CAX, CAU, CAW
    
    def explain_top(mas, spec):
        """
        Explain why the top operator of spec is satisfied.
        
        mas -- the concerned MAS;
        spec -- the given specification; must be satisfied by mas.
        """
        
        # Get satisfying initial state
        sat = evalATL(mas, spec)
        state = mas.pick_one_state(
                                 (sat & mas.bddEnc.statesInputsMask & mas.init))
                                
        # Switch between operators
        if type(spec) is CEF:
            phi = BDD.true(mas.bddEnc.DDmanager)
            psi = evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_ceu(mas, state, agents, phi, psi)
            
        elif type(spec) is CEG:
            phi = evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_ceg(mas, state, agents, phi)
            
        elif type(spec) is CEX:
            phi = evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_cex(mas, state, agents, phi)
            
        elif type(spec) is CEU:
            phi = evalATL(mas, spec.left)
            psi = evalATL(mas, spec.right)
            agents = {atom.value for atom in spec.group}
            return explain_ceu(mas, state, agents, phi, psi)
            
        elif type(spec) is CEW:
            phi = evalATL(mas, spec.left)
            psi = evalATL(mas, spec.right)
            agents = {atom.value for atom in spec.group}
            return explain_cew(mas, state, agents, phi, psi)
            
        elif type(spec) is CAF:
            phi = BDD.true(mas.bddEnc.DDmanager)
            psi = evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_cau(mas, state, agents, phi, psi)
            
        elif type(spec) is CAG:
            phi = evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_cag(mas, state, agents, phi)
            
        elif type(spec) is CAX:
            phi = evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_cax(mas, state, agents, phi)
            
        elif type(spec) is CAU:
            phi = evalATL(mas, spec.left)
            psi = evalATL(mas, spec.right)
            agents = {atom.value for atom in spec.group}
            return explain_cau(mas, state, agents, phi, psi)
            
        elif type(spec) is CAW:
            phi = evalATL(mas, spec.left)
            psi = evalATL(mas, spec.right)
            agents = {atom.value for atom in spec.group}
            return explain_caw(mas, state, agents, phi, psi)
            
        else:
            return Explanation(state)
            
        
    def cntex_top(mas, spec):
        """
        Explain why the top operator of spec is violated.
        
        mas -- the concerned MAS;
        spec -- the given specification; must be violated by mas.
        """
        
        # Get violating initial state
        sat = evalATL(mas, spec)
        state = mas.pick_one_state(
                                (~sat & mas.bddEnc.statesInputsMask & mas.init))
                                
        # Switch between operators
        if type(spec) is CEF:
            # ~<a> F p = [a] G ~p
            phi = ~evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_cag(mas, state, agents, phi)
            
        elif type(spec) is CEG:
            # ~<a> G p = [a] F ~p = [a] true U ~p
            phi = BDD.true(mas.bddEnc.DDmanager)
            psi = ~evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_cau(mas, state, agents, phi, psi)
            
        elif type(spec) is CEX:
            # ~<a> X p = [a] X ~p
            phi = ~evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_cax(mas, state, agents, phi)
            
        elif type(spec) is CEU:
            # ~<a> p U q = [q] ~q W ~p & ~q
            p = evalATL(mas, spec.left)
            q = evalATL(mas, spec.right)
            phi = ~q
            psi = ~p & ~q
            agents = {atom.value for atom in spec.group}
            return explain_caw(mas, state, agents, phi, psi)
            
        elif type(spec) is CEW:
            # ~<a> p W q = [q] ~q U ~p & ~q
            p = evalATL(mas, spec.left)
            q = evalATL(mas, spec.right)
            phi = ~q
            psi = ~p & ~q
            agents = {atom.value for atom in spec.group}
            return explain_cau(mas, state, agents, phi, psi)
            
        elif type(spec) is CAF:
            # ~[a] F p = <a> G ~p
            phi = ~evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_ceg(mas, state, agents, phi)
            
        elif type(spec) is CAG:
            # ~[a] G p = <a> F ~p = <a> true U ~p
            phi = BDD.true(mas.bddEnc.DDmanager)
            psi = ~evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_ceu(mas, state, agents, phi, psi)
            
        elif type(spec) is CAX:
            # ~[a] X p = <a> X ~p
            phi = ~evalATL(mas, spec.child)
            agents = {atom.value for atom in spec.group}
            return explain_cex(mas, state, agents, phi)
            
        elif type(spec) is CAU:
            # ~[a] p U q = <q> ~q W ~p & ~q
            p = evalATL(mas, spec.left)
            q = evalATL(mas, spec.right)
            phi = ~q
            psi = ~p & ~q
            agents = {atom.value for atom in spec.group}
            return explain_cew(mas, state, agents, phi, psi)
            
        elif type(spec) is CAW:
            # ~[a] p W q = <q> ~q U ~p & ~q
            p = evalATL(mas, spec.left)
            q = evalATL(mas, spec.right)
            phi = ~q
            psi = ~p & ~q
            agents = {atom.value for atom in spec.group}
            return explain_ceu(mas, state, agents, phi, psi)
            
        else:
            return Explanation(state)
    
    
    # Parse arguments
    description = """ATL model checking explainer.
    
    The truth value of the given property is printed on stderr,
    a dot-formated explanation is printed on stdout.
    A counter-example is printed if the property is violated,
    a witness is printed otherwise.
    Only the top operator is explained.
    """
    parser = argparse.ArgumentParser(description=description)
    # Populate arguments: the model, the property
    parser.add_argument('model', help='the MAS as an SMV model')
    parser.add_argument('property', help='the property to explain')
    args = parser.parse_args()
    
    init_nusmv()
    # Initialize the model
    glob.load_from_file(args.model)
    mas = glob.mas()
    
    try:
        # Parse property
        spec = parseATL(args.property)[0]
        
        # Check property
        satisfied = check(mas, spec)
        
        print('Specification', str(spec), 'is', str(satisfied), file=sys.stderr)
        
        if satisfied:
            print(explain_top(mas, spec).dot())
        else:
            print(cntex_top(mas, spec).dot())
    except ParseException as e:
        print("[ERROR] Cannot parse specification:", str(e))
    except PyNuSMVError as e:
        print("[ERROR]", str(e))
    