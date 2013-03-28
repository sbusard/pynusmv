import unittest

from pynusmv.init import init_nusmv, deinit_nusmv

from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.opt import opt as nsopt
from pynusmv.nusmv.cmd import cmd as nscmd


class TestYaccParser(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
    
    
    def test_simple_expression(self):
        
        car = nsnode.car
        cdr = nsnode.cdr
        
        tests_ok = ["3 + 1", "a - b", "a = 5", "3 + 1 = 5",
                    # -- is the comment, this input is well formed
                    "cat -- dog is < than cat ++ dog"]
        
        for test in tests_ok:
            node, err = nsparser.ReadSimpExprFromString(test)
            self.assertEqual(err, 0)
            self.assertIsNone(nsparser.Parser_get_syntax_errors_list())
            self.assertEqual(node.type, nsparser.SIMPWFF)
            
        tests_ko = [
                    "3 + ", "a / b)", "a ** b ++ c", "strings are good",
                    "test on", "(a + b) - c * 4[-]",
                    "test - c\n= 5[]", "next(i) = 3"#, "a % b"
                    ]
        # FIXME Segmentation fault when trying to read an expression containing
        # a % character
        
        for test in tests_ko:
            node, err = nsparser.ReadSimpExprFromString(test)
            if node is not None:
                self.assertEqual(node.type, nsparser.SIMPWFF)
            self.assertTrue(err > 0)
            # err = 1 means there is a parse error
            # err = 2 means there is an exception
            if err == 1:
                self.assertIsNotNone(nsparser.Parser_get_syntax_errors_list())
                        
            errors = nsparser.Parser_get_syntax_errors_list()
            while errors is not None:
                error = car(errors)
                err = nsparser.Parser_get_syntax_error(error)
                self.assertIsNotNone(err)
                self.assertEqual(len(err), 4)
                
                errors = cdr(errors)
                
    
    def test_next_expression(self):
        
        car = nsnode.car
        cdr = nsnode.cdr
        
        tests_ok = ["next(c) + 1 = 5", "next(a) - b",
                    "next(cat) -- dog is < than cat ++ dog",
                    "a + next(b) < c", "next(3)"]
        
        for test in tests_ok:
            node, err = nsparser.ReadNextExprFromString(test)
            self.assertEqual(err, 0)
            self.assertIsNone(nsparser.Parser_get_syntax_errors_list())
            self.assertEqual(node.type, nsparser.NEXTWFF)
            
        tests_ko = [
                    "next(next(i))", "next(a"#, "a + next(c) % 5"
                    ]
        # FIXME Segmentation fault when trying to read an expression containing
        # a % character
        
        for test in tests_ko:
            node, err = nsparser.ReadNextExprFromString(test)
            if node is not None:
                self.assertEqual(node.type, nsparser.NEXTWFF)
            self.assertTrue(err > 0)
            # err = 1 means there is a parse error
            # err = 2 means there is an exception
            if err == 1:
                self.assertIsNotNone(nsparser.Parser_get_syntax_errors_list())
                        
            errors = nsparser.Parser_get_syntax_errors_list()
            while errors is not None:
                error = car(errors)
                err = nsparser.Parser_get_syntax_error(error)
                self.assertIsNotNone(err)
                self.assertEqual(len(err), 4)
                
                errors = cdr(errors)
                
                
    def test_identifier(self):
        
        car = nsnode.car
        cdr = nsnode.cdr
        
        tests_ok = ["a", "a[3]", "a[b]", "a3", "ab_cd", "o0"]
        
        for test in tests_ok:
            node, err = nsparser.ReadIdentifierExprFromString(test)
            self.assertEqual(err, 0)
            self.assertIsNone(nsparser.Parser_get_syntax_errors_list())
            self.assertEqual(node.type, nsparser.COMPID)
            
        tests_ko = [
                    "c = 3", "a[]", "a[3", "next(i)", "0x52"
                    ]
        
        for test in tests_ko:
            node, err = nsparser.ReadIdentifierExprFromString(test)
            if node is not None:
                self.assertEqual(node.type, nsparser.COMPID)
            self.assertTrue(err > 0)
            # err = 1 means there is a parse error
            # err = 2 means there is an exception
            if err == 1:
                self.assertIsNotNone(nsparser.Parser_get_syntax_errors_list())
                        
            errors = nsparser.Parser_get_syntax_errors_list()
            while errors is not None:
                error = car(errors)
                err = nsparser.Parser_get_syntax_error(error)
                self.assertIsNotNone(err)
                self.assertEqual(len(err), 4)
                
                errors = cdr(errors)
                
                
    def test_smv_correct_file(self):
        ret = nsparser.ReadSMVFromFile("tests/pynusmv/models/counters.smv")
        self.assertEqual(ret, 0)
        self.assertIsNone(nsparser.Parser_get_syntax_errors_list())
        
    
    def test_smv_incorrect_file(self):
        
        car = nsnode.car
        cdr = nsnode.cdr
        
        f = "tests/pynusmv/models/counter-syntax-error.smv"
        nsopt.set_input_file(nsopt.OptsHandler_get_instance(), f)
        ret = nsparser.Parser_ReadSMVFromFile(f)
        
        # When parsing a model with parser_is_lax enabled, the parser gets
        # as many correct parts of the model as possible and build a partial
        # model with it.
        # In this example, a partial model is built, this is why the command
        # returns 0.
        
        self.assertEqual(ret, 0)
        
        errors = nsparser.Parser_get_syntax_errors_list()
        self.assertIsNotNone(errors)
        
        while errors is not None:
            error = car(errors)
            err = nsparser.Parser_get_syntax_error(error)
            self.assertIsNotNone(err)
            self.assertEqual(len(err), 4)
            
            errors = cdr(errors)