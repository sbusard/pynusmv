import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.mc.mc import eval_simple_expression
from pynusmv.dd.bdd import BDD
from tools.multimodal import glob

class TestMMFsm(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
        
    @unittest.skip    
    def test_fsm(self):
        glob.load_from_file("tests/tools/multimodal/bitCounter.smv")
        fsm = glob.mm_fsm()
        self.assertIsNotNone(fsm)
        
    @unittest.skip    
    def test_post_bitCounter(self):
        glob.load_from_file("tests/tools/multimodal/bitCounter.smv")
        fsm = glob.mm_fsm()
        self.assertIsNotNone(fsm)
        
        ll = eval_simple_expression(fsm, "low.low")
        lu = eval_simple_expression(fsm, "low.up")
        ul = eval_simple_expression(fsm, "up.low")
        uu = eval_simple_expression(fsm, "up.up")
        upinc = eval_simple_expression(fsm, "upinc")
        true = BDD.true(fsm.bddEnc.DDmanager)
        false = BDD.false(fsm.bddEnc.DDmanager)
        
        # bitCounter values
        # 8 => ~ll & ~lu & ~ul & uu
        # 9 => ll & ~lu & ~ul & uu
        
        # All trans
        self.assertEqual(fsm.post(~ll & ~lu & ~ul & uu & ~upinc),
                         ll & ~lu & ~ul & uu & ~upinc)
        self.assertEqual(fsm.post(~ll & ~lu & ~ul & uu & upinc),
                         ll & ~lu & ul & uu & upinc)
                         
        # One trans
        self.assertEqual(fsm.post(ll & lu, trans = ["low"]), ~ll & ~lu)
        self.assertEqual(fsm.post(ll & lu & ul & ~uu & upinc, trans = ["low"]),
                         ~ll & ~lu)
        self.assertEqual(fsm.post(ll & lu, trans = ["up"]), true)
        
        # Several trans
        self.assertEqual(fsm.post(ll & lu & ul & ~uu & upinc,
                                  trans = ["low", ""]),
                         ~ll & ~lu & ~upinc)
                         
                         
    def test_post_counters(self):
        glob.load_from_file("tests/tools/multimodal/counters.smv")
        fsm = glob.mm_fsm()
        self.assertIsNotNone(fsm)
        
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        c2c1 = eval_simple_expression(fsm, "c2.c = 1")
        rc1 = eval_simple_expression(fsm, "run = rc1")
        rc2 = eval_simple_expression(fsm, "run = rc2")
        true = BDD.true(fsm.bddEnc.DDmanager)
        false = BDD.false(fsm.bddEnc.DDmanager)
        
        # All trans
        res = fsm.post(c1c0 & c2c0, inputs = rc1)
        self.assertTrue(res.isnot_false())        
        self.assertEqual(res, (c1c1 & c2c0))
        
        res = fsm.post(c1c0 & c2c0, inputs = true)
        self.assertTrue(res.isnot_false())
        self.assertEqual(res, (c1c0 & c2c1) | (c1c1 & c2c0))
        
        # One trans
        self.assertEqual(fsm.post(c1c0, trans = ["c1"]), c1c0 | c1c1)
        self.assertEqual(fsm.post(c1c0, inputs = rc1, trans = ["c1"]), c1c1)
        
            
    def test_pre(self):
        glob.load_from_file("tests/tools/multimodal/counters.smv")
        fsm = glob.mm_fsm()
        self.assertIsNotNone(fsm)
        
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c1c2 = eval_simple_expression(fsm, "c1.c = 2")
        c1c3 = eval_simple_expression(fsm, "c1.c = 3")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        c2c1 = eval_simple_expression(fsm, "c2.c = 1")
        c2c2 = eval_simple_expression(fsm, "c2.c = 2")
        c2c3 = eval_simple_expression(fsm, "c2.c = 3")
        rc1 = eval_simple_expression(fsm, "run = rc1")
        rc2 = eval_simple_expression(fsm, "run = rc2")
        true = BDD.true(fsm.bddEnc.DDmanager)
        false = BDD.false(fsm.bddEnc.DDmanager)
        
        # All trans
        self.assertEqual(fsm.pre(c1c1 & c2c1), (c1c1 & c2c0) | (c1c0 & c2c1))
        self.assertEqual(fsm.pre(c1c1 & c2c1, inputs = rc2), c1c1 & c2c0)
        
        self.assertTrue((c2c3 & c1c1) <= fsm.post(c2c3 & c1c0))
        
        res = fsm.pre(c1c2)
        self.assertEqual(res, c1c1 | c1c2)
        
        # One trans
        self.assertEqual(fsm.pre(c1c1, trans = ["c1"]), c1c0 | c1c1)
        self.assertEqual(fsm.pre(c1c1, inputs = rc1, trans = ["c1"]),
                         c1c0)
        self.assertEqual(fsm.pre(c2c2, trans = ["c1"]), true)
        self.assertEqual(fsm.pre(c2c2, inputs = rc2, trans = ["c1"]), true)
        
    
    def test_inputs_between_states(self):
        glob.load_from_file("tests/tools/multimodal/counters.smv")
        fsm = glob.mm_fsm()
        self.assertIsNotNone(fsm)
        
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c1c2 = eval_simple_expression(fsm, "c1.c = 2")
        c1c3 = eval_simple_expression(fsm, "c1.c = 3")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        c2c1 = eval_simple_expression(fsm, "c2.c = 1")
        c2c2 = eval_simple_expression(fsm, "c2.c = 2")
        c2c3 = eval_simple_expression(fsm, "c2.c = 3")
        rc1 = eval_simple_expression(fsm, "run = rc1")
        rc2 = eval_simple_expression(fsm, "run = rc2")
        true = BDD.true(fsm.bddEnc.DDmanager)
        false = BDD.false(fsm.bddEnc.DDmanager)
        
        # All trans
        self.assertEqual(fsm.get_inputs_between_states(c1c0, c1c1), rc1)
        self.assertEqual(fsm.get_inputs_between_states(c1c0, c1c0), rc2)
        self.assertEqual(fsm.get_inputs_between_states(c1c0, c1c2), false)
        self.assertEqual(
                fsm.get_inputs_between_states(c1c0 | c1c1, c1c0 | c1c1),
                true)
        
        # One trans
        self.assertEqual(
                fsm.get_inputs_between_states(c1c0, c1c1, trans = ["c1"]),
                rc1)
        self.assertEqual(
                fsm.get_inputs_between_states(c1c0, c1c1, trans = ["c2"]),
                true)
        self.assertEqual(
                fsm.get_inputs_between_states(c1c0, c1c2, trans = ["c1"]),
                false)
        self.assertEqual(
                fsm.get_inputs_between_states(c1c0, c1c2, trans = ["c2"]),
                true)
        self.assertEqual(
                fsm.get_inputs_between_states(c2c0, c2c1, trans = ["c1"]),
                true)
        self.assertEqual(
                fsm.get_inputs_between_states(c2c0, c2c1, trans = ["c2"]),
                rc2)
        