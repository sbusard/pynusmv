import unittest

from pyparsing import ParseException

from tools.atlkFO.parsing import parseATLK
from tools.atlkFO.ast import (Spec, TrueExp, FalseExp, Init, Reachable,
                            Atom, Not, And, Or, Implies, Iff, 
                            AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                            nK, nE, nD, nC, K, E, D, C,
                            CAF, CAG, CAX, CAU, CAW, CEF, CEG, CEX, CEU, CEW)


class TestParsing(unittest.TestCase):
        
    def test_atom(self):
        s = "'c <= 3'"
        asts = parseATLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, Atom))
        self.assertEqual(ast.value, "c <= 3")
        
    
    def test_fail(self):
        kos = ["", "'test", "A<'test'>H 'q'", "O", "A<'a'>E<'e'>G 'true'",
               "E<'ac'>[A<'ac'>['a' E<'ac'>['b' W 'c'] W 'd'] "
               "U A<'ac'>['e' U 'f']]", "A<'t't'>G 'q'"]
        
        for s in kos:
            with self.assertRaises(ParseException):
                parseATLK(s)
                
                
    def test_CEF(self):
        s = "<'agent'>F 'test'"
        asts = parseATLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, CEF))
        for ag in ast.group:
            self.assertTrue(isinstance(ag, Atom))
        self.assertEqual(len(ast.group), 1)
        self.assertEqual(ast.group[0].value, 'agent')
        self.assertTrue(isinstance(ast.child, Atom))
        self.assertEqual(ast.child.value, "test")
        
        
    def test_deep_spec(self):
        s = ("<'a','b'>[['c'][['e'] F ('p' & 'q') W K<'a'> 'p'] U <'d'>G "
             "nC<'g','e'> ('c' -> EF 'k')]")
        asts = parseATLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, CEU))
        
        self.assertEqual(len(ast.group), 2)
        self.assertTrue(isinstance(ast.group[0], Atom))
        self.assertEqual(ast.group[0].value, "a")
        self.assertTrue(isinstance(ast.group[1], Atom))
        self.assertEqual(ast.group[1].value, "b")
        
        self.assertTrue(isinstance(ast.left, CAW))
        self.assertEqual(len(ast.left.group), 1)
        self.assertTrue(isinstance(ast.left.group[0], Atom))
        self.assertEqual(ast.left.group[0].value, "c")
        self.assertTrue(isinstance(ast.left.left, CAF))
        self.assertEqual(len(ast.left.left.group), 1)
        self.assertTrue(isinstance(ast.left.left.group[0], Atom))
        self.assertEqual(ast.left.left.group[0].value, "e")
        self.assertTrue(isinstance(ast.left.left.child, And))
        self.assertTrue(isinstance(ast.left.left.child.left, Atom))
        self.assertEqual(ast.left.left.child.left.value, "p")
        self.assertTrue(isinstance(ast.left.left.child.right, Atom))
        self.assertEqual(ast.left.left.child.right.value, "q")
        self.assertTrue(isinstance(ast.left.right, K))
        self.assertTrue(isinstance(ast.left.right.agent, Atom))
        self.assertEqual(ast.left.right.agent.value, "a")
        self.assertTrue(isinstance(ast.left.right.child, Atom))
        self.assertEqual(ast.left.right.child.value, "p")
        
        self.assertTrue(isinstance(ast.right, CEG))
        self.assertEqual(len(ast.right.group), 1)
        self.assertTrue(isinstance(ast.right.group[0], Atom))
        self.assertEqual(ast.right.group[0].value, "d")
        self.assertTrue(isinstance(ast.right.child, nC))
        self.assertEqual(len(ast.right.child.group), 2)
        self.assertTrue(isinstance(ast.right.child.group[0], Atom))
        self.assertEqual(ast.right.child.group[0].value, "g")
        self.assertTrue(isinstance(ast.right.child.group[1], Atom))
        self.assertEqual(ast.right.child.group[1].value, "e")
        self.assertTrue(isinstance(ast.right.child.child, Implies))
        self.assertTrue(isinstance(ast.right.child.child.left, Atom))
        self.assertEqual(ast.right.child.child.left.value, "c")
        self.assertTrue(isinstance(ast.right.child.child.right, EF))
        self.assertTrue(isinstance(ast.right.child.child.right.child, Atom))
        self.assertEqual(ast.right.child.child.right.child.value, "k")