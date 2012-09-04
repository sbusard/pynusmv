import unittest

from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.cmd import cmd as nscmd

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.glob import glob
from pynusmv.utils.exception import (NuSMVNoReadFileError,
                                     NuSMVCannotFlattenError,
                                     NUSMVModelAlreadyFlattenedError,
                                     NuSMVNeedFlatModelError,
                                     NuSMVNeedFlatHierarchyError)
from pynusmv.parser.parser import parse_simple_expression

class Testglob(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
        
    def test_parsing(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        
    
    def test_parsed_tree(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        pt = glob.parsed_tree()
        self.assertIsNotNone(pt)
        self.assertEqual(pt.type, nsparser.CONS)
        
   
    def test_parsed_tree_after_reparsing(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        sexp = parse_simple_expression("c1.c = 3")
        pt = glob.parsed_tree()
        self.assertIsNotNone(pt)
        self.assertEqual(pt.type, nsparser.CONS)
        
    
    def test_no_parsed_tree(self):
        with self.assertRaises(NuSMVNoReadFileError):
            pt = glob.parsed_tree()
            
            
    def test_load_allow_flattening_with_command(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        ret = nscmd.Cmd_SecureCommandExecute("flatten_hierarchy")
        self.assertEqual(ret, 0)
        
    
    def test_flat_hierarchy(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        fh = glob.flat_hierarchy()
        self.assertIsNotNone(fh)
        
    
    def test_no_flattening_hierarchy(self):
        with self.assertRaises(NuSMVNoReadFileError):
            glob.flatten_hierarchy()
    
    def test_no_flat_hierarchy(self):
        with self.assertRaises(NuSMVNoReadFileError):
            glob.flat_hierarchy()
            
            
    def test_load_and_flat_allow_encoding_with_command(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        glob.flatten_hierarchy()
        ret = nscmd.Cmd_SecureCommandExecute("encode_variables")
        
        
    def test_no_encoding(self):
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            glob.encode_variables()
    
    def test_no_bdd_encoding(self):
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            glob.bdd_encoding()
    
    def test_no_encoding_after_parsing(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            glob.encode_variables()
    
    def test_no_bdd_encoding_after_parsing(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            glob.bdd_encoding()
    
    
    def test_bdd_encoding(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        glob.flatten_hierarchy()
        be = glob.bdd_encoding()
        self.assertIsNotNone(be)
        
        
    def test_flat_command_allow_prop_db(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        ret = nscmd.Cmd_SecureCommandExecute("flatten_hierarchy")
        self.assertEqual(ret, 0)
        pd = glob.prop_database()
        self.assertIsNotNone(pd)
        
        
    def test_no_prop_database(self):
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            glob.prop_database()
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            glob.load_from_file("tests/pynusmv/models/counters.smv")
            glob.prop_database()
        
        
    def test_prop_database(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        glob.flatten_hierarchy()
        pd = glob.prop_database()
        self.assertIsNotNone(pd)
        
        
    def test_no_flat_model(self):
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            glob.build_flat_model()
            
    
    def test_no_flat_model_after_parsing(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            glob.build_flat_model()
    

    def test_flat_model(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        glob.flatten_hierarchy()
        glob.build_flat_model()
        

    def test_no_model(self):
        with self.assertRaises(NuSMVNeedFlatModelError):
            glob.build_model()
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        with self.assertRaises(NuSMVNeedFlatModelError):
            glob.build_model()
        glob.flatten_hierarchy()
        with self.assertRaises(NuSMVNeedFlatModelError):
            glob.build_model()
    

    def test_model(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        glob.flatten_hierarchy()
        glob.encode_variables()
        glob.build_flat_model()
        glob.build_model()
        
        
    def test_compute_model(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        glob.compute_model()
        
    
    def test_no_compute_model(self):
        with self.assertRaises(NuSMVNoReadFileError):
            glob.compute_model()
            
            
    def test_file_error(self):
        with self.assertRaises(IOError):
            glob.load_from_file(
                            "tests/pynusmv/models/no-model.smv")
                            
                            
    def test_semantics_error(self):
        glob.load_from_file(
                            "tests/pynusmv/models/counter-semantics-error.smv")
        with self.assertRaises(NuSMVCannotFlattenError):
            glob.flatten_hierarchy()