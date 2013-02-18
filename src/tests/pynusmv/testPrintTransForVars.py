import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.fsm import BddFsm

from pynusmv.prop import PropDb
from pynusmv import glob


from pynusmv.nusmv.prop import prop as nsprop
from pynusmv.nusmv.fsm.sexp import sexp as nssexp
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.node import node as nsnode

class TestPrintTransForVars(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def model(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/counters.smv")
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def print_vars_and_trans(self, modelpath):
        fsm = BddFsm.from_filename(modelpath)
        self.assertIsNotNone(fsm)
        
        propDb = glob.prop_database()
        master = propDb.master
        
        print("MODEL:", modelpath)
        print("============================================================")
        
        sexpfsm_ptr = nsprop.Prop_get_scalar_sexp_fsm(master._ptr)
        var_list = nssexp.SexpFsm_get_vars_list(sexpfsm_ptr)
        
        var_list_length = nsutils.NodeList_get_length(var_list)
        print("var_list length:", var_list_length)
        
        var_list_iter = nsutils.NodeList_get_first_iter(var_list)
        while var_list_iter is not None:
            item = nsutils.NodeList_get_elem_at(var_list, var_list_iter)
            
            print(nsnode.sprint_node(item))
            print("--------------------------")
            
            var_init = nssexp.SexpFsm_get_var_init(sexpfsm_ptr, item)
            print(nsnode.sprint_node(var_init))
                        
            print("--------------------------")
            
            var_trans = nssexp.SexpFsm_get_var_trans(sexpfsm_ptr, item)
            print(nsnode.sprint_node(var_trans))
            
            print("--------------------------")
            print()
            
            var_list_iter = nsutils.ListIter_get_next(var_list_iter)
        
        
    def test_print_trans_counters(self):
        self.print_vars_and_trans("tests/pynusmv/models/counters.smv")
        
        
    def test_print_trans_admin(self):
        self.print_vars_and_trans("tests/pynusmv/models/admin.smv")
        
        
    def test_print_trans_constraints(self):
        self.print_vars_and_trans("tests/pynusmv/models/constraints.smv")
        
        
    def test_print_trans_mutex(self):
        self.print_vars_and_trans("tests/pynusmv/models/mutex.smv")
        
        
    def test_print_trans_simple_scheduler(self):
        self.print_vars_and_trans("tests/pynusmv/models/simple-scheduler.smv")
        
        
    def test_print_trans_counters_assign(self):
        self.print_vars_and_trans("tests/pynusmv/models/counters-assign.smv")