__all__ = ['BddFsm']

from .nusmv.fsm.bdd import bdd as bddFsm
from .nusmv.enc.bdd import bdd as bddEnc
from .nusmv.cmd import cmd as nscmd
from .nusmv.trans.bdd import bdd as nsbddtrans
     
from .enc.enc import BddEnc
from .dd.bdd import BDD
from .dd.state import State
from .dd.inputs import Inputs
from .utils.pointerwrapper import PointerWrapper
from .utils.misc import fixpoint
from .trans.trans import BddTrans
from .utils.exception import NuSMVBddPickingError

class BddFsm(PointerWrapper):
    """
    Python class for BddFsm structure.
    
    The BddFsm class contains a pointer to a BddFsm in NuSMV and provides a set
    of operations on this FSM.
    
    BddFsm do not have to be freed.
    """
    
    def __init__(self, ptr, freeit = False):
        """
        Create a new BddFsm.
        
        ptr -- the pointer of the NuSMV FSM
        freeit -- whether or not free the pointer
        """
        super().__init__(ptr, freeit = freeit)
        self._reachable = None
    
        
    @property
    def bddEnc(self):
        """The BDD encoding of this FSM."""
        return BddEnc(bddFsm.BddFsm_get_bdd_encoding(self._ptr))
        
    
    @property
    def init(self):
        """The BDD of initial states of this FSM."""
        return BDD(bddFsm.BddFsm_get_init(self._ptr), self.bddEnc.DDmanager,
                   freeit = True)
                   
                   
    @property
    def trans(self):
        """The transition relation of this FSM."""
        # Do not free the trans, this FSM is the owner of it
        return BddTrans(bddFsm.BddFsm_get_trans(self._ptr),
                        self.bddEnc,
                        self.bddEnc.DDmanager,
                        freeit = False)
        
    @trans.setter
    def trans(self, new_trans):
        """Set this FSM transition to new_trans."""
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
        """The BDD of states satisfying the invariants of the FSM."""
        return BDD(bddFsm.BddFsm_get_state_constraints(self._ptr),
                   self.bddEnc.DDmanager, freeit = True)
                   
                   
    @property
    def inputs_constraints(self):
        """The BDD of inputs satisfying the invariants of the FSM."""
        return BDD(bddFsm.BddFsm_get_input_constraints(self._ptr),
                   self.bddEnc.DDmanager, freeit = True)
                   
                   
    @property
    def fairness_constraints(self):
        """The list of fairness constraints, as BDDs."""
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
        Return the pre-image of states in this FSM.
        
        If inputs is not None, it is used as constraints to get pre-states
        that are reachable through these inputs.
        """
        if inputs is None:
            return BDD(bddFsm.BddFsm_get_backward_image(self._ptr, states._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        else:
            return BDD(bddFsm.BddFsm_get_constrained_backward_image(
                            self._ptr, states._ptr, inputs._ptr),
                       self.bddEnc.DDmanager, freeit = True)
        
        
    def post(self, states, inputs = None):
        """
        Return the post-image of states in this FSM.
        
        If inputs is not None, it is used as constraints to get post-states
        that are reachable through these inputs.
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
        Return a BDD representing a state of bdd.
        
        If bdd is false or an error occurs while picking one state (for example
        if the bdd does not contain any state but inputs),
        a NuSMVBddPickingError occurs.
        """
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick state from false BDD.")
        state = bddEnc.pick_one_state(self.bddEnc._ptr, bdd._ptr)
        if state is None:
            raise NuSMVBddPickingError("Cannot pick state from BDD.")
        return State(state, self, freeit = True)

    
    def pick_one_inputs(self, bdd):
        """
        Return a BDD representing a possible inputs of bdd.
        
        If bdd is false or an error occurs while picking one inputs (for example
        if the bdd does not contain any inputs but states),
        a NuSMVBddPickingError occurs.
        """
        if bdd.is_false():
            raise NuSMVBddPickingError("Cannot pick inputs from false BDD.")
        inputs = bddEnc.pick_one_input(self.bddEnc._ptr, bdd._ptr)
        if inputs is None:
            raise NuSMVBddPickingError("Cannot pick inputs from BDD.")
        return Inputs(inputs, self, freeit = True)
        
    
    def get_inputs_between_states(self, current, next):
        """
        Return the BDD representing the possible inputs
        between current and next.
        """
        inputs = bddFsm.BddFsm_states_to_states_get_inputs(self._ptr,
                                                           current._ptr,
                                                           next._ptr)
        return Inputs(inputs, self, freeit = True)
        
        
    def count_states(self, bdd):
        """Return the number of states of the given BDD, as a double."""
        # Apply mask before counting states
        bdd = bdd & self.bddEnc.statesMask
        return bddEnc.BddEnc_count_states_of_bdd(self.bddEnc._ptr, bdd._ptr)
        
        
    def count_inputs(self, bdd):
        """Return the number of inputs of the given BDD, as a double."""
        # Apply mask before counting inputs
        bdd = bdd & self.bddEnc.inputsMask
        return bddEnc.BddEnc_count_inputs_of_bdd(self.bddEnc._ptr, bdd._ptr)
        
        
    def pick_all_states(self, bdd):
        """
        Return a tuple of all states belonging to bdd.
        
        Raise a NuSMVBddPickingError if something is wrong.
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
        Return a tuple of all inputs belonging to bdd.
        
        Raise a NuSMVBddPickingError if something is wrong.
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
        
        
    @property    
    def reachable_states(self):
        """Return a BDD representing the set of reachable states of the FSM."""
        if self._reachable is None:
            #self._reachable = fixpoint(lambda Z: (self.init | self.post(Z)),
            #                           BDD.false(self.bddEnc.DDmanager))
            self._reachable = BDD(bddFsm.BddFsm_get_reachable_states(self._ptr),
                                  self.bddEnc.DDmanager)
        return self._reachable
        
        
    # ==========================================================================
    # ===== Static methods =====================================================
    # ==========================================================================
    
    def from_filename(filepath):
        """
        Return the FSM corresponding to the model in filepath.
        
        This function modifies the global environment of NuSMV.
        """
        from . import glob
        glob.load_from_file(filepath)
        glob.compute_model()
        propDb = glob.prop_database()
        return propDb.master.bddFsm
        # TODO Remove this and use glob module instead