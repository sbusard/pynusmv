import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from tools.mas import glob
from tools.ctlk.eval import evalCTLK, ex, eu, eg, nk, nd, ne, nc
from tools.ctlk.parsing import parseCTLK

from tools.ctlk.explain import (explain_ex, explain_eg, explain_eu,
                                explain_nk, explain_ne, explain_nd, explain_nc,
                                explain_reachable)

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
        
        
    def simplemodel(self):
        glob.load_from_file("tests/tools/ctlk/agents.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def test_ex_simple(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        # lt & lf & g |= EX !lt
        # because (lt & lf & g, !lt & lf (g | !g))
        state = fsm.pick_one_state(lt & lf & g)
        witness = explain_ex(fsm, state, ~lt)
        self.assertTrue(witness[0] == state)
        self.assertTrue(witness[2] <= ~lt)
        self.assertTrue(witness[2] <= ~lt & lf)
        self.assertTrue((witness[2] == ~lt & lf & g) |
                        (witness[2] == ~lt & lf & ~g))
        self.assertTrue(fsm.get_inputs_between_states(state, witness[2])
                                                                 .isnot_false())
        self.assertTrue(witness[1] <= fsm.get_inputs_between_states(state,
                                                                    witness[2]))
                                                                    
                                                                    
    def test_ex_dincry(self):
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
        
        # unknown & c1h & c2h & c3h |= odd
        # because (unk & c1h & c2h & c3h & c1p & !c2p & !c3p,
        #          odd & c1h & c2h & c3h & c1p & !c2p & !c3p)
        state = fsm.pick_one_state(unk & c1h & c2h & c3h & c1p)
        self.assertTrue(state <= ex(fsm, odd))
        self.assertTrue(state <= (unk & c1h & c2h & c3h & c1p))
        self.assertTrue(state <= fsm.init)
        witness = explain_ex(fsm, state, odd)
        self.assertTrue(witness[0] == state)
        self.assertTrue(witness[1].isnot_false())
        self.assertTrue(witness[2] <= odd)
        self.assertTrue(witness[2] <= odd & c1p)
        self.assertTrue(fsm.get_inputs_between_states(state, witness[2])
                                                                 .isnot_false())
        self.assertTrue(witness[1] <= fsm.get_inputs_between_states(state,
                                                                    witness[2]))
                                                                    
                                                                    
    def test_eu_simple(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        # lt & lf & g |= E[ g U ~lf & ~lt & ~g ]
        eus = eu(fsm, g, ~lf & ~lt & ~g)
        state = fsm.pick_one_state(eus)
        witness = explain_eu(fsm, state, g, ~lf & ~lt & ~g)
        self.assertTrue(witness[0] == state)
        self.assertTrue(witness[0] <= g | ~lf & ~lt & ~g)
        
        for (s, i, sp) in zip(witness[:-2:2], witness[1:-2:2], witness[2:-2:2]):
            self.assertTrue(s <= g)
            self.assertTrue(sp <= g)
            self.assertIsNotNone(i)
            self.assertTrue(i <= fsm.get_inputs_between_states(s, sp))        
        
        self.assertIsNotNone(witness[-2])
        self.assertTrue(witness[-2] <=
                        fsm.get_inputs_between_states(witness[-3], witness[-1]))
        self.assertTrue(witness[-1] <= ~lf)
        self.assertTrue(witness[-1] <= ~lt)
        self.assertTrue(witness[-1] <= ~g)
        self.assertTrue(witness[-1] == ~lf & ~lt & ~g)
                                                                    
                                                                    
    def test_eu_dincry(self):
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
        
        # init |= E[c1p U odd]
        eus = eu(fsm, c1p, odd)
        self.assertTrue((eus & fsm.init).isnot_false())
        state = fsm.pick_one_state(eus & fsm.init)
        witness = explain_eu(fsm, state, c1p, odd)
        self.assertTrue(witness[0] == state)
        self.assertTrue(witness[0] <= c1p | odd)
        
        for (s, i, sp) in zip(witness[:-2:2], witness[1:-2:2], witness[2:-2:2]):
            self.assertTrue(s <= c1p)
            self.assertTrue(sp <= c1p)
            self.assertIsNotNone(i)
            self.assertTrue(i <= fsm.get_inputs_between_states(s, sp))        
        
        self.assertIsNotNone(witness[-2])
        self.assertTrue(witness[-2] <=
                        fsm.get_inputs_between_states(witness[-3], witness[-1]))
        self.assertTrue(witness[-1] <= odd)
        
    def test_eg_simple(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        # lt & lf & !g |= EG !g
        egs = eg(fsm, ~g)
        self.assertTrue(egs.isnot_false())
        state = fsm.pick_one_state(egs)
        (path, loop) = explain_eg(fsm, state, ~g)
        self.assertTrue(path[0] == state)
        self.assertTrue(path[0] <= ~g & egs)
        
        for (s, i, sp) in zip(path[::2], path[1::2], path[2::2]):
            self.assertTrue(s <= ~g)
            self.assertTrue(sp <= ~g)
            self.assertIsNotNone(i)
            self.assertTrue(i <= fsm.get_inputs_between_states(s, sp))
            
        self.assertIsNotNone(loop)
        self.assertEqual(len(loop), 2)
        self.assertIsNotNone(loop[0])
        self.assertIsNotNone(loop[1])
        
        self.assertTrue(loop[1] in path)
        self.assertTrue(loop[0] <=
                               fsm.get_inputs_between_states(path[-1], loop[1]))
                                                                    
                                                                    
    def test_eg_dincry(self):
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
        
        # init |= EG c2.payer
        egs = eg(fsm, c2p)
        self.assertTrue((egs & fsm.init).isnot_false())
        state = fsm.pick_one_state(egs & fsm.init)
        (path, loop) = explain_eg(fsm, state, c2p)
        self.assertTrue(path[0] == state)
        self.assertTrue(path[0] <= c2p)
        
        for (s, i, sp) in zip(path[::2], path[1::2], path[2::2]):
            self.assertTrue(s <= c2p)
            self.assertTrue(sp <= c2p)
            self.assertIsNotNone(i)
            self.assertTrue(i <= fsm.get_inputs_between_states(s, sp))
            
        self.assertIsNotNone(loop)
        self.assertEqual(len(loop), 2)
        self.assertIsNotNone(loop[0])
        self.assertIsNotNone(loop[1])
        
        self.assertTrue(loop[1] in path)
        self.assertTrue(loop[0] <=
                               fsm.get_inputs_between_states(path[-1], loop[1]))
                               
                               
    def test_nk_simple(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        # nK<at> af.local
        nks = nk(fsm, "at", lf)
        self.assertTrue(nks.isnot_false())
        state = fsm.pick_one_state(nks)
        (s, ag, sp) = explain_nk(fsm, state, "at", lf)
        self.assertEqual(s, state)
        self.assertEqual(ag, {"at"})
        self.assertTrue(sp <= lf)
        self.assertTrue(sp <= fsm.equivalent_states(state, {"at"}))
        self.assertTrue(sp <= fsm.reachable_states)
                                   
                                                                    
    def test_nk_dincry(self):
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
        
        # nK<c2> ~c3.payer
        nks = nk(fsm, "c2", ~c3p)
        self.assertTrue(nks.isnot_false())
        state = fsm.pick_one_state(nks)
        (s, ag, sp) = explain_nk(fsm, state, "c2", ~c3p)
        self.assertEqual(s, state)
        self.assertEqual(ag, {"c2"})
        self.assertTrue(sp <= ~c3p)
        self.assertTrue(sp <= fsm.equivalent_states(state, {"c2"}))
        self.assertTrue(sp <= fsm.reachable_states)
        
        
    def test_ne_simple(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        # nE<at, af> g
        nes = ne(fsm, ["at", "af"], g)
        self.assertTrue(nes.isnot_false())
        state = fsm.pick_one_state(nes)
        (s, ag, sp) = explain_ne(fsm, state, ["at", "af"], g)
        self.assertEqual(s, state)
        self.assertTrue(ag in [{"at"}, {"af"}])
        self.assertTrue(sp <= g)
        self.assertTrue((sp <= fsm.equivalent_states(state, {"at"})) |
                        (sp <= fsm.equivalent_states(state, {"af"})))
        self.assertTrue(sp <= fsm.reachable_states)
                                   
                                                                    
    def test_ne_dincry(self):
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
        
        # nE<c1,c2> ~c3.payer
        nes = ne(fsm, ["c1", "c2"], ~c3p)
        self.assertTrue(nes.isnot_false())
        state = fsm.pick_one_state(nes)
        (s, ag, sp) = explain_ne(fsm, state, ["c1", "c2"], ~c3p)
        self.assertEqual(s, state)
        self.assertTrue(ag in [{"c1"}, {"c2"}])
        self.assertTrue(sp <= ~c3p)
        self.assertTrue((sp <= fsm.equivalent_states(state, {"c1"})) |
                        (sp <= fsm.equivalent_states(state, {"c2"})))
        self.assertTrue(sp <= fsm.reachable_states)
        

    def test_nd_simple(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        # nD<at, af> at.local
        nds = nd(fsm, ["at", "af"], lt)
        self.assertTrue(nds.isnot_false())
        state = fsm.pick_one_state(nds)
        (s, ag, sp) = explain_nd(fsm, state, ["at", "af"], lt)
        self.assertEqual(s, state)
        self.assertEqual(ag, {"at", "af"})
        self.assertTrue(sp <= lt)
        self.assertTrue((sp <= fsm.equivalent_states(state, {"at"})) &
                        (sp <= fsm.equivalent_states(state, {"af"})))
        self.assertTrue(sp <= fsm.reachable_states)
                                   
                                                                    
    def test_nd_dincry(self):
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
        
        # nD<c1,c2> ~c3.payer
        nds = nd(fsm, ["c1", "c2"], ~c3p)
        self.assertTrue(nds.isnot_false())
        state = fsm.pick_one_state(nds)
        (s, ag, sp) = explain_nd(fsm, state, ["c1", "c2"], ~c3p)
        self.assertEqual(s, state)
        self.assertEqual(ag, {"c1", "c2"})
        self.assertTrue(sp <= ~c3p)
        self.assertTrue((sp <= fsm.equivalent_states(state, {"c1"})) &
                        (sp <= fsm.equivalent_states(state, {"c2"})))
        self.assertTrue(sp <= fsm.reachable_states)
  
      
    def test_nc_simple(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        # ~lt & ~lf & g |= nc<at, af> ~lt & ~lf & ~g
        ncs = nc(fsm, ["at"], ~lt & ~lf & g)
        self.assertTrue(ncs.isnot_false())
        state = fsm.pick_one_state(~lt & lf & g)
        self.assertTrue(state <= ncs)
        witness = explain_nc(fsm, state, ["at"], ~lt & ~lf & g)
        self.assertTrue(witness[0] == state)
        for (s, ag, sp) in zip(witness[::2], witness[1::2], witness[2::2]):
            self.assertTrue(s.isnot_false())
            self.assertTrue(ag in [{"at"}])           
            self.assertTrue(sp.isnot_false())
            self.assertTrue(sp <= fsm.equivalent_states(s, {"at"}))
            
        self.assertTrue(witness[-1] <= ~lt & ~lf & g)
                                   
                                                                    
    def test_nc_dincry(self):
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
        
        # nC<c1,c2> ~c3.payer
        ncs = nc(fsm, ["c1", "c2"], c3h)
        self.assertTrue(ncs.isnot_false())
        state = fsm.pick_one_state(ncs)
        witness = explain_nc(fsm, state, ["c1", "c2"], c3h)
        self.assertTrue(witness[0] == state)
        for (s, ag, sp) in zip(witness[::2], witness[1::2], witness[2::2]):
            self.assertTrue(s.isnot_false())
            self.assertTrue(ag in [{"c1"}, {"c2"}])
            self.assertTrue(sp.isnot_false())
            self.assertTrue((sp <= fsm.equivalent_states(s, {"c1"})) |
                            (sp <= fsm.equivalent_states(s, {"c2"})))
            
        self.assertTrue(witness[-1] <= c3h)
        
        
    def test_reachable_simple(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        state = fsm.pick_one_state(~lf & ~lt & ~g)
        self.assertTrue(state <= fsm.reachable_states)
        witness = explain_reachable(fsm, state)
        self.assertIsNotNone(witness)
        self.assertEqual(witness[0], state)
        
        for (s, i, sp) in zip(witness[::2], witness[1::2], witness[2::2]):
            self.assertTrue(s <= fsm.reachable_states)
            self.assertTrue(sp <= fsm.reachable_states)
            self.assertIsNotNone(i)
            self.assertTrue(i <= fsm.get_inputs_between_states(sp, s))
        
        self.assertTrue(witness[-1] <= fsm.init)
        
        
    def test_reachable_dincry(self):
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
        
        state = fsm.pick_one_state(c1p & ~c2p & ~c3p & odd)
        self.assertTrue(state <= fsm.reachable_states)
        witness = explain_reachable(fsm, state)
        self.assertIsNotNone(witness)
        self.assertEqual(witness[0], state)
        
        for (s, i, sp) in zip(witness[::2], witness[1::2], witness[2::2]):
            self.assertTrue(s <= fsm.reachable_states)
            self.assertTrue(sp <= fsm.reachable_states)
            self.assertIsNotNone(i)
            self.assertTrue(i <= fsm.get_inputs_between_states(sp, s))
        
        self.assertTrue(witness[-1] <= fsm.init)