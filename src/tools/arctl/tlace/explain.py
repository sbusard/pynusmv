"""ARCTL TLACE explanations."""

from ..eval import evalArctl
from ..ast import (TrueExp, FalseExp, Atom, Not, And, Or, Implies, Iff,
                   AaF, AaG, AaX, AaU, AaW, EaF, EaG, EaX, EaU, EaW)
from .tlace import (Tlacenode, Tlacebranch)
from ..explain import explain_eax, explain_eag, explain_eau

def explain_witness(fsm, state, spec):
    """
    Explain why state of fsm satisfies spec.
    
    Return a TLACE node explaining why state of fsm satifies spec.
    """
    
    if type(spec) is TrueExp:
        # state is its own explanation, no need for annotation
        return Tlacenode(state, None, None, None)
        
    elif type(spec) is FalseExp:    
        print("[ERROR] ARCTL TLACE explain_witness:",
              "cannot explain why state satisfies False",
              spec)
        # TODO Error, cannot explain why state |= False
        return None
    
    elif type(spec) is Atom:
        # state is its own explanation
        return Tlacenode(state, (spec,), None, None)
        
    elif type(spec) is Not:
        return explain_countex(fsm, state, spec.child)
        
    elif type(spec) is And:
        # Get left and right explanations, then merge them
        left = explain_witness(fsm, state, spec.left)
        right = explain_witness(fsm, state, spec.right)
        return Tlacenode(state,
                         left.atomics + right.atomics,
                         left.branches + right.branches,
                         left.universals + right.universals
                        )
        
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
        return Tlacenode(state, None, None, (spec,))
                       
    elif type(spec) in {EaF, EaG, EaX, EaU, EaW}:
        branch = explain_branch(fsm, state, spec, spec)
        return Tlacenode(state, None, (branch,), None)
        
    else:
        # TODO Generate error
        print("[ERROR] ARCTL TLACE explain_witness:",
              "unrecognized specification type",
              spec)
        return None
        
        
def explain_countex(fsm, state, spec):
    """
    Explain why state of fsm violates spec.
    
    Return a TLACE node explaining why state of fsm violates spec.
    """
    
    if type(spec) is TrueExp:    
        print("[ERROR] ARCTL TLACE explain_countex:",
              "cannot explain why state violates True",
              spec)
        # TODO Error, cannot explain why state |/= True
        return None
        
    elif type(spec) is FalseExp:
        return explain_witness(fsm, state, TrueExp())
    
    elif type(spec) is Atom:
        # state is its own explanation
        return Tlacenode(state, (Not(spec),), None, None)
        
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
            return Tlacenode(state, None, None,
                             (Not(EaX(spec.action, Atom('TRUE'))),))
        
    elif type(spec) is AaU:
        return explain_witness(fsm, state,
                               EaW(spec.action,
                                   Not(spec.right),
                                   And(Not(spec.left), Not(spec.right))))
        
    elif type(spec) is AaW:
        return explain_witness(fsm, state,
                               EaU(spec.action,
                                   Not(spec.right),
                                   And(Not(spec.left), Not(spec.right))))
                     
    elif type(spec) is EaF:
        return explain_witness(fsm, state,
                               AaG(spec.action, Not(spec.child)))
        
    elif type(spec) is EaG:
        return explain_witness(fsm, state,
                               AaF(spec.action, Not(spec.child)))
        
    elif type(spec) is EaX:
        # E<a>X f is false because A<a>X ~f is true or E<a>X true is false
        eaxnf = evalArctl(fsm, AaX(spec.action, Not(spec.child)))
        if state <= eaxnf:
            return explain_witness(fsm, state,
                                   AaX(spec.action, Not(spec.child)))
        else:
            return Tlacenode(state, None, None,
                             (Not(EaX(spec.action, Atom('TRUE'))),))
        
    elif type(spec) is EaU:
        return explain_witness(fsm, state,
                               AaW(spec.action,
                                   Not(spec.right),
                                   And(Not(spec.left), Not(spec.right))))
        
    elif type(spec) is EaW:
        return explain_witness(fsm, state,
                               AaU(spec.action,
                                   Not(spec.right),
                                   And(Not(spec.left), Not(spec.right))))
        
    else:
        # TODO Generate error
        print("[ERROR] ARCTL explain_countex: unrecognized specification type",
              spec)
        return None
        
        
def explain_branch(fsm, state, spec, originalspec):
    """
    Return a TLACE branch explaining why state of fsm satisfies spec.
    """
    if type(spec) is EaX:
        alpha = evalArctl(fsm, spec.action)
        phi = evalArctl(fsm, spec.child)
        path = explain_eax(fsm, state, alpha, phi)
        branch = (Tlacenode(path[0]),
                  path[1],
                  explain_witness(fsm, path[2], spec.child))
        return Tlacebranch(originalspec, branch)

    elif type(spec) is EaF:
        return explain_branch(fsm, state,
                              EaU(spec.action, Atom('TRUE'), spec.child),
                              originalspec)

    elif type(spec) is EaG:
        alpha = evalArctl(fsm, spec.action)
        phi = evalArctl(fsm, spec.child)
        (path, (inloop, loopstate)) = explain_eag(fsm, state, alpha, phi)

        branch = []
        # intermediate states
        loop = None
        for s, i in zip(path[::2], path[1::2]):
            wit = explain_witness(fsm, s, spec.child)
            branch.append(wit)
            branch.append(i)
            # manage the loop
            if loopstate is not None and loopstate == s:
                loop = wit
                
        # last state
        # if loopstate is None, this means that the explanation is a full
        # finite path. We have to explain why this state satisfies ~E<a>X TRUE
        lastnode = explain_witness(fsm, path[-1], spec.child)
        if loopstate is None:
            # Add annotation to show that ~E<a>X TRUE is true
            lastnode = Tlacenode(lastnode.state,
                                 lastnode.atomics,
                                 lastnode.branches,
                                 lastnode.universals +
                                 (Not(EaX(spec.action, Atom('TRUE'))),)
                                )
        branch.append(lastnode)
        
        finalloop = loop and (inloop, loop) or None
        
        return Tlacebranch(originalspec, tuple(branch), finalloop)

    elif type(spec) is EaU:
        alpha = evalArctl(fsm, spec.action)
        phi = evalArctl(fsm, spec.left)
        psi = evalArctl(fsm, spec.right)
        path = explain_eau(fsm, state, alpha, phi, psi)

        branch = []
        # intermediate states
        for s, i in zip(path[::2], path[1::2]):
            branch.append(explain_witness(fsm, s, spec.left))
            branch.append(i)
        # last state
        branch.append(explain_witness(fsm, path[-1], spec.right))

        return Tlacebranch(originalspec, tuple(branch))

    elif type(spec) is EaW:
        eauspec = EaU(spec.action, spec.left, spec.right)
        eagspec = EaG(spec.action, spec.left)
        if state <= evalArctl(fsm, eauspec):
            return explain_branch(fsm, state, eauspec, originalspec)
        else:
            return explain_branch(fsm, state, eagspec, originalspec)

    else:
        # TODO Generate error
        print("[ERROR] ARCTL explain_branch:",
              "unrecognized specification type",
              spec)
        return None