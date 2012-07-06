import unittest
import sys

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd

from pynusmv.prop.propDb import PropDb
from pynusmv.dd.bdd import BDD

class TestBDD(unittest.TestCase):
    
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
    
    
    def test_get_true(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        self.assertIsNotNone(true)
        self.assertTrue(true.is_true())
        self.assertFalse(true.is_false())
        self.assertFalse(true.isnot_true())
        self.assertTrue(true.isnot_false())
        

    def test_get_false(self):
        (fsm, enc, manager) = self.init_model()
        
        false = BDD.false(manager)
        self.assertIsNotNone(false)
        self.assertTrue(false.is_false())
        self.assertFalse(false.is_true())
        self.assertFalse(false.isnot_false())
        self.assertTrue(false.isnot_true())
        
        
    def test_true_false_comparisons(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        
        self.assertTrue(false != true)        
        self.assertFalse(false == true)
        self.assertTrue(false == false)
        self.assertTrue(true == true)
        self.assertTrue((false != true) == (not false == true))
        
        self.assertTrue(false <= true)
        self.assertTrue(false < true)
        self.assertFalse(false > true)
        self.assertFalse(false >= true)
        
        self.assertFalse(true <= false)
        self.assertFalse(true < false)
        self.assertTrue(true > false)
        self.assertTrue(true >= false)
        
    
    def test_init_comparisons(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        
        self.assertIsNotNone(init)
        
        self.assertTrue(init != true)
        self.assertTrue(init != false)
        self.assertFalse(init == true)
        self.assertFalse(init == false)
        
        self.assertTrue(false < init < true)
        self.assertTrue(false <= init < true)
        self.assertTrue(false < init <= true)
        self.assertFalse(init < false)
        self.assertFalse(init <= false)
        self.assertFalse(true < init)
        self.assertFalse(true <= init)
        
    
    def test_true_false_logic(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        
        self.assertTrue((true & false) == (true * false))
        self.assertTrue(true & false == false)
        self.assertTrue(true & true == true)
        self.assertTrue(false & false == false)
        self.assertTrue(init & true == init)
        self.assertTrue(init & false == false)
        
        
        self.assertTrue((true | false) == (true + false))
        self.assertTrue(true | false == true)
        self.assertTrue(true | true == true)
        self.assertTrue(false | false == false)
        self.assertTrue(init | true == true)
        self.assertTrue(init | false == init)
        