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
    
    
    def test_not(self):
        s = "~'a'"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), Not)
        self.assertEqual(type(ast.child), Atom)
        self.assertEqual(ast.child.value, "a")
        
    
    def test_and(self):
        s = "'a' & ('b' & 'c')"
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
        
    
    def test_implies(self):
        s = "'a' -> 'b'"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), Implies)
        self.assertEqual(type(ast.left), Atom)
        self.assertEqual(ast.left.value, "a")
        self.assertEqual(type(ast.right), Atom)
        self.assertEqual(ast.right.value, "b")
        
        
    def test_logicals(self):
        s = "'a' & 'c' -> (~'b' | 'c')"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), Implies)
        self.assertEqual(type(ast.left), And)
        self.assertEqual(type(ast.left.left), Atom)
        self.assertEqual(ast.left.left.value, "a")
        self.assertEqual(type(ast.left.right), Atom)
        self.assertEqual(ast.left.right.value, "c")
        self.assertEqual(type(ast.right), Or)
        self.assertEqual(type(ast.right.left), Not)
        self.assertEqual(type(ast.right.left.child), Atom)
        self.assertEqual(ast.right.left.child.value, "b")
        self.assertEqual(type(ast.right.right), Atom)
        self.assertEqual(ast.right.right.value, "c")
        
    
    def test_actions(self):
        s = "A<'a' & 'b' | 'c'>F 'b'"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), AaF)
        self.assertEqual(type(ast.action), Or)
        self.assertEqual(type(ast.action.left), And)
        self.assertEqual(type(ast.action.left.left), Atom)
        self.assertEqual(ast.action.left.left.value, "a")
        self.assertEqual(type(ast.action.left.right), Atom)
        self.assertEqual(ast.action.left.right.value, "b")
        self.assertEqual(type(ast.action.right), Atom)
        self.assertEqual(ast.action.right.value, "c")
        
        self.assertEqual(type(ast.child), Atom)
        self.assertEqual(ast.child.value, "b")
        
        
    def test_x(self):
        s = "E<'a'>X (A<'b'> X 'c' & E<'f'>X E<'g'>X 'h')"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), EaX)
        self.assertEqual(type(ast.action), Atom)
        self.assertEqual(ast.action.value, "a")
        self.assertEqual(type(ast.child), And)
        self.assertEqual(type(ast.child.left), AaX)
        self.assertEqual(type(ast.child.left.action), Atom)
        self.assertEqual(ast.child.left.action.value, "b")
        self.assertEqual(type(ast.child.left.child), Atom)
        self.assertEqual(ast.child.left.child.value, "c")
        self.assertEqual(type(ast.child.right), EaX)
        self.assertEqual(type(ast.child.right.action), Atom)
        self.assertEqual(ast.child.right.action.value, "f")
        self.assertEqual(type(ast.child.right.child), EaX)
        self.assertEqual(type(ast.child.right.child.action), Atom)
        self.assertEqual(ast.child.right.child.action.value, "g")
        self.assertEqual(type(ast.child.right.child.child), Atom)
        self.assertEqual(ast.child.right.child.child.value, "h")
        
        
    def test_ef_and(self):
        s = "E<'a'>F 'admin = alice' & E<'b'>F 'admin = bob'"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), And)
        self.assertEqual(type(ast.left), EaF)
        self.assertEqual(type(ast.left.action), Atom)
        self.assertEqual(ast.left.action.value, "a")
        self.assertEqual(type(ast.left.child), Atom)
        self.assertEqual(ast.left.child.value, "admin = alice")
        self.assertEqual(type(ast.right), EaF)
        self.assertEqual(type(ast.right.action), Atom)
        self.assertEqual(ast.right.action.value, "b")
        self.assertEqual(type(ast.right.child), Atom)
        self.assertEqual(ast.right.child.value, "admin = bob")
        
        
    def test_not_eax(self):
        s = "E<'a'>X ~ 'c'"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), EaX)
        self.assertEqual(type(ast.action), Atom)
        self.assertEqual(ast.action.value, "a")
        self.assertEqual(type(ast.child), Not)
        self.assertEqual(type(ast.child.child), Atom)
        self.assertEqual(ast.child.child.value, "c")
        
        
    def test_afnext(self):
        s = "A<'TRUE'>F (~ E<'TRUE'>X 'TRUE')"
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), AaF)
        self.assertEqual(type(ast.action), Atom)
        self.assertEqual(ast.action.value, "TRUE")
        self.assertEqual(type(ast.child), Not)
        self.assertEqual(type(ast.child.child), EaX)
        self.assertEqual(type(ast.child.child.action), Atom)
        self.assertEqual(ast.child.child.action.value, "TRUE")
        self.assertEqual(type(ast.child.child.child), Atom)
        self.assertEqual(ast.child.child.child.value, "TRUE")
        
    
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
      
        
    def test_full2(self):
        s = ("A<'past' | 'present'>G (E<'true'>F ('future is now' & 'later')"
             "<-> A<'min'>['past' U 'present' -> 'future'])")
        asts = parseArctl(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), AaG)
        self.assertEqual(type(ast.action), Or)
        self.assertEqual(type(ast.action.left), Atom)
        self.assertEqual(ast.action.left.value, "past")
        self.assertEqual(type(ast.action.right), Atom)
        self.assertEqual(ast.action.right.value, "present")
        self.assertEqual(type(ast.child), Iff)
        self.assertEqual(type(ast.child.left), EaF)
        self.assertEqual(type(ast.child.left.action), Atom)
        self.assertEqual(ast.child.left.action.value, "true")
        self.assertEqual(type(ast.child.left.child), And)
        self.assertEqual(type(ast.child.left.child.left), Atom)
        self.assertEqual(ast.child.left.child.left.value, "future is now")
        self.assertEqual(type(ast.child.left.child.right), Atom)
        self.assertEqual(ast.child.left.child.right.value, "later")
        self.assertEqual(type(ast.child.right), AaU)
        self.assertEqual(type(ast.child.right.action), Atom)
        self.assertEqual(ast.child.right.action.value, "min")
        self.assertEqual(type(ast.child.right.left), Atom)
        self.assertEqual(ast.child.right.left.value, "past")
        self.assertEqual(type(ast.child.right.right), Implies)
        self.assertEqual(type(ast.child.right.right.left), Atom)
        self.assertEqual(ast.child.right.right.left.value, "present")
        self.assertEqual(type(ast.child.right.right.right), Atom)
        self.assertEqual(ast.child.right.right.right.value, "future")