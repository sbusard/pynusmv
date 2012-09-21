from ..ast import (TrueExp, FalseExp, Init, Reachable,
                   Atom, Not, And, Or, Implies, Iff, 
                   AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                   nK, nE, nD, nC, K, E, D, C)
from ..eval import evalCTLK
from ..explain import (explain_ex, explain_eg, explain_eu,
                       explain_nk, explain_ne, explain_nd, explain_nc,
                       explain_reachable)
from .tlace import Tlacenode, TemporalBranch, EpistemicBranch

def explain_witness(fsm, state, spec):
    """
    Explain why state of fsm satisfies spec.
    
    state must satisfy spec in fsm. No check is made to ensure that.
    Return a TLACE node explaining why state of fsm satifies spec.
    """
    
    if type(spec) is TrueExp:
        # state is its own explanation, no need for annotation
        return Tlacenode(state, None, None, None)
        
    elif type(spec) is FalseExp:    
        print("[ERROR] CTLK TLACE explain_witness:",
              "cannot explain why state satisfies False",
              spec)
        # TODO Error, cannot explain why state |= False
        return None
        
    elif type(spec) is Init:
        return Tlacenode(state, (spec,), None, None)
    
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
        specbdd = evalCTLK(fsm, spec.left)
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
        
    elif type(spec) in {AF, AG, AX, AU, AW, K, E, D, C}:
        # Cannot explain with a single path
        return Tlacenode(state, None, None, (spec,))
                       
    elif type(spec) in {EF, EG, EX, EU, EW, nK, nE, nD, nC, Reachable}:
        branch = explain_branch(fsm, state, spec, spec)
        return Tlacenode(state, None, (branch,), None)
        
    else:
        # TODO Generate error
        print("[ERROR] CTLK TLACE explain_witness:",
              "unrecognized specification type",
              spec)
        return None
        
        
def explain_countex(fsm, state, spec):
    """
    Explain why state of fsm violates spec.
    
    state must not satisfy spec in fsm. No check is made to ensure that.
    Return a TLACE node explaining why state of fsm violates spec.
    """
    
    if type(spec) is TrueExp:    
        print("[ERROR] CTLK TLACE explain_countex:",
              "cannot explain why state violates True",
              spec)
        # TODO Error, cannot explain why state |/= True
        return None
        
    elif type(spec) is FalseExp:
        return explain_witness(fsm, state, TrueExp())
        
    elif type(spec) is Init:
        return Tlacenode(state, (Not(spec),), None, None)
        
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
        
    elif type(spec) is AF:
        # AF p = ~EG ~p
        return explain_witness(fsm, state, EG(Not(spec.child)))
        
    elif type(spec) is AG:
        # AG p = ~EF ~p
        return explain_witness(fsm, state, EF(Not(spec.child)))
        
    elif type(spec) is AX:
        # AX p = ~EX ~p
        return explain_witness(fsm, state, EX(Not(spec.child)))
        
    elif type(spec) is AU:
        # A[p U q] = ~E[~q W ~p & ~q]
        return explain_witness(fsm, state,
                               EW(Not(spec.right),
                                  And(Not(spec.left), Not(spec.right))))
        
    elif type(spec) is AW:
        # A[p W q] = ~E[~q U ~p & ~q]
        return explain_witness(fsm, state,
                               EU(Not(spec.right),
                                  And(Not(spec.left), Not(spec.right))))
                     
    elif type(spec) is EF:
        # EF p = ~AG ~p
        return explain_witness(fsm, state, AG(Not(spec.child)))
        
    elif type(spec) is EG:
        # EG p = ~AF ~p
        return explain_witness(fsm, state, AF(Not(spec.child)))
        
    elif type(spec) is EX:
        # EX p = ~AX ~p
        return explain_witness(fsm, state, AX(Not(spec.child)))
        
    elif type(spec) is EU:
        # E[p U q] = ~A[~q W ~p & ~q]
        return explain_witness(fsm, state,
                               AW(Not(spec.right),
                                  And(Not(spec.left), Not(spec.right))))
        
    elif type(spec) is EW:
        # E[p W q] = ~A[~q U ~p & ~q]
        return explain_witness(fsm, state,
                               AU(Not(spec.right),
                                  And(Not(spec.left), Not(spec.right))))
                                  
    elif type(spec) is nK:
        # nK<ag> p = ~K<ag> ~p
        return explain_witness(fsm, state, K(spec.agent, Not(spec.child)))
                                  
    elif type(spec) is nE:
        # nE<group> p = ~E<group> ~p
        return explain_witness(fsm, state, E(spec.group, Not(spec.child)))
                                  
    elif type(spec) is nD:
        # nD<group> p = ~D<group> ~p
        return explain_witness(fsm, state, D(spec.group, Not(spec.child)))
                                  
    elif type(spec) is nC:
        # nC<group> p = ~C<group> ~p
        return explain_witness(fsm, state, C(spec.group, Not(spec.child)))
                                  
    elif type(spec) is K:
        # K<ag> p = ~nK<ag> ~p
        return explain_witness(fsm, state, nK(spec.agent, Not(spec.child)))
                                  
    elif type(spec) is E:
        # E<group> p = ~nE<group> ~p
        return explain_witness(fsm, state, nE(spec.group, Not(spec.child)))
                                  
    elif type(spec) is D:
        # D<group> p = ~nD<group> ~p
        return explain_witness(fsm, state, nD(spec.group, Not(spec.child)))
                                  
    elif type(spec) is C:
        # C<group> p = ~nC<group> ~p
        return explain_witness(fsm, state, nC(spec.group, Not(spec.child)))
        
    elif type(spec) is Reachable:
        # Is its own explanation
        return Tlacenode(state, None, None, (Not(spec),))
        
    else:
        # TODO Generate error
        print("[ERROR] CTLK explain_countex: unrecognized specification type",
              spec)
        return None
        
        
def explain_branch(fsm, state, spec, originalspec):
    """
    Return a TLACE branch explaining why state of fsm satisfies spec.
    
    state must satisfy spec in fsm. No check is made to ensure that.
    spec must be an existential operator (EX, EF, EU, EG, EW, nK, nE, nD, nC).
    originalspec is the specification with which the created branch will be
    annotated.
    """
    if type(spec) is EX:
        phi = evalCTLK(fsm, spec.child)
        path = explain_ex(fsm, state, phi)
        branch = (Tlacenode(path[0]),
                  path[1],
                  explain_witness(fsm, path[2], spec.child))
        return TemporalBranch(originalspec, branch)

    elif type(spec) is EF:
        return explain_branch(fsm, state,
                              EU(TrueExp(), spec.child),
                              originalspec)

    elif type(spec) is EG:
        phi = evalCTLK(fsm, spec.child)
        (path, (inloop, loopstate)) = explain_eg(fsm, state, phi)

        branch = []
        # intermediate states
        loop = None
        for s, i in zip(path[::2], path[1::2]):
            wit = explain_witness(fsm, s, spec.child)
            branch.append(wit)
            branch.append(i)
            # manage the loop
            if loopstate == s:
                loop = wit
                
        # last state and loop
        wit = explain_witness(fsm, path[-1], spec.child)
        branch.append(wit)
        if loopstate == path[-1]:
            loop = wit
        finalloop = (inloop, loop)
        
        return TemporalBranch(originalspec, tuple(branch), finalloop)

    elif type(spec) is EU:
        phi = evalCTLK(fsm, spec.left)
        psi = evalCTLK(fsm, spec.right)
        path = explain_eu(fsm, state, phi, psi)

        branch = []
        # intermediate states
        for s, i in zip(path[::2], path[1::2]):
            branch.append(explain_witness(fsm, s, spec.left))
            branch.append(i)
        # last state
        branch.append(explain_witness(fsm, path[-1], spec.right))

        return TemporalBranch(originalspec, tuple(branch))

    elif type(spec) is EW:
        # E[p W q] = E[p U q] | EG p
        euspec = EU(spec.left, spec.right)
        egspec = EG(spec.left)
        if state <= evalCTLK(fsm, euspec):
            return explain_branch(fsm, state, euspec, originalspec)
        else:
            return explain_branch(fsm, state, egspec, originalspec)
            
    elif type(spec) is Reachable:
        # Get the inversed path
        path = explain_reachable(fsm, state)
        
        # Construct the path of nodes
        branch = []
        for (s, i) in zip(path[::2], path[1::2]):
            branch.append(Tlacenode(s))
            branch.append(i)
        # Special case for the last node: it is Init
        branch.append(explain_witness(fsm, path[-1], Init()))
        
        return TemporalBranch(originalspec, tuple(branch))
            
    elif type(spec) is nK:        
        # Get the equivalent state
        phi = evalCTLK(fsm, spec.child)
        path = explain_nk(fsm, state, spec.agent.value, phi)
        
        # Explain why the equiv state satisfies Reachable and phi
        # and construct the branch
        branch = [Tlacenode(path[0]), path[1],
                  explain_witness(fsm, path[2], And(Reachable(), spec.child))]
        
        # Return the epistemic branch
        return EpistemicBranch(originalspec, tuple(branch))
        
    elif type(spec) is nE:        
        # Get the equivalent state
        phi = evalCTLK(fsm, spec.child)
        path = explain_ne(fsm, state, [ag.value for ag in spec.group], phi)
        
        # Explain why the equiv state satisfies Reachable and phi
        # and construct the branch
        branch = [Tlacenode(path[0]), path[1],
                  explain_witness(fsm, path[2], And(Reachable(), spec.child))]
        
        # Return the epistemic branch
        return EpistemicBranch(originalspec, tuple(branch))
        
    elif type(spec) is nD:        
        # Get the equivalent state
        phi = evalCTLK(fsm, spec.child)
        path = explain_nd(fsm, state, [ag.value for ag in spec.group], phi)
        
        # Explain why the equiv state satisfies Reachable and phi
        # and construct the branch
        branch = [Tlacenode(path[0]), path[1],
                  explain_witness(fsm, path[2], And(Reachable(), spec.child))]
        
        # Return the epistemic branch
        return EpistemicBranch(originalspec, tuple(branch))
        
    if type(spec) is nC:
        # Get the knowledge path
        phi = evalCTLK(fsm, spec.child)
        path = explain_nc(fsm, state, [ag.value for ag in spec.group], phi)
        
        # Discard the first element of path: this is state and does not have
        # to be explained reachable
        branch = [Tlacenode(path[0]), path[1]]
        path = path[2:]
        
        # Build the branch : show that all intermediate states are reachable
        for (s, ag) in zip(path[::2], path[1::2]):
            branch.append(explain_witness(fsm, s, Reachable()))
            branch.append(ag)
        
        # Complete the branch : show that the last one is reachable
        # satisfies phi
        branch.append(explain_witness(fsm, path[-1],
                                                  And(Reachable(), spec.child)))
        
        # Return the epistemic branch
        return EpistemicBranch(originalspec, tuple(branch))

    else:
        # TODO Generate error
        print("[ERROR] CTLK explain_branch:",
              "unrecognized specification type",
              spec)
        return None