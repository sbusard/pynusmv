import unittest
import pyparsing

from pynusmv import parser
from pynusmv.model import (Expression, Identifier, Self, Dot, ArrayAccess,
                           ArrayExpr)

class TestParsing(unittest.TestCase):
    
    
    def test_identifier(self):
        identifiers = ["c", "L", "_",
                       "ab", "BB", "aB",
                       "_a", "_$#-", "b_bc",
                       "_q$", "r02_1", "r-"
                      ]
                  
        for identifier in identifiers:
            parsed = parser.parseAllString(parser.identifier, identifier)
            self.assertEqual(str(parsed), identifier)
            self.assertEqual(parsed, parser.Identifier(identifier))
        
        noidentifiers = ["$", "-", "#", "1",
                         "$sqfd", "#qsdf", "-091a",
                         "000", "1aze",
                         "()", "", "/",
                         "(qsdfqsdf", ";s;", ""
                        ]
        
        for noidentifier in noidentifiers:
            with self.assertRaises(pyparsing.ParseException):
                parser.parseAllString(parser.identifier, noidentifier)
    
    def test_complex_identifier(self):
        identifiers = ["c", "self",
                       "a.b", "a.self.b", "self.c",
                       "a[b][c]",
                       #"module[counter][value].rest[shift].self[c]"
                      ]
        expected = [Identifier("c"), Self(),
                    Dot(Identifier("a"), Identifier("b")),
                    Dot(Identifier("a"), Dot(Self(), Identifier("b"))),
                    Dot(Self(), Identifier("c")),
                    ArrayAccess(ArrayAccess(Identifier("a"), Identifier("b")),
                          Identifier("c")),
                    Dot(ArrayAccess(ArrayAccess(Identifier("module"),
                                        Identifier("counter")),
                                  Identifier("value")),
                            Dot(ArrayAccess(Identifier("rest"),
                                          Identifier("shift")),
                                    ArrayAccess(Self(), Identifier("c"))))]
                  
        for i in range(len(identifiers)):
            try:
                res = parser.parseAllString(parser.complex_identifier,
                                             identifiers[i])
                #print(identifiers[i], "=>", res)
                self.assertIsNotNone(res)
                self.assertTrue(isinstance(res, Expression))
                self.assertEqual(res, expected[i])
            except pyparsing.ParseException as e:
                print(e.line)
                print(" " * (e.column-1) + "^")
                raise e
        
        noidentifiers = ["$", "-", "#", "1",
                         "$sqfd", "#qsdf", "-091a",
                         "000", "1aze",
                         "()", "", "/",
                         "(qsdfqsdf", ";s;",
                         "[test]", "", ".c"
                        ]
        
        for noidentifier in noidentifiers:
            with self.assertRaises(pyparsing.ParseException):
                parser.parseAllString(parser.complex_identifier,
                                       noidentifier)
                                                        
    def test_constants(self):
        constants = ["TRUE", "FALSE",
                     "3", "-3", "190", "-132091309",
                     "test", "sqdfmoi", "true", 
                     "0b_459a_23fe", "0uB_23", "0o43_234"
                    ]
                  
        for constant in constants:
            res = parser.parseAllString(parser.constant, constant)
            #print(constant, "=>", res)
            self.assertIsNotNone(res)
        
        noconstants = ["30-32340",
                       "0o-42_234", "",
                       "TRUEN",
                      ]
        
        for noconstant in noconstants:
            with self.assertRaises(pyparsing.ParseException):
                parser.parseAllString(parser.constant, noconstant)
    
    
    def test_simple_expr(self):
        exprs = ["3", "TRUE", "0b_459a_23fe", "{test}",
                 "{test}", "{a, b, c}",
                 "word1(unsigned(b))", "extend(next(i), (a))",
                 "test[3]", "ar[3:5]", "res[-4:8]", "b[1][3:5]",
                 "!test", "!word1(r)",
                 "!c::r", "a::b::c",
                 "-r.c", "- -8",
                 "a*b", "a * c.r / z",
                 "a+ b + c", "a*b + c*d", "(a+b) * c", "a - b",
                 "a << b", "c >> d", "3 << 4 << 5 >> 6",
                 "{r} union {z} union 3",
                 "z in {r}", "r[3] in rr",
                 "a = b", "a != c < r", "z >= r <= sdf",
                 "a & b+c", "r[3] & r[4]",
                 "a | b&c", "d xor f xnor z",
                 "a ? b : c", "a | b ? c : d", "a ? b | c : d",
                 "a ? b : c | d", "a ? b ? c : d : e",
                 "a[z] ? -r.c : z",
                 "a <-> b", "z <-> c <-> r",
                 "a -> b", "a -> b -> c",
                 "case a: b; c: d; esac",
                 "case a : case b: c; esac; b: d; esac",
                 "case step = 1  : {keep, swap}; step != 1 : {none}; esac",
                 "step = 1",
                 "a.b", "c[d]", "c[d].e",
                ]
                  
        for expr in exprs:
            try:
                res = parser.parseAllString(parser.simple_expression, expr)
                #print(expr, "=>", res, "(" + str(type(res)) + ")")
                self.assertIsNotNone(res)
            except pyparsing.ParseException as e:
                print(e.line)
                print(" " * (e.column-1) + "^")
                raise e
        
        noexprs = ["3..b", "0r_459a_23fe", "b..3",
                 "{test,}", "{}",
                 "worde1(unsigned(b))", "extend(next(i))",
                 "test[3]]" "res[-4:::8]", "[1][3:5]", "[q]", "q[r",
                 "!te!st", "!word!1(r)",
                 "!c:r", "a:b:c", "a::", "::b",
                 "-8-",
                 "a**b",
                 "a+ b + ", "*b + c*d","a -",
                 ">> d",
                 "{r} union {z} union", "{r union} union b"
                 "z in", "r[3] i rr",
                 "a ? bc", "ac : d", "a ? b ? c d : e",
                 "a <->", "z <-> <-> r",
                 "-> b", "a -> -> c",
                 "case a: b; c: d esac",
                 "case a :; b: d; esac"
                  ]
        
        for noexpr in noexprs:
            with self.assertRaises(pyparsing.ParseException):
                parser.parseAllString(parser.simple_expression, noexpr)
    
    def test_type(self):
        types = ["boolean",
                 "word[3]", "word[c+1]",
                 "unsigned word[r]", "signed word[t[4]]",
                 "{a, b, c, 3, 6, 5}",
                 "r..d","3..4", "-5..c",
                 "array 1..2 of boolean",
                 "array -3..4 of boolean",
                 "process counter(t, r, s)",
                 "test(c+1)",
                 "main",
                ]
                  
        for t in types:
            try:
                res = parser.parseAllString(parser.type_identifier, t)
                #print(t, "=>", res)
                self.assertIsNotNone(res)
            except pyparsing.ParseException as e:
                print(e)
                print(e.line)
                print(" " * (e.column-1) + "^")
                raise e
        
        notypes = ["word[]", "un word[r]",
                   "{}", "{", "{1, 2, 3, r+2}", "{a, b, c", "c, b, d}",
                   "r..d in b", "a.b",
                   "array of word[5]", "array 3..4", "array 5..6 of"
                  ]
        
        for notype in notypes:
            with self.assertRaises(pyparsing.ParseException):
                parser.parseAllString(parser.type_identifier, notype)
    
    def test_define(self):
        code = "DEFINE b := [[a], [b], [c, d]];"
        b = Identifier("b")
        define = parser.parseAllString(parser.define_section, code)
        self.assertEqual(type(define.body[b]), ArrayExpr)
    
    def test_var_section(self):
        code = "VAR c1 : 0..2; c2 : counter(t);"
        section = parser.parseAllString(parser.var_section, code)
        
        self.assertEqual(str(section.body[parser.Identifier("c1")]), "0 .. 2")
    
    def test_trans_section(self):
        code = "TRANS next(c1) = run ? c1+1 mod 2 : 0"
        section = parser.parseAllString(parser.trans_constraint, code)
        
        self.assertEqual(len(section.body), 1)
        self.assertEqual(str(section.body[0]),
                         "next(c1) = run ? c1 + 1 mod 2 : 0")
    
    def test_module(self):
        module = """
MODULE counter(run)
    VAR
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0 & c2 = 0
    TRANS next(c1) = run ? c1+1 mod 2 : 0
    TRANS next(c2) = run ? c2+1 mod 2 : 0
        """
        
        parsed = parser.parseAllString(parser.module, module)
        
        self.assertEqual(str(parsed), module)
    
    def test_module_without_args(self):
        module = """
MODULE counter()
    VAR
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0 & c2 = 0
    TRANS next(c1) = run ? c1+1 mod 2 : 0
    TRANS next(c2) = run ? c2+1 mod 2 : 0
        """
        
        parsed = parser.parseAllString(parser.module, module)
        
        self.assertEqual(str(parsed), module)
    
    def test_module_with_several_args(self):
        module = """
MODULE counter(a, b, c)
    VAR
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0 & c2 = 0
    TRANS next(c1) = run ? c1+1 mod 2 : 0
    TRANS next(c2) = run ? c2+1 mod 2 : 0
        """
        
        parsed = parser.parseAllString(parser.module, module)
        
        self.assertEqual(str(parsed), module)