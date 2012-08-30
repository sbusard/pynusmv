import unittest
import sys

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.parser import parser
from pynusmv.nusmv.node import node as nsnode

from pynusmv.spec.spec import Spec
from pynusmv.prop.propDb import PropDb

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.fsm.globals import Globals

class TestParser(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
    
    def test_run_checkctlspec(self):
        
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                           " tests/pynusmv/models/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        node, err = parser.ReadSimpExprFromString("admin = alice")
        self.assertIsNotNone(node)
        node = nsnode.car(node)
        self.assertIsNotNone(node)
        node = Spec(node)
        self.assertIsNotNone(node)
        
        
    def test_precedences(self):
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                           " tests/pynusmv/models/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        propDb = Globals.prop_database()
        self.assertTrue(len(propDb) >= 2)
        
        prop = propDb[1]
        spec = prop.exprcore.cdr # Ignoring NULL context
        self.assertEqual(spec.type, parser.AND)
        self.assertEqual(spec.car.type, parser.EF)
        self.assertEqual(spec.cdr.type, parser.EF)
        
        