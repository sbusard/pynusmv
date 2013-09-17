import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob

from pynusmv.nusmv.dd import dd as nsdd


class TestInterrupt(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def model(self):
        glob.load_from_file("tests/pynusmv/models/inputs.smv")
        glob.compute_model()
        fsm = glob.prop_database().master.bddFsm
        self.assertIsNotNone(fsm)
        return fsm
        
    @unittest.skip("Takes too long, used to check CTRL-C")
    def test_interrupt(self):
        fsm = self.model()
        
        init = fsm.init
        initptr = init._ptr
        reachptr = fsm.reachable_states._ptr
        managerptr = fsm.bddEnc.DDmanager._ptr
        for i in range(10000000):
            initptr = nsdd.bdd_and(managerptr,initptr,initptr)
            #init = init & fsm.reachable_states