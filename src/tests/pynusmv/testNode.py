import unittest

from pynusmv.node import find_hierarchy
from pynusmv.parser import parse_next_expression
from pynusmv.init import init_nusmv, deinit_nusmv

class TestNode(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()