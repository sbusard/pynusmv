import unittest

from pynusmv.init import (init_nusmv, deinit_nusmv, reset_nusmv)
from pynusmv.exception import NuSMVInitError
from pynusmv.fsm import BddFsm

class TestInit(unittest.TestCase):
    
    def test_init(self):
        init_nusmv()
        # Should not produce error
        fsm = BddFsm.from_filename("tests/pynusmv/models/admin.smv")
        reset_nusmv()
        # Should not produce error
        fsm = BddFsm.from_filename("tests/pynusmv/models/admin.smv")
        deinit_nusmv()
        
        
    def test_two_init(self):
        with self.assertRaises(NuSMVInitError):
            init_nusmv()
            init_nusmv()
        deinit_nusmv()
        
    
    def test_init_deinit_stats(self):
        init_nusmv()
        deinit_nusmv(ddinfo=True)