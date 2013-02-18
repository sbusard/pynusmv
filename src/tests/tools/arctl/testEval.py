import unittest

from pynusmv.fsm import BddFsm
from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv

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
    
    
    def init_model(self):
        fsm = BddFsm.from_filename("tests/tools/arctl/model.smv")
        self.assertIsNotNone(fsm)
        return fsm
        
    
    def init_finite_model(self):
        fsm = BddFsm.from_filename("tests/tools/arctl/finite_model.smv")
        self.assertIsNotNone(fsm)
        return fsm
        

    def test_atom(self):
        fsm = self.init_model()
        
        specs = parseArctl("'c'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        self.assertEqual(type(spec), Atom)
        self.assertEqual(spec.value, "c")
        
        c = evalArctl(fsm, spec)
        self.assertIsNotNone(c)
        self.assertTrue(fsm.init <= c)
        
        
    def test_not(self):
        fsm = self.init_model()
        
        specs = parseArctl("~'c'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        nc = evalArctl(fsm, spec)
        self.assertTrue((nc & fsm.init).is_false())
        
        
    def test_and(self):
        fsm = self.init_model()
        
        candi = evalStr(fsm, "('c' & 'i')")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertEqual(candi, c & i)
        
        
    def test_or(self):
        fsm = self.init_model()
        
        cordi = evalStr(fsm, "('c' | 'i')")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertEqual(cordi, c | i)
        
        
    def test_implies(self):
        fsm = self.init_model()
        
        cimpli = evalStr(fsm, "('c' -> 'i')")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertTrue(~c <= cimpli)
        self.assertTrue(i <= cimpli)
        self.assertEqual(~c | i, cimpli)
        
        
    def test_iff(self):
        fsm = self.init_model()
        
        ciffi = evalStr(fsm, "('c' <-> 'i')")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertTrue(c & i <= ciffi)
        self.assertTrue(~c & ~i <= ciffi)
        self.assertEqual((c & i) | (~c & ~i), ciffi)
        
        
    def test_eax(self):
        fsm = self.init_model()
        
        eaaxi = evalStr(fsm, "E<'a'>X 'i'")
        self.assertTrue((fsm.init & eaaxi).is_false())
        eaaxnc = evalStr(fsm, "E<'a'>X ~'c'")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertTrue(eaaxnc.isnot_false())
        self.assertEqual(eaaxnc, c.iff(i))
        
    
    def test_neaxt(self):
        fsm = self.init_finite_model()
        
        candi = evalStr(fsm, "('c' & ~'i')")
        neaxt = evalStr(fsm, "~E<'a'>X 'TRUE'")
        
        self.assertTrue(candi <= neaxt)
        
        
    def test_aax(self):
        fsm = self.init_model()
        aaxnc = evalStr(fsm, "A<'a'>X ~'c'")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertEqual(aaxnc, c.iff(i))
    
      
    def test_aax_finite(self):
        fsm = self.init_finite_model()
        aaxnc = evalStr(fsm, "A<'a'>X ~'c'")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertEqual(fsm.init, aaxnc)
        self.assertEqual(aaxnc, c & i)
        
    
    def test_eaf(self):
        fsm = self.init_model()
        eafnc = evalStr(fsm, "E<'a'>F ~'c'")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertEqual(eafnc, ~(c & ~i))
        
        
    def test_eaf_finite(self):
        fsm = self.init_finite_model()
        eafnc = evalStr(fsm, "E<'a'>F ~'c'")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        self.assertTrue(c.iff(i) <= eafnc)
      
  
    def test_aaf(self):
        fsm = self.init_model()
        aafni = evalStr(fsm, "A<'a'>F ~'i'")
        true = BDD.true(fsm.bddEnc.DDmanager)
        self.assertEqual(aafni, true)
            
    
    def test_aaf_finite(self):
        fsm = self.init_finite_model()
        aafni = evalStr(fsm, "A<'a'>F ~'i'")
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        
        self.assertEqual(aafni, c | ~i)
        
    
    def test_eag(self):
        fsm = self.init_model()
        enagc = evalStr(fsm, "E<~'a'>G 'c'")
        c = evalStr(fsm, "'c'")
        i = evalStr(fsm, "'i'")
        
        self.assertTrue(fsm.init <= enagc)
        self.assertEqual(enagc, c)
        
    
    def test_eag_finite(self):
        fsm = self.init_finite_model()
        enagc = evalStr(fsm, "E<~'a'>G 'c'")
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        
        self.assertEqual(enagc, c)
        
        
    def test_aag(self):
        fsm = self.init_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")

        anagc = evalStr(fsm, "A<~'a'>G 'c'")
        self.assertEqual(anagc, c)
        
        atgc = evalStr(fsm, "A<'TRUE'>G 'c'")
        self.assertEqual(atgc, c & ~i)
        
        
    def test_aag_finite(self):
        fsm = self.init_finite_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        anagc = evalStr(fsm, "A<~'a'>G 'c'")
        self.assertEqual(anagc, c)
        
        
    def test_eau(self):
        fsm = self.init_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        eacui = evalStr(fsm, "E<'a'>['c' U 'i']")
        self.assertEqual(fsm.init | (~c & i), eacui)
        
        
    def test_eau_finite(self):
        fsm = self.init_finite_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        enacuni = evalStr(fsm, "E<~'a'>['c'U~'i']")
        self.assertEqual(enacuni, c | ~i)
        
        
    def test_aau(self):
        fsm = self.init_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        aacuni = evalStr(fsm, "A<'a'>['c'U~'i']")
        self.assertEqual(aacuni, c | ~i)
        
        
    def test_aau_finite(self):
        fsm = self.init_finite_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        aacuni = evalStr(fsm, "A<~'a'>['c'U~'i']")
        self.assertEqual(aacuni, c | ~i)
        
        
    def test_eaw(self):
        fsm = self.init_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        eacui = evalStr(fsm, "E<'a'>['c' W 'i']")
        self.assertEqual(c | i, eacui)
        
        
    def test_eaw_finite(self):
        fsm = self.init_finite_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        enacuni = evalStr(fsm, "E<~'a'>['c'W~'i']")
        self.assertEqual(enacuni, c | ~i)
        
        
    def test_aaw(self):
        fsm = self.init_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        aacuni = evalStr(fsm, "A<'a'>['c'W~'i']")
        self.assertEqual(aacuni, c | ~i)
        
        
    def test_aaw_finite(self):
        fsm = self.init_finite_model()
        c, i = evalStr(fsm, "'c'"), evalStr(fsm, "'i'")
        aacuni = evalStr(fsm, "A<~'a'>['c'W~'i']")
        self.assertEqual(aacuni, c | ~i)