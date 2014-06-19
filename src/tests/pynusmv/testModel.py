import unittest

from pynusmv import model

class TestModel(unittest.TestCase):
    
    def test_simple_main_module(self):
        class main(model.Module):
            VAR = "c : 0..2;"
            
            INIT = "c = 0"
            TRANS = "next(c) = c+1 mod 2"
        
        expected = """
MODULE main
    VAR
        c : 0..2;
    INIT
        c = 0
    TRANS
        next(c) = c+1 mod 2
                    """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_module_with_args(self):
        class Counter(model.Module):
            ARGS = ["limit"]
            VAR = "c : 0..limit;"
            INIT = "c = 0"
            TRANS = "next(c) = c+1 mod limit"
        
        expected = """
MODULE Counter(limit)
    VAR
        c : 0..limit;
    INIT
        c = 0
    TRANS
        next(c) = c+1 mod limit
                   """
        
        self.assertEqual(str(Counter), expected.strip())
    
    def test_trimmed_content(self):
        class main(model.Module):
            VAR = """
                  c1 : 0..2;
                  c2 : 0..2;
                  """
            INIT = "c1 = 0 & c2 = 0"
            TRANS = """
                    next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
                    """
        
        expected = """
MODULE main
    VAR
        c1 : 0..2;
        c2 : 0..2;
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
            VAR = ["c1 : 0..2;",
                   ("c2", "0..2")]
        
        expected = """
MODULE main
    INIT
        c1 = 0 & c2 = 0
    TRANS
        next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
    VAR
        c1 : 0..2;
        c2 : 0..2;
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
        
        expected = """
MODULE main
    VAR
        c1 : 0..2;
        c2 : 0..2;
    INIT
        c1 = 0 & c2 = 0
    TRANS
        next(c1) = c1+1 mod 2 & next(c2) = c2+1 mod 2
                   """
        
        self.assertEqual(str(main), expected.strip())
    
    def test_trans_init_list(self):
        class main(model.Module):
            VAR = {"c1": "0..2",
                   "c2": "0..2"}
            INIT = ["c1 = 0", "c2 = 0"]
            TRANS = ["next(c1) = c1+1 mod 2",
                     "next(c2) = c2+1 mod 2"]
        
        expected = """
MODULE main
    VAR
        c1 : 0..2;
        c2 : 0..2;
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
    