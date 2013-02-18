import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.fsm import BddFsm
from pynusmv.fsm import BddTrans
from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression as evalSexp

from pynusmv.prop import PropDb
from pynusmv.fsm import BddTrans


from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.prop import prop as nsprop
from pynusmv.nusmv.fsm import fsm as nsfsm
from pynusmv.nusmv.fsm.sexp import sexp as nssexp
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.enc import enc as nsenc
from pynusmv.nusmv.enc.bdd import bdd as nsbddenc
from pynusmv.nusmv.trans.bdd import bdd as nsbddtrans
from pynusmv.nusmv.opt import opt as nsopt
from pynusmv.nusmv.set import set as nsset

car = nsnode.car
cdr = nsnode.cdr

class TestFlatHierarchy(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def model(self, filepath):
        fsm = BddFsm.from_filename(filepath)
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def print_mainFlatHierarchy(self, filepath):
        fsm = self.model(filepath)
        
        flatHierarchy = nscompile.cvar.mainFlatHierarchy
        
        print("-------------------------------------------")
        print(filepath)
        print("-------------------------------------------")
        
        # VARS
        print("----- VARS --------------------------------")
        var_set = nscompile.FlatHierarchy_get_vars(flatHierarchy)
        ite = nsset.Set_GetFirstIter(var_set)
        while not nsset.Set_IsEndIter(ite):
            var = nsset.Set_GetMember(var_set, ite)
            print(nsnode.sprint_node(var))
            ite = nsset.Set_GetNextIter(ite)
        
        # INIT
        print("----- INIT --------------------------------")
        init = nscompile.FlatHierarchy_get_init(flatHierarchy)
        print(nsnode.sprint_node(init))
        
        # INPUT
        print("----- INPUT -------------------------------")
        inp = nscompile.FlatHierarchy_get_trans(flatHierarchy)
        print(nsnode.sprint_node(inp))
        
        # TRANS
        print("----- TRANS -------------------------------")
        trans = nscompile.FlatHierarchy_get_trans(flatHierarchy)
        print(nsnode.sprint_node(trans))
        
        # TRANS
        print("----- ASSIGN ------------------------------")
        assign = nscompile.FlatHierarchy_get_assign(flatHierarchy)
        # assign is a list of pairs <process-name, its-assignements>
        while assign is not None:
            pa_pair = car(assign)
            
            process = car(pa_pair)
            print("process:", nsnode.sprint_node(process))
            
            expr = cdr(pa_pair)
            print(nsnode.sprint_node(expr))
            
            assign = cdr(assign)
            
        # There seems to be at least one assign, even if the model does not
        # contain any one; in this case, the assign is empty.
        
    
    def test_counters(self):
        self.print_mainFlatHierarchy("tests/pynusmv/models/counters.smv")
    
    
    def test_counters_assign(self):
        self.print_mainFlatHierarchy("tests/pynusmv/models/counters-assign.smv")
    
    
    def test_admin(self):
        self.print_mainFlatHierarchy("tests/pynusmv/models/admin.smv")
    
    
    def test_constraints(self):
        self.print_mainFlatHierarchy("tests/pynusmv/models/constraints.smv")
    
    
    def test_mutex(self):
        self.print_mainFlatHierarchy("tests/pynusmv/models/mutex.smv")
    
    
    def test_scheduler(self):
        self.print_mainFlatHierarchy("tests/pynusmv/models/simple-scheduler.smv")
        
    
    def test_trans_flattening(self):
        pass # TODO Construct/get a TRANS and flatten it
        