import unittest
import sys

from pynusmv.nusmv.cmd import cmd

from pynusmv.prop import PropDb
from pynusmv.dd import (BDD, enable_dynamic_reordering,
                        disable_dynamic_reordering, dynamic_reordering_enabled,
                        reorder)
from pynusmv.fsm import BddFsm
from pynusmv.mc import eval_simple_expression
from pynusmv.exception import MissingManagerError
from pynusmv import glob

from pynusmv.init import init_nusmv, deinit_nusmv

class TestBDD(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def init_model(self):
        fsm = BddFsm.from_filename("tests/pynusmv/models/admin.smv")
        return (fsm, fsm.bddEnc, fsm.bddEnc.DDmanager)
    
    
    def test_reordering(self):
        fsm, enc, manager = self.init_model()
        
        self.assertFalse(dynamic_reordering_enabled())
        enable_dynamic_reordering()
        self.assertTrue(dynamic_reordering_enabled())
        disable_dynamic_reordering()
        self.assertFalse(dynamic_reordering_enabled())
    
    def test_force_reordering(self):
        glob.load("tests/pynusmv/models/admin.smv")
        glob.compute_model(variables_ordering="tests/pynusmv/models/admin.ord")
        fsm = glob.prop_database().master.bddFsm
        
        self.assertTupleEqual(("admin", "state"),
                              fsm.bddEnc.get_variables_ordering())
        reorder(fsm.bddEnc.DDmanager)
        self.assertTupleEqual(("state", "admin"),
                              fsm.bddEnc.get_variables_ordering())
    
    
    def test_get_true(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        self.assertIsNotNone(true)
        self.assertTrue(true.is_true())
        self.assertFalse(true.is_false())
        self.assertFalse(true.isnot_true())
        self.assertTrue(true.isnot_false())
        
        true = BDD.true()
        self.assertIsNotNone(true)
        self.assertTrue(true.is_true())
        self.assertFalse(true.is_false())
        self.assertFalse(true.isnot_true())
        self.assertTrue(true.isnot_false())
        
        true = BDD.true(fsm)
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
        self.assertTrue(false.isnot_true())
        self.assertFalse(false.isnot_false())
        self.assertTrue(false.isnot_true())
        
        false = BDD.false()
        self.assertIsNotNone(false)
        self.assertTrue(false.is_false())
        self.assertFalse(false.is_true())
        self.assertTrue(false.isnot_true())
        self.assertFalse(false.isnot_false())
        self.assertTrue(false.isnot_true())
        
        false = BDD.false()
        self.assertIsNotNone(false)
        self.assertTrue(false.is_false())
        self.assertFalse(false.is_true())
        self.assertTrue(false.isnot_true())
        self.assertFalse(false.isnot_false())
        self.assertTrue(false.isnot_true())
    
    
    def test_dup(self):
        (fsm, enc, manager) = self.init_model()
        
        false = BDD.false(manager)
        true = BDD.true(manager)
        init = fsm.init
        
        self.assertEqual(false, false.dup())
        self.assertEqual(true, true.dup())
        self.assertEqual(init, init.dup())
        
        self.assertNotEqual(true, init.dup())
        self.assertNotEqual(init, false.dup())
        
        
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
        
        
    def test_uncomparable_sets(self):
        (fsm, enc, manager) = self.init_model()
        
        alice = eval_simple_expression(fsm, "admin = alice")
        processing = eval_simple_expression(fsm, "state = processing")
        
        self.assertFalse(alice <= processing)
        self.assertFalse(processing <= alice)
        self.assertFalse(alice < processing)
        self.assertFalse(processing < alice)
        self.assertFalse(alice == processing)
        self.assertTrue(alice != processing)
        self.assertFalse(alice >= processing)
        self.assertFalse(processing >= alice)
        self.assertFalse(alice > processing)
        self.assertFalse(processing > alice)
        
    
    def test_hashes(self):
        (fsm, enc, manager) = self.init_model()
        
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        noadmin = eval_simple_expression(fsm, "admin = none")
        alice = eval_simple_expression(fsm, "admin = alice")
        processing = eval_simple_expression(fsm, "state = processing")
        
        self.assertEqual(hash(true), hash(~false))
        self.assertNotEqual(hash(true), hash(false))
        self.assertEqual(hash(init & noadmin), hash(init))
        
        bdddict = {true, false, init, noadmin, alice, processing}
        self.assertEqual(len(bdddict), 6)
    
    def test_minimize(self):
        (fsm, enc, manager) = self.init_model()
        
        noadmin = eval_simple_expression(fsm, "admin = none")
        processing = eval_simple_expression(fsm, "state = processing")
        
        self.assertIsNotNone(processing.minimize(noadmin))
        self.assertTrue(processing.minimize(noadmin).isnot_false())
    
    
    def test_size(self):
        (fsm, enc, manager) = self.init_model()
        true = BDD.true(manager)
        false = BDD.false(manager)
        init = fsm.init
        noadmin = eval_simple_expression(fsm, "admin = none")
        alice = eval_simple_expression(fsm, "admin = alice")
        processing = eval_simple_expression(fsm, "state = processing")
        
        self.assertEqual(BDD.true().size, 1)
        self.assertEqual(BDD.false().size, 1)
        self.assertEqual(fsm.pick_one_state(BDD.true()).size,
                         len(fsm.bddEnc.get_variables_ordering("bits")) + 1)
        self.assertEqual(init.size, 5)
        self.assertEqual(processing.size, 3)
        