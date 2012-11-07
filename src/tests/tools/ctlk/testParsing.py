import unittest

from pyparsing import ParseException

from tools.ctlk.parsing import parseCTLK
from tools.ctlk.ast import (Spec, TrueExp, FalseExp, Init, Reachable,
                            Atom, Not, And, Or, Implies, Iff, 
                            AF, AG, AX, AU, AW, EF, EG, EX, EU, EW,
                            nK, nE, nD, nC, K, E, D, C)

class TestParsing(unittest.TestCase):
    
    def test_subclassing(self):
        self.assertTrue(issubclass(TrueExp, Spec))
        self.assertTrue(issubclass(FalseExp, Spec))
        self.assertTrue(issubclass(Init, Spec))
        self.assertTrue(issubclass(Reachable, Spec))
        self.assertTrue(issubclass(Atom, Spec))
        self.assertTrue(issubclass(Not, Spec))
        self.assertTrue(issubclass(And, Spec))
        self.assertTrue(issubclass(Or, Spec))
        self.assertTrue(issubclass(Implies, Spec))
        self.assertTrue(issubclass(Iff, Spec))
        self.assertTrue(issubclass(AF, Spec))
        self.assertTrue(issubclass(AG, Spec))
        self.assertTrue(issubclass(AX, Spec))
        self.assertTrue(issubclass(AU, Spec))
        self.assertTrue(issubclass(AW, Spec))
        self.assertTrue(issubclass(EF, Spec))
        self.assertTrue(issubclass(EG, Spec))
        self.assertTrue(issubclass(EX, Spec))
        self.assertTrue(issubclass(EU, Spec))
        self.assertTrue(issubclass(EW, Spec))
        self.assertTrue(issubclass(nK, Spec))
        self.assertTrue(issubclass(nE, Spec))
        self.assertTrue(issubclass(nD, Spec))
        self.assertTrue(issubclass(nC, Spec))
        self.assertTrue(issubclass(K, Spec))
        self.assertTrue(issubclass(E, Spec))
        self.assertTrue(issubclass(D, Spec))
        self.assertTrue(issubclass(C, Spec))
    
    def test_atom(self):
        s = "'c <= 3'"
        asts = parseCTLK(s)
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
                parseCTLK(s)
                
                
    def test_true(self):
        s = "True"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, TrueExp))
        
        
    def test_false(self):
        s = "False"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, FalseExp))
        
    
    def test_init(self):
        s = "Init"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, Init))
        
    
    def test_reachable(self):
        s = "Reachable"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, Reachable))
    
    
    def test_not(self):
        s = "~'a'"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, Not))
        self.assertTrue(isinstance(ast.child, Spec))
        self.assertTrue(isinstance(ast.child, Atom))
        self.assertEqual(ast.child.value, "a")
        
    
    def test_and(self):
        s = "'a' & ('b' & 'c')"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, And))
        self.assertTrue(isinstance(ast.left, Spec))
        self.assertTrue(isinstance(ast.left, Atom))
        self.assertEqual(ast.left.value, "a")
        self.assertTrue(isinstance(ast.right, Spec))
        self.assertTrue(isinstance(ast.right, And))
        self.assertTrue(isinstance(ast.right.left, Spec))
        self.assertTrue(isinstance(ast.right.left, Atom))
        self.assertEqual(ast.right.left.value, "b")
        self.assertTrue(isinstance(ast.right.right, Spec))
        self.assertTrue(isinstance(ast.right.right, Atom))
        self.assertEqual(ast.right.right.value, "c")
        
    
    def test_implies(self):
        s = "'a' -> 'b'"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, Implies))
        self.assertTrue(isinstance(ast.left, Spec))
        self.assertTrue(isinstance(ast.left, Atom))
        self.assertEqual(ast.left.value, "a")
        self.assertTrue(isinstance(ast.right, Spec))
        self.assertTrue(isinstance(ast.right, Atom))
        self.assertEqual(ast.right.value, "b")
        
        
    def test_iff(self):
        s = "'a' <-> 'b = 3'"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, Iff))
        self.assertTrue(isinstance(ast.left, Spec))
        self.assertTrue(isinstance(ast.left, Atom))
        self.assertEqual(ast.left.value, "a")
        self.assertTrue(isinstance(ast.right, Spec))
        self.assertTrue(isinstance(ast.right, Atom))
        self.assertEqual(ast.right.value, "b = 3")
        
        
    def test_logicals(self):
        s = "'a' & 'c' -> (~'b' | 'c')"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertTrue(isinstance(ast, Spec))
        self.assertTrue(isinstance(ast, Implies))
        self.assertTrue(isinstance(ast.left, Spec))
        self.assertTrue(isinstance(ast.left, And))
        self.assertTrue(isinstance(ast.left.left, Spec))
        self.assertTrue(isinstance(ast.left.left, Atom))
        self.assertEqual(ast.left.left.value, "a")
        self.assertTrue(isinstance(ast.left.right, Spec))
        self.assertTrue(isinstance(ast.left.right, Atom))
        self.assertEqual(ast.left.right.value, "c")
        self.assertTrue(isinstance(ast.right, Spec))
        self.assertTrue(isinstance(ast.right, Or))
        self.assertTrue(isinstance(ast.right.left, Spec))
        self.assertTrue(isinstance(ast.right.left, Not))
        self.assertTrue(isinstance(ast.right.left.child, Spec))
        self.assertTrue(isinstance(ast.right.left.child, Atom))
        self.assertEqual(ast.right.left.child.value, "b")
        self.assertTrue(isinstance(ast.right.right, Spec))
        self.assertTrue(isinstance(ast.right.right, Atom))
        self.assertEqual(ast.right.right.value, "c")
        
        
    def test_ex(self):
        s = "EX 'a'"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), EX)
        self.assertEqual(type(ast.child), Atom)
        self.assertEqual(ast.child.value, "a")
        
        
    def test_ex_and(self):
        s = "EX 'a' & 'b'"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), And)
        self.assertEqual(type(ast.left), EX)
        self.assertEqual(type(ast.left.child), Atom)
        self.assertEqual(ast.left.child.value, "a")
        self.assertEqual(type(ast.right), Atom)
        self.assertEqual(ast.right.value, "b")
        
    
    def test_eu(self):
        s = "E['r' & 'd' U 's' | EF 'q']"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), EU)
        self.assertEqual(type(ast.left), And)
        self.assertEqual(type(ast.left.left), Atom)
        self.assertEqual(ast.left.left.value, "r")
        self.assertEqual(type(ast.left.right), Atom)
        self.assertEqual(ast.left.right.value, "d")
        self.assertEqual(type(ast.right), Or)
        self.assertEqual(type(ast.right.left), Atom)
        self.assertEqual(ast.right.left.value, "s")
        self.assertEqual(type(ast.right.right), EF)
        self.assertEqual(type(ast.right.right.child), Atom)
        self.assertEqual(ast.right.right.child.value, "q")
        
        
    def test_eu_d(self):
        s = "E[D<'a'>'r' & 'd' U 's' | nD<'b','c'> 'q']"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), EU)
        self.assertEqual(type(ast.left), And)
        self.assertEqual(type(ast.left.left), D)
        self.assertEqual(type(ast.left.left.group), list)
        self.assertEqual(len(ast.left.left.group), 1)
        self.assertEqual(type(ast.left.left.group[0]), Atom)
        self.assertEqual(ast.left.left.group[0].value, "a")
        self.assertEqual(type(ast.left.right), Atom)
        self.assertEqual(ast.left.right.value, "d")        
        self.assertEqual(type(ast.right), Or)
        self.assertEqual(type(ast.right.left), Atom)
        self.assertEqual(ast.right.left.value, "s")
        self.assertEqual(type(ast.right.right), nD)
        self.assertEqual(type(ast.right.right.group), list)
        self.assertEqual(len(ast.right.right.group), 2)
        self.assertEqual(type(ast.right.right.group[0]), Atom)
        self.assertEqual(ast.right.right.group[0].value, "b")
        self.assertEqual(type(ast.right.right.group[1]), Atom)
        self.assertEqual(ast.right.right.group[1].value, "c")
        self.assertEqual(type(ast.right.right.child), Atom)
        self.assertEqual(ast.right.right.child.value, "q")
        
        
    def test_k(self):
        s = "K<'c'> 'a'"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), K)
        self.assertEqual(type(ast.agent), Atom)
        self.assertEqual(ast.agent.value, "c")
        self.assertEqual(type(ast.child), Atom)
        self.assertEqual(ast.child.value, "a")
        
    
    def test_nk_and(self):
        s = "nK<'a'> 'q' & 'r'"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), And)
        self.assertEqual(type(ast.left), nK)
        self.assertEqual(type(ast.left.agent), Atom)
        self.assertEqual(ast.left.agent.value, "a")
        self.assertEqual(type(ast.left.child), Atom)
        self.assertEqual(ast.left.child.value, "q")
        self.assertEqual(type(ast.right), Atom)
        self.assertEqual(ast.right.value, "r")
        
        
    def test_weird(self):
        s = """
            AF (
                (K<'c1'> (AF (K<'c1'> 'c2.payer' | K<'c1'> ~'c2.payer')))
                |
                (K<'c1'> (~AF (K<'c1'> 'c2.payer' | K<'c1'> ~'c2.payer')))
               )
            """
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), AF)
        self.assertEqual(type(ast.child), Or)
        self.assertEqual(type(ast.child.left), K)
        self.assertEqual(type(ast.child.left.agent), Atom)
        self.assertEqual(ast.child.left.agent.value, "c1")
        self.assertEqual(type(ast.child.left.child), AF)
        self.assertEqual(type(ast.child.left.child.child), Or)
        self.assertEqual(type(ast.child.left.child.child.left), K)
        self.assertEqual(type(ast.child.left.child.child.right), K)
        self.assertEqual(type(ast.child.left.child.child.right.agent), Atom)
        self.assertEqual(ast.child.left.child.child.right.agent.value, "c1")
        self.assertEqual(type(ast.child.left.child.child.right.child), Not)
        self.assertEqual(
                       type(ast.child.left.child.child.right.child.child), Atom)
        self.assertEqual(
                 ast.child.left.child.child.right.child.child.value, "c2.payer")
        self.assertEqual(type(ast.child.right), K)
        
    
    def test_epistemic_temporal_logical(self):
        s = "nK<'ag'> AX ~'b'"
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), nK)
        self.assertEqual(type(ast.agent), Atom)
        self.assertEqual(ast.agent.value, "ag")
        self.assertEqual(type(ast.child), AX)
        self.assertEqual(type(ast.child.child), Not)
        self.assertEqual(type(ast.child.child.child), Atom)
        self.assertEqual(ast.child.child.child.value, "b")
        
    
    def test_full(self):
        s = """
            A[
                K<'a'> EF Init
                &
                AG E<'a','b'> (True | 'r')
            U
                nC<'c','a'> E['s' W False -> AX 's']
            ] -> Reachable
            """
        asts = parseCTLK(s)
        self.assertEqual(len(asts), 1)
        
        ast = asts[0]
        self.assertEqual(type(ast), Implies)
        self.assertEqual(type(ast.left), AU)
        self.assertEqual(type(ast.left.left), And)
        
        # K<'a'> EF Init
        self.assertEqual(type(ast.left.left.left), K)
        self.assertEqual(type(ast.left.left.left.agent), Atom)
        self.assertEqual(ast.left.left.left.agent.value, "a")
        self.assertEqual(type(ast.left.left.left.child), EF)
        self.assertEqual(type(ast.left.left.left.child.child), Init)
        
        # AG E<'a','b'> (True | 'r')
        self.assertEqual(type(ast.left.left.right), AG)
        self.assertEqual(type(ast.left.left.right.child), E)
        self.assertEqual(type(ast.left.left.right.child.group), list)
        self.assertEqual(len(ast.left.left.right.child.group), 2)
        self.assertEqual(type(ast.left.left.right.child.group[0]), Atom)
        self.assertEqual(ast.left.left.right.child.group[0].value, "a")
        self.assertEqual(type(ast.left.left.right.child.group[1]), Atom)
        self.assertEqual(ast.left.left.right.child.group[1].value, "b")
        self.assertEqual(type(ast.left.left.right.child.child), Or)
        self.assertEqual(type(ast.left.left.right.child.child.left), TrueExp)
        self.assertEqual(type(ast.left.left.right.child.child.right), Atom)
        self.assertEqual(ast.left.left.right.child.child.right.value, "r")
                
        # nC<'c','a'> E['s' W False -> AX 's']
        self.assertEqual(type(ast.left.right), nC)
        self.assertEqual(type(ast.left.right.group), list)
        self.assertEqual(len(ast.left.right.group), 2)
        self.assertEqual(type(ast.left.right.group[0]), Atom)
        self.assertEqual(ast.left.right.group[0].value, "c")
        self.assertEqual(type(ast.left.right.group[1]), Atom)
        self.assertEqual(ast.left.right.group[1].value, "a")
        self.assertEqual(type(ast.left.right.child), EW)
        self.assertEqual(type(ast.left.right.child.left), Atom)
        self.assertEqual(ast.left.right.child.left.value, "s")
        self.assertEqual(type(ast.left.right.child.right), Implies)
        self.assertEqual(type(ast.left.right.child.right.left), FalseExp)
        self.assertEqual(type(ast.left.right.child.right.right), AX)
        self.assertEqual(type(ast.left.right.child.right.right.child), Atom)
        self.assertEqual(ast.left.right.child.right.right.child.value, "s")        
        
        self.assertEqual(type(ast.right), Reachable)