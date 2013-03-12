import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from tools.mas import glob
from tools.ctlk.ast import (TrueExp, FalseExp, Init, Reachable,
                            Atom, Not, And, Or, Implies, Iff, 
                            AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                            nK, nE, nD, nC, K, E, D, C)
from tools.ctlk.parsing import parseCTLK
from tools.ctlk.eval import evalCTLK
from tools.ctlk.tlace.tlace import Tlacenode, TemporalBranch, EpistemicBranch
from tools.ctlk.tlace.explain import explain_witness, explain_countex
from tools.ctlk.tlace.xml import xml_countex, xml_witness

class TestXML(unittest.TestCase):
    
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
        fsm = self.simplemodel()
        
        spec = parseCTLK("True")[0]
        self.assertIsNotNone(spec)
        state = fsm.pick_one_state(fsm.init & evalCTLK(fsm, spec))
        self.assertIsNotNone(state)
        
        expl = explain_witness(fsm, state, spec)
        self.assertIsNotNone(expl)
        self.assertEqual(type(expl), Tlacenode)
        self.assertEqual(expl.state, state)
        self.assertEqual(len(expl.atomics), 0)
        self.assertEqual(len(expl.branches), 0)
        self.assertEqual(len(expl.universals), 0)
        
        print(xml_witness(fsm, expl, spec))
        
        
    def test_false(self):
        fsm = self.simplemodel()
        
        spec = parseCTLK("False")[0]
        self.assertIsNotNone(spec)
        state = fsm.pick_one_state(fsm.init & ~evalCTLK(fsm, spec))
        self.assertIsNotNone(state)
        
        expl = explain_countex(fsm, state, spec)
        self.assertIsNotNone(expl)
        self.assertEqual(type(expl), Tlacenode)
        self.assertEqual(expl.state, state)
        self.assertEqual(len(expl.atomics), 0)
        self.assertEqual(len(expl.branches), 0)
        self.assertEqual(len(expl.universals), 0)
        
        print(xml_countex(fsm, expl, spec))
        
        
    def test_init(self):
        fsm = self.simplemodel()
        
        spec = parseCTLK("Init")[0]
        self.assertIsNotNone(spec)
        state = fsm.pick_one_state(fsm.init & evalCTLK(fsm, spec))
        self.assertIsNotNone(state)
        
        expl = explain_witness(fsm, state, spec)
        self.assertIsNotNone(expl)
        self.assertEqual(type(expl), Tlacenode)
        self.assertEqual(expl.state, state)
        self.assertEqual(len(expl.atomics), 1)
        self.assertEqual(type(expl.atomics[0]), Init)
        self.assertEqual(len(expl.branches), 0)
        self.assertEqual(len(expl.universals), 0)
        
        print(xml_witness(fsm, expl, spec))
        
        
    def test_reachable(self):
        fsm = self.simplemodel()
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        spec = parseCTLK("Reachable")[0]
        self.assertIsNotNone(spec)
        state = fsm.pick_one_state(~lt & ~lf & ~g)
        self.assertIsNotNone(state)
        self.assertTrue(state <= evalCTLK(fsm, spec))
        
        expl = explain_witness(fsm, state, spec)
        self.assertIsNotNone(expl)
        self.assertEqual(type(expl), Tlacenode)
        self.assertEqual(expl.state, state)
        self.assertEqual(len(expl.atomics), 0)
        self.assertEqual(len(expl.branches), 1)
        self.assertEqual(len(expl.universals), 0)
        
        branch = expl.branches[0]
        self.assertEqual(type(branch), TemporalBranch)
        self.assertEqual(type(branch.specification), Reachable)
        path = branch.path
        self.assertIsNone(branch.loop)
        for (s, i, sp) in zip(path[::2], path[1::2], path[2::2]):
            self.assertIsNotNone(s)
            self.assertTrue(s.state <= fsm.reachable_states)
            self.assertEqual(len(s.atomics), 0)
            self.assertEqual(len(s.branches), 0)
            self.assertEqual(len(s.universals), 0)
            self.assertIsNotNone(sp)
            self.assertTrue(sp.state <= fsm.reachable_states)
            self.assertEqual(len(sp.branches), 0)
            self.assertEqual(len(sp.universals), 0)
            self.assertTrue(i <=
                               fsm.get_inputs_between_states(sp.state, s.state))
        self.assertEqual(len(path[-1].atomics), 1)
        self.assertEqual(type(path[-1].atomics[0]), Init)
        
        print(xml_witness(fsm, expl, spec))
        
        
    def test_full_witness(self):
        fsm = self.model()
        
        spec = parseCTLK("EF (EX nK<'c1'> 'c2.payer' & EG nD<'c1','c2'> 'c3.payer')")[0]
        self.assertIsNotNone(spec)
        state = fsm.pick_one_state(fsm.init & evalCTLK(fsm, spec))
        self.assertIsNotNone(state)
        
        expl = explain_witness(fsm, state, spec)
        self.assertIsNotNone(expl)
        
        print(xml_witness(fsm, expl, spec))
        
        
        spec = parseCTLK("E[nE<'c1','c2'> 'c3.payer' U"
                        " E[ nC<'c1','c3'> ('c1.payer' | 'c2.payer') W"
                        " nC<'c2'> ('c1.coin = head' <-> 'c2.coin = head')]]")[0]
        self.assertIsNotNone(spec)
        state = fsm.pick_one_state(fsm.init & evalCTLK(fsm, spec))
        self.assertIsNotNone(state)
        
        expl = explain_witness(fsm, state, spec)
        self.assertIsNotNone(expl)
        
        print(xml_witness(fsm, expl, spec))
        
        
        spec = parseCTLK("AG(~'c1.payer' | E<'c1'> ('c1.payer' -> "
                         "C<'c2','c3'> AF 'c1.payer'))")[0]
        self.assertIsNotNone(spec)
        state = fsm.pick_one_state(fsm.init & evalCTLK(fsm, spec))
        self.assertIsNotNone(state)
        
        expl = explain_witness(fsm, state, spec)
        self.assertIsNotNone(expl)
        
        print(xml_witness(fsm, expl, spec))
        
        
    def test_full_countex(self):
        fsm = self.model()
        
        spec = parseCTLK("'c2.payer' -> A[AX ('c1.payer' | ~'c1.payer')"
                         " U K<'c1'> ('c2.payer')]")[0]
        self.assertIsNotNone(spec)
        self.assertTrue((fsm.init & ~evalCTLK(fsm, spec)).isnot_false())
        state = fsm.pick_one_state(fsm.init & ~evalCTLK(fsm, spec))
        self.assertIsNotNone(state)
        
        expl = explain_countex(fsm, state, spec)
        self.assertIsNotNone(expl)
        
        print(xml_countex(fsm, expl, spec))
        
        
        spec = parseCTLK("AG(~'c1.payer' | E<'c1'> ('c1.payer' -> "
                         "C<'c2','c3'> AF 'c1.payer'))")[0]
        self.assertIsNotNone(spec)
        self.assertTrue((fsm.init & ~evalCTLK(fsm, spec)).isnot_false())
        state = fsm.pick_one_state(fsm.init & ~evalCTLK(fsm, spec))
        self.assertIsNotNone(state)
        
        expl = explain_countex(fsm, state, spec)
        self.assertIsNotNone(expl)
        
        print(xml_countex(fsm, expl, spec))