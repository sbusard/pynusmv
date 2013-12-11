"""
ARCTL explain functions.
"""

from pynusmv.dd import BDD
from .eval import _ex, eag, _eu, eau, eax, evalArctl
from .ast import (TrueExp, FalseExp, Atom, Not, And, Or, Implies, Iff,
                  AaF, AaG, AaX, AaU, AaW, EaF, EaG, EaX, EaU, EaW)
    
    
def explain_witness(fsm, state, spec):
    """
    Explain why state of fsm satisfies spec.
    
    Return a single path explaining (maybe partially) why state of fsm satisfies
    spec. The returned structure is a tuple ((s0, i1, s1, ..., sn), (in, loop))
    where (s0, ..., sn) is a path of fsm explaining spec, and (in, loop)
    represents a loop of this path. If the path is finite, (in, loop)
    is (None, None).
    """
    
    if type(spec) is TrueExp:
        # state is its own explanation
        return ((state,), (None, None))

    elif type(spec) is FalseExp:    
        print("[ERROR] ARCTL explain_witness:",
              "cannot explain why state satisfies False",
              spec)
        # TODO Error, cannot explain why state |= False
        return None
    
    elif type(spec) is Atom:
        # state is its own explanation
        return ((state,), (None, None))
        
    elif type(spec) is Not:
        return explain_countex(fsm, state, spec.child)
        
    elif type(spec) is And:
        # Do not branch. Choose left child
        return explain_witness(fsm, state, spec.left)
        
    elif type(spec) is Or:
        # If state satisfies spec.left, explain it
        # otherwise, state satisfies spec.right, so explain it
        specbdd = evalArctl(fsm, spec.left)
        if state <= specbdd:
            return explain_witness(fsm, state, spec.left)
        else:
            return explain_witness(fsm, state, spec.right)
        
    elif type(spec) is Implies:
        # a -> b is ~a | b
        return explain_witness(fsm, state, Or(Not(spec.left), spec.right))
        
    elif type(spec) is Iff:
        # a <-> b is (a & b) | (~a & ~b)
        return explain_witness(
                fsm, state,
                Or(And(spec.left, spec.right),
                   And(Not(spec.left), Not(spec.right)))
               )
        
    elif type(spec) in {AaF, AaG, AaX, AaU, AaW}:
        # Cannot explain with a single path
        return((state,), (None, None))
                       
    elif type(spec) is EaF:
        path = explain_eau(fsm, state, evalArctl(fsm, spec.action),
                           BDD.true(fsm.bddEnc.DDmanager),
                           evalArctl(fsm, spec.child))
                           
        (npath, loops) = explain_witness(fsm, path[-1], spec.child)
        return (path + npath[1:], loops)
                       
    elif type(spec) is EaG:
        return explain_eag(fsm, state, evalArctl(fsm, spec.action),
                           evalArctl(fsm, spec.child))
                       
    elif type(spec) is EaX:
        path = explain_eax(fsm, state, evalArctl(fsm, spec.action),
                           evalArctl(fsm, spec.child))
        (npath, loops) = explain_witness(fsm, path[-1], spec.child)
        return (path + npath[1:], loops)
                       
    elif type(spec) is EaU:
        path = explain_eau(fsm, state, evalArctl(fsm, spec.action),
                           evalArctl(fsm, spec.left),
                           evalArctl(fsm, spec.right))
                           
        (npath, loops) = explain_witness(fsm, path[-1], spec.right)
        return (path + npath[1:], loops)
                       
    elif type(spec) is EaW:
        # eaw(a, p, q) = ~aau(a, ~q, ~p & ~q)
        return explain_countex(fsm, state,
                                AaU(spec.action, Not(spec.right),
                                    And(Not(spec.left), Not(spec.right))))
        
    else:
        # TODO Generate error
        print("[ERROR] ARCTL explain_witness: unrecognized specification type",
              spec)
        return None
        
        
def explain_countex(fsm, state, spec):
    """
    Explain why state of fsm violates spec.
    
    Return a single path explaining (maybe partially) why state of fsm violates
    spec. The returned structure is a tuple ((s0, i1, s1, ..., sn), (in, loop))
    where (s0, ..., sn) is a path of fsm explaining spec, and (in, loop)
    represents a possible loop of this path. If the path is finite, (in, loop)
    is (None, None).
    """

    if type(spec) is TrueExp:    
        print("[ERROR] ARCTL explain_countex:",
              "cannot explain why state violates True",
              spec)
        # TODO Error, cannot explain why state |/= True
        return None
    
    elif type(spec) is FalseExp:
        # state is its own explanation
        return ((state,), (None, None))
    
    if type(spec) is Atom:
        # state is its own explanation
        return ((state,), (None, None))
        
    elif type(spec) is Not:
        return explain_witness(fsm, state, spec.child)
        
    elif type(spec) is And:
        # ~(a & b) = ~a | ~b
        return explain_witness(fsm, state, Or(Not(spec.left), Not(spec.right)))
        
    elif type(spec) is Or:
        # ~(a | b) = ~a & ~b
        return explain_witness(fsm, state, And(Not(spec.left), Not(spec.right)))
        
    elif type(spec) is Implies:
        # ~(a -> b) = a & ~b
        return explain_witness(fsm, state, And(spec.left, Not(spec.right)))
        
    elif type(spec) is Iff:
        # ~(a <-> b) = (a & ~b) | (~a & b)
        return explain_witness(fsm, state,
                               Or(And(spec.left, Not(spec.right)),
                                  And(Not(spec.left), spec.right)))
        
    elif type(spec) is AaF:
        # ~aaf(a, p) = _eu(a, ~p, ~p & ~_ex(a, true)) | _eg(a, ~p) = eag(a, ~p)
        return explain_witness(fsm, state, EaG(spec.action, Not(spec.child)))
        
    elif type(spec) is AaG:
        return explain_witness(fsm, state, EaF(spec.action, Not(spec.child)))
        
    elif type(spec) is AaX:
        # A<a>X f is false because E<a>X ~f is true or E<a>X true is false
        eaxnf = evalArctl(fsm, EaX(spec.action, Not(spec.child)))
        if state <= eaxnf:
            return explain_witness(fsm, state,
                                   EaX(spec.action, Not(spec.child)))
        else:
            return ((state,), (None, None))
        
    elif type(spec) is AaU:
        return explain_witness(fsm, state,
                               EaW(spec.action,
                                   Not(spec.left),
                                   And(Not(spec.left), Not(spec.right))))
        
    elif type(spec) is AaW:
        return explain_witness(fsm, state,
                               EaU(spec.action,
                                   Not(spec.left),
                                   And(Not(spec.left), Not(spec.right))))
                     
    elif type(spec) in {EaF, EaG, EaX, EaU, EaW}:
        # Cannot explain
        return ((state,), (None, None))
        
    else:
        # TODO Generate error
        print("[ERROR] ARCTL explain_countex: unrecognized specification type",
              spec)
        return None


def explain_eax(fsm, state, alpha, phi):
    """
    Explain why state of fsm satisfies E<a>X p.
    
    fsm -- the BddFsm of the model;
    state -- a State of fsm satisfying E<a>X p;
    alpha -- a BDD of fsm representing the inputs satisfying a;
    phi -- a BDD of fsm representing the states satisfying p.
    
    Return a tuple (state, inputs, state') where
        inputs belongs to alpha
        state' belongs to phi
        state' is reachable from state through inputs.
    """
    
    # Get next state: is a next of state and satisfies phi
    nexts = fsm.post(state, alpha)
    next = fsm.pick_one_state(nexts & phi)
    
    # Get inputs: is an input between state and next and satisfies alpha
    inputs = fsm.get_inputs_between_states(state, next)
    inputs = fsm.pick_one_inputs(inputs & alpha)
    
    return (state, inputs, next)
    
    
def explain_eag(fsm, state, alpha, phi):
    """
    Explain why state of fsm satisfies E<a>G p.
    
    fsm -- the BddFsm of the model;
    state -- a State of fsm satisfying E<a>G p;
    alpha -- a BDD of fsm representing the inputs satisfying a;
    phi -- a BDD of fsm representing the states satisfying p.
    
    Return a tuple (path, (inputs, loop)).
    
    If inputs and loop are None, this means that the witness is a full finite
    path and that path[-1] satisfies ~E<'alpha'>X 'TRUE' (dead state). In this
    case, path is a tuple (s0, i1, ..., sn) where sj |= phi for all j,
    ij |= alpha for all j and sj -ij+1-> sj+1 for all j.
    
    Otherwise, path is a tuple (s0, i1, ..., sn)
    where
        s0 = state
        sj -ij+1-> sj+1 for all j : 0 <= j < n
        sn -inputs-> loop
        loop = sj for some j : 0 <= j <= n
        sj belongs to phi for all j : 0 <= j <= n
        ij belongs to alpha for all j : 0 <= j <= n
        inputs belongs to alpha.
    """
    # eag(a, p) = _eu(a, p, p & ~_ex(a, true)) | _eg(a, p)
    
    paext = phi & ~_ex(fsm, alpha, BDD.true(fsm.bddEnc.DDmanager))
    euppaext = _eu(fsm, alpha, phi, paext)
    
    # If state satisfies _eu(a, p, p & ~_ex(a, true)),
    # use explain_eau to extract such a path
    if state <= euppaext:
        return (explain_eau(fsm, state, alpha, phi, paext), (None, None))
    
    # Otherwise,
    else:    
        # Get allstates = E<alpha>G phi
        allstates = eag(fsm, alpha, phi)
    
        # Start path at s
        path = [state]
        # While path[-1] cannot reach itself through states of eag,
        while (path[-1] &
               eax(fsm,
                   alpha,
                   eau(fsm, alpha, allstates, path[-1]))).is_false():
            # choose a successor of path[-1] in eag and add it in path
            path.append(fsm.pick_one_state(fsm.post(state, alpha) & allstates))
    
        # At this point, path[-1] can reach itself through states of eag
        # Explain it with explain_eau and explain_eax
        eaus = eau(fsm, alpha, allstates, path[-1])
        first = explain_eax(fsm, path[-1], alpha, eaus)
        second = explain_eau(fsm, first[-1], alpha, allstates, path[-1])
        fs = tuple(path) + first[1:] + second[1:]
    
        # Store the loop
        inputs = fs[-2]
        loop = fs[-1]
    
        # Return the path and the loop
        return (fs[:-2], (inputs, loop))
    
    
def explain_eau(fsm, state, alpha, phi, psi):
    """
    Explain why state of fsm satisfies E<a>[p U q].
    
    fsm -- the BddFsm of the model;
    state -- a State of fsm satisfying E<a>[p U q];
    alpha -- a BDD of fsm representing the inputs satisfying a;
    phi -- a BDD of fsm representing the states satisfying p;
    psi -- a BDD of fsm representing the states satisfying q.
    
    Return a tuple (s0, i1, ..., sn) where
        s0 = state
        sj -ij+1-> sj+1 for all j : 0 <= j < n
        sj belongs to phi for all j : 0 <= j < n
        sn belongs to psi
        ij belongs to alpha for all j : 0 <= j <= n.
    """
    
    # Compute fixpoint and store intermediate BDDs
    funct = lambda Z: (psi | (phi & _ex(fsm, alpha, Z)))
    old = BDD.false(fsm.bddEnc.DDmanager)
    new = funct(old)
    paths = [new]
    # Stop when reaching state
    # This is ensured since state satisfies E<a>[p U q]
    while (state & new).is_false():
        old = new
        new = funct(old)
        paths.append(new - old)
        
    # paths contains intermediate BDDs
    # paths[i] contains the BDD of all states of phi
    # that can reach a state of psi
    # through states of phi, in i steps
    # restricted to paths of alpha actions
    
    # paths[-1] contains state, skip it
    paths = paths[:-1]
    s = state
    path = [s]
    for states in paths[::-1]:
        sp = fsm.pick_one_state(fsm.post(s, alpha) & states)
        i = fsm.pick_one_inputs(fsm.get_inputs_between_states(s, sp) & alpha)
        path.append(i)
        path.append(sp)
        s = sp
    
    return tuple(path)