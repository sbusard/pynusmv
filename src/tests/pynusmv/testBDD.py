import unittest
import sys

from pynusmv.nusmv.cmd import cmd

from pynusmv.prop.propDb import PropDb
from pynusmv.dd.bdd import BDD
from pynusmv.fsm.bddFsm import BddFsm

from pynusmv.init.init import init_nusmv, deinit_nusmv

class TestBDD(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def init_model(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/admin.smv")
        return (fsm, fsm.bddEnc, fsm.bddEnc.DDmanager)
    
    
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
        
        
    def test_true_false_equalities(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        
        self.assertTrue(false != true)        
        self.assertFalse(false == true)
        self.assertTrue(false == false)
        self.assertTrue(true == true)
        self.assertTrue((false != true) == (not false == true))
        

    def test_true_false_comparisons(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        
        self.assertTrue(false <= true)
        self.assertTrue(false < true)
        self.assertFalse(false > true)
        self.assertFalse(false >= true)
        
        self.assertFalse(true <= false)
        self.assertFalse(true < false)
        self.assertTrue(true > false)
        self.assertTrue(true >= false)
        
    
    def test_init_equalities(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        
        self.assertIsNotNone(init)
        
        self.assertTrue(init != true)
        self.assertTrue(init != false)
        self.assertFalse(init == true)
        self.assertFalse(init == false)
        

    def test_init_comparisons(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        
        self.assertIsNotNone(init)
        
        self.assertTrue(false < init < true)
        self.assertTrue(false <= init < true)
        self.assertTrue(false < init <= true)
        self.assertFalse(init < false)
        self.assertFalse(init <= false)
        self.assertFalse(true < init)
        self.assertFalse(true <= init)
        
    
    def test_true_false_and(self):
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
        
        
    def test_true_false_or(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init        
        
        self.assertTrue((true | false) == (true + false))
        self.assertTrue(true | false == true)
        self.assertTrue(true | true == true)
        self.assertTrue(false | false == false)
        self.assertTrue(init | true == true)
        self.assertTrue(init | false == init)
        
        
    def test_true_false_xor(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        
        self.assertTrue(true ^ false == true)
        self.assertTrue(true ^ true == false)
        self.assertTrue(false ^ false == false)
        self.assertTrue(init ^ true == ~init)
        self.assertTrue(init ^ false == init)
        
        
    def test_true_false_not(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        
        self.assertTrue(~true == -true)
        self.assertTrue(~false == -false)
        self.assertTrue(~true == false)
        self.assertTrue(~false == true)
        self.assertTrue(false < ~init < true)
        
        
    def test_true_false_sub(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        
        self.assertTrue(true - false == true)
        self.assertTrue(true - true == false)
        self.assertTrue(false - true == false)
        self.assertTrue(false - false == false)
        self.assertTrue(init - true == false)
        self.assertTrue(init - false == init)
        self.assertTrue(true - init == ~init)
        self.assertTrue(false - init == false)
        