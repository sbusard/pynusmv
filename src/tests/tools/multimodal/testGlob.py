import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from tools.multimodal import glob

class TestGlob(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
        
        
    def test_glob(self):
        glob.load_from_file("tests/tools/multimodal/bitCounter.smv")
        fsm = glob.mm_fsm()
        self.assertIsNotNone(fsm)