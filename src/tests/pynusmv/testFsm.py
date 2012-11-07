import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.fsm.bddFsm import BddFsm
from pynusmv.dd.bdd import BDD
from pynusmv.mc.mc import eval_simple_expression as evalSexp
from pynusmv.utils.exception import NuSMVBddPickingError

class TestFsm(unittest.TestCase):
    
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
        
        
    def test_post_counters(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/counters.smv")
        self.assertIsNotNone(fsm)
        
        c1c0 = evalSexp(fsm, "c1.c = 0")
        c1c1 = evalSexp(fsm, "c1.c = 1")
        c2c0 = evalSexp(fsm, "c2.c = 0")
        c2c1 = evalSexp(fsm, "c2.c = 1")
        rc1 = evalSexp(fsm, "run = rc1")
        rc2 = evalSexp(fsm, "run = rc2")
        
        self.assertEqual(fsm.post(c1c0 & c2c0), (c1c1 & c2c0) | (c1c0 & c2c1))
        
        
    def test_post(self): 
        fsm = self.model()
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertEqual(~p & q, fsm.post(~p & q))
        self.assertEqual(p & ~q, fsm.post(p & q, ~a))
        
        self.assertEqual(1, fsm.count_states(fsm.post(~p & q)))
        self.assertEqual(2, fsm.count_states(fsm.post(p & q)))
        self.assertEqual(1, fsm.count_states(fsm.post(p & q, a)))
        self.assertEqual(0, fsm.count_states(fsm.post(p & ~q, ~a)))
        
        
    def test_pick_one_state(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        s = fsm.pick_one_state(p)
        self.assertTrue(false < s < p < true)
        with self.assertRaises(NuSMVBddPickingError):
            s = fsm.pick_one_state(false)
        s = fsm.pick_one_state(true)
        self.assertTrue(s.isnot_false())
        self.assertTrue(false < s < p < true or false < s < ~p < true)
        self.assertTrue(false < s < q < true or false < s < ~q < true)
        
        
    def test_pick_one_state_error(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        with self.assertRaises(NuSMVBddPickingError):
            s = fsm.pick_one_state(false)
            
        with self.assertRaises(NuSMVBddPickingError):
            s = fsm.pick_one_state(a)
        
        
        
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
        
        
    def test_pick_one_inputs_error(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        with self.assertRaises(NuSMVBddPickingError):
            i = fsm.pick_one_inputs(false)
            
        with self.assertRaises(NuSMVBddPickingError):
            i = fsm.pick_one_inputs(p)
        
        
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
        
        
    def test_count_states(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertEqual(fsm.count_states(p & q), 1)
        self.assertEqual(fsm.count_states(true), 4)
        self.assertEqual(fsm.count_states(false), 0)
        self.assertEqual(fsm.count_states(p), 2)
        
        self.assertEqual(fsm.count_states(p & q & a), 0) # WHY ?
    
    
    def test_count_inputs(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertEqual(fsm.count_inputs(a), 1)
        self.assertEqual(fsm.count_inputs(~a), 1)
        self.assertEqual(fsm.count_inputs(true), 2)
        self.assertEqual(fsm.count_inputs(false), 0)
        self.assertEqual(fsm.count_inputs(p & q & a), 0) # WHY ?
        
        
    def test_count_inputs_no_in_model(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/modules.smv")
        self.assertIsNotNone(fsm)

        ni = evalSexp(fsm, "n.inmod")
        mi = evalSexp(fsm, "m.inmod")
        top = evalSexp(fsm, "top")
        true = evalSexp(fsm, "TRUE")

        self.assertEqual(0, fsm.count_inputs(ni))

 
    def test_pick_states(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        pstates = fsm.pick_all_states(p)
        self.assertEqual(len(pstates), 2)
        self.assertTrue(false < pstates[0] < p)
        self.assertTrue(false < pstates[1] < p)
        self.assertTrue(pstates[0] != pstates[1])
        
        # FIXME Raise a segmentation fault
        #astates = fsm.pick_all_states(a)
        #self.assertEqual(len(astates), 2) # WHY ?
        
          
    def test_pick_inputs(self):
        fsm = self.model()
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        ainputs = fsm.pick_all_inputs(a)
        self.assertEqual(len(ainputs), 1)
        self.assertTrue(false < ainputs[0] <= a)
        self.assertTrue(ainputs[0] == a)
        
        tinputs = fsm.pick_all_inputs(true)
        self.assertEqual(len(tinputs), 2)
        self.assertTrue(tinputs[0] == a or tinputs[0] == ~a)
        self.assertTrue(tinputs[1] == a or tinputs[1] == ~a)
        self.assertTrue(tinputs[0] != tinputs[1])
        
        pinputs = fsm.pick_all_states(p)
        self.assertEqual(len(pinputs), 2) # WHY ?
        
             
    def test_pick_no_inputs(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/modules.smv")
        self.assertIsNotNone(fsm)

        ni = evalSexp(fsm, "n.inmod")
        mi = evalSexp(fsm, "m.inmod")
        top = evalSexp(fsm, "top")
        true = evalSexp(fsm, "TRUE")

        with self.assertRaises(NuSMVBddPickingError):
            fsm.pick_all_inputs(ni)
        
        
    def test_get_trans(self):
        fsm = self.model()
        
        trans = fsm.trans
        self.assertIsNotNone(trans)
        
        bdd = trans.monolithic
        self.assertIsNotNone(bdd)
        
        
    def test_set_trans(self):
        fsm = self.model()
        
        trans = fsm.trans
        self.assertIsNotNone(trans)
        
        fsm.trans = trans
        
        
    def test_fairness_from_nusmv(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/counters-fair.smv")
        self.assertIsNotNone(fsm)
        
        from pynusmv.nusmv.fsm.bdd import bdd as nsbddfsm
        
        justiceList = nsbddfsm.BddFsm_get_justice(fsm._ptr)
        fairnessList = nsbddfsm.justiceList2fairnessList(justiceList)
        
        ite = nsbddfsm.FairnessList_begin(fairnessList)
        fairBdds = []
        while not nsbddfsm.FairnessListIterator_is_end(ite):
            fairBdds.append(nsbddfsm.JusticeList_get_p(justiceList, ite))
            ite = nsbddfsm.FairnessListIterator_next(ite)
        self.assertEqual(len(fairBdds), 2)
        
        
    def test_fairness(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/counters-fair.smv")
        self.assertIsNotNone(fsm)
        
        false = BDD.false(fsm.bddEnc.DDmanager)
        true = BDD.true(fsm.bddEnc.DDmanager)
        rc1 = evalSexp(fsm, "run = rc1")
        rc2 = evalSexp(fsm, "run = rc2")
        
        fairBdds = fsm.fairness_constraints
        self.assertEqual(len(fairBdds), 2)
        for fair in fairBdds:
            self.assertTrue(fair == rc1 or fair == rc2)