import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.fsm.fsm import BddFsm
from pynusmv.dd.bdd import BDD
from pynusmv.mc.mc import eval_simple_expression as evalSexp

class TestInit(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def model(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/constraints.smv")
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def test_init(self):
        fsm = self.model()
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        
        self.assertEqual(p & q, fsm.init)
        
        
    def test_state_constraints(self):
        fsm = self.model()
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        
        self.assertEqual(p | q, fsm.state_constraints)
        
        
    def test_inputs_constraints(self):
        fsm = self.model()
        
        true = BDD.true(fsm.bddEnc.DDmanager)
        
        self.assertEqual(true, fsm.inputs_constraints)
        
        
    def test_pre(self): 
        fsm = self.model()
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertEqual(q, fsm.pre(~p & q))
        self.assertEqual(p & q, fsm.pre(~p & q, a))
        
        
    def test_post(self): 
        fsm = self.model()
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertEqual(~p & q, fsm.post(~p & q))
        self.assertEqual(p & ~q, fsm.post(p & q, ~a))
        
        
    def test_pick_one_state(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        s = fsm.pick_one_state(p)
        self.assertTrue(false < s < p < true)
        s = fsm.pick_one_state(false)
        self.assertTrue(s.is_false())
        s = fsm.pick_one_state(true)
        self.assertTrue(s.isnot_false())
        self.assertTrue(false < s < p < true or false < s < ~p < true)
        self.assertTrue(false < s < q < true or false < s < ~q < true)
        
        
    def test_pick_one_inputs(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        ac = fsm.pick_one_inputs(a)
        self.assertTrue(false < ac <= a < true)
        self.assertTrue(ac == a)
        self.assertTrue(ac.isnot_false())
        ac = fsm.pick_one_inputs(true)
        self.assertTrue(false < ac < true)
        self.assertTrue(ac == a or ac == ~a)
        
        
    def test_get_inputs(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        ac = fsm.get_inputs_between_states(p & q, p & ~q)
        self.assertTrue(ac == ~a)
        ac = fsm.get_inputs_between_states(p, p)
        self.assertTrue(ac == true)
        
        self.assertTrue(q == (p & q) | (~p & q))
        self.assertTrue(q.iff(~p) == (~p & q) | (p & ~q))
        ac = fsm.get_inputs_between_states(q, q.iff(~p))
        self.assertTrue(ac == true)