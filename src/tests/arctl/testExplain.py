import unittest

from pynusmv.fsm.fsm import BddFsm
from pynusmv.init.init import init_nusmv, deinit_nusmv

from tools.arctl.parsing import parseArctl
from tools.arctl.eval import evalArctl, evalArctl_from_string as evalStr
from tools.arctl.explain import explain_eax


class TestExplain(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def init_model(self):
        fsm = BddFsm.from_filename("tests/arctl/counters.smv")
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