import unittest
import sys

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.parser import parser
from pynusmv.nusmv.node import node as nsnode

from pynusmv.node.node import Node

class TestParser(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
        
    def tearDown(self):    
        cinit.NuSMVCore_quit()
    
    def test_run_checkctlspec(self):
        
        ret = cmd.Cmd_SecureCommandExecute("read_model -i tests/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        node, err = parser.ReadSimpExprFromString("admin = alice")
        self.assertIsNotNone(node)
        node = nsnode.car(node)
        self.assertIsNotNone(node)
        node = Node(node)
        self.assertIsNotNone(node)
        