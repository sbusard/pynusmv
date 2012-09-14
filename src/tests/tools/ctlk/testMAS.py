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
        
        
    def test_simple_pre(self):
        glob.load_from_file("tests/tools/ctlk/agents.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertEqual(fsm.pre(lt & ~lf & ~g),
                         (lt & lf & ~g) | (~lt & ~lf & g))
        self.assertEqual(fsm.pre(g), true)
        self.assertEqual(fsm.pre(lf), lf.iff(g))
        
        
    def test_simple_post(self):
        glob.load_from_file("tests/tools/ctlk/agents.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertEqual(fsm.post(lt & lf & g), ~lt & lf & g | ~lt & lf & ~g)
        self.assertEqual(fsm.post(lt & g), ~lt)
        self.assertEqual(fsm.post(lt & ~g), lt)
        self.assertEqual(fsm.post(lt & lf), lt.iff(~lf))
        
        
    def test_inputs_between_states(self):
        fsm = self.model()
        # TODO Test
        
        
    def test_equivalent_states(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        true = eval_simple_expression(fsm, "TRUE")
        
        self.assertEqual(fsm.equivalent_states(c1p, {"c1"}), c1p)
        self.assertEqual(fsm.equivalent_states(c1p, {"c2"}), true)
        
        
    def test_reachable_states_for_simple_model(self):
        glob.load_from_file("tests/tools/ctlk/agents.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        true = eval_simple_expression(fsm, "TRUE")
        self.assertEqual(fsm.reachable_states, true)
        
        
    def test_reachable_states(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        true = eval_simple_expression(fsm, "TRUE")
        
        self.assertTrue(fsm.reachable_states.isnot_true())
        self.assertTrue(fsm.reachable_states.isnot_false())
        self.assertTrue((fsm.init & c1p & c2p).is_false())
        self.assertTrue((fsm.post(fsm.init) & c1p & c2p).is_false())
        
        tmp = fsm.reachable_states & (c1p & c2p)
        while tmp.isnot_false():
            s = fsm.pick_one_state(tmp)
            print(s.get_str_values())
            tmp -= s
        
        self.assertTrue((fsm.reachable_states & (c1p & c2p)).is_false())