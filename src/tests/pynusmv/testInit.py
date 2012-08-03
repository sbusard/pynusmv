import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv, reset_nusmv
from pynusmv.fsm.fsm import BddFsm

class TestInit(unittest.TestCase):
    
    def test_init(self):
        init_nusmv()
        # Should not produce error
        fsm = BddFsm.from_filename("tests/pynusmv/admin.smv")
        reset_nusmv()
        # Should not produce error
        fsm = BddFsm.from_filename("tests/pynusmv/admin.smv")
        deinit_nusmv()