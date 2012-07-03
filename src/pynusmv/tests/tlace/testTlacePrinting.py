import unittest

from ...nusmv.cinit import cinit
from ...nusmv.cmd import cmd
from ...nusmv.compile.symb_table import symb_table
from ...nusmv.enc.bdd import bdd as bddEnc

from ...fsm.fsm import BddFsm
from ...node.node import Node
from ...prop.propDb import PropDb

from ...tools.tlace.check import check
from ...tools.tlace.xml import print_xml_representation

class TestTlacePrinting(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
    
    
    def test_print_violated_spec_admin(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i " 
                                     "pynusmv/tests/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = PropDb.get_global_database()
        master = propDb.master
        fsm = propDb.master.bddfsm
        self.assertTrue(propDb.get_size() >= 1, "propDb has no properties")
        prop = propDb.get_prop_at_index(0)
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")
        
        print_xml_representation(fsm, res[1], spec)
        
        
    def test_print_violated_spec_egmod(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i " 
                                     "pynusmv/tests/tlace/eg.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = PropDb.get_global_database()
        master = propDb.master
        fsm = propDb.master.bddfsm
        self.assertTrue(propDb.get_size() >= 1, "propDb has no properties")
        prop = propDb.get_prop_at_index(0)
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")

        print_xml_representation(fsm, res[1], spec)
        
    
    def test_print_violated_spec_egmod2(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i " 
                                     "pynusmv/tests/tlace/eg.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = PropDb.get_global_database()
        master = propDb.master
        fsm = propDb.master.bddfsm
        self.assertTrue(propDb.get_size() >= 2, "propDb has no properties")
        prop = propDb.get_prop_at_index(1)
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")

        print_xml_representation(fsm, res[1], spec)