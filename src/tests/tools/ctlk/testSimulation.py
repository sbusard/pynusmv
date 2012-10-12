import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.mc.mc import eval_simple_expression

from tools.ctlk import glob
from tools.ctlk.parsing import parseCTLK
from tools.ctlk.simulation.simulation import choose_next_state


class TestExplain(unittest.TestCase):
    
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
        
    @unittest.skip # Skip it to keep automatic tests execution    
    def test_simulation(self):
        fsm = self.model()
        c2p = eval_simple_expression(fsm, "c2.payer")
        state = choose_next_state(fsm, fsm.init)
        
        print()
        if state is None:
            print("No chosen state.")
        else:
            values = state.get_str_values()
            for var in values:
                print(var, "=", values[var])