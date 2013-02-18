import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob
from pynusmv.parser import parse_simple_expression

from pynusmv.nusmv.compile.symb_table import symb_table as nssymb_table
from pynusmv.nusmv.compile.type_checking import type_checking as nstype_checking

class TestTypeChecking(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
        
    def test_get_mod_instance_type(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        glob.compute_model()
        
        sexp = parse_simple_expression("c1")
        self.assertIsNotNone(sexp)
        
        st = glob.symb_table()
        tp = nssymb_table.SymbTable_get_type_checker(st._ptr)
        expr_type = nstype_checking.TypeChecker_get_expression_type(
                                                           tp, sexp, None)
        self.assertTrue(nssymb_table.SymbType_is_error(expr_type))