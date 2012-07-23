import unittest

from tools.arctl.parsing.arctl import (arctl, Atom, A, G)

class TestArctl(unittest.TestCase):
    
    def test_atom(self):
        s = "'c <= 3'"
        asts = arctl.parseString(s, parseAll = True)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(type(ast), Atom)
        self.assertEqual(ast.value, "c <= 3")
        
    
    
    
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