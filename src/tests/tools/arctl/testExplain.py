import unittest

from pynusmv.fsm import BddFsm
from pynusmv.init import init_nusmv, deinit_nusmv

from tools.arctl.parsing import parseArctl
from tools.arctl.eval import evalArctl, evalArctl_from_string as evalStr
from tools.arctl.explain import (explain_eax, explain_eau, explain_eag,
                                 explain_witness)


class TestExplain(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def init_model(self):
        fsm = BddFsm.from_filename("tests/tools/arctl/counters.smv")
        self.assertIsNotNone(fsm)
        return fsm
        
    def test_eax_explain(self):
        fsm = self.init_model()
        self.assertIsNotNone(fsm)
        
        spec = parseArctl("E<'TRUE'>X 'c1.c = 1'")[0]
        specbdd = evalArctl(fsm, spec)
        
        s = fsm.pick_one_state(specbdd & fsm.init)
        self.assertIsNotNone(s)
        
        ac = evalStr(fsm, "'TRUE'")
        phi = evalStr(fsm, "'c1.c = 1'")
        
        path = explain_eax(fsm, s, ac, phi)
        
        self.assertTrue(s == path[0])
        self.assertTrue(path[1] <= ac)
        self.assertTrue(path[2] <= phi)
        self.assertTrue(path[2] <= fsm.post(s, path[1]))
       
       
    def test_eau_explain(self):
        fsm = self.init_model()
        self.assertIsNotNone(fsm)
        
        spec = parseArctl("E<'TRUE'>['c1.c < 2' U 'c1.c = 2']")[0]
        specbdd = evalArctl(fsm, spec)
        
        s = fsm.pick_one_state(specbdd & fsm.init)
        self.assertTrue((specbdd & fsm.init).isnot_false())
        self.assertIsNotNone(s)
        self.assertTrue(s.isnot_false())
        
        ac = evalStr(fsm, "'TRUE'")
        phi = evalStr(fsm, "'c1.c < 2'")
        psi = evalStr(fsm, "'c1.c = 2'")
        
        path = explain_eau(fsm, s, ac, phi, psi)
        
        # Check that path contains states of phi and psi
        self.assertTrue(s == path[0])
        for state, inputs in zip(path[::2], path[1::2]):
            self.assertTrue(state <= phi)
            self.assertTrue(inputs <= ac)
        self.assertTrue(path[-1] <= psi)
        
        # Check that path is a path of fsm
        for s, i, sp in zip(path[::2], path[1::2], path[2::2]):
            self.assertTrue(sp <= fsm.post(s, i))
            
            
    def test_eag_explain(self):
        fsm = self.init_model()
        self.assertIsNotNone(fsm)
        
        spec = parseArctl("E<'run = rc1'>G 'c1.c <= 2'")[0]
        specbdd = evalArctl(fsm, spec)
        
        s = fsm.pick_one_state(specbdd & fsm.init)
        self.assertTrue((specbdd & fsm.init).isnot_false())
        self.assertIsNotNone(s)
        self.assertTrue(s.isnot_false())
        
        ac = evalStr(fsm, "'run = rc1'")
        phi = evalStr(fsm, "'c1.c <= 2'")
        
        (path, (inputs, loop)) = explain_eag(fsm, s, ac, phi)
        
        # loop and inputs are not None: the explanation is an infinite path
        self.assertIsNotNone(inputs)
        self.assertIsNotNone(loop)
        
        # Check that path contains states of phi and psi
        self.assertTrue(s == path[0])
        for state, inputs in zip(path[::2], path[1::2]):
            self.assertTrue(state <= phi)
            self.assertTrue(inputs <= ac)
        self.assertTrue(path[-1] <= phi)
        
        # Check that path is a path of fsm
        for s, i, sp in zip(path[::2], path[1::2], path[2::2]):
            self.assertTrue(sp <= fsm.post(s, i))
            
        # Check that loop is effectively a loop
        self.assertTrue(loop in path)
        self.assertTrue(loop <= fsm.post(path[-1], inputs))
        
        
    def test_explain_atom(self):
        fsm = self.init_model()
        self.assertIsNotNone(fsm)
        
        spec = parseArctl("'c1.c <= 2'")[0]
        specbdd = evalArctl(fsm, spec)
        
        s = fsm.pick_one_state(specbdd & fsm.init)
        self.assertTrue(s.isnot_false())
        
        (path, (inp, loop)) = explain_witness(fsm, s, spec)
        # Check that no loop
        self.assertIsNone(inp)
        self.assertIsNone(loop)
        
        # Check path
        self.assertEqual(path, (s,))
        
        
    def test_explain_and_atom(self):
        fsm = self.init_model()
        self.assertIsNotNone(fsm)
        
        spec = parseArctl("'c1.c <= 2' & 'c2.c < 1'")[0]
        specbdd = evalArctl(fsm, spec)
        
        s = fsm.pick_one_state(specbdd & fsm.init)
        self.assertTrue(s.isnot_false())
        
        (path, (inp, loop)) = explain_witness(fsm, s, spec)
        # Check that no loop
        self.assertIsNone(inp)
        self.assertIsNone(loop)
        
        # Check path
        self.assertEqual(path, (s,))
        