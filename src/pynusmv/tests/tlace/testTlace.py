import unittest

from ...nusmv.cinit import cinit
from ...nusmv.cmd import cmd

from ...fsm.fsm import BddFsm
from ...node.node import Node
from ...prop.propDb import PropDb

from ...tools.tlace.check import check

class TestTlace(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
    
    
    def test_check_violated_spec(self):
        # Initialize the model
        ret = cmd.Cmd_CommandExecute("read_model -i " 
                                     "pynusmv/tests/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_CommandExecute("go")
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
        
    
    def test_check_violated_spec2(self):
        # Initialize the model
        ret = cmd.Cmd_CommandExecute("read_model -i"
                                     "pynusmv/tests/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_CommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = PropDb.get_global_database()
        master = propDb.master
        fsm = propDb.master.bddfsm
        self.assertTrue(propDb.get_size() >= 2, "propDb misses some props")
        prop = propDb.get_prop_at_index(1)
        self.assertIsNotNone(prop, "prop should not be None")
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0], "spec should be violated")
        self.assertIsNotNone(res[1], "TLACE should be given")
        
        
    def test_check_satisfied_spec(self):
        # Initialize the model
        ret = cmd.Cmd_CommandExecute("read_model -i"
                                     "pynusmv/tests/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_CommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = PropDb.get_global_database()
        master = propDb.master
        fsm = propDb.master.bddfsm
        self.assertTrue(propDb.get_size() >= 3, "propDb misses some props")
        prop = propDb.get_prop_at_index(2)
        self.assertIsNotNone(prop, "prop should not be None")
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertTrue(res[0])
        self.assertIsNone(res[1])
        
    
    def test_check_ex_spec(self):
        # Initialize the model
        ret = cmd.Cmd_CommandExecute("read_model -i"
                                     "pynusmv/tests/tlace/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_CommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = PropDb.get_global_database()
        master = propDb.master
        fsm = propDb.master.bddfsm
        self.assertTrue(propDb.get_size() >= 4, "propDb misses some props")
        prop = propDb.get_prop_at_index(3)
        self.assertIsNotNone(prop, "prop should not be None")
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0])
        self.assertIsNotNone(res[1])