import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser
from pynusmv.nusmv.prop import prop
from pynusmv.nusmv.fsm.bdd import bdd as nsfsm
from pynusmv.nusmv.mc import mc

from pynusmv.fsm.fsm import BddFsm

from pynusmv.init.init import init_nusmv, deinit_nusmv

class TestMC(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
    
    
    def test_mc(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                           " tests/pynusmv/models/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        # Check CTL specs
        propDb = prop.PropPkg_get_prop_database()
        for i in range(prop.PropDb_get_size(propDb)):
            p = prop.PropDb_get_prop_at_index(propDb, i)
            if prop.Prop_get_type(p) == prop.Prop_Ctl:
                mc.Mc_CheckCTLSpec(p)