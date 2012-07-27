import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd

from pynusmv.fsm.fsm import BddFsm
from pynusmv.init.init import init_nusmv, deinit_nusmv

from pynusmv.nusmv.compile.symb_table import symb_table
from pynusmv.nusmv.compile.type_checking import type_checking

from pynusmv.mc.mc import eval_ctl_spec, eval_simple_expression
from pynusmv.spec.spec import atom
from pynusmv.prop.propDb import PropDb

class TestAtom(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
        
    def tearDown(self):
        deinit_nusmv()
        
    
    def init_model(self):
        fsm = BddFsm.from_filename("tests/admin.smv")
        return (fsm, fsm.bddEnc, fsm.bddEnc.DDmanager)
        
    
    def test_success(self):
        spec = atom("admin = alice")
        fsm, _, _ = self.init_model()
        specbdd = eval_ctl_spec(fsm, spec)
        self.assertTrue((specbdd & fsm.init).is_false())
        
    
    def test_success_or(self):
        spec = atom("admin = alice | admin = bob")
        fsm, _, _ = self.init_model()
        specbdd = eval_ctl_spec(fsm, spec)
        self.assertTrue((specbdd & fsm.init).is_false())
        
        
    def test_success_sexp(self):
        fsm, _, _ = self.init_model()
        specbdd = eval_simple_expression(fsm, "state = waiting")
        self.assertTrue(specbdd.isnot_false())
        self.assertTrue((specbdd & fsm.init).is_false())
        
        
    def test_wrong_type(self):
        spec = atom("admin = starting")
        fsm, _, _ = self.init_model()
        specbdd = eval_ctl_spec(fsm, spec)
        self.assertTrue(specbdd.is_false())
    
        
    def test_unknown(self):
        spec = atom("admin = folded")
        fsm, _, _ = self.init_model()
        
        st = fsm.bddEnc.symbTable
        tc = symb_table.SymbTable_get_type_checker(st)
        self.assertFalse(
                type_checking.TypeChecker_is_expression_wellformed(tc,
                                    spec._ptr, None))
        
        
    # Bug since "admin" is not a boolean
    @unittest.skip
    def test_bool(self):
        spec = atom("admin")
        fsm, _, _ = self.init_model()
        
        st = fsm.bddEnc.symbTable
        tc = symb_table.SymbTable_get_type_checker(st)
        
        specbdd = eval_ctl_spec(fsm, spec)