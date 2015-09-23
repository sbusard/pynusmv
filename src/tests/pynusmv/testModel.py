import unittest
import collections

from pynusmv import model, parser, node
from pynusmv.init import init_nusmv, deinit_nusmv

class TestModel(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    def test_create_atom(self):
        name = "test"
        identifier = node.Atom(name)
        self.assertEqual(identifier.name, name)
        self.assertEqual(str(identifier), name)
    
    def test_hashable_atom(self):
        name = "test"
        atom = node.Atom(name)
        self.assertTrue(isinstance(atom, collections.Hashable))
    
    def test_create_number(self):
        value = 3
        number = node.Number(value)
        self.assertEqual(number.value, value)
        self.assertEqual(str(number), str(value))
    
    def test_create_word(self):
        value = "0sb4_0111"
        sword = node.NumberSignedWord(value)
        self.assertEqual(sword.value, value)
        self.assertEqual(str(sword), "0sd4_7")
        
        value = "0ub4_1111"
        uword = node.NumberUnsignedWord(value)
        self.assertEqual(uword.value, value)
        self.assertEqual(str(uword), "0ud4_15")
    
    def test_expression_from_pointer(self):
        expr = "test"
        ptr = parser.parse_next_expression(expr)
        expr_node = node.Node.from_ptr(ptr)
        self.assertEqual(str(expr_node), expr)
    
    def test_equal_atoms(self):
        name = "test"
        name2 = "test2"
        a1 = node.Atom(name)
        a2 = node.Atom(name)
        a3 = node.Atom(name2)
        
        self.assertEqual(a1, a2)
        self.assertNotEqual(a1, a3)
    
    def test_getattr_expr(self):
        mod = node.Atom("mod")
        self.assertEqual(type(mod.act), node.Dot)
        self.assertEqual(type(mod.name), str)
        self.assertEqual(mod.name, "mod")
    
    def test_declaration(self):
        c = node.DVar(node.Range(0, 2))
        expr = (c + 1) == 3
        self.assertNotEqual(str(expr), "c + 1 = 3")
        c.name = "c"
        self.assertEqual(str(expr), "c + 1 = 3")
    
    def test_scalar(self):
        s = node.Scalar(["a", "b", "c"])
        self.assertListEqual([str(value) for value in s.values],
                             ["a", "b", "c"])
        self.assertEqual(str(s), "{a, b, c}")
    
    def test_modtype(self):
        m = node.Modtype("mod", ["a", "b", "c"])
        self.assertEqual(str(m), "mod(a, b, c)")
    
    def test_simple_main_module(self):
        class main(model.Module):
            VAR = "c: 0..2;"
            
            INIT = "c = 0"
            TRANS = "next(c) = c+1 mod 2"
        
        expected = """
MODULE main
    VAR
        c: 0 .. 2;
    INIT
        c = 0
    TRANS
        next(c) = c + 1 mod 2
                    """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_module_with_args(self):
        class Counter(model.Module):
            ARGS = ["limit"]
            VAR = "c: 0 .. limit;"
            INIT = "c = 0"
            TRANS = "next(c) = c+1 mod limit"
        
        expected = """
MODULE Counter(limit)
    VAR
        c: 0 .. limit;
    INIT
        c = 0
    TRANS
        next(c) = c + 1 mod limit
                   """
        
        self.assertEqual(str(Counter), expected.strip())
    
    def test_trimmed_content(self):
        class main(model.Module):
            VAR = """
                  c1: 0..2;
                  c2: 0..2;
                  """
            INIT = "c1 = 0 & c2 = 0"
            TRANS = """
                    next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
                    """
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
    INIT
        (c1 = 0 & c2 = 0)
    TRANS
        (next(c1) = c1 + 1 mod 2 & next(c2) = c2 + 1 mod 2)
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_var_list(self):
        class main(model.Module):
            INIT = "c1 = 0 & c2 = 0"
            TRANS = """
                    next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
                    """
            VAR = ["c1: 0..2;",
                  ("c2", "0..2")]
        
        expected = """
MODULE main
    INIT
        (c1 = 0 & c2 = 0)
    TRANS
        (next(c1) = c1 + 1 mod 2 & next(c2) = c2 + 1 mod 2)
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
                   """
        self.assertEqual(str(main), expected.strip())
    
    def test_var_dict(self):
        class main(model.Module):
            VAR = {"c1": "0..2",
                   "c2": "0..2"}
            INIT = "c1 = 0 & c2 = 0"
            TRANS = """
                    next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
                    """
        
        expected12 = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
    INIT
        (c1 = 0 & c2 = 0)
    TRANS
        (next(c1) = c1 + 1 mod 2 & next(c2) = c2 + 1 mod 2)
                   """
        
        expected21 = """
MODULE main
    VAR
        c2: 0 .. 2;
        c1: 0 .. 2;
    INIT
        (c1 = 0 & c2 = 0)
    TRANS
        (next(c1) = c1 + 1 mod 2 & next(c2) = c2 + 1 mod 2)
                   """
        
        self.assertTrue(str(main) == expected12.strip() or
                        str(main) == expected21.strip())
    
    def test_trans_init_list(self):
        class main(model.Module):
            VAR = collections.OrderedDict((("c1", "0..2"), ("c2", "0..2")))
            INIT = ["c1 = 0", "c2 = 0"]
            TRANS = ["next(c1) = c1+1 mod 2",
                     "next(c2) = c2+1 mod 2"]
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
    INIT
        c1 = 0
    INIT
        c2 = 0
    TRANS
        next(c1) = c1 + 1 mod 2
    TRANS
        next(c2) = c2 + 1 mod 2
                   """
        
        self.assertEqual(str(main), expected.strip())

    def test_module_from_metaclass_instantiation(self):
        variables = collections.OrderedDict()
        variables["c1"] = "0 .. 2"
        variables["c2"] = "0 .. 2"
        
        namespace = collections.OrderedDict()
        namespace["VAR"] = variables
        
        namespace["INIT"] = ["c1 = 0", "c2 = 0"]
        namespace["TRANS"] = ["next(c1) = c1+1 mod 2",
                              "next(c2) = c2+1 mod 2"]
        main = model.ModuleMetaClass("main", (model.Module,), namespace)
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
    INIT
        c1 = 0
    INIT
        c2 = 0
    TRANS
        next(c1) = c1 + 1 mod 2
    TRANS
        next(c2) = c2 + 1 mod 2
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    
    def test_module_from_single_expression(self):
        class main(model.Module):
            c1 = node.Atom("c1")
            c2 = node.Atom("c2")
            VAR = collections.OrderedDict(((c1, node.Range(0,2)),
                                           (c2, node.Range(0,2))))
            
            INIT = (c1 == 0) & (c2 == 0)
            TRANS = ((node.Next(c1) == (c1 + 1) % 2)
                     &
                     (node.Next(c2) == (c2 + 1) % 2))
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
    INIT
        (c1 = 0 & c2 = 0)
    TRANS
        (next(c1) = (c1 + 1) mod 2 & next(c2) = (c2 + 1) mod 2)
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_module_from_list_of_expressions(self):
        class main(model.Module):
            c1 = node.Atom("c1")
            c2 = node.Atom("c2")
            VAR = collections.OrderedDict(((c1, node.Range(0,2)),
                                           (c2, node.Range(0,2))))
            
            INIT = [c1 == 0, c2 == 0]
            TRANS = [node.Next(c1) == (c1 + 1) % 2,
                     node.Next(c2) == (c2 + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
    INIT
        c1 = 0
    INIT
        c2 = 0
    TRANS
        next(c1) = (c1 + 1) mod 2
    TRANS
        next(c2) = (c2 + 1) mod 2
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_module_with_declared_variables(self):
        class main(model.Module):
            c1 = node.DVar(node.Range(0, 2))
            c2 = node.DVar(node.Range(0, 2))
            
            INIT = [c1 == 0, c2 == 0]
            TRANS = [node.Next(c1) == (c1 + 1) % 2,
                     node.Next(c2) == (c2 + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
    INIT
        c1 = 0
    INIT
        c2 = 0
    TRANS
        next(c1) = (c1 + 1) mod 2
    TRANS
        next(c2) = (c2 + 1) mod 2
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_module_with_mixed_declared_variables(self):
        class main(model.Module):
            c1 = node.DVar(node.Range(0, 2))
            VAR = collections.OrderedDict((("c2", "0..2"),))
            
            INIT = [c1 == 0, node.Atom("c2") == 0]
            TRANS = [node.Next(c1) == (c1 + 1) % 2,
                     node.Next("c2") == (node.Atom("c2") + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
    INIT
        c1 = 0
    INIT
        c2 = 0
    TRANS
        next(c1) = (c1 + 1) mod 2
    TRANS
        next(c2) = (c2 + 1) mod 2
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_module(self):
        class Counter(model.Module):
            run = node.Atom("run")
            ARGS = [run]
            c = node.DVar(node.Range(0, 2))
            INIT = c == 0
            TRANS = [run.implies(c.next() == (c + 1) % 2),
                     (~run).implies(c.next() == c)]
        
        class main(model.Module):
            run = node.DIVar(node.Boolean())
            c1 = node.DVar(Counter(run))
        
        counter_expected = """
MODULE Counter(run)
    VAR
        c: 0 .. 2;
    INIT
        c = 0
    TRANS
        (run -> next(c) = (c + 1) mod 2)
    TRANS
        (!run -> next(c) = c)
                           """
        
        main_expected = """
MODULE main
    IVAR
        run: boolean;
    VAR
        c1: Counter(run);
        """
        
        self.assertEqual(str(Counter), counter_expected.strip())
        self.assertEqual(str(main), main_expected.strip())