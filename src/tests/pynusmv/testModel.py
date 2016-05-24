import unittest
import collections

from pynusmv import model, parser

class TestModel(unittest.TestCase):
    
    def test_getattr_expr(self):
        mod = model.Identifier("mod")
        self.assertEqual(type(mod.act), model.Dot)
        self.assertEqual(type(mod.name), str)
    
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
        c: 0 .. limit;
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
        c1: 0 .. 2;
        c2: 0 .. 2;
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
        c1: 0 .. 2;
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
            VAR = collections.OrderedDict(((c1, model.Range(0,2)),
                                           (c2, model.Range(0,2))))
            
            INIT = (c1 == 0) & (c2 == 0)
            TRANS = ((model.Next(c1) == (c1 + 1) % 2)
                     &
                     (model.Next(c2) == (c2 + 1) % 2))
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
        c2: 0 .. 2;
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
            VAR = collections.OrderedDict(((c1, model.Range(0,2)),
                                           (c2, model.Range(0,2))))
            
            INIT = [c1 == 0, c2 == 0]
            TRANS = [model.Next(c1) == (c1 + 1) % 2,
                     model.Next(c2) == (c2 + 1) % 2]
        
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
            c1 = model.Var(model.Range(0, 2))
            c2 = model.Var(model.Range(0, 2))
            
            INIT = [c1 == 0, c2 == 0]
            TRANS = [model.Next(c1) == (c1 + 1) % 2,
                     model.Next(c2) == (c2 + 1) % 2]
        
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
            c1 = model.Var(model.Range(0, 2))
            VAR = collections.OrderedDict((("c2", "0..2"),))
            
            INIT = [c1 == 0, model.Identifier("c2") == 0]
            TRANS = [model.Next(c1) == (c1 + 1) % 2,
                     model.Next("c2") == (model.Identifier("c2") + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
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
            c = model.Var(model.Range(0, 2))
            INIT = c == 0
            TRANS = [run.implies(c.next() == (c + 1) % 2),
                     (~run).implies(c.next() == c)]
        
        class main(model.Module):
            run = model.IVar(model.Boolean())
            c1 = model.Var(Counter(run))
        
        counter_expected = """
MODULE Counter(run)
    VAR
        c: 0 .. 2;
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
            c1 = model.Var(model.Range(0, 2))
            VAR = collections.OrderedDict((("c2", "0..2"),))
            
            INIT = [c1 == 0, model.Identifier("c2") == 0]
            TRANS = [model.Next(c1) == (c1 + 1) % 2,
                     model.Next("c2") == (model.Identifier("c2") + 1) % 2]
        
        expected = """
MODULE main
    VAR
        c1: 0 .. 2;
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
        copy.VAR[model.Identifier("c3")] = model.Range(0, 2)
        self.assertEqual(str(main), expected.strip())
        self.assertNotEqual(str(copy), expected.strip())


    def test_assign(self):
        class main(model.Module):

            J = model.Var(model.Boolean(), "J")
            K = model.Var(model.Boolean(), "K")
            Q = model.Var(model.Boolean(), "Q")

            INIT = [(Q == model.Falseexp())]

            ASSIGN = {model.Next(Q): model.Case(((J & K, ~Q),
                                                 (J, model.Trueexp()),
                                                 (K, model.Falseexp()),
                                                 (model.Trueexp(), Q)))}

        expected = """
MODULE main
    VAR
        J: boolean;
        K: boolean;
        Q: boolean;
    INIT
        Q = FALSE
    ASSIGN
        next(Q) := 
            case
                J & K: ! Q;
                J: TRUE;
                K: FALSE;
                TRUE: Q;
            esac;
                   """
        self.assertEqual(str(main), expected.strip())


    def test_module_comment(self):
        class main(model.Module):
            COMMENT = "This is the main module"
            v = model.Var(model.Boolean())
        expected = """
-- This is the main module
MODULE main
    VAR
        v: boolean;
"""
        self.assertEqual(str(main), expected.strip())


    def test_expression_comment(self):
        class main(model.Module):
            v = model.Var(model.Boolean())
            INIT = [model.Comment(~v, "Initially FALSE")]
        expected = """
MODULE main
    VAR
        v: boolean;
    INIT
        ! v -- Initially FALSE

"""
        self.assertEqual(str(main), expected.strip())


    def test_expression_inline_comment(self):
        class main(model.Module):
            v = model.Var(model.Boolean())
            INIT = [model.Comment(~v, "Initially FALSE") | v]
        expected = """
MODULE main
    VAR
        v: boolean;
    INIT
        ! v -- Initially FALSE
        | v

"""
        self.assertEqual(str(main), expected.strip())


    def test_case_comment(self):
        class main(model.Module):
            v = model.Var(model.Boolean())
            i = model.IVar(model.Boolean())
            INIT = [model.Case(((model.Trueexp(),
                                 model.Comment(~v, "Initially FALSE")),))]
            TRANS = [model.Case(((v,
                                  model.Comment(
                                  model.Comment(v.next(), "Stay true"),
                                  "when v is already true")),
                                 (~i, model.Comment(v.next() == v,
                                                    "Stay the same")),
                                 (~v & i, v.next()),
                                 (v & i, model.Comment(~v.next(),
                                                       "Change to false"))))]
        expected = """
MODULE main
    VAR
        v: boolean;
    IVAR
        i: boolean;
    INIT
        case
            TRUE: ! v; -- Initially FALSE
        esac
    TRANS
        case
            v: next(v); -- Stay true
            -- when v is already true
            ! i: next(v) = v; -- Stay the same
            ! v & i: next(v);
            v & i: ! next(v); -- Change to false
        esac
"""
        self.assertEqual(str(main), expected.strip())


    def test_variables_comment(self):
        variables = model.Variables(collections.OrderedDict(
                                    ((model.Identifier("v"),
                                      model.Comment(model.Boolean(),
                                                    "The variable")),
                                     (model.Identifier("w"),
                                      model.Comment(
                                      model.Comment(model.Boolean(),
                                                    "Another variable"),
                                      "and a new name!")))))
        expected = """
VAR
    v: boolean; -- The variable
    w: 
        boolean; -- Another variable
        -- and a new name!
"""
        self.assertEqual(str(variables), expected.strip())


    def test_var_comment(self):
        class main(model.Module):
            v = model.Var(model.Comment(model.Boolean(), "The variable"))
            w = model.Var(model.Comment(model.Comment(model.Boolean(),
                                        "Another variable"),
                          "and a new name!"))
        expected = """
MODULE main
    VAR
        v: boolean; -- The variable
        w: 
            boolean; -- Another variable
            -- and a new name!

"""
        self.assertEqual(str(main), expected.strip())


    def test_comments(self):
        class main(model.Module):
            COMMENT = "This is the main module"

            myvar = model.Var(model.Comment(model.Boolean(), "My variable"))

            var2 = model.Identifier("var2")
            VAR = {var2: model.Comment(model.Range(0, 3), "The counter")}

            INIT = [model.Comment(~myvar, "myvar is false") &
                    model.Comment(var2 == 0, "we start at 0")]

            TRANS = [
                     model.Case(((myvar,
                                  model.Comment(
                                  model.Comment(var2.next() ==
                                                ((var2 + 1) % 4),
                                          "Increase var2"),
                                  "Only when myvar is true"
                                  )
                                 ),
                                 (model.Trueexp(),
                                  model.Comment(var2.next() == var2,
                                          "otherwise do nothing"))))
                    ]
        expected = """
-- This is the main module
MODULE main
    VAR
        myvar: boolean; -- My variable
        var2: 0 .. 3; -- The counter
    INIT
        ! myvar -- myvar is false
        & var2 = 0 -- we start at 0
    TRANS
        case
            myvar: next(var2) = (var2 + 1) mod 4; -- Increase var2
            -- Only when myvar is true
            TRUE: next(var2) = var2; -- otherwise do nothing
        esac
"""
        self.assertEqual(str(main), expected.strip())