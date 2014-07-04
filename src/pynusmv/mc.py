"""
The :mod:`pynusmv.mc` module provides some functions of NuSMV dealing with
model checking, like CTL model checking.
"""

__all__ = ['eval_simple_expression', 'eval_ctl_spec', 'explainEX', 'explainEU',
           'explainEG']


from .nusmv.node import node as nsnode
from .nusmv.dd import dd as nsdd
from .nusmv.mc import mc

from .fsm import BddFsm
from .dd import BDD, State, Inputs, BDDList
from .prop import atom


def eval_simple_expression(fsm, sexp):
    """
    Return the set of states of `fsm` satisfying `sexp`, as a BDD.
    `sexp` is first parsed, then evaluated on `fsm`.

    :param fsm: the concerned FSM
    :type fsm: :class:`BddFsm <pynusmv.fsm.BddFsm>`
    :param sexp: a simple expression, as a string
    :rtype: :class:`BDD <pynusmv.dd.BDD>`

    """
    return eval_ctl_spec(fsm, atom(sexp))


def eval_ctl_spec(fsm, spec, context=None):
    """
    Return the set of states of `fsm` satisfying `spec` in `context`, as a BDD.

    :param fsm: the concerned FSM
    :type fsm: :class:`BddFsm <pynusmv.fsm.BddFsm>`
    :param spec: a specification about `fsm`
    :type spec: :class:`Spec <pynusmv.prop.Spec>`
    :param context: the context in which evaluate `spec`
    :type context: :class:`Spec <pynusmv.prop.Spec>`
    :rtype: :class:`BDD <pynusmv.dd.BDD>`

    """
    enc = fsm.bddEnc
    specbdd = BDD(mc.eval_ctl_spec(fsm._ptr, enc._ptr,
                                   spec._ptr,
                                   context and context._ptr or None),
                  enc.DDmanager, freeit=True)
    return specbdd


def explainEX(fsm, state, a):
    """
    Explain why `state` of `fsm` satisfies `EX phi`, where `a` is
    the set of states of `fsm` satisfying `phi`, represented by a BDD.

    :param fsm: the system
    :type fsm: :class:`BddFsm <pynusmv.fsm.BddFsm>`
    :param state: a state of `fsm`
    :type state: :class:`State <pynusmv.dd.State>`
    :param a: the set of states of `fsm` satisfying `phi`
    :type a: :class:`BDD <pynusmv.dd.BDD>`

    Return `(s, i, s')` tuple where `s` (:class:`State <pynusmv.dd.State>`)
    is the given state, `s'` (:class:`State <pynusmv.dd.State>`) is a successor
    of `s` belonging to `a` and `i` (:class:`Inputs <pynusmv.dd.Inputs>`)
    is the inputs to go from `s` to `s'` in `fsm`.

    """
    enc = fsm.bddEnc
    manager = enc.DDmanager
    path = nsnode.cons(nsnode.bdd2node(nsdd.bdd_dup(state._ptr)), None)
    bddlist = BDDList(mc.ex_explain(fsm._ptr, enc._ptr, path, a._ptr),
                      manager)

    # bddlist is reversed!
    statep = State.from_bdd(bddlist[0], fsm)
    inputs = Inputs.from_bdd(bddlist[1], fsm)
    state = State.from_bdd(bddlist[2], fsm)

    return (state, inputs, statep)


def explainEU(fsm, state, a, b):
    """
    Explain why `state` of `fsm` satisfies `E[phi U psi]`,
    where `a is the set of states of `fsm` satisfying `phi`
    and `b` is the set of states of `fsm` satisfying `psi`, both represented
    by BDDs.

    :param fsm: the system
    :type fsm: :class:`BddFsm <pynusmv.fsm.BddFsm>`
    :param state: a state of `fsm`
    :type state: :class:`State <pynusmv.dd.State>`
    :param a: the set of states of `fsm` satisfying `phi`
    :type a: :class:`BDD <pynusmv.dd.BDD>`
    :param b: the set of states of `fsm` satisfying `psi`
    :type b: :class:`BDD <pynusmv.dd.BDD>`

    Return a tuple `t` composed of states (:class:`State <pynusmv.dd.State>`)
    and inputs (:class:`Inputs <pynusmv.dd.Inputs>`),
    such that `t[0]` is `state`, `t[-1]` belongs to `b`, and every other state
    of `t` belongs to `a`. The states of `t` are separated by inputs.
    Furthermore, `t` represents a path in `fsm`.

    """

    enc = fsm.bddEnc
    manager = enc.DDmanager
    path = nsnode.cons(nsnode.bdd2node(nsdd.bdd_dup(state._ptr)), None)
    bddlist = BDDList(mc.eu_explain(fsm._ptr, enc._ptr,
                                    path, a._ptr, b._ptr), manager)
    bddlist = bddlist.to_tuple()

    path = []
    path.insert(0, State.from_bdd(bddlist[0], fsm))
    for i in range(1, len(bddlist), 2):
        inputs = Inputs.from_bdd(bddlist[i], fsm)
        state = State.from_bdd(bddlist[i + 1], fsm)

        path.insert(0, inputs)
        path.insert(0, state)

    return tuple(path)


def explainEG(fsm, state, a):
    """
    Explain why `state` of `fsm` satisfies `EG phi`, where `a` the set of
    states of `fsm` satisfying `phi`, represented by a BDD.

    :param fsm: the system
    :type fsm: :class:`BddFsm <pynusmv.fsm.BddFsm>`
    :param state: a state of `fsm`
    :type state: :class:`State <pynusmv.dd.State>`
    :param a: the set of states of `fsm` satisfying `phi`
    :type a: :class:`BDD <pynusmv.dd.BDD>`

    Return a tuple `(t, (i, loop))`
    where `t` is a tuple composed of states (:class:`State <pynusmv.dd.State>`)
    and inputs (:class:`Inputs <pynusmv.dd.Inputs>`),
    such that `t[0]` is state and every other state of `t`
    belongs to `a`. The states of `t` are separated by inputs.
    Furthermore, `t` represents a path in `fsm`.
    `loop` represents the start of the loop contained in `t`,
    i.e. `t[-1]` can lead to `loop` through `i`, and `loop` is a state of `t`.

    """

    enc = fsm.bddEnc
    manager = enc.DDmanager
    path = nsnode.cons(nsnode.bdd2node(nsdd.bdd_dup(state._ptr)), None)
    bddlist = BDDList(mc.eg_explain(fsm._ptr, enc._ptr, path, a._ptr),
                      manager)
    bddlist = bddlist.to_tuple()

    path = []
    # Discard last state and input, store them as loop indicators
    loopstate = State.from_bdd(bddlist[0], fsm)
    loopinputs = Inputs.from_bdd(bddlist[1], fsm)

    # Consume first state
    curstate = State.from_bdd(bddlist[2], fsm)
    if curstate._ptr == loopstate._ptr:
        loopstate = curstate

    path.insert(0, curstate)

    for i in range(3, len(bddlist), 2):
        inputs = Inputs.from_bdd(bddlist[i], fsm)
        curstate = State.from_bdd(bddlist[i + 1], fsm)
        if curstate._ptr == loopstate._ptr:
            loopstate = curstate

        path.insert(0, inputs)
        path.insert(0, curstate)
    return (tuple(path), (loopinputs, loopstate))
