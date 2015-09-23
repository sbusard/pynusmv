import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from tools.mas import glob
from tools.ctlk.eval import evalCTLK
from tools.ctlk.parsing import parseCTLK

from tools.ctlk.eval import nd

class TestEval(unittest.TestCase):
    
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
        
        
    def simplemodel(self):
        glob.load_from_file("tests/tools/ctlk/agents.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def test_true(self):
        fsm = self.model()
        
        true = eval_simple_expression(fsm, "TRUE")
        
        specs = parseCTLK("True")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        self.assertEqual(true, evalCTLK(fsm, spec))
        
        
    def test_false(self):
        fsm = self.model()
        
        false = eval_simple_expression(fsm, "FALSE")
        
        specs = parseCTLK("False")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        self.assertEqual(false, evalCTLK(fsm, spec))
        
        
    def test_init(self):
        fsm = self.model()
        
        specs = parseCTLK("Init")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        self.assertEqual(fsm.init, evalCTLK(fsm, spec))
        
        
    def test_reachable(self):
        fsm = self.model()
        
        specs = parseCTLK("Reachable")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        self.assertEqual(fsm.reachable_states, evalCTLK(fsm, spec))
        
    
    def test_atom(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        
        specs = parseCTLK("'c1.payer'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        c1pCTLK = evalCTLK(fsm, spec)
        self.assertEqual(c1p, c1pCTLK)
        
        
    def test_not(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        
        specs = parseCTLK("~'c1.payer'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        c1pCTLK = evalCTLK(fsm, spec)
        self.assertEqual(~c1p, c1pCTLK)
        
        
    def test_and(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        
        specs = parseCTLK("'c1.payer' & 'countsay = odd'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        c1paco = evalCTLK(fsm, spec)
        self.assertEqual(c1p & odd, c1paco)
        
               
    def test_or(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        
        specs = parseCTLK("'c1.payer' | 'countsay = odd'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        c1poco = evalCTLK(fsm, spec)
        self.assertEqual(c1p | odd, c1poco)
        
    
    def test_implies(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        
        specs = parseCTLK("'c1.payer' -> 'countsay = odd'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        c1pico = evalCTLK(fsm, spec)
        self.assertEqual(c1p.imply(odd), c1pico)
        
        
    def test_iff(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        
        specs = parseCTLK("'c1.payer' <-> 'countsay = odd'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        
        c1pico = evalCTLK(fsm, spec)
        self.assertEqual(c1p.iff(odd), c1pico)
        
        
    def test_ex(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        c3p = eval_simple_expression(fsm, "c3.payer")
        c1h = eval_simple_expression(fsm, "c1.coin = head")
        c2h = eval_simple_expression(fsm, "c2.coin = head")
        c3h = eval_simple_expression(fsm, "c3.coin = head")
        odd = eval_simple_expression(fsm, "countsay = odd")
        unk = eval_simple_expression(fsm, "countsay = unknown")
        
        specs = parseCTLK("EX ('c1.payer' & ~'c2.payer' & ~'c3.payer')")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        ex = evalCTLK(fsm, spec)
        self.assertEqual(ex, c1p & ~c2p & ~c3p & fsm.bddEnc.statesMask)
        
        specs = parseCTLK("EX 'countsay = odd'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        ex = evalCTLK(fsm, spec)
            
        self.assertTrue(odd & fsm.bddEnc.statesMask <= ex)
        self.assertTrue((unk & c1p & ~c2p & ~c3p) & fsm.bddEnc.statesMask
                        <= ex)
        self.assertTrue((unk & c1p & c2p & c3p) & fsm.bddEnc.statesMask
                        <= ex)
        
        
    def test_simple_ex(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        
        specs = parseCTLK("EX 'af.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        ex = evalCTLK(fsm, spec)
        # ex : 
        #  l1, !l2, !g
        #  !l1, l2, g
        #  l1, l2, g
        #  !l1, !l2, !g
        self.assertEqual(ex, lf.iff(g))
        
        specs = parseCTLK("EX ('af.local' & 'at.local' & 'global')")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        ex = evalCTLK(fsm, spec)
        self.assertEqual(ex, lt & ~lf & ~g | ~lt & lf & g)
        
        
    def test_k_ex(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        
        specs = parseCTLK("K<'at'> EX 'global'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        kex = evalCTLK(fsm, spec)
        self.assertEqual(kex, true)
        
        specs = parseCTLK("K<'at'> EX ~'global'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        kexn = evalCTLK(fsm, spec)
        self.assertEqual(kexn, true)
        
        
    def test_k_ef(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        
        specs = parseCTLK("K<'at'> EF 'af.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        kef = evalCTLK(fsm, spec)
        self.assertEqual(kef, true)
        
        
    def test_k(self):
        fsm = self.simplemodel()

        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")

        specs = parseCTLK("K<'at'> 'at.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        katt = evalCTLK(fsm, spec)
        self.assertEqual(katt, lt)
        
        specs = parseCTLK("K<'at'> 'af.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        katf = evalCTLK(fsm, spec)
        self.assertEqual(katf, false)
        
        specs = parseCTLK("K<'at'> 'global'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        katf = evalCTLK(fsm, spec)
        self.assertEqual(katf, g)
        
        
    def test_k_dincry(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        c3p = eval_simple_expression(fsm, "c3.payer")
        c1h = eval_simple_expression(fsm, "c1.coin = head")
        c2h = eval_simple_expression(fsm, "c2.coin = head")
        c3h = eval_simple_expression(fsm, "c3.coin = head")
        odd = eval_simple_expression(fsm, "countsay = odd")
        unk = eval_simple_expression(fsm, "countsay = unknown")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        specs = parseCTLK("AG ('c1.payer' "
                                      "-> K<'c1'> (~'c2.payer' & ~'c3.payer'))")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        ik = evalCTLK(fsm, spec)                
        self.assertTrue(fsm.init <= ik)
        
    
    def test_e_simple(self):
        fsm = self.simplemodel()

        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")

        specs = parseCTLK("'global' -> E<'at','af'> 'global'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        geg = evalCTLK(fsm, spec) 
        self.assertEqual(geg, true)
        
        specs = parseCTLK("'af.local' & E<'at','af'> 'af.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        afeaf = evalCTLK(fsm, spec)
        self.assertEqual(afeaf, false)
        
        
    def test_e_dincry(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        c3p = eval_simple_expression(fsm, "c3.payer")
        c1h = eval_simple_expression(fsm, "c1.coin = head")
        c2h = eval_simple_expression(fsm, "c2.coin = head")
        c3h = eval_simple_expression(fsm, "c3.coin = head")
        odd = eval_simple_expression(fsm, "countsay = odd")
        even = eval_simple_expression(fsm, "countsay = even")
        unk = eval_simple_expression(fsm, "countsay = unknown")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        specs = parseCTLK("'countsay = even' -> E<'c1','c2'> ~'c1.payer'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        een1 = evalCTLK(fsm, spec)                
        self.assertEqual(een1, true)
        
        specs = parseCTLK("E<'c2','c3'> ~'c1.payer'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        en1 = evalCTLK(fsm, spec)  
        self.assertTrue(even & fsm.reachable_states <= en1)
        
        
    def test_d_simple(self):
        fsm = self.simplemodel()

        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")

        specs = parseCTLK("'af.local' -> D<'at','af'> 'af.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        geg = evalCTLK(fsm, spec) 
        self.assertEqual(geg, true)
        
        specs = parseCTLK("'af.local' & D<'at'> 'af.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        geg = evalCTLK(fsm, spec) 
        self.assertEqual(geg, false)
        
        specs = parseCTLK("'af.local' -> D<'af'> 'af.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        geg = evalCTLK(fsm, spec) 
        self.assertEqual(geg, true)
        
        
    def test_dincry_equiv(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        c3p = eval_simple_expression(fsm, "c3.payer")
        c1h = eval_simple_expression(fsm, "c1.coin = head")
        c2h = eval_simple_expression(fsm, "c2.coin = head")
        c3h = eval_simple_expression(fsm, "c3.coin = head")
        odd = eval_simple_expression(fsm, "countsay = odd")
        even = eval_simple_expression(fsm, "countsay = even")
        unk = eval_simple_expression(fsm, "countsay = unknown")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        s = c1p & ~c2p & ~c3p
        eqs2 = fsm.equivalent_states(s, {"c2"})
        eqs3 = fsm.equivalent_states(s, {"c3"})
        self.assertEqual(eqs2 & eqs3, ~c2p & ~c3p)
    
    
    def test_d_dincry(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        c3p = eval_simple_expression(fsm, "c3.payer")
        c1h = eval_simple_expression(fsm, "c1.coin = head")
        c2h = eval_simple_expression(fsm, "c2.coin = head")
        c3h = eval_simple_expression(fsm, "c3.coin = head")
        odd = eval_simple_expression(fsm, "countsay = odd")
        even = eval_simple_expression(fsm, "countsay = even")
        unk = eval_simple_expression(fsm, "countsay = unknown")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        specs = parseCTLK("('c1.payer' & 'countsay != unknown') "
                          "-> D<'c2','c3'> 'c1.payer'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        cud1 = evalCTLK(fsm, spec)        
        self.assertTrue(fsm.reachable_states <= cud1)


    def test_d_distributedmodel(self):  
        glob.load_from_file("tests/tools/ctlk/distributed_knowledge.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        l1 = eval_simple_expression(fsm, "a1.local")
        l2 = eval_simple_expression(fsm, "a2.local")
        l3 = eval_simple_expression(fsm, "a3.local")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        specs = parseCTLK("'a1.local' -> D<'a1','a2'> 'a1.local'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        a1da1 = evalCTLK(fsm, spec)        
        self.assertEqual(true, a1da1)
                
        
    def test_c_simple(self):
        fsm = self.simplemodel()

        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")

        specs = parseCTLK("'global' -> C<'at','af'> 'global'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        gcg = evalCTLK(fsm, spec) 
        self.assertEqual(gcg, true)
        
        specs = parseCTLK("C<'at','af'> 'global' | C<'at','af'> ~'global'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        gcg = evalCTLK(fsm, spec) 
        self.assertEqual(gcg, true)
        
        
    def test_c_dincry(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        c3p = eval_simple_expression(fsm, "c3.payer")
        c1h = eval_simple_expression(fsm, "c1.coin = head")
        c2h = eval_simple_expression(fsm, "c2.coin = head")
        c3h = eval_simple_expression(fsm, "c3.coin = head")
        odd = eval_simple_expression(fsm, "countsay = odd")
        even = eval_simple_expression(fsm, "countsay = even")
        unk = eval_simple_expression(fsm, "countsay = unknown")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        specs = parseCTLK("'countsay = odd' -> "
                          "C<'c1','c2','c3'> "
                          "('c1.payer' | 'c2.payer' | 'c3.payer')")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        oc123 = evalCTLK(fsm, spec)        
        self.assertTrue(fsm.reachable_states <= oc123)
        
        specs = parseCTLK("'countsay = even' -> "
                          "C<'c1','c2','c3'> "
                          "(~'c1.payer' & ~'c2.payer' & ~'c3.payer')")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        ec123 = evalCTLK(fsm, spec)        
        self.assertTrue(fsm.reachable_states <= ec123)