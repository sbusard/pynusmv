import unittest
import collections

from pynusmv import model, parser

class TestModel(unittest.TestCase):
    
    def test_getattr_expr(self):
        mod = model.Identifier("mod")
        self.assertEqual(type(mod.act), model.Context)
        self.assertEqual(type(mod.name), str)
    
    def test_simple_main_module(self):
        class main(model.Module):
            VAR = "c: 0..2;"
            
            INIT = "c = 0"
            TRANS = "next(c) = c+1 mod 2"
        
        expected = """
MODULE main
    VAR
        c: 0..2;
    INIT
        c = 0
    TRANS
        next(c) = c+1 mod 2
                    """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_module_with_args(self):
        class Counter(model.Module):
            ARGS = ["limit"]
            VAR = "c: 0..limit;"
            INIT = "c = 0"
            TRANS = "next(c) = c+1 mod limit"
        
        expected = """
MODULE Counter(limit)
    VAR
        c: 0..limit;
    INIT
        c = 0
    TRANS
        next(c) = c+1 mod limit
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
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0 & c2 = 0
    TRANS
        next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
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
        c1 = 0 & c2 = 0
    TRANS
        next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
    VAR
        c1: 0..2;
        c2: 0..2;
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
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0 & c2 = 0
    TRANS
        next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
                   """
        
        expected21 = """
MODULE main
    VAR
        c2: 0..2;
        c1: 0..2;
    INIT
        c1 = 0 & c2 = 0
    TRANS
        next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
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
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0
    INIT
        c2 = 0
    TRANS
        next(c1) = c1+1 mod 2
    TRANS
        next(c2) = c2+1 mod 2
                   """
        
        self.assertTrue(str(main) == expected.strip())

    def test_module_from_metaclass_instantiation(self):
        variables = collections.OrderedDict()
        variables["c1"] = "0..2"
        variables["c2"] = "0..2"
        
        namespace = collections.OrderedDict()
        namespace["VAR"] = variables
        
        namespace["INIT"] = ["c1 = 0", "c2 = 0"]
        namespace["TRANS"] = ["next(c1) = c1+1 mod 2",
                              "next(c2) = c2+1 mod 2"]
        main = model.ModuleMetaClass("main", (model.Module,), namespace)
        
        expected = """
MODULE main
    VAR
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0
    INIT
        c2 = 0
    TRANS
        next(c1) = c1+1 mod 2
    TRANS
        next(c2) = c2+1 mod 2
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    
    def test_module_from_single_expression(self):
        class main(model.Module):
            c1 = model.Identifier("c1")
            c2 = model.Identifier("c2")
            VAR = collections.OrderedDict(((c1, model.TRange(0,2)),
                                           (c2, model.TRange(0,2))))
            
            INIT = (c1 == 0) & (c2 == 0)
            TRANS = ((model.Next(c1) == (c1 + 1) % 2)
                     &
                     (model.Next(c2) == (c2 + 1) % 2))
        
        expected = """
MODULE main
    VAR
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0 & c2 = 0
    TRANS
        next(c1) = (c1 + 1) mod 2 & next(c2) = (c2 + 1) mod 2
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_module_from_list_of_expressions(self):
        class main(model.Module):
            c1 = model.Identifier("c1")
            c2 = model.Identifier("c2")
            VAR = collections.OrderedDict(((c1, model.TRange(0,2)),
                                           (c2, model.TRange(0,2))))
            
            INIT = [c1 == 0, c2 == 0]
            TRANS = [model.Next(c1) == (c1 + 1) % 2,
                     model.Next(c2) == (c2 + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0..2;
        c2: 0..2;
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
            c1 = model.Var(model.TRange(0, 2))
            c2 = model.Var(model.TRange(0, 2))
            
            INIT = [c1 == 0, c2 == 0]
            TRANS = [model.Next(c1) == (c1 + 1) % 2,
                     model.Next(c2) == (c2 + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0..2;
        c2: 0..2;
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
            c1 = model.Var(model.TRange(0, 2))
            VAR = collections.OrderedDict((("c2", "0..2"),))
            
            INIT = [c1 == 0, model.Identifier("c2") == 0]
            TRANS = [model.Next(c1) == (c1 + 1) % 2,
                     model.Next("c2") == (model.Identifier("c2") + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0..2;
        c2: 0..2;
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
            run = model.Identifier("run")
            ARGS = [run]
            c = model.Var(model.TRange(0, 2))
            INIT = c == 0
            TRANS = [run.implies(c.next() == (c + 1) % 2),
                     (~run).implies(c.next() == c)]
        
        class main(model.Module):
            run = model.IVar(model.TBoolean())
            c1 = model.Var(Counter(run))
        
        counter_expected = """
MODULE Counter(run)
    VAR
        c: 0..2;
    INIT
        c = 0
    TRANS
        run -> next(c) = (c + 1) mod 2
    TRANS
        ! run -> next(c) = c
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


    def test_copy_module(self):
        class main(model.Module):
            c1 = model.Var(model.TRange(0, 2))
            VAR = collections.OrderedDict((("c2", "0..2"),))
            
            INIT = [c1 == 0, model.Identifier("c2") == 0]
            TRANS = [model.Next(c1) == (c1 + 1) % 2,
                     model.Next("c2") == (model.Identifier("c2") + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0..2;
        c2: 0..2;
    INIT
        c1 = 0
    INIT
        c2 = 0
    TRANS
        next(c1) = (c1 + 1) mod 2
    TRANS
        next(c2) = (c2 + 1) mod 2
                   """
        
        copy = main.copy()
        copy.VAR[model.Identifier("c3")] = model.TRange(0, 2)
        self.assertEqual(str(main), expected.strip())
        self.assertNotEqual(str(copy), expected.strip())