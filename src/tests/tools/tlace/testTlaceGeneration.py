import unittest

from pynusmv.nusmv.cmd import cmd

from pynusmv.fsm import BddFsm
from pynusmv.prop import PropDb
from pynusmv import glob

from tools.tlace.check import check

from pynusmv.init import init_nusmv, deinit_nusmv

class TestTlaceGeneration(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
    
    
    def test_check_violated_spec(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i " 
                                     "tests/tools/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        self.assertTrue(propDb.get_size() >= 1, "propDb has no properties")
        prop = propDb.get_prop_at_index(0)
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")
        
    
    def test_check_violated_spec2(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                     "tests/tools/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        self.assertTrue(propDb.get_size() >= 2, "propDb misses some props")
        prop = propDb.get_prop_at_index(1)
        self.assertIsNotNone(prop, "prop should not be None")
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")
        
    
    def test_check_satisfied_spec(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                     "tests/tools/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        self.assertTrue(propDb.get_size() >= 3, "propDb misses some props")
        prop = propDb.get_prop_at_index(2)
        self.assertIsNotNone(prop, "prop should not be None")
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertTrue(res[0], "spec should be satisfied")
        self.assertIsNone(res[1], "TLACE should not exist")
        
    
    def test_check_ex_spec(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                     "tests/tools/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        self.assertTrue(propDb.get_size() >= 4, "propDb misses some props")
        prop = propDb.get_prop_at_index(3)
        self.assertIsNotNone(prop, "prop should not be None")
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")