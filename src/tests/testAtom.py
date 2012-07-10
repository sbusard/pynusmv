import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd

from pynusmv.nusmv.compile.symb_table import symb_table
from pynusmv.nusmv.compile.type_checking import type_checking

from pynusmv.mc.mc import eval_ctl_spec
from pynusmv.node.specnode import atom
from pynusmv.prop.propDb import PropDb

class TestAtom(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
        
    
    def init_model(self):
        ret = cmd.Cmd_SecureCommandExecute("read_model -i tests/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        propDb = PropDb.get_global_database()
        fsm = propDb.master.bddFsm
        enc = fsm.bddEnc
        return (fsm, enc, enc.DDmanager)
        
    
    def test_success(self):
        spec = atom("admin = alice")
        fsm, _, _ = self.init_model()
        specbdd = eval_ctl_spec(fsm, spec)
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
        self.assertFalse(type_checking.TypeChecker_is_expression_wellformed(tc, spec._ptr, None))
        
        
    # Bug since "admin" is not a boolean
    @unittest.skip
    def test_bool(self):
        spec = atom("admin")
        fsm, _, _ = self.init_model()
        
        st = fsm.bddEnc.symbTable
        tc = symb_table.SymbTable_get_type_checker(st)
        
        specbdd = eval_ctl_spec(fsm, spec)