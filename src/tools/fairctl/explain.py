from pynusmv.dd import BDD

from .eval import ex, eg, ceu
from tools.explanation.explanation import Explanation
    

def explain_ex(fsm, state, phi):
    """
    Explain why state of fsm satisfies EX phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying EX phi;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies EX phi, we have to exhibit a successor
    # of state belonging to phi.
    # We also have to show that the reached state is fair.
    pass # TODO
    
    
def explain_eu(fsm, state, phi, psi, states=None):
    """
    Explain why state of fsm satisfies E phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying E phi U psi;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi;
    states -- a dictionary of state->explanation pairs.
    """
    pass # TODO
    
    
def explain_eg(fsm, state, phi):
    """
    Explain why state of fsm satisfies EG phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying EG phi;
    phi -- the set of states of fsm satifying phi.
    """
    pass # TODO
    
    
def explain_ew(fsm, state, phi, psi):
    """
    Explain why state of fsm satisfies E phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying E phi W psi;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    pass # TODO
    
    
def explain_ax(fsm, state, phi):
    """
    Explain why state of fsm satisfies AX phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying AX phi;
    phi -- the set of states of fsm satifying phi.
    """
    # To show that state satisfies AX phi, we have to show that all successors
    # satisfy phi or are not fair.
    pass # TODO
    
    
def explain_au(fsm, state, phi, psi, states=None):
    """
    Explain why state of fsm satisfies A phi U psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying A phi U psi;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi;
    states -- a dictionary of state->explanation pairs.
    """
    pass # TODO
    
    
def explain_ag(fsm, state, phi):
    """
    Explain why state of fsm satisfies AG phi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying AG phi;
    phi -- the set of states of fsm satifying phi.
    """
    pass # TODO
    
    
def explain_aw(fsm, state, phi, psi):
    """
    Explain why state of fsm satisfies A phi W psi.
    
    fsm -- the fsm;
    state -- a state of fsm satisfying A phi W psi;
    phi -- the set of states of fsm satifying phi;
    psi -- the set of states of fsm satifying psi.
    """
    pass # TODO
    
    
def explain_fair(fsm, state):
    """
    Explain why state of fsm is a fair state.
    
    fsm -- the fsm;
    state -- a fair state of fsm.
    """
    pass # TODO
    
    
def explain_not_fair(fsm, state):
    """
    Explain why state of fsm is not a fair state.
    
    fsm -- the fsm;
    state -- a non-fair state of fsm.
    """
    pass # TODO