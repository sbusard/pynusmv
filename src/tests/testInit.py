import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.fsm.fsm import BddFsm

class TestInit(unittest.TestCase):
    
    def test_init(self):
        init_nusmv()
        fsm = BddFsm.from_filename("tests/admin.smv") # Should not produce error
        deinit_nusmv()