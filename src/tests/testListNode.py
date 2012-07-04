import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser

from pynusmv.node.listnode import ListNode

class TestListNode(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
    
    
    def test_listnode(self):
        ln = ListNode.from_list([None, None, None])
        self.assertEqual(len(ln), 3)
        for n in ln:
            self.assertIsNone(n._ptr)