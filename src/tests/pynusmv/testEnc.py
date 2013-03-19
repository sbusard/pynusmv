import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.fsm import BddFsm
from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression as evalSexp
from pynusmv.exception import NuSMVBddPickingError

class TestFsm(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def model(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/constraints.smv")
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def test_statesMask(self):
        fsm = self.model()
        enc = fsm.bddEnc
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertEqual(enc.statesMask, (p | ~p) & (q | ~q))
    
        
    def test_inputsMask(self):
        fsm = self.model()
        enc = fsm.bddEnc
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertEqual(enc.inputsMask, a | ~a)
        
        
    def test_statesCube(self):
        fsm = self.model()
        enc = fsm.bddEnc
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertTrue(p & q <= enc.statesCube)
        self.assertFalse(a <= enc.statesCube)
        self.assertFalse(~a <= enc.statesCube)
    
        
    def test_inputsCube(self):
        fsm = self.model()
        enc = fsm.bddEnc
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertTrue(a <= enc.inputsCube)
        self.assertFalse(p & q <= enc.inputsCube)
        
    
    def test_inputs_vars_cube(self):
        fsm = self.model()
        enc = fsm.bddEnc
        
        p = evalSexp(fsm, "p")
        q = evalSexp(fsm, "q")
        a = evalSexp(fsm, "a")
        
        self.assertTrue(a <= enc.cube_for_inputs_vars({'a'}))        