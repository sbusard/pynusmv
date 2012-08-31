import unittest
import sys

from pynusmv.nusmv.node import node as nsnode

from pynusmv.parser.parser import *
from pynusmv.utils.exception import NuSMVParsingError
from pynusmv.init.init import init_nusmv, deinit_nusmv

class TestPyNuSMVParsing(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
    
    def test_good_simple_expressions(self):
        exprs = ["a.car = 3",
                 "e <= 56",
                 "a[4] & t[v]",
                 "inn | outt -> TRUE",
                 "AA + B = C - D",
                 "a -- b = c" # OK because b = c is commented so the expr is 'a'
                ]
        
        for expr in exprs:
            node = parse_simple_expression(expr)
            self.assertIsNotNone(node)
            self.assertIsNotNone(nsnode.sprint_node(node))
            
            
    def test_bad_simple_expressions(self):
        exprs = ["a =",
                 "e <=> 56",
                 "--",
                 "a[4] % t[v]", # The lexer prints an error message at stderr
                                # cannot be avoided
                 "in | outt -> TRUE",
                 "A + B = C - D",
                 "b + 1 -",
                 "next(i) = 3",
                 "init(d) + 5",
                 "a := b"
                ]
        
        for expr in exprs:
            with self.assertRaises(NuSMVParsingError):
                node = parse_simple_expression(expr)
                
                
    def test_good_next_expressions(self):
        exprs = ["next(a) = a",
                 "e.ne <= next(53)",
                 "a[4] & t[next(v)]",
                 "inn | next(outt) -> TRUE",
                 "AA + B = next(C) - D",
                 "a -- b = init(c) % 5" # OK because b = init(c) % 5
                                        # is commented so the expr is 'a'
                ]
        
        for expr in exprs:
            node = parse_next_expression(expr)
            self.assertIsNotNone(node)
            self.assertIsNotNone(nsnode.sprint_node(node))
            
            
    def test_bad_next_expressions(self):
        exprs = ["a =",
                 "next(e) <=> 56",
                 "next()--",
                 "init(d.c.a) + 3",
                 "a[4] % next(t)[v]", # The lexer prints an error message at stderr
                                # cannot be avoided
                 "in | outt -> TRUE",
                 "A + B = C - D",
                 "b + next(1) -",
                 "a := b"
                ]
        
        for expr in exprs:
            with self.assertRaises(NuSMVParsingError):
                node = parse_next_expression(expr)
                
                
                
    def test_good_identifiers(self):
        exprs = ["a",
                 "a[4]",
                 "t[v.r]",
                 "inn",
                 "AA",
                 "a.b"
                ]
        
        for expr in exprs:
            node = parse_identifier(expr)
            self.assertIsNotNone(node)
            self.assertIsNotNone(nsnode.sprint_node(node))
            
            
    def test_bad_identifiers(self):
        exprs = ["a =",
                 "e <=> 56",
                 "--",
                 "a[4] % t[v]", # The lexer prints an error message at stderr
                                # cannot be avoided
                 "in | outt -> TRUE",
                 "A + B = C - D",
                 "b + 1 -",
                 "next(i) = 3",
                 "a := b",
                 "54",
                 "0x42"
                ]
        
        for expr in exprs:
            with self.assertRaises(NuSMVParsingError):
                node = parse_identifier(expr)