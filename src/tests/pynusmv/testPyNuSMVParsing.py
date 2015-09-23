import unittest
import sys

from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser as nsparser

from pynusmv.parser import *
from pynusmv.exception import NuSMVParsingError
from pynusmv.init import init_nusmv, deinit_nusmv

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
                
                
    def test_trans_in_context(self):
        car = nsnode.car
        cdr = nsnode.cdr
        
        orig = "(next(c) = c) IN c1"
        
        trans = "next(c) = c"
        context = "c1"
        
        origp = parse_next_expression(orig)
        
        transp = parse_next_expression(trans)
        contextp = parse_identifier(context)
        # contextp is ATOM. We want DOT(None, ATOM)
        contextp = nsnode.find_node(nsparser.DOT, None, contextp)
        
        fullp = nsnode.find_node(nsparser.CONTEXT, contextp, transp)
        
        self.checkNodeTypeEqual(origp, fullp)
        
        
    def checkNodeTypeEqual(self, left, right):
        car = nsnode.car
        cdr = nsnode.cdr
        
        if left is None:
            self.assertEqual(left, right)
        else:
            if left.type not in {
                nsparser.NUMBER_SIGNED_WORD,
                nsparser.NUMBER_UNSIGNED_WORD,
                nsparser.NUMBER_FRAC,
                nsparser.NUMBER_EXP,
                nsparser.NUMBER_REAL,
                nsparser.NUMBER,
                nsparser.ATOM,
                nsparser.FAILURE
            }:
                self.assertEqual(left.type, right.type)
                self.checkNodeTypeEqual(car(left), car(right))
                self.checkNodeTypeEqual(cdr(left), cdr(right))
        
        
        