import unittest

from pyparsing import ParseException

from tools.arctl.parsing import arctl
from tools.arctl.ast import Atom, A, G, And, E, U, W

class TestParsing(unittest.TestCase):
    
    def test_atom(self):
        s = "'c <= 3'"
        asts = arctl.parseString(s, parseAll = True)
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
                arctl.parseString(s, parseAll = True)
                
                
    def test_and(self):
        s = "('a' & ('b' & 'c'))"
        asts = arctl.parseString(s, parseAll = True)
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
        asts = arctl.parseString(s, parseAll = True)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), E)
        self.assertEqual(type(ast.action), Atom)
        self.assertEqual(ast.action.value, "ac")
        self.assertEqual(type(ast.path), U)
        self.assertEqual(type(ast.path.left), A)
        self.assertEqual(type(ast.path.left.action), Atom)
        self.assertEqual(ast.path.left.action.value, "ac")
        self.assertEqual(type(ast.path.left.path), W)
        self.assertEqual(type(ast.path.left.path.right), Atom)
        self.assertEqual(ast.path.left.path.right.value, "d")
    
    
    def test_full(self):
        s = ("A<'past'>G (E<'true'>F 'future is now'"
             "<-> A<'min'>['past' U 'present'])")
        asts = arctl.parseString(s, parseAll = True)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), A)
        self.assertEqual(type(ast.action), Atom)
        self.assertEqual(ast.action.value, "past")
        self.assertEqual(type(ast.path), G)