import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.fsm import BddFsm
from tools.arctl.check import checkArctl
from tools.arctl.parsing import parseArctl

class TestCheck(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def init_model(self):
        fsm = BddFsm.from_filename("tests/tools/arctl/counters.smv")
        self.assertIsNotNone(fsm)
        return fsm
        
    
    def test_atom(self):
        fsm = self.init_model()
        
        spec = parseArctl("'c1.c = 0'")[0]
        self.assertTrue(checkArctl(fsm, spec))
        
        spec = parseArctl("'c2.c = 1'")[0]
        self.assertFalse(checkArctl(fsm, spec)[0])
        
        
    def test_eax(self):
        fsm = self.init_model()
        
        spec = parseArctl("E<'run = rc1'>X 'c1.c = 1'")[0]
        self.assertTrue(checkArctl(fsm, spec)[0])
        
        spec = parseArctl("E<'run = rc2'>X 'c1.c = 1'")[0]
        self.assertFalse(checkArctl(fsm, spec)[0])
        
        spec = parseArctl("E<~'run = rc1'>X 'c1.c = 1'")[0]
        self.assertFalse(checkArctl(fsm, spec)[0])
        
        
    def test_aax(self):
        fsm = self.init_model()
        
        spec = parseArctl("A<'run = rc1'>X 'c1.c = 1'")[0]
        self.assertTrue(checkArctl(fsm, spec)[0])
        
        spec = parseArctl("A<'run = rc1'>X 'c2.c = 1'")[0]
        self.assertFalse(checkArctl(fsm, spec)[0])
        
        
    def test_x(self):
        fsm = self.init_model()
        
        spec = parseArctl("E<'FALSE'>X 'TRUE'")[0]
        self.assertFalse(checkArctl(fsm, spec)[0])
        
        spec = parseArctl("A<'FALSE'>X 'TRUE'")[0]
        self.assertFalse(checkArctl(fsm, spec)[0])
        
        spec = parseArctl("~E<'FALSE'>X 'TRUE'")[0]
        self.assertTrue(checkArctl(fsm, spec)[0])