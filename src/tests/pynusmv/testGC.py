import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.fsm import BddFsm

class TestGC(unittest.TestCase):
    
    def test_gc(self):
        """
        This test should not produce a segfault due to freed memory after
        deiniting NuSMV. This is thanks to the PyNuSMV GC system that keeps
        track of all wrapped pointers and free them when deiniting NuSMV, if
        needed.
        """
        init_nusmv()
        
        fsm = BddFsm.from_filename("tests/pynusmv/models/admin.smv")
        init = fsm.init
        
        deinit_nusmv()