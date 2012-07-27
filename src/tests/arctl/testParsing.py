import unittest

from pyparsing import ParseException

from tools.arctl.parsing import parseArctl
from tools.arctl.ast import (Atom, Not, And, Or, Implies, Iff, 
                             AaF, AaG, AaX, AaU, AaW,
                             EaF, EaG, EaX, EaU, EaW)

class TestParsing(unittest.TestCase):
    
    def test_atom(self):
        s = "'c <= 3'"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), Atom)
        self.assertEqual(ast.value, "c <= 3")
        
        
    def test_fail(self):
        kos = ["", "'test", "A<'test'>H 'q'", "O", "A<'a'>E<'e'>G 'true'",
               "E<'ac'>[A<'ac'>['a' E<'ac'>['b' W 'c'] W 'd'] "
               "U A<'ac'>['e' U 'f']]", "A<'t't'>G 'q'"]
        
        for s in kos:
            with self.assertRaises(ParseException):
                parseArctl(s)
                
                
    def test_and(self):
        s = "('a' & ('b' & 'c'))"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), And)
        self.assertEqual(type(ast.left), Atom)
        self.assertEqual(ast.left.value, "a")
        self.assertEqual(type(ast.right), And)
        self.assertEqual(type(ast.right.left), Atom)
        self.assertEqual(ast.right.left.value, "b")
        self.assertEqual(type(ast.right.right), Atom)
        self.assertEqual(ast.right.right.value, "c")
        
    
    def test_eauw(self):
        s = "E<'ac'>[A<'ac'>[E<'ac'>['b' W 'c'] W 'd'] U A<'ac'>['e' U 'f']]"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), EaU)
        self.assertEqual(type(ast.action), Atom)
        self.assertEqual(ast.action.value, "ac")
        self.assertEqual(type(ast.left), AaW)
        self.assertEqual(type(ast.left.action), Atom)
        self.assertEqual(ast.left.action.value, "ac")
        self.assertEqual(type(ast.left.right), Atom)
        self.assertEqual(ast.left.right.value, "d")
    
    
    def test_full(self):
        s = ("A<'past'>G (E<'true'>F 'future is now'"
             "<-> A<'min'>['past' U 'present'])")
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), AaG)
        self.assertEqual(type(ast.action), Atom)
        self.assertEqual(ast.action.value, "past")
        self.assertEqual(type(ast.child), Iff)
        self.assertEqual(type(ast.child.left), EaF)
        self.assertEqual(type(ast.child.left.action), Atom)
        self.assertEqual(ast.child.left.action.value, "true")
        self.assertEqual(type(ast.child.left.child), Atom)
        self.assertEqual(ast.child.left.child.value, "future is now")
        self.assertEqual(type(ast.child.right), AaU)