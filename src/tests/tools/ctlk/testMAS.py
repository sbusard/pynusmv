import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.mc.mc import eval_simple_expression

from tools.ctlk import glob

class TestMAS(unittest.TestCase):
    
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
        
        
    def test_pre(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        unknown = eval_simple_expression(fsm, "countsay = unknown")
        odd = eval_simple_expression(fsm, "countsay = odd")
        even = eval_simple_expression(fsm, "countsay = even")
        true = eval_simple_expression(fsm, "TRUE")
        
        self.assertTrue(odd <= fsm.pre(odd))
        self.assertTrue(fsm.init <= unknown)
        self.assertTrue(fsm.pre(unknown).is_false())
        
        
    def test_post(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        unknown = eval_simple_expression(fsm, "countsay = unknown")
        odd = eval_simple_expression(fsm, "countsay = odd")
        even = eval_simple_expression(fsm, "countsay = even")
        true = eval_simple_expression(fsm, "TRUE")
        
        self.assertEqual(fsm.post(odd), odd)
        self.assertEqual(fsm.post(unknown), odd + even)
        self.assertTrue(fsm.post(c1p) <= c1p)
        
        
        
    def test_inputs_between_states(self):
        fsm = self.model()
        # TODO Test
        
        
    def test_equivalent_states(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        true = eval_simple_expression(fsm, "TRUE")
        
        self.assertEqual(fsm.equivalent_states(c1p, ["c1"]), c1p)
        self.assertEqual(fsm.equivalent_states(c1p, ["c2"]), true)
        self.assertEqual(fsm.equivalent_states(c1p), c1p)