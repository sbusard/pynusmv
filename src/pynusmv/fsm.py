"""
The :mod:`pynusmv.fsm` module provides some functionalities about FSMs
represented and stored by NuSMV:

* :class:`BddFsm` represents the model encoded into BDDs. This gives access
  to elements of the FSM like BDD encoding, initial states, reachable states,
  transition relation, pre and post operations, etc.
* :class:`BddEnc` represents the BDD encoding, with some functionalities like
  getting the state mask or the input variables mask.
* :class:`SymbTable` represents the symbols table of the model.
* :class:`BddTrans` represents a transition relation encoded with BDDs. It
  provides access to pre and post operations.

"""


__all__ = ['BddFsm', 'BddEnc', 'SymbTable', 'BddTrans']


from .nusmv.fsm.bdd import bdd as bddFsm
from .nusmv.enc.bdd import bdd as bddEnc
from .nusmv.enc.base import base as nsbaseEnc
from .nusmv.cmd import cmd as nscmd
from .nusmv.trans.bdd import bdd as nsbddtrans
from .nusmv.set import set as nsset
from .nusmv.compile.symb_table import symb_table as nssymbtable
from .nusmv.compile import compile as nscompile
from .nusmv.node import node as nsnode
from .nusmv.prop import prop as nsprop
from .nusmv.fsm.sexp import sexp as nssexp
from .nusmv.utils import utils as nsutils

from .dd import BDD, State, Inputs, StateInputs, DDManager
from .utils import PointerWrapper, fixpoint
from .exception import NuSMVBddPickingError
from .parser import parse_simple_expression


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
    
    def __init__(self, ptr, freeit = False):
        """
        Create a new BddFsm.
        
        :param ptr: the pointer of the NuSMV FSM
        :param boolean freeit: whether or not free the pointer
        
        """
        super().__init__(ptr, freeit = freeit)
        self._reachable = None
    
        
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
                   freeit = True)
                   
                   
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
                        freeit = False)
        
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
                   self.bddEnc.DDmanager, freeit = True)
                   
                   
    @property
    def inputs_constraints(self):
        """
        The BDD of inputs satisfying the invariants of the FSM.
        
        """
        return BDD(bddFsm.BddFsm_get_input_constraints(self._ptr),
                   self.bddEnc.DDmanager, freeit = True)
                   
                   
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
                    self.bddEnc.DDmanager, freeit = True))
            ite = bddFsm.FairnessListIterator_next(ite)
            
        return fairBdds
                   
                   
    def pre(self, states, inputs = None):
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
            return BDD(bddFsm.BddFsm_get_backward_image(self._ptr, states._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        else:
            return BDD(bddFsm.BddFsm_get_constrained_backward_image(
                            self._ptr, states._ptr, inputs._ptr),
                       self.bddEnc.DDmanager, freeit = True)
                       
    
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
                   self.bddEnc.DDmanager, freeit = True)
        
        
    def post(self, states, inputs = None):
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
            return BDD(bddFsm.BddFsm_get_forward_image(self._ptr, states._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        else:
            return BDD(bddFsm.BddFsm_get_constrained_forward_image(
                            self._ptr, states._ptr, inputs._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        
        
    def pick_one_state(self, bdd):
        """
        Return a BDD representing a state of `bdd`.
        
        :rtype: :class:`State <pynusmv.dd.State>`
        :raise: a :exc:`NuSMVBddPickingError 
                <pynusmv.exception.NuSMVBddPickingError>`
                if `bdd` is false or an error occurs while picking one state
                (for example if the bdd does not contain any state but inputs)
        
        """
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick state from false BDD.")
        state = bddEnc.pick_one_state(self.bddEnc._ptr, bdd._ptr)
        if state is None:
            raise NuSMVBddPickingError("Cannot pick state from BDD.")
        return State(state, self, freeit = True)

    
    def pick_one_inputs(self, bdd):
        """
        Return a BDD representing an inputs of `bdd`.
        
        :rtype: :class:`Inputs <pynusmv.dd.Inputs>`
        :raise: a :exc:`NuSMVBddPickingError 
                <pynusmv.exception.NuSMVBddPickingError>`
                if `bdd` is false or an error occurs while picking one inputs
                (for example if the bdd does not contain any inputs but states)
        
        """
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick inputs from false BDD.")
        inputs = bddEnc.pick_one_input(self.bddEnc._ptr, bdd._ptr)
        if inputs is None:
            raise NuSMVBddPickingError("Cannot pick inputs from BDD.")
        return Inputs(inputs, self, freeit = True)
        
    
    def get_inputs_between_states(self, current, next):
        """
        Return the BDD representing the possible inputs between `current` and
        `next`.
        
        :param current: the source states
        :type current: :class:`BDD <pynusmv.dd.BDD>`
        :param current: the destination states
        :type current: :class:`BDD <pynusmv.dd.BDD>`
        :rtype: :class:`BDD <pynusmv.dd.BDD>`
        
        """
        inputs = bddFsm.BddFsm_states_to_states_get_inputs(self._ptr,
                                                           current._ptr,
                                                           next._ptr)
        return Inputs(inputs, self, freeit = True)
        
        
    def count_states(self, bdd):
        """
        Return the number of states of the given BDD.
        
        :param bdd: the concerned BDD
        :type bdd: :class:`BDD <pynusmv.dd.BDD>`
                
        """
        # Apply mask before counting states
        bdd = bdd & self.bddEnc.statesMask
        return bddEnc.BddEnc_count_states_of_bdd(self.bddEnc._ptr, bdd._ptr)
        
        
    def count_inputs(self, bdd):
        """
        Return the number of inputs of the given BDD
        
        :param bdd: the concerned BDD
        :type bdd: :class:`BDD <pynusmv.dd.BDD>`
        
        """
        # Apply mask before counting inputs
        bdd = bdd & self.bddEnc.inputsMask
        return bddEnc.BddEnc_count_inputs_of_bdd(self.bddEnc._ptr, bdd._ptr)
        
        
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
        bdd = bdd & self.bddEnc.statesMask
        # Get all states
        (err, t) = bddEnc.pick_all_terms_states(self.bddEnc._ptr, bdd._ptr)
        if err:
            raise NuSMVBddPickingError("Cannot pick all states.")
        else:
            return tuple(State(te, self) for te in t)
            
            
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
        
        mask = self.bddEnc.inputsMask
        
        # Apply mask
        bdd = bdd & mask
        # Get all states
        (err, t) = bddEnc.pick_all_terms_inputs(self.bddEnc._ptr, bdd._ptr)
        if err:
            raise NuSMVBddPickingError("Cannot pick all inputs.")
        else:
            return tuple(Inputs(te, self) for te in t)
    
            
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
        
        # Get all states inputs
        (err, t) = bddEnc.pick_all_terms_states_inputs(self.bddEnc._ptr,
                                                       bdd._ptr)
        if err:
            raise NuSMVBddPickingError("Cannot pick all states.")
        else:
            return tuple(StateInputs(te, self) for te in t)
        
        
    @property    
    def reachable_states(self):
        """
        Return a the set of reachable states of this FSM, represented as a BDD.
        
        :rtype: :class:`BDD <pynusmv.dd.BDD>`
        
        """
        if self._reachable is None:
            #self._reachable = fixpoint(lambda Z: (self.init | self.post(Z)),
            #                           BDD.false(self.bddEnc.DDmanager))
            self._reachable = BDD(bddFsm.BddFsm_get_reachable_states(self._ptr),
                                  self.bddEnc.DDmanager)
        return self._reachable
        
        
    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
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
        # TODO Remove this and use glob module instead


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
                   self.DDmanager, freeit = True)
                   
    
    @property               
    def inputsMask(self):
        """
        The mask for all input variables, represented as a BDD.
        
        :rtype: :class:`BDD <pynusmv.dd.BDD>`
        
        """
    
        return BDD(bddEnc.BddEnc_get_input_vars_mask_bdd(self._ptr),
                   self.DDmanager, freeit = True)
                   
    @property    
    def statesCube(self):
        """
        The cube for all state variables, represented as a BDD.
        
        :rtype: :class:`BDD <pynusmv.dd.BDD>`
        
        """
        
        return BDD(bddEnc.BddEnc_get_state_frozen_vars_cube(self._ptr),
                   self.DDmanager, freeit = True)
                   
    
    @property               
    def inputsCube(self):
        """
        The cube for all input variables, represented as a BDD.
        
        :rtype: :class:`BDD <pynusmv.dd.BDD>`
        
        """
    
        return BDD(bddEnc.BddEnc_get_input_vars_cube(self._ptr),
                   self.DDmanager, freeit = True)
                   
                   
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
            isVar = nssymbtable.SymbTable_is_symbol_input_var(st._ptr, var_node)
            if isVar and varname in variables:
                var_nodes.add(var_node)
            ite = nsutils.ListIter_get_next(ite)
        
        varset = nsset.Set_MakeEmpty()
        for var in var_nodes:
            varset = nsset.Set_AddMember(varset, var)
        
        cube_ptr = bddEnc.BddEnc_get_vars_cube(self._ptr, varset, 
                                               nssymbtable.VFT_INPUT)
                        
        nsset.Set_ReleaseSet(varset)
        
        return BDD(cube_ptr, self.DDmanager, freeit=True)
        
                   
class SymbTable(PointerWrapper):
    """
    Python class for symbols table.
    
    """
    # Symbols tables are never freed. NuSMV takes care of it.
    pass
    
    
class BddTrans(PointerWrapper):
    """
    Python class for transition relation encoded with BDDs.
    
    A BddTrans represents a transition relation and provides pre and post
    operations on BDDs, possibly restricted to given actions.
    
    """
    
    def __init__(self, ptr, enc = None, manager = None, freeit = True):
        """
        Create a new BddTrans.
        
        :param ptr: a NuSMV pointer to a BddTrans
        :param enc: the BDD encoding of the transition relation
        :type enc: :class:`BddEnc`
        :param manager: the DD manager of the BDDs used to encode the relation
        :type manager: :class:`DDManager <pynusmv.dd.DDManager>`
        :param freeit: whether or not freeing the pointer
        
        """
        super().__init__(ptr, freeit)
        self._enc = enc
        self._manager = manager
    
    
    @property
    def monolithic(self):
        """
        This transition relation represented as a monolithic BDD.
        
        :rtype: :class:`BDD <pynusmv.dd.BDD>`
        
        """
        ptr = nsbddtrans.BddTrans_get_monolithic_bdd(self._ptr) 
        return BDD(ptr, self._manager, freeit = True)
        
        
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
            states._manager, freeit = True)
            
        if inputs is not None:
            nexts = nexts & inputs
        img = nsbddtrans.BddTrans_get_backward_image_state(
                                                          self._ptr, nexts._ptr)
        return BDD(img, self._manager, freeit = True)
        
    
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
        return BDD(img, self._manager, freeit = True)