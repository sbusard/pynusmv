import unittest

from ..nusmv.cinit import cinit
from ..nusmv.cmd import cmd
from ..nusmv.node import node as nsnode
from ..nusmv.parser import parser

from ..node.node import Node

class TestRun(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
    
    
    def test_new_node_from_nusmv(self):
        n = Node(nsnode.find_node(parser.TRUEEXP, None, None))
        self.assertIsNotNone(n)
        self.assertEqual(n.type, parser.TRUEEXP)
        self.assertIsNone(n.car)
        self.assertIsNone(n.cdr)
        
    
    def test_new_node_with_Node_find_node(self):
        n = Node.find_node(parser.FALSEEXP)
        self.assertIsNotNone(n)
        self.assertEqual(n.type, parser.FALSEEXP)
        self.assertIsNone(n.car)
        self.assertIsNone(n.cdr) 
        

    def test_nested_nodes(self):
        n1 = Node.find_node(parser.TRUEEXP)
        self.assertIsNotNone(n1)
        self.assertEqual(n1.type, parser.TRUEEXP)
        n2 = Node.find_node(parser.FALSEEXP)
        self.assertIsNotNone(n2)
        self.assertEqual(n2.type, parser.FALSEEXP)
        n = Node.find_node(parser.OR, n1, n2)
        self.assertIsNotNone(n)
        self.assertIsNotNone(n.car)
        self.assertIsNotNone(n.cdr)
        self.assertEqual(n.type, parser.OR)
        self.assertEqual(n.car.type, parser.TRUEEXP)
        self.assertEqual(n.cdr.type, parser.FALSEEXP)