import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.fsm.fsm import BddFsm

class TestGC(unittest.TestCase):
    
    def test_gc(self):
        init_nusmv()
        
        fsm = BddFsm.from_filename("tests/pynusmv/admin.smv")
        init = fsm.init
        
        deinit_nusmv()