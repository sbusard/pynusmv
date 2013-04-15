import unittest

from pynusmv.dd import BDD, StateInputs
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob
from pynusmv.mc import eval_simple_expression

from pynusmv.nusmv.dd import dd as nsdd


class TestPick(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def model(self):
        glob.load_from_file("tests/pynusmv/models/inputs.smv")
        glob.compute_model()
        fsm = glob.prop_database().master.bddFsm
        self.assertIsNotNone(fsm)
        return fsm
        
    def show_si(self, fsm, bdd):
        for si in fsm.pick_all_states_inputs(bdd):
            print(si.get_str_values())
            
    def show_s(self, fsm, bdd):
        for s in fsm.pick_all_states(bdd):
            print(s.get_str_values())
            
    def show_i(self, fsm, bdd):
        for i in fsm.pick_all_inputs(bdd):
            print(i.get_str_values())


    def test_pick_inputs(self):
        fsm = self.model()
        
        p = eval_simple_expression(fsm, "p")
        a = eval_simple_expression(fsm, "a")
        b0 = eval_simple_expression(fsm, "b = 0")
        b1 = eval_simple_expression(fsm, "b = 1")
        b2 = eval_simple_expression(fsm, "b = 2")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertEqual(len(fsm.pick_all_inputs(a & b1)), 1)
        
        
    def test_pick_states_inputs(self):
        fsm = self.model()
        
        p = eval_simple_expression(fsm, "p")
        a = eval_simple_expression(fsm, "a")
        b0 = eval_simple_expression(fsm, "b = 0")
        b1 = eval_simple_expression(fsm, "b = 1")
        b2 = eval_simple_expression(fsm, "b = 2")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        for si in fsm.pick_all_states_inputs(p & a & b1):
            print(si.get_str_values())
        
        self.assertEqual(len(fsm.pick_all_states_inputs(p & a & b1)), 1)