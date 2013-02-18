from .tlacenode import Tlacenode
from .tlacebranch import Tlacebranch

from pynusmv.nusmv.parser import parser

from pynusmv.prop import (true as sptrue, false as spfalse, imply, iff,
                               ex, eg, ef, eu, ew, ax, ag, af, au, aw)
from pynusmv.mc import eval_ctl_spec, explainEX, explainEG, explainEU

def explain(fsm, state, spec):
    """
    Return a TLACE node explaining why state of fsm violates spec.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    spec -- a pynusmv.spec.spec.Spec node representing the specification.
    
    Return a tlacenode.Tlacenode explaining why state of fsm violates spec.
    """
    return countex(fsm, state, spec, None)
    
    
def countex(fsm, state, spec, context):
    """
    Return a TLACE node explaining why state of fsm violates spec.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    spec -- a pynusmv.spec.spec.Spec node representing the specification.
    context -- a pynusmv.spec.spec.Spec representing the context of spec in fsm.
    
    Return a tlacenode.Tlacenode explaining why state of fsm violates spec.
    """
    
    if spec.type == parser.CONTEXT:
        return countex(fsm, state, spec.cdr, spec.car)
        
    elif spec.type == parser.FALSEEXP:
        newspec = sptrue()
        
    elif spec.type == parser.NOT:
        newspec = spec.car
        
    elif spec.type == parser.OR:
        newspec = (~spec.car) & (~spec.cdr)
    
    elif spec.type == parser.AND:
        newspec = (~spec.car) | (~spec.cdr)
    
    elif spec.type == parser.IMPLIES:
        newspec = spec.car & (~spec.cdr)
                            
    elif spec.type == parser.IFF:
        newspec = (spec.car & (~spec.cdr)) | ((~spec.car) & spec.cdr)
                    
    elif spec.type == parser.EX:
        newspec = ax(~spec.car)
                                 
    elif spec.type == parser.EF:
        newspec = ag(~spec.car)
                                 
    elif spec.type == parser.EG:
        newspec = af(~spec.car)
                                 
    elif spec.type == parser.EU:
        newspec = aw(~spec.cdr, (~spec.car) & (~spec.cdr))
                    
    elif spec.type == parser.EW:
        newspec = au(~spec.cdr, (~spec.car) & (~spec.cdr))
                    
    elif spec.type == parser.AX:
        newspec = ex(~spec.car)
        
    elif spec.type == parser.AF:
        newspec = eg(~spec.car)
                                 
    elif spec.type == parser.AG:
        newspec = ef(~spec.car)
                                 
    elif spec.type == parser.AU:
        newspec = ew(~spec.cdr, (~spec.car) & (~spec.cdr))
                        
    elif spec.type == parser.AW:
        newspec = eu(~spec.cdr, (~spec.car) & (~spec.cdr))
                        
    else:
        if spec.type == parser.NOT:
            newspec = spec.car
        else:
            newspec = ~spec
        return Tlacenode(state, (newspec,), None, None)
        
    return witness(fsm, state, newspec, context)
    
    
def witness(fsm, state, spec, context):
    """
    Return a TLACE node explaining why state of fsm satisfies spec.
    
    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    spec -- a pynusmv.spec.spec.Spec node representing the specification.
    context -- a pynusmv.spec.spec.Spec representing the context of spec in fsm.
    
    Return a tlacenode.Tlacenode explaining why state of fsm satisfies spec.
    """

    if spec.type == parser.CONTEXT:
        return witness(fsm, state, spec.cdr, spec.car)
        
    elif spec.type == parser.TRUEEXP:
        return Tlacenode(state, None, None, None)
        
    elif spec.type == parser.NOT:
        return countex(fsm, state, spec.car, context)
        
    elif spec.type == parser.OR:
        if state.entailed(eval_ctl_spec(fsm, spec.car, context)):
            return witness(fsm, state, spec.car, context)
        else:
            return witness(fsm, state, spec.cdr, context)
    
    elif spec.type == parser.AND:
        n1 = witness(fsm, state, spec.car, context)
        n2 = witness(fsm, state, spec.cdr, context)
        return Tlacenode(state,
                         n1.atomics + n2.atomics,
                         n1.branches + n2.branches,
                         n1.universals + n2.universals)
    
    elif spec.type == parser.IMPLIES:
        newspec = (~spec.car) | spec.cdr
        return witness(fsm, state, newspec, context)
                            
    elif spec.type == parser.IFF:
        newspec = (spec.car & spec.cdr) | ((~spec.car) & (~spec.cdr))
        return witness(fsm, state, newspec, context)
                    
    elif (spec.type == parser.EX or
          spec.type == parser.EF or
          spec.type == parser.EG or
          spec.type == parser.EU or
          spec.type == parser.EW):
        return Tlacenode(state, None,
                         (witness_branch(fsm, state, spec, context, spec),),
                         None)
                    
    elif (spec.type == parser.AX or
          spec.type == parser.AF or
          spec.type == parser.AG or
          spec.type == parser.AU or
          spec.type == parser.AW):
        return Tlacenode(state, None, None, (spec,))
                        
    else:
        # deal with unmanaged formulas by returning a single
        # TLACE node. This includes the case of atomic propositions.
        # All unrecognized operators are considered as atomics
        return Tlacenode(state, (spec,), None, None)
        
        
def witness_branch(fsm, state, spec, context, originalspec):
    """
    Return a TLACE branch explaining why state of fsm satisfies spec.

    fsm -- a pynusmv.fsm.BddFsm representing the system.
    state -- a pynusmv.dd.BDD representing a state of fsm.
    spec -- a pynusmv.spec.spec.Spec node representing the specification.
    context -- a pynusmv.spec.spec.Spec representing the context of spec in fsm.
    originalspec -- a pynusmv.spec.spec.Spec representing the original spec;
                    used to annotate the produced branch, despite updated
                    specs.

    Return a tlacebranch.Tlacebranch explaining why state of fsm satisfies spec.
    
    Throw a NonExistentialSpecError if spec is not existential.
    """
    
    if spec.type == parser.EX:
        f = eval_ctl_spec(fsm, spec.car, context)
        path = explainEX(fsm, state, f)
        branch = (Tlacenode(path[0]),
                  path[1],
                  witness(fsm, path[2], spec.car, context))
        return Tlacebranch(originalspec, branch)
        
    elif spec.type == parser.EF:
        newspec = eu(sptrue(), spec.car)
        return witness_branch(fsm, state, newspec, context, originalspec)
        
    elif spec.type == parser.EG:
        f = eval_ctl_spec(fsm, spec.car, context)
        (path, (inloop, loopstate)) = explainEG(fsm, state, f)
        
        branch = []
        # intermediate states
        for s, i in zip(path[::2], path[1::2]):
            wit = witness(fsm, s, spec.car, context)
            branch.append(wit)
            branch.append(i)
            # manage the loop
            if s == loopstate:
                loop = wit
        # last state
        branch.append(witness(fsm, path[-1], spec.car, context))
        
        return Tlacebranch(originalspec, tuple(branch), (inloop, loop))
        
    elif spec.type == parser.EU:
        f = eval_ctl_spec(fsm, spec.car, context)
        g = eval_ctl_spec(fsm, spec.cdr, context)
        path = explainEU(fsm, state, f, g)
        
        branch = []
        # intermediate states
        for s, i in zip(path[::2], path[1::2]):
            branch.append(witness(fsm, s, spec.car, context))
            branch.append(i)
        # last state
        branch.append(witness(fsm, path[-1], spec.cdr, context))
        
        return Tlacebranch(originalspec, tuple(branch))
        
    elif spec.type == parser.EW:
        euspec = eu(spec.car, spec.cdr)
        egspec = eg(spec.car)
        if state.entailed(eval_ctl_spec(fsm, euspec, context)):
            return witness_branch(fsm, state, euspec, context, originalspec)
        else:
            return witness_branch(fsm, state, egspec, context, originalspec)
        
    else:
        # Default case, throw an exception because spec is not existential
        raise NonExistentialSpecError()



class NonExistentialSpecError(Exception):
    """
    Exception for given non existential temporal formula.
    """
    pass
