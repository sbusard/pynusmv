"""
The :mod:`pynusmv.fsm` module provides some functionalities about FSMs
represented and stored by NuSMV:

* :class:`BddFsm` represents the model encoded into BDDs. This gives access
  to elements of the FSM like BDD encoding, initial states, reachable states,
  transition relation, pre and post operations, etc.
* :class:`BddTrans` represents a transition relation encoded with BDDs. It
  provides access to pre and post operations.
* :class:`BddEnc` represents the BDD encoding, with some functionalities like
  getting the state mask or the input variables mask.
* :class:`SymbTable` represents the symbols table of the model.

"""


__all__ = ['BddFsm', 'BddTrans', 'BddEnc', 'SymbTable']

import tempfile

from .nusmv.fsm.bdd import bdd as bddFsm
from .nusmv.enc.bdd import bdd as bddEnc
from .nusmv.enc.base import base as nsbaseEnc
from .nusmv.trans.bdd import bdd as nsbddtrans
from .nusmv.set import set as nsset
from .nusmv.compile.symb_table import symb_table as nssymb_table
from .nusmv.compile import compile as nscompile
from .nusmv.node import node as nsnode
from .nusmv.prop import prop as nsprop
from .nusmv.fsm.sexp import sexp as nssexp
from .nusmv.utils import utils as nsutils

from .nusmv.fsm import fsm as nsfsm
from .nusmv.opt import opt as nsopt
from .nusmv.compile.type_checking import type_checking as nstype_checking

from .dd import BDD, State, Inputs, StateInputs, DDManager, Cube
from .utils import PointerWrapper, AttributeDict
from .exception import (NuSMVBddPickingError, NuSMVFlatteningError,
                        NuSMVTypeCheckingError, NuSMVSymbTableError)
from .parser import parse_next_expression
from . import node


class BddFsm(PointerWrapper):

    """
    Python class for FSM structure, encoded into BDDs.

    The BddFsm provides some functionalities on the FSM: getting initial and
    reachable states as a BDD, getting or replacing the transition relation,
    getting fairness, state and inputs constraints, getting pre and post images
    of BDDs, possibly through particular actions, picking and counting states
    and actions of given BDDs.

    """
    # BddFsm do not have to be freed.

    def __init__(self, ptr, freeit=False):
        """
        Create a new BddFsm.

        :param ptr: the pointer of the NuSMV FSM
        :param boolean freeit: whether or not free the pointer

        """
        super(BddFsm, self).__init__(ptr, freeit=freeit)
        self._reachable = None
        self._deadlock = None
        self._fair = None
    
    def __deepcopy__(self, memo):
        # No need to copy this FSM
        return self

    @property
    def bddEnc(self):
        """
        The BDD encoding of this FSM.

        """
        return BddEnc(bddFsm.BddFsm_get_bdd_encoding(self._ptr))

    @property
    def init(self):
        """
        The BDD of initial states of this FSM.

        """
        return BDD(bddFsm.BddFsm_get_init(self._ptr), self.bddEnc.DDmanager,
                   freeit=True)

    @property
    def trans(self):
        """
        The transition relation (:class:`BddTrans`) of this FSM.
        Can also be replaced.

        """
        # Do not free the trans, this FSM is the owner of it
        return BddTrans(bddFsm.BddFsm_get_trans(self._ptr),
                        self.bddEnc,
                        self.bddEnc.DDmanager,
                        freeit=False)

    @trans.setter
    def trans(self, new_trans):
        """
        Set this FSM transition to `new_trans`.

        """
        # Copy the transition such that this FSM is the owner
        new_trans_ptr = nsbddtrans.BddTrans_copy(new_trans._ptr)
        # Get old trans
        old_trans_ptr = bddFsm.BddFsm_get_trans(self._ptr)
        # Set the new trans
        self._ptr.trans = new_trans_ptr
        # Free old trans
        nsbddtrans.BddTrans_free(old_trans_ptr)

    @property
    def state_constraints(self):
        """
        The BDD of states satisfying the invariants of the FSM.

        """
        return BDD(bddFsm.BddFsm_get_state_constraints(self._ptr),
                   self.bddEnc.DDmanager, freeit=True)

    @property
    def inputs_constraints(self):
        """
        The BDD of inputs satisfying the invariants of the FSM.

        """
        return BDD(bddFsm.BddFsm_get_input_constraints(self._ptr),
                   self.bddEnc.DDmanager, freeit=True)

    @property
    def fairness_constraints(self):
        """
        The list of fairness constraints, as BDDs.

        """
        justiceList = bddFsm.BddFsm_get_justice(self._ptr)
        fairnessList = bddFsm.justiceList2fairnessList(justiceList)

        ite = bddFsm.FairnessList_begin(fairnessList)
        fairBdds = []
        while not bddFsm.FairnessListIterator_is_end(ite):
            fairBdds.append(
                BDD(bddFsm.JusticeList_get_p(justiceList, ite),
                    self.bddEnc.DDmanager, freeit=True))
            ite = bddFsm.FairnessListIterator_next(ite)

        return fairBdds
    
    @property
    def reachable_states(self):
        """
        The set of reachable states of this FSM, represented as a BDD.

        """
        if self._reachable is None:
            self._reachable = BDD(bddFsm.BddFsm_get_reachable_states( 
                                  self._ptr),
                                  self.bddEnc.DDmanager)
        return self._reachable
    
    @reachable_states.setter
    def reachable_states(self, reachable_states):
        self._reachable = reachable_states
        bddFsm.BddFsm_set_reachable_states(self._ptr, reachable_states._ptr)

    @property
    def deadlock_states(self):
        """
        The set of reachable states of the system with no successor.

        """
        if self._deadlock is None:
            self._deadlock = BDD(bddFsm.
                                 BddFsm_get_deadlock_states(self._ptr),
                                 self.bddEnc.DDmanager)
        return self._deadlock
    
    @property
    def fair_states(self):
        """
        The set of fair states of this FSM, represented as a BDD.

        """
        if self._fair is None:
            self._fair = BDD(bddFsm.BddFsm_get_fair_states(self._ptr),
                             self.bddEnc.DDmanager)
        return self._fair
    
    def pre(self, states, inputs=None):
        """
        Return the pre-image of `states` in this FSM.
        If `inputs` is not `None`, it is used as constraints to get
        pre-states that are reachable through these inputs.

        :param states: the states from which getting the pre-image
        :type states: :class:`BDD <pynusmv.dd.BDD>`
        :param inputs: the inputs through which getting the pre-image
        :type inputs: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """
        if inputs is None:
            return BDD(bddFsm.BddFsm_get_backward_image(
                       self._ptr,states._ptr),
                       self.bddEnc.DDmanager,
                       freeit=True)
        else:
            return BDD(bddFsm.BddFsm_get_constrained_backward_image
                       (self._ptr, states._ptr, inputs._ptr),
                       self.bddEnc.DDmanager, freeit=True)

    def weak_pre(self, states):
        """
        Return the weak pre-image of `states` in this FSM. This means that it
        returns a BDD representing the set of states with corresponding inputs
        <s,i> such that there is a state in `state` reachable from s through i.

        :param states: the states from which getting the weak pre-image
        :type states: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """
        return BDD(bddFsm.BddFsm_get_weak_backward_image(self._ptr,
                                                         states._ptr),
                   self.bddEnc.DDmanager, freeit=True)

    def post(self, states, inputs=None):
        """
        Return the post-image of `states` in this FSM.
        If `inputs` is not `None`, it is used as constraints to get
        post-states that are reachable through these inputs.

        :param states: the states from which getting the post-image
        :type states: :class:`BDD <pynusmv.dd.BDD>`
        :param inputs: the inputs through which getting the post-image
        :type inputs: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """
        if inputs is None:
            return BDD(bddFsm.BddFsm_get_forward_image(self._ptr,
                                                       states._ptr),
                       self.bddEnc.DDmanager, freeit=True)
        else:
            return BDD(bddFsm.BddFsm_get_constrained_forward_image
                       (self._ptr, states._ptr, inputs._ptr),
                       self.bddEnc.DDmanager, freeit=True)

    def pick_one_state(self, bdd):
        """
        Return a BDD representing a state of `bdd`.

        :rtype: :class:`State <pynusmv.dd.State>`
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if `bdd` is false or an error occurs while picking one state

        """
        # Abstract inputs
        bdd = bdd.forsome(self.bddEnc.inputsCube)
        
        # Apply states mask
        bdd = bdd & self.bddEnc.statesMask

        # The BDD contains no states
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick state from false BDD.")

        state = bddEnc.pick_one_state(self.bddEnc._ptr, bdd._ptr)
        if state is None:
            raise NuSMVBddPickingError("Cannot pick state from BDD.")
        return State(state, self, freeit=True)
    
    def pick_one_state_random(self, bdd):
        """
        Return a BDD representing a state of `bdd`, picked at random.

        :rtype: :class:`State <pynusmv.dd.State>`
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if `bdd` is false or an error occurs while picking one state

        """
        # Abstract inputs
        bdd = bdd.forsome(self.bddEnc.inputsCube)
        
        # Apply states mask
        bdd = bdd & self.bddEnc.statesMask

        # The BDD contains no states
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick state from false BDD.")

        state = bddEnc.pick_one_state_rand(self.bddEnc._ptr, bdd._ptr)
        if state is None:
            raise NuSMVBddPickingError("Cannot pick state from BDD.")
        return State(state, self, freeit=True)

    def pick_one_inputs(self, bdd):
        """
        Return a BDD representing an inputs of `bdd`.

        :rtype: :class:`Inputs <pynusmv.dd.Inputs>`
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if `bdd` is false or an error occurs while picking one inputs

        """
        # Abstract inputs
        bdd = bdd.forsome(self.bddEnc.statesCube)
        
        # Apply inputs mask
        bdd = bdd & self.bddEnc.inputsMask

        # The BDD contains no states
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick inputs from false BDD.")
        inputs = bddEnc.pick_one_input(self.bddEnc._ptr, bdd._ptr)
        if inputs is None:
            raise NuSMVBddPickingError("Cannot pick inputs from BDD.")
        return Inputs(inputs, self, freeit=True)
    
    def pick_one_inputs_random(self, bdd):
        """
        Return a BDD representing an inputs of `bdd`, picked at random.

        :rtype: :class:`Inputs <pynusmv.dd.Inputs>`
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if `bdd` is false or an error occurs while picking one inputs

        """
        # Abstract inputs
        bdd = bdd.forsome(self.bddEnc.statesCube)
        
        # Apply inputs mask
        bdd = bdd & self.bddEnc.inputsMask

        # The BDD contains no states
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick inputs from false BDD.")
        inputs = bddEnc.pick_one_input_rand(self.bddEnc._ptr, bdd._ptr)
        if inputs is None:
            raise NuSMVBddPickingError("Cannot pick inputs from BDD.")
        return Inputs(inputs, self, freeit=True)

    def pick_one_state_inputs(self, bdd):
        """
        Return a BDD representing a state/inputs pair of `bdd`.

        :rtype: :class:`StateInputs <pynusmv.dd.StateInputs>`
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if `bdd` is false or an error occurs while picking one pair

        """
        bdd = bdd & self.bddEnc.statesInputsMask
        # The BDD contains no pairs
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick state/inputs"
                                       " from false BDD.")
        si = bddEnc.pick_one_state_input(self.bddEnc._ptr, bdd._ptr)
        if si is None:
            raise NuSMVBddPickingError("Cannot pick state/inputs from BDD.")
        return StateInputs(si, self, freeit=True)
    
    def pick_one_state_inputs_random(self, bdd):
        """
        Return a BDD representing a state/inputs pair of `bdd`, picked at
        random.

        :rtype: :class:`StateInputs <pynusmv.dd.StateInputs>`
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if `bdd` is false or an error occurs while picking one pair

        """
        bdd = bdd & self.bddEnc.statesInputsMask
        # The BDD contains no pairs
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick state/inputs"
                                       " from false BDD.")
        si = bddEnc.pick_one_state_input_rand(self.bddEnc._ptr, bdd._ptr)
        if si is None:
            raise NuSMVBddPickingError("Cannot pick state/inputs from BDD.")
        return StateInputs(si, self, freeit=True)

    def get_inputs_between_states(self, current, next_):
        """
        Return the BDD representing the possible inputs between `current` and
        `next_`.

        :param current: the source states
        :type current: :class:`BDD <pynusmv.dd.BDD>`
        :param next_: the destination states
        :type next_: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """
        inputs = bddFsm.BddFsm_states_to_states_get_inputs(self._ptr,
                                                           current._ptr,
                                                           next_._ptr)
        return (BDD(inputs, self.bddEnc.DDmanager, freeit=True)
                & self.bddEnc.inputsMask)

    def count_states(self, bdd):
        """
        Return the number of states of the given BDD.

        :param bdd: the concerned BDD
        :type bdd: :class:`BDD <pynusmv.dd.BDD>`

        """
        # Apply mask before counting states
        bdd = bdd & self.bddEnc.statesMask
        return int(bddEnc.
                   BddEnc_count_states_of_bdd(self.bddEnc._ptr, bdd._ptr))

    def count_inputs(self, bdd):
        """
        Return the number of inputs of the given BDD

        :param bdd: the concerned BDD
        :type bdd: :class:`BDD <pynusmv.dd.BDD>`

        """
        # Apply mask before counting inputs
        bdd = bdd & self.bddEnc.inputsMask
        return int(bddEnc.
                   BddEnc_count_inputs_of_bdd(self.bddEnc._ptr, bdd._ptr))

    def count_states_inputs(self, bdd):
        """
        Return the number of state/inputs pairs of the given BDD

        :param bdd: the concerned BDD
        :type bdd: :class:`BDD <pynusmv.dd.BDD>`

        """
        # Apply mask before counting inputs
        bdd = bdd & self.bddEnc.statesInputsMask
        return int(
            bddEnc. BddEnc_count_states_inputs_of_bdd(
                self.bddEnc._ptr,
                bdd._ptr))

    def pick_all_states(self, bdd):
        """
        Return a tuple of all states belonging to `bdd`.

        :param bdd: the concerned BDD
        :type bdd: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: tuple(:class:`State <pynusmv.dd.State>`)
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if something is wrong

        """
        # FIXME Still get segmentation faults. Need investigation.
        # tests/pynusmv/testFsm.py seems to raise segmentation faults

        # Apply mask
        bdd = bdd.forsome(self.bddEnc.inputsCube) & self.bddEnc.statesMask
        # Get all states
        (err, t) = bddEnc.pick_all_terms_states(self.bddEnc._ptr, bdd._ptr)
        if err:
            raise NuSMVBddPickingError("Cannot pick all states.")
        else:
            return frozenset(State(te, self) for te in t)

    def pick_all_inputs(self, bdd):
        """
        Return a tuple of all inputs belonging to `bdd`.

        :param bdd: the concerned BDD
        :type bdd: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: tuple(:class:`Inputs <pynusmv.dd.Inputs>`)
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if something is wrong

        """
        # FIXME Still get segmentation faults. Need investigation.
        # tests/pynusmv/testFsm.py seems to raise segmentation faults

        # Apply mask
        bdd = bdd.forsome(self.bddEnc.statesCube) & self.bddEnc.inputsMask
        if bdd.is_false():
            return frozenset()
        # Get all inputs
        (err, t) = bddEnc.pick_all_terms_inputs(self.bddEnc._ptr, bdd._ptr)
        if err:
            raise NuSMVBddPickingError("Cannot pick all inputs.")
        else:
            return frozenset(Inputs(te, self) for te in t)

    def pick_all_states_inputs(self, bdd):
        """
        Return a tuple of all states/inputs pairs belonging to `bdd`.

        :param bdd: the concerned BDD
        :type bdd: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: tuple(:class:`StateInputs <pynusmv.dd.StateInputs>`)
        :raise: a :exc:`NuSMVBddPickingError
                <pynusmv.exception.NuSMVBddPickingError>`
                if something is wrong

        """
        # FIXME Still get segmentation faults. Need investigation.
        # tests/pynusmv/testFsm.py seems to raise segmentation faults

        # Apply mask
        bdd = bdd & self.bddEnc.statesInputsMask

        if bdd.is_false():
            return frozenset()

        # Get all states inputs
        (err, t) = bddEnc.pick_all_terms_states_inputs(self.bddEnc._ptr,
                                                       bdd._ptr)
        if err:
            raise NuSMVBddPickingError("Cannot pick all state/inputs pairs.")
        else:
            return frozenset(StateInputs(te, self) for te in t)

    # =========================================================================
    # ===== Static methods ====================================================
    # =========================================================================

    @staticmethod
    def from_filename(filepath):
        """
        Return the FSM corresponding to the model in `filepath`.

        :param filepath: the path to the SMV model

        """
        # This function modifies the global environment of NuSMV.
        from . import glob
        glob.load_from_file(filepath)
        glob.compute_model()
        propDb = glob.prop_database()
        return propDb.master.bddFsm

    @staticmethod
    def from_string(model):
        """
        Return the FSM corresponding to the model defined by the given string.

        :param model: a String representing the SMV model

        """
        # Create temp file
        with tempfile.NamedTemporaryFile(suffix=".smv") as tmp:
            tmp.write(model.encode("UTF-8"))
            tmp.flush()
            return BddFsm.from_filename(tmp.name)

    @staticmethod
    def from_modules(*modules):
        """
        Return the FSM corresponding to the model defined by the given list
        of modules.

        :param modules: the modules defining the NuSMV model. Must contain a
                        `main` module.
        :type modules: a list of :class:`Module <pynusmv.model.Module>`
                       subclasses

        """
        return BddFsm.from_string("\n".join(str(module) for module in modules))


class BddTrans(PointerWrapper):

    """
    Python class for transition relation encoded with BDDs.

    A BddTrans represents a transition relation and provides pre and post
    operations on BDDs, possibly restricted to given actions.

    """

    def __init__(self, ptr, enc=None, manager=None, freeit=True):
        """
        Create a new BddTrans.

        :param ptr: a NuSMV pointer to a BddTrans
        :param enc: the BDD encoding of the transition relation
        :type enc: :class:`BddEnc`
        :param manager: the DD manager of the BDDs used to encode the relation
        :type manager: :class:`DDManager <pynusmv.dd.DDManager>`
        :param freeit: whether or not freeing the pointer

        """
        super(BddTrans, self).__init__(ptr, freeit)
        self._enc = enc
        self._manager = manager

    def _free(self):
        if self._freeit and self._ptr is not None:
            # Free it because such a BddTrans is not owned by anyone
            # nsbddtrans.BddTrans_free(self._ptr)
            # FIXME Freeing BddTrans can cause Segmentation fault. Should learn
            # more about NuSMV internal to detect what and how we should free
            # it
            self._freeit = False

    @property
    def monolithic(self):
        """
        This transition relation represented as a monolithic BDD.

        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """
        ptr = nsbddtrans.BddTrans_get_monolithic_bdd(self._ptr)
        return BDD(ptr, self._manager, freeit=True)

    def pre(self, states, inputs=None):
        """
        Compute the pre-image of `states`, through `inputs` if not `None`.

        :param states: the concerned states
        :type states: :class:`BDD <pynusmv.dd.BDD>`
        :param inputs: possible inputs
        :type inputs: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """
        nexts = BDD(
            bddEnc.BddEnc_state_var_to_next_state_var(self._enc._ptr,
                                                      states._ptr),
            states._manager, freeit=True)

        if inputs is not None:
            nexts = nexts & inputs
        img = nsbddtrans.BddTrans_get_backward_image_state(
            self._ptr, nexts._ptr)
        return BDD(img, self._manager, freeit=True)

    def post(self, states, inputs=None):
        """
        Compute the post-image of `states`, through `inputs` if not `None`.

        :param states: the concerned states
        :type states: :class:`BDD <pynusmv.dd.BDD>`
        :param inputs: possible inputs
        :type inputs: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """
        if inputs is not None:
            states = states & inputs
        img = nsbddtrans.BddTrans_get_forward_image_state(
            self._ptr, states._ptr)
        img = bddEnc.BddEnc_next_state_var_to_state_var(self._enc._ptr, img)
        return BDD(img, self._manager, freeit=True)

    # =========================================================================
    # ===== Static methods ====================================================
    # =========================================================================

    @classmethod
    def from_trans(cls, symb_table, trans, context=None):
        """
        Return a new BddTrans from the given trans.

        :param symb_table: the symbols table used to flatten the trans
        :type symb_table: :class:`SymbTable`
        :param trans: the parsed string of the trans, not flattened
        :param context: an additional parsed context, in which trans will be
                        flattened, if not None
        :rtype: :class:`BddTrans`
        :raise: a :exc:`NuSMVFlatteningError
                <pynusmv.exception.NuSMVFlatteningError>`
                if `trans` cannot be flattened under `context`

        """

        trans = node.find_hierarchy(trans)
        flattrans, err = nscompile.FlattenSexp(symb_table._ptr, trans,
                                               context)
        if err:
            raise NuSMVFlatteningError("Cannot flatten TRANS")

        # Build the BDD trans
        fsmbuilder = nscompile.Compile_get_global_fsm_builder()
        from .glob import bdd_encoding
        enc = bdd_encoding()
        ddmanager = enc.DDmanager

        clusters = nsfsm.FsmBuilder_clusterize_expr(fsmbuilder, enc._ptr,
                                                    flattrans)
        cluster_options = nsbddtrans.ClusterOptions_create(
            nsopt.OptsHandler_get_instance())

        newtransptr = nsbddtrans.BddTrans_create(
            ddmanager._ptr,
            clusters,
            bddEnc.BddEnc_get_state_vars_cube(enc._ptr),
            bddEnc.BddEnc_get_input_vars_cube(enc._ptr),
            bddEnc.BddEnc_get_next_state_vars_cube(enc._ptr),
            nsopt.get_partition_method(
                nsopt.OptsHandler_get_instance()),
            cluster_options)

        nsbddtrans.ClusterOptions_destroy(cluster_options)

        return BddTrans(newtransptr, enc, ddmanager, freeit=True)

    @classmethod
    def from_string(cls, symb_table, strtrans, strcontext=None):
        """
        Return a new BddTrans from the given strtrans, in given strcontex.

        :param symb_table: the symbols table used to flatten the trans
        :type symb_table: :class:`SymbTable`
        :param strtrans: the string representing the trans
        :type strtrans: str
        :param strcontext: an additional string representing a context,
                           in which trans will be flattened, if not None
        :rtype: :class:`BddTrans`
        :raise: a :exc:`NuSMVTypeCheckingError
                <pynusmv.exception.NuSMVTypeCheckingError>`
                if `strtrans` is wrongly typed under `context`

        """
        type_checker = nssymb_table.SymbTable_get_type_checker(symb_table._ptr)

        if strcontext is not None:
            strtrans = "(" + strtrans + ")" + " IN " + strcontext

        # Parse the string
        trans = parse_next_expression(strtrans)

        # FIXME Provoke sometimes segmentation faults
        # Type check
        # expr_type = nstype_checking.TypeChecker_get_expression_type(
        #    type_checker, trans, None)
        # if not nssymb_table.SymbType_is_boolean(expr_type):
        #    raise NuSMVTypeCheckingError("The given TRANS is wrongly typed.")

        # Call from_trans method
        return cls.from_trans(symb_table, trans)


class BddEnc(PointerWrapper):

    """
    Python class for BDD encoding.

    A BddEnc provides some basic functionalities like getting the DD manager
    used to manage BDDs, the symbols table or the state and inputs masks.
    """
    # BddEnc do not have to be freed.

    @property
    def DDmanager(self):
        """
        The DD manager of this encoding.

        :rtype: :class:`DDManager <pynusmv.dd.DDManager>`

        """
        return DDManager(bddEnc.BddEnc_get_dd_manager(self._ptr))

    @property
    def symbTable(self):
        """
        The symbols table of this encoding.

        :rtype: :class:`SymbTable`

        """
        base_enc = bddEnc.bddenc2baseenc(self._ptr)
        return SymbTable(nsbaseEnc.BaseEnc_get_symb_table(base_enc))

    @property
    def statesMask(self):
        """
        The mask for all state variables, represented as a BDD.

        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """

        return BDD(bddEnc.BddEnc_get_state_frozen_vars_mask_bdd(self._ptr),
                   self.DDmanager, freeit=True)

    @property
    def inputsMask(self):
        """
        The mask for all input variables, represented as a BDD.

        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """

        return BDD(bddEnc.BddEnc_get_input_vars_mask_bdd(self._ptr),
                   self.DDmanager, freeit=True)

    @property
    def statesInputsMask(self):
        """
        The mask for all input and state variables, represented as a BDD.

        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """

        return self.statesMask & self.inputsMask

    @property
    def statesCube(self):
        """
        The cube for all state variables, represented as a BDD.

        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """

        return Cube(bddEnc.BddEnc_get_state_frozen_vars_cube(self._ptr),
                    self.DDmanager, freeit=True)

    @property
    def inputsCube(self):
        """
        The cube for all input variables, represented as a BDD.

        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """

        return Cube(bddEnc.BddEnc_get_input_vars_cube(self._ptr),
                    self.DDmanager, freeit=True)

    def cube_for_inputs_vars(self, variables):
        """
        Return the cube for the given input variables.

        :param variables: a list of input variable names
        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """

        from . import glob

        master = glob.prop_database().master
        sexp_fsm = nsprop.Prop_get_scalar_sexp_fsm(master._ptr)
        st = glob.symb_table()

        inputs = nssexp.SexpFsm_get_vars_list(sexp_fsm)

        var_nodes = set()
        ite = nsutils.NodeList_get_first_iter(inputs)
        while not nsutils.ListIter_is_end(ite):
            var_node = nsutils.NodeList_get_elem_at(inputs, ite)
            varname = nsnode.sprint_node(var_node)
            isVar = nssymb_table.SymbTable_is_symbol_input_var(
                st._ptr, var_node)
            if isVar and varname in variables:
                var_nodes.add(var_node)
            ite = nsutils.ListIter_get_next(ite)

        varset = nsset.Set_MakeEmpty()
        for var in var_nodes:
            varset = nsset.Set_AddMember(varset, var)

        cube_ptr = bddEnc.BddEnc_get_vars_cube(self._ptr, varset,
                                               nssymb_table.VFT_INPUT)

        nsset.Set_ReleaseSet(varset)

        return Cube(cube_ptr, self.DDmanager, freeit=True)

    def cube_for_state_vars(self, variables):
        """
        Return the cube for the given state variables.

        :param variables: a list of state variable names
        :rtype: :class:`BDD <pynusmv.dd.BDD>`

        """

        from . import glob

        master = glob.prop_database().master
        sexp_fsm = nsprop.Prop_get_scalar_sexp_fsm(master._ptr)
        st = glob.symb_table()

        inputs = nssexp.SexpFsm_get_vars_list(sexp_fsm)

        var_nodes = set()
        ite = nsutils.NodeList_get_first_iter(inputs)
        while not nsutils.ListIter_is_end(ite):
            var_node = nsutils.NodeList_get_elem_at(inputs, ite)
            varname = nsnode.sprint_node(var_node)
            isVar = nssymb_table.SymbTable_is_symbol_state_var(st._ptr,
                                                               var_node)
            if isVar and varname in variables:
                var_nodes.add(var_node)
            ite = nsutils.ListIter_get_next(ite)

        varset = nsset.Set_MakeEmpty()
        for var in var_nodes:
            varset = nsset.Set_AddMember(varset, var)

        cube_ptr = bddEnc.BddEnc_get_vars_cube(self._ptr, varset,
                                               nssymb_table.VFT_STATE)

        nsset.Set_ReleaseSet(varset)

        return Cube(cube_ptr, self.DDmanager, freeit=True)

    @property
    def inputsVars(self):
        """
        Return the set of inputs variables names.

        :rtype: frozenset(str)

        """

        from . import glob

        master = glob.prop_database().master
        sexp_fsm = nsprop.Prop_get_scalar_sexp_fsm(master._ptr)
        st = glob.symb_table()

        variables = nssexp.SexpFsm_get_vars_list(sexp_fsm)

        varnames = set()
        ite = nsutils.NodeList_get_first_iter(variables)
        while not nsutils.ListIter_is_end(ite):
            var_node = nsutils.NodeList_get_elem_at(variables, ite)
            varname = nsnode.sprint_node(var_node)
            isVar = nssymb_table.SymbTable_is_symbol_input_var(
                st._ptr, var_node)
            if isVar:
                varnames.add(varname)
            ite = nsutils.ListIter_get_next(ite)

        return frozenset(varnames)

    @property
    def stateVars(self):
        """
        Return the set of state variables names.

        :rtype: frozenset(str)

        """

        from . import glob

        master = glob.prop_database().master
        sexp_fsm = nsprop.Prop_get_scalar_sexp_fsm(master._ptr)
        st = glob.symb_table()

        variables = nssexp.SexpFsm_get_vars_list(sexp_fsm)

        varnames = set()
        ite = nsutils.NodeList_get_first_iter(variables)
        while not nsutils.ListIter_is_end(ite):
            var_node = nsutils.NodeList_get_elem_at(variables, ite)
            varname = nsnode.sprint_node(var_node)
            isVar = nssymb_table.SymbTable_is_symbol_state_frozen_var(
                st._ptr, var_node)
            if isVar:
                varnames.add(varname)
            ite = nsutils.ListIter_get_next(ite)

        return frozenset(varnames)

    @property
    def definedVars(self):
        """
        Return the set of defined variables names.

        :rtype: frozenset(str)

        """

        from . import glob

        master = glob.prop_database().master
        sexp_fsm = nsprop.Prop_get_scalar_sexp_fsm(master._ptr)
        st = glob.symb_table()

        variables = nssexp.SexpFsm_get_symbols_list(sexp_fsm)

        varnames = set()
        ite = nsutils.NodeList_get_first_iter(variables)
        while not nsutils.ListIter_is_end(ite):
            var_node = nsutils.NodeList_get_elem_at(variables, ite)
            varname = nsnode.sprint_node(var_node)
            isVar = nssymb_table.SymbTable_is_symbol_define(st._ptr, var_node)
            if isVar:
                varnames.add(varname)
            ite = nsutils.ListIter_get_next(ite)

        return frozenset(varnames)

    def get_variables_ordering(self, var_type="scalar"):
        """
        Return the order of variables.

        :param var_type: the type of variables needed; `"scalar"` for only
                         scalar variables (one variable per model variable),
                         `"bits"` for bits for each scalar variables
                         (default: "scalar")

        :rtype: tuple(str)

        """
        ord_type = (bddEnc.DUMP_BITS
                    if var_type == "bits"
                    else bddEnc.DUMP_DEFAULT)
        var_list = bddEnc.BddEnc_get_var_ordering(self._ptr, ord_type)
        variables = []
        it = nsutils.NodeList_get_first_iter(var_list)
        while not nsutils.ListIter_is_end(it):
            node = nsutils.NodeList_get_elem_at(var_list, it)
            variables.append(nsnode.sprint_node(node))
            it = nsutils.ListIter_get_next(it)
        nsutils.NodeList_destroy(var_list)
        return tuple(variables)

    def force_variables_ordering(self, order):
        """
        Reorder variables based on the given order.

        :param order: a list of variables names (scalar and/or bits) of the
                      system; variables that are not part of the system are
                      ignored (a warning is printed), variables of the system
                      that are not in order are put at the end of the new
                      order.

        ..note:: For more information on variables orders, see NuSMV
                 documentation.
        """
        # Create temp file
        with tempfile.NamedTemporaryFile(suffix=".ord") as tmp:
            tmp.write("\n".join(str(var) for var in order).encode("UTF-8"))
            tmp.flush()
            bddEnc.BddEnc_force_order_from_filename(self._ptr, tmp.name)
        

class SymbTable(PointerWrapper):

    """
    Python class for symbols table.

    """

    ins_policies = AttributeDict(
        SYMB_LAYER_POS_DEFAULT=nssymb_table.SYMB_LAYER_POS_DEFAULT,
        SYMB_LAYER_POS_FORCE_TOP=nssymb_table.SYMB_LAYER_POS_FORCE_TOP,
        SYMB_LAYER_POS_TOP=nssymb_table.SYMB_LAYER_POS_TOP,
        SYMB_LAYER_POS_BOTTOM=nssymb_table.SYMB_LAYER_POS_BOTTOM,
        SYMB_LAYER_POS_FORCE_BOTTOM=nssymb_table.SYMB_LAYER_POS_FORCE_BOTTOM
    )
    
    SYMBOL_STATE_VAR = nssymb_table.SYMBOL_STATE_VAR
    SYMBOL_FROZEN_VAR = nssymb_table.SYMBOL_FROZEN_VAR
    SYMBOL_INPUT_VAR = nssymb_table.SYMBOL_INPUT_VAR

    @property
    def layer_names(self):
        """The names of the layers of this symbol table."""
        layer_names = []

        layers = nssymb_table.SymbTable_get_layers(self._ptr)
        it = nsutils.NodeList_get_first_iter(layers)
        while not nsutils.ListIter_is_end(it):
            layer = nssymb_table.node2layer(
                nsutils.NodeList_get_elem_at(layers, it))
            layer_name = nssymb_table.SymbLayer_get_name(layer)

            layer_names.append(layer_name)

            it = nsutils.ListIter_get_next(it)

        return tuple(layer_names)

    def create_layer(self, layer_name,
                     ins_policy=ins_policies.SYMB_LAYER_POS_DEFAULT):
        """
        Create a new layer in this symbol table.

        :param layer_name: the name of the created layer
        :type layer_name: :class:`str`
        :param ins_policy: the insertion policy for inserting the new layer
        """
        if nssymb_table.SymbTable_get_layer(self._ptr, layer_name) is not None:
            raise NuSMVSymbTableError("Layer %s already exists." % layer_name)
        nssymb_table.SymbTable_create_layer(self._ptr, layer_name, ins_policy)

    def get_variable_type(self, variable):
        """
        Return the type of the given variable.

        :param variable: the name of the variable
        :type variable: :class:`Node`
        :rtype: a NuSMV `SymbType_ptr`

        .. warning:: The returned pointer must not be altered or freed.
        """
        rs = nssymb_table.SymbTable_resolve_symbol(self._ptr, variable._ptr,
                                                   None)
        variable = node.Node.from_ptr(nssymb_table.
                                      ResolveSymbol_get_resolved_name(rs))
        return nssymb_table.SymbTable_get_var_type(self._ptr,
                                                   variable._ptr)

    def can_declare_var(self, layer, variable):
        """
        Return whether the given `variable` name can be declared in `layer`.

        :param layer: the name of the layer
        :type layer: :class:`str`
        :param variable: the name of the variable
        :type variable: :class:`Node <pynusmv.node.Node>`
        :rtype: :class:`bool`
        """
        rs = nssymb_table.SymbTable_resolve_symbol(self._ptr, variable._ptr,
                                                   None)
        variable = node.Node.from_ptr(nssymb_table.
                                      ResolveSymbol_get_resolved_name(rs))
        layer = self._get_layer(layer)
        return (nssymb_table.
                SymbLayer_can_declare_var(layer, variable._ptr) != 0)

    def declare_input_var(self, layer, ivar, type_):
        """
        Declare a new input variable in this symbol table.

        :param layer: the name of the layer in which insert the variable
        :type layer: :class:`str`
        :param ivar: the name of the input variable
        :type ivar: :class:`Node <pynusmv.node.Node>`
        :param type_: the type of the declared input variable
        :type type_: :class:`Node <pynusmv.node.Node>`
        :raise: a :exc:`NuSMVSymbTableError
            <pynusmv.exception.NuSMVSymbTableError>` if the variable is already
            defined in the given layer

        .. warning:: `type_` must be already resolved, that is, the body
                     of `type_` must be leaf values.
        """

        rs = nssymb_table.SymbTable_resolve_symbol(self._ptr, ivar._ptr,
                                                   None)
        ivar = node.Node.from_ptr(nssymb_table.
                                  ResolveSymbol_get_resolved_name(rs))
        if not self.can_declare_var(layer, ivar):
            raise NuSMVSymbTableError("Variable " + str(ivar) + " cannot be "
                                      "declared in " + layer + ".")

        if isinstance(type_, node.Node):
            type_ = self._get_type_from_node(type_)
        layer = self._get_layer(layer)
        nssymb_table.SymbLayer_declare_input_var(layer, ivar._ptr, type_)

    def declare_state_var(self, layer, var, type_):
        """
        Declare a new state variable in this symbol table.

        :param layer: the name of the layer in which insert the variable
        :type layer: :class:`str`
        :param var: the name of the state variable
        :type var: :class:`Node <pynusmv.node.Node>`
        :param type_: the type of the declared state variable
        :type type_: :class:`Node <pynusmv.node.Node>`
        :raise: a :exc:`NuSMVSymbTableError
            <pynusmv.exception.NuSMVSymbTableError>` if the variable is already
            defined in the given layer

        .. warning:: `type_` must be already resolved, that is, the body
                     of `type_` must be leaf values.
        """

        rs = nssymb_table.SymbTable_resolve_symbol(self._ptr, var._ptr,
                                                   None)
        var = node.Node.from_ptr(node.find_hierarchy
                                 (nssymb_table.
                                  ResolveSymbol_get_resolved_name(rs)))
        if not self.can_declare_var(layer, var):
            raise NuSMVSymbTableError("Variable " + str(var) + " cannot be "
                                      "declared in " + layer + ".")

        if isinstance(type_, node.Node):
            type_ = self._get_type_from_node(type_)
        layer = self._get_layer(layer)
        nssymb_table.SymbLayer_declare_state_var(layer, var._ptr, type_)

    def declare_frozen_var(self, layer, fvar, type_):
        """
        Declare a new frozen variable in this symbol table.

        :param layer: the name of the layer in which insert the variable
        :type layer: :class:`str`
        :param fvar: the name of the frozen variable
        :type fvar: :class:`Node <pynusmv.node.Node>`
        :param type_: the type of the declared frozen variable
        :type type_: :class:`Node <pynusmv.node.Node>`
        :raise: a :exc:`NuSMVSymbTableError
            <pynusmv.exception.NuSMVSymbTableError>` if the variable is already
            defined in the given layer

        .. warning:: `type_` must be already resolved, that is, the body
                     of `type_` must be leaf values.
        """

        rs = nssymb_table.SymbTable_resolve_symbol(self._ptr, fvar._ptr,
                                                   None)
        fvar = node.Node.from_ptr(nssymb_table.
                                  ResolveSymbol_get_resolved_name(rs))
        if not self.can_declare_var(layer, fvar):
            raise NuSMVSymbTableError("Variable " + str(fvar) + " cannot be "
                                      "declared in " + layer + ".")

        if isinstance(type_, node.Node):
            type_ = self._get_type_from_node(type_)
        layer = self._get_layer(layer)
        nssymb_table.SymbLayer_declare_frozen_var(layer, fvar._ptr, type_)
    
    def declare_var(self, layer, name, type_, kind):
        """
        Declare a new variable in this symbol table.
        
        :param layer: the name of the layer in which insert the variable
        :type layer: :class:`str`
        :param name: the name of the variable
        :type name: :class:`Node <pynusmv.node.Node>`
        :param type_: the type of the declared variable
        :type type_: :class:`Node <pynusmv.node.Node>`
        :param kind: the kind of the declared variable
        :raise: a :exc:`NuSMVSymbTableError
            <pynusmv.exception.NuSMVSymbTableError>` if the variable is already
            defined in the given layer

        .. warning:: `type_` must be already resolved, that is, the body
                     of `type_` must be leaf values.
        """
        if kind is self.SYMBOL_STATE_VAR:
            self.declare_state_var(layer, name, type_)
        elif kind is self.SYMBOL_FROZEN_VAR:
            self.declare_frozen_var(layer, name, type_)
        elif kind is self.SYMBOL_INPUT_VAR:
            self.declare_input_var(layer, name, type_)

    def is_input_var(self, ivar):
        """
        Return whether the given `var` name is a declared input variable.

        :param ivar: the name of the input variable
        :type ivar: :class:`Node <pynusmv.node.Node>`
        :raise: a :exc:`NuSMVSymbTableError
            <pynusmv.exception.NuSMVSymbTableError>` if the variable is not
            defined in this symbol table
        """
        rs = nssymb_table.SymbTable_resolve_symbol(self._ptr, ivar._ptr,
                                                   None)
        ivar = node.Node.from_ptr(nssymb_table.
                                  ResolveSymbol_get_resolved_name(rs))
        if not nssymb_table.SymbTable_is_symbol_declared(self._ptr, ivar._ptr):
            raise NuSMVSymbTableError(str(ivar) + " is not declared.")
        return (nssymb_table.SymbTable_is_symbol_state_var(self._ptr,
                                                           ivar._ptr) != 0)

    def is_state_var(self, var):
        """
        Return whether the given `var` name is a declared state variable.

        :param var: the name of the state variable
        :type var: :class:`Node <pynusmv.node.Node>`
        :raise: a :exc:`NuSMVSymbTableError
            <pynusmv.exception.NuSMVSymbTableError>` if the variable is not
            defined in this symbol table
        """
        rs = nssymb_table.SymbTable_resolve_symbol(self._ptr, var._ptr,
                                                   None)
        var = node.Node.from_ptr(nssymb_table.
                                 ResolveSymbol_get_resolved_name(rs))
        if not nssymb_table.SymbTable_is_symbol_declared(self._ptr, var._ptr):
            raise NuSMVSymbTableError(str(var) + " is not declared.")
        return (nssymb_table.SymbTable_is_symbol_state_var(self._ptr, var._ptr)
                != 0)

    def is_frozen_var(self, fvar):
        """
        Return whether the given `var` name is a declared frozen variable.

        :param fvar: the name of the frozen variable
        :type fvar: :class:`Node <pynusmv.node.Node>`
        :raise: a :exc:`NuSMVSymbTableError
            <pynusmv.exception.NuSMVSymbTableError>` if the variable is not
            defined in this symbol table
        """
        rs = nssymb_table.SymbTable_resolve_symbol(self._ptr, fvar._ptr,
                                                   None)
        fvar = node.Node.from_ptr(nssymb_table.
                                  ResolveSymbol_get_resolved_name(rs))
        if not nssymb_table.SymbTable_is_symbol_declared(self._ptr, fvar._ptr):
            raise NuSMVSymbTableError(str(fvar) + " is not declared.")
        return (nssymb_table.SymbTable_is_symbol_state_var(self._ptr,
                                                           fvar._ptr) != 0)

    def _get_layer(self, layer_name):
        """
        Get the NuSMV pointer for the layer with `layer_name`.

        :param :class:`str` layer_name: the name of the layer to get
        :raise: a :exc:`NuSMVSymbTableError
            <pynusmv.exception.NuSMVSymbTableError>` if the given layer does
            not exist
        """
        if layer_name not in self.layer_names:
            raise NuSMVSymbTableError("Unknown layer:" + layer_name)
        return nssymb_table.SymbTable_get_layer(self._ptr, layer_name)

    def _get_type_from_node(self, type_):
        """
        Return the NuSMV pointer of the SymbType corresponding to `type_`.

        :param type_: the type
        :type type_: :class:`Type <pynusmv.node.Type>`

        .. warning:: `type_` must be already resolved, that is, the body
                     of `type_` must be leaf values.
        """

        # Boolean
        if isinstance(type_, node.Boolean):
            return nssymb_table.SymbTablePkg_boolean_type()

        # Unsigned word
        elif isinstance(type_, node.UnsignedWord):
            return (nssymb_table.
                    SymbType_create(nssymb_table.SYMB_TYPE_UNSIGNED_WORD,
                                    type_.length._ptr))

        # Signed word
        elif isinstance(type_, node.SignedWord):
            return (nssymb_table.
                    SymbType_create(nssymb_table.SYMB_TYPE_SIGNED_WORD,
                                    type_.length._ptr))

        # Range
        elif isinstance(type_, node.Range):
            # Since body is composed of leaf values, start and stop are numbers
            # and can be directly used as numbers
            start = type_.start.value
            stop = type_.stop.value

            res = None
            for i in range(stop, start - 1, -1):
                res = node.Cons(Number(i), res)
            return (nssymb_table.SymbType_create(nssymb_table.SYMB_TYPE_ENUM,
                                                 res._ptr))

        # Scalar
        elif isinstance(type_, node.Scalar):
            return (nssymb_table.
                    SymbType_create(nssymb_table.SYMB_TYPE_ENUM,
                                    type_.car._ptr))

        else:
            raise NuSMVSymbTableError("Cannot create type for " +
                                      str(type_) + ".")
