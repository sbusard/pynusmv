import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser
from pynusmv.nusmv.prop import prop
from pynusmv.nusmv.fsm.bdd import bdd as nsfsm

from pynusmv.fsm.fsm import BddFsm
from pynusmv.node.node import Node

class TestBuildModel(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
    
    
    def test_build_model(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i tests/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        propDb = prop.PropPkg_get_prop_database()
        fsm_ptr = prop.PropDb_master_get_bdd_fsm(propDb)
        self.assertIsNotNone(fsm_ptr)
        fsm = BddFsm(fsm_ptr)
        enc = fsm.BddEnc
        self.assertIsNotNone(enc)
        init = fsm.init
        self.assertIsNotNone(init)