import unittest

from pynusmv.init import init_nusmv, deinit_nusmv

from tools.mas import glob

class TestGlob(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
        
    def test_dining_crypto(self):
        glob.load_from_file("tests/tools/ctlk/dining-crypto.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        agents = fsm._epistemic.keys()
        self.assertSetEqual(set(agents), {"c1", "c2", "c3"})