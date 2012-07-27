import unittest

from pynusmv.fsm.fsm import BddFsm
from pynusmv.init.init import init_nusmv, deinit_nusmv

from tools.arctl.parsing import parseArctl
from tools.arctl.ast import (Atom, Not, And, Or, Implies, Iff,
                             EaX, EaG, EaF, EaU, EaW,
                             AaX, AaG, AaF, AaU, AaW)
from tools.arctl.eval import evalArctl, evalArctl_from_string as evalStr


class TestEval(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def init(self):
        fsm = BddFsm.from_filename("tests/arctl/model.smv")
        self.assertIsNotNone(fsm)
        return fsm
        
    # FIXME There is a segfault when deinit_nusmv is called
    # if bdd is not deleted
    def test_atom(self):
        fsm = self.init()
        
        specs = parseArctl("'c'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        self.assertEqual(type(spec), Atom)
        self.assertEqual(spec.value, "c")
        
        c = evalArctl(fsm, spec)
        self.assertIsNotNone(c)
        self.assertTrue(fsm.init <= c)
        del c
        
        
    def test_not(self):
        fsm = self.init()
        
        specs = parseArctl("~'c'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        nc = evalArctl(fsm, spec)
        self.assertTrue((nc & fsm.init).is_false())
        
        del nc
        
        
    def test_and(self):
        fsm = self.init()
        
        candi = evalStr(fsm, "('c' & 'i')")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertEqual(candi, c & i)
        
        del candi, c, i
        
        
    def test_or(self):
        fsm = self.init()
        
        cordi = evalStr(fsm, "('c' | 'i')")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertEqual(cordi, c | i)
        
        del cordi, c, i
        
        
    def test_implies(self):
        fsm = self.init()
        
        cimpli = evalStr(fsm, "('c' -> 'i')")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertTrue(~c <= cimpli)
        self.assertTrue(i <= cimpli)
        self.assertEqual(~c | i, cimpli)
        
        del cimpli, c, i
        
        
    def test_iff(self):
        fsm = self.init()
        
        ciffi = evalStr(fsm, "('c' <-> 'i')")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertTrue(c & i <= ciffi)
        self.assertTrue(~c & ~i <= ciffi)
        self.assertEqual((c & i) | (~c & ~i), ciffi)
        
        del ciffi, c, i