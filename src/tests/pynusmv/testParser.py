import unittest
import sys

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.node import node as nsnode

from pynusmv.prop import Spec
from pynusmv.prop import PropDb

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob
from pynusmv.parser import parse_next_expression, parse_ctl_spec
from pynusmv.node import find_hierarchy

from pynusmv.exception import NuSMVParsingError

class TestParser(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def show_types(self, node, indent=""):
        if node is not None:
            if node.type not in {nsparser.NUMBER, nsparser.ATOM}:
                print(indent + str(node.type))
                self.show_types(nsnode.car(node), indent + " ")
                self.show_types(nsnode.cdr(node), indent + " ")
            else:
                print(indent + str(node.type), ":",
                      nsnode.sprint_node(node))
        else:
            print(indent + "None")
    
    def test_run_checkctlspec(self):
        
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                           " tests/pynusmv/models/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        node, err = nsparser.ReadSimpExprFromString("admin = alice")
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
        
        propDb = glob.prop_database()
        self.assertTrue(len(propDb) >= 2)
        
        prop = propDb[1]
        spec = prop.exprcore.cdr # Ignoring NULL context
        self.assertEqual(spec.type, nsparser.AND)
        self.assertEqual(spec.car.type, nsparser.EF)
        self.assertEqual(spec.cdr.type, nsparser.EF)
    
    def test_contexted_expression(self):
        parsed = parse_next_expression("test")
        self.assertEqual(parsed.type, nsparser.ATOM)
    
    def test_equal_atoms(self):
        test = parse_next_expression("test")
        test = nsnode.find_node(test.type, nsnode.car(test), nsnode.cdr(test))
        same = parse_next_expression("test")
        same = nsnode.find_node(same.type, nsnode.car(same), nsnode.cdr(same))
        other = parse_next_expression("testx")
        other = nsnode.find_node(other.type, nsnode.car(other),
                                             nsnode.cdr(other))
        
        self.assertTrue(nsnode.node_equal(test, same) != 0)
        self.assertTrue(nsnode.node_equal(test, other) == 0)
    
    def test_equal_expressions(self):
        test = parse_next_expression("test.r")
        test = find_hierarchy(test)
        nsnode.free_node(test)
        same = parse_next_expression("test.r")
        same = find_hierarchy(same)
        nsnode.free_node(same)
        other = parse_next_expression("testx.rx")
        other = find_hierarchy(other)
        nsnode.free_node(other)
        
        self.assertTrue(nsnode.node_equal(test, same) != 0)
        self.assertTrue(nsnode.node_equal(test, other) == 0)
    
    def test_set(self):
        test = parse_next_expression("{a, b, c}")
        test = find_hierarchy(test)
        nsnode.free_node(test)
        
        print("UNION:", nsparser.UNION)
        print("ATOM:", nsparser.ATOM)
        self.show_types(test)
    
    def test_ctl(self):
        spec = parse_ctl_spec("AG EF x")
        self.assertIsNotNone(spec)
        
        with self.assertRaises(NuSMVParsingError):
            spec = parse_ctl_spec("A A")