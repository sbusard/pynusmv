import unittest

from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.cmd import cmd as nscmd

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.globals.globals import Globals
from pynusmv.utils.exception import (NuSMVNoReadFileError,
                                     NuSMVCannotFlattenError,
                                     NUSMVModelAlreadyFlattenedError,
                                     NuSMVNeedFlatModelError,
                                     NuSMVNeedFlatHierarchyError)
from pynusmv.parser.parser import parse_simple_expression

class TestGlobals(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
        
    def test_parsing(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        
    
    @unittest.skip # TODO See Globals source code    
    def test_parsed_tree(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        pt = Globals.parsed_tree()
        self.assertIsNotNone(pt)
        self.assertEqual(pt.type, nsparser.CONS)
        
   
    @unittest.skip # TODO See Globals source code     
    def test_parsed_tree_after_reparsing(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        sexp = parse_simple_expression("c1.c = 3")
        pt = Globals.parsed_tree()
        self.assertIsNotNone(pt)
        self.assertEqual(pt.type, nsparser.CONS)
        
    
    @unittest.skip # TODO See Globals source code
    def test_no_parsed_tree(self):
        with self.assertRaises(NuSMVNoReadFileError):
            pt = Globals.parsed_tree()
            
            
    def test_load_allow_flattening_with_command(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        ret = nscmd.Cmd_SecureCommandExecute("flatten_hierarchy")
        self.assertEqual(ret, 0)
        
    
    @unittest.skip # TODO See Globals source code    
    def test_flat_hierarchy(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        fh = Globals.flat_hierarchy()
        self.assertIsNotNone(fh)
        
    
    def test_no_flattening_hierarchy(self):
        with self.assertRaises(NuSMVNoReadFileError):
            Globals.flatten_hierarchy()
    
    @unittest.skip # TODO See Globals source code
    def test_no_flat_hierarchy(self):
        with self.assertRaises(NuSMVNoReadFileError):
            Globals.flat_hierarchy()
            
            
    def test_load_and_flat_allow_encoding_with_command(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        Globals.flatten_hierarchy()
        ret = nscmd.Cmd_SecureCommandExecute("encode_variables")
        
        
    def test_no_encoding(self):
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            Globals.encode_variables()
    
    @unittest.skip # TODO See Globals source code
    def test_no_bdd_encoding(self):
        with self.assertRaises(NuSMVNoReadFileError):
            Globals.bdd_encoding()
    
    def test_no_encoding_after_parsing(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            Globals.encode_variables()
    
    @unittest.skip # TODO See Globals source code
    def test_no_bdd_encoding_after_parsing(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            Globals.bdd_encoding()
    
    
    @unittest.skip # TODO See Globals source code        
    def test_bdd_encoding(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        Globals.flatten_hierarchy()
        be = Globals.bdd_encoding()
        self.assertIsNotNone(be)
        
        
    def test_flat_command_allow_prop_db(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        ret = nscmd.Cmd_SecureCommandExecute("flatten_hierarchy")
        self.assertEqual(ret, 0)
        pd = Globals.prop_database()
        self.assertIsNotNone(pd)
        
        
    def test_no_prop_database(self):
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            Globals.prop_database()
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            Globals.load_from_file("tests/pynusmv/models/counters.smv")
            Globals.prop_database()
        
        
    def test_prop_database(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        Globals.flatten_hierarchy()
        pd = Globals.prop_database()
        self.assertIsNotNone(pd)
        
        
    def test_no_flat_model(self):
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            Globals.build_flat_model()
            
    
    def test_no_flat_model_after_parsing(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        with self.assertRaises(NuSMVNeedFlatHierarchyError):
            Globals.build_flat_model()
    

    def test_flat_model(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        Globals.flatten_hierarchy()
        Globals.build_flat_model()
        

    def test_no_model(self):
        with self.assertRaises(NuSMVNeedFlatModelError):
            Globals.build_model()
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        with self.assertRaises(NuSMVNeedFlatModelError):
            Globals.build_model()
        Globals.flatten_hierarchy()
        with self.assertRaises(NuSMVNeedFlatModelError):
            Globals.build_model()
    

    def test_model(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        Globals.flatten_hierarchy()
        Globals.encode_variables()
        Globals.build_flat_model()
        Globals.build_model()
        
        
    def test_compute_model(self):
        Globals.load_from_file("tests/pynusmv/models/counters.smv")
        Globals.compute_model()
        
    
    def test_no_compute_model(self):
        with self.assertRaises(NuSMVNoReadFileError):
            Globals.compute_model()
            
            
    def test_file_error(self):
        with self.assertRaises(IOError):
            Globals.load_from_file(
                            "tests/pynusmv/models/no-model.smv")
                            
                            
    def test_semantics_error(self):
        Globals.load_from_file(
                            "tests/pynusmv/models/counter-semantics-error.smv")
        with self.assertRaises(NuSMVCannotFlattenError):
            Globals.flatten_hierarchy()