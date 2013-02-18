import unittest

from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.compile.symb_table import symb_table
from pynusmv.nusmv.enc.bdd import bdd as bddEnc

from pynusmv.fsm import BddFsm
from pynusmv.prop import PropDb
from pynusmv import glob

from tools.tlace.check import check
from tools.tlace.xml import xml_representation

from pynusmv.init import init_nusmv, deinit_nusmv

class TestTlacePrinting(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
    
    
    def test_print_violated_spec_admin(self):
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
        
        print(xml_representation(fsm, res[1], spec))
        
        
    def test_print_violated_spec_egmod(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i " 
                                     "tests/tools/tlace/eg.smv")
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

        print(xml_representation(fsm, res[1], spec))
        
    
    def test_print_violated_spec_egmod2(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i " 
                                     "tests/tools/tlace/eg.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        self.assertTrue(propDb.get_size() >= 2, "propDb has no properties")
        prop = propDb.get_prop_at_index(1)
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")

        print(xml_representation(fsm, res[1], spec))
        
    
    def test_print_violated_spec_admin_af(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i " 
                                     "tests/tools/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        self.assertTrue(propDb.get_size() >= 5, "propDb has no properties")
        prop = propDb.get_prop_at_index(4)
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")

        print(xml_representation(fsm, res[1], spec))
        
        
    def test_print_violated_spec_admin_ax(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i " 
                                     "tests/tools/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        self.assertTrue(len(propDb) >= 6, "propDb has no properties")
        prop = propDb[5]
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")

        print(xml_representation(fsm, res[1], spec))