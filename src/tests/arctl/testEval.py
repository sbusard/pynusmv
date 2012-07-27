import unittest

from pynusmv.fsm.fsm import BddFsm
from pynusmv.init.init import init_nusmv, deinit_nusmv

from tools.arctl.parsing import parseArctl
from tools.arctl.ast import Atom
from tools.arctl.eval import eval as evalArctl


class TestEval(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
        
    # FIXME There is a segfault when deinit_nusmv is called
    @unittest.skip 
    def test_atom(self):
        fsm = BddFsm.from_filename("tests/arctl/model.smv")
        self.assertIsNotNone(fsm)
        
        specs = parseArctl("'c & i'")
        self.assertEqual(len(specs), 1)
        spec = specs[0]
        self.assertEqual(type(spec), Atom)
        self.assertEqual(spec.value, "c & i")
        
        candi = evalArctl(fsm, spec)
        self.assertIsNotNone(candi)
        self.assertEqual(fsm.init, candi)