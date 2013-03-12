import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from tools.mas import glob
from tools.ctlk.parsing import parseCTLK
from tools.ctlk.simulation.stateChoice import (choose_one_state,
                                               choose_next_state)


class TestStateChoice(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
        
    def model(self):
        glob.load_from_file("tests/tools/ctlk/dining-crypto.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
    
    @unittest.skip # Skip it to keep tests automatic 
    def test_one_state(self):
        fsm = self.model()
        c2p = eval_simple_expression(fsm, "c2.payer")
        state = choose_one_state(fsm, fsm.init)
        
        print()
        if state is None:
            print("No chosen state.")
        else:
            values = state.get_str_values()
            for var in values:
                print(var, "=", values[var])

    
    @unittest.skip # Skip it to keep tests automatic         
    def test_next_state_with_inputs(self):
        fsm = self.model()
        c2p = eval_simple_expression(fsm, "c2.payer")
        (inputs, state) = choose_next_state(fsm, fsm.pick_one_state(fsm.init))
        
        print()
        if state is None:
            print("No chosen state.")
        else:
            if inputs is not None:
                values = inputs.get_str_values()
                for var in values:
                    print(var, "=", values[var])
                print("-" * 40)
            values = state.get_str_values()
            for var in values:
                print(var, "=", values[var])
                
    
    @unittest.skip # Skip it to keep tests automatic
    def test_next_state_without_inputs(self):
        glob.load_from_file("tests/pynusmv/models/modules.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        c2p = eval_simple_expression(fsm, "top")
        (inputs, state) = choose_next_state(fsm, fsm.pick_one_state(fsm.init))
        
        print()
        if state is None:
            print("No chosen state.")
        else:
            if inputs is not None:
                values = inputs.get_str_values()
                for var in values:
                    print(var, "=", values[var])
                print("-" * 40)
            values = state.get_str_values()
            for var in values:
                print(var, "=", values[var])