import unittest

from ..nusmv.cinit import cinit
from ..nusmv.cmd import cmd
from ..nusmv.node import node as nsnode
from ..nusmv.parser import parser
from ..nusmv.prop import prop
from ..nusmv.fsm.bdd import bdd as nsfsm

from ..fsm.fsm import BddFsm
from ..node.node import Node

class TestBuildModel(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
    
    
    def test_build_model(self):
        # Initialize the model
        ret = cmd.Cmd_CommandExecute("read_model -i pynusmv/tests/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_CommandExecute("go")
        self.assertEqual(ret, 0)
        
        propDb = prop.PropPkg_get_prop_database()
        fsm_ptr = prop.PropDb_master_get_bdd_fsm(propDb)
        self.assertIsNotNone(fsm_ptr)
        fsm = BddFsm(fsm_ptr)
        enc = fsm.BddEnc
        self.assertIsNotNone(enc)
        init = fsm.init
        self.assertIsNotNone(init)