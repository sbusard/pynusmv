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
    
    
    def test_check(self):
        # Initialize the model
        ret = cmd.Cmd_CommandExecute("read_model -i pynusmv/tests/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_CommandExecute("go")
        self.assertEqual(ret, 0)
        
        propDb = PropDb.get_global_database()
        master = propDb.master
        fsm = propDb.master.bddfsm
        prop = propDb.get_prop_at_index(0)
        spec = prop.exprcore
        
        res = check(fsm, spec)
        self.assertFalse(res[0])