import unittest

from pynusmv.fsm import BddFsm
from pynusmv.init import init_nusmv, deinit_nusmv

from tools.arctl.parsing import parseArctl
from tools.arctl.check import checkArctl
from tools.arctl.tlace.explain import explain_witness, explain_countex


class TestExplain(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def init_model(self):
        fsm = BddFsm.from_filename("tests/tools/arctl/counters.smv")
        self.assertIsNotNone(fsm)
        return fsm
        
    
    def init_finite_model(self):
        fsm = BddFsm.from_filename("tests/tools/arctl/finite_counters.smv")
        self.assertIsNotNone(fsm)
        return fsm
        

    def test_full(self):
        fsm = self.init_model()
        
        spec = parseArctl("A<'run = rc1'>["
                            "E<'TRUE'>G 'c1.c < 3' W "
                            "A<'TRUE'>X 'c1.c = 0' ]")[0]
                            
        (result, expl) = checkArctl(fsm, spec, explain_witness, explain_countex)
        self.assertTrue(result)
        
        
        
    def test_full_finite(self):
        fsm = self.init_finite_model()
        
        spec = parseArctl("A<'TRUE'>F (~ E<'TRUE'>X 'TRUE')")[0]
        
        (result, expl) = checkArctl(fsm, spec, explain_witness, explain_countex)
        self.assertTrue(result)