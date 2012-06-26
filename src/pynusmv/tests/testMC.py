import unittest

from ..nusmv.cinit import cinit
from ..nusmv.cmd import cmd
from ..nusmv.node import node as nsnode
from ..nusmv.parser import parser
from ..nusmv.prop import prop
from ..nusmv.fsm.bdd import bdd as nsfsm
from ..nusmv.mc import mc

from ..fsm.fsm import BddFsm
from ..node.node import Node

class TestMC(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
    
    
    def test_mc(self):
        # Initialize the model
        ret = cmd.Cmd_CommandExecute("read_model -i pynusmv/tests/admin.smv")
        self.assertEqual(ret, 0)
        cmd.Cmd_CommandExecute("go")
        self.assertEqual(ret, 0)
        
        # Check CTL specs
        propDb = prop.PropPkg_get_prop_database()
        for i in range(prop.PropDb_get_size(propDb)):
            p = prop.PropDb_get_prop_at_index(propDb, i)
            if prop.Prop_get_type(p) == prop.Prop_Ctl:
                mc.Mc_CheckCTLSpec(p)