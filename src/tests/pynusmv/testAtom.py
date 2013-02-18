import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd

from pynusmv.fsm import BddFsm
from pynusmv.init import init_nusmv, deinit_nusmv


from pynusmv.mc import eval_ctl_spec, eval_simple_expression
from pynusmv.prop import atom, NuSMVTypeCheckingError
from pynusmv.prop import PropDb

class TestAtom(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
        
    def tearDown(self):
        deinit_nusmv()
        
    
    def init_model(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/admin.smv")
        return (fsm, fsm.bddEnc, fsm.bddEnc.DDmanager)
        
    
    def test_success(self):    
        fsm, _, _ = self.init_model()
        spec = atom("admin = alice")
        specbdd = eval_ctl_spec(fsm, spec)
        self.assertTrue((specbdd & fsm.init).is_false())
        
    
    def test_success_or(self):    
        fsm, _, _ = self.init_model()
        spec = atom("admin = alice | admin = bob")
        specbdd = eval_ctl_spec(fsm, spec)
        self.assertTrue((specbdd & fsm.init).is_false())
        
        
    def test_success_sexp(self):
        fsm, _, _ = self.init_model()
        specbdd = eval_simple_expression(fsm, "state = waiting")
        self.assertTrue(specbdd.isnot_false())
        self.assertTrue((specbdd & fsm.init).is_false())
        
        
    def test_wrong_type(self):
        fsm, _, _ = self.init_model()
        spec = atom("admin = starting")
        specbdd = eval_ctl_spec(fsm, spec)
        self.assertTrue(specbdd.is_false())
    
        
    def test_unknown(self):    
        fsm, _, _ = self.init_model()
        with self.assertRaises(NuSMVTypeCheckingError):
            spec = atom("admin = folded")
        
        
    def test_bool(self):
        fsm, _, _ = self.init_model()
        with self.assertRaises(NuSMVTypeCheckingError):
            spec = atom("admin")