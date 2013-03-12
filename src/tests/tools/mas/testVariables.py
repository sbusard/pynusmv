import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob

from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.set import set as nsset
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.compile.symb_table import symb_table as nssymb_table

class TestVariables(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def test_fsm(self):
        glob.load_from_file("tests/tools/ctlk/dining-crypto.smv")
        glob.compute_model()
        fsm = glob.prop_database().master.bddFsm
        self.assertIsNotNone(fsm)
        
        
    def test_printing_parsed_tree(self):
        car = nsnode.car
        cdr = nsnode.cdr
        
        glob.load_from_file("tests/tools/ctlk/dining-crypto.smv")
        
        # Get parsed tree
        tree = nsparser.cvar.parsed_tree
        
        
        print(tree.type) # 145 = CONS
        print(car(tree).type) # 117 = MODULE
        print(car(car(tree)).type) # 119 = MODTYPE
        print(nsnode.sprint_node(car(car(car(tree))))) # main
        print(cdr(tree).type) # 145 = CONS
        print(car(cdr(tree)).type) # 117 = MODULE
        print(car(car(car(cdr(tree)))).type) # 161 = ATOM
        print(nsnode.sprint_node(car(car(car(cdr(tree)))))) # cryptograph
        print(cdr(cdr(tree))) # None
        
        
        
        
    def get_instances_for_module(self, modtree):
        """
        Return the list of variables that are instances of modules
        in module modtree.
        
        modtree is a part of the AST of the SMV model. modtree.type = MODULE
        """
        # MODULE(MODTYPE(...), declarationlist)
        varlist = []
        declarations = nsnode.cdr(modtree)
        while declarations is not None:
            decl = nsnode.car(declarations)
            if decl.type == nsparser.VAR:
                decl = nsnode.car(decl)
                while decl is not None:
                    var = nsnode.car(decl)
                    
                    # var_id : type => COLON(ATOM, type)
                    if nsnode.cdr(var).type == nsparser.MODTYPE:
                        varlist.append(var)
                    
                    decl = nsnode.cdr(decl)
                            
            declarations = nsnode.cdr(declarations)
            
        return varlist
        
        
    def print_instances(self, filepath):
        print("----- Instances for", filepath)
        
        car = nsnode.car
        cdr = nsnode.cdr
        
        glob.load_from_file(filepath)
        
        # Get parsed tree
        tree = nsparser.cvar.parsed_tree
        
        
        print("--- Main instances")
        # main module is car(tree)
        main_vars = self.get_instances_for_module(car(tree))
        
        instances_args = {}
        
        for var in main_vars:
            # var = COLON(ATOM, MODTYPE(ATOM, CONS))
            
            varname = nsnode.sprint_node(car(var))
            instances_args[varname] = []
            
            args = cdr(cdr(var))
            argslist = []
            while args is not None:
                arg = car(args)
                instances_args[varname].append(arg)
                argslist.append(nsnode.sprint_node(arg))
                args = cdr(args)
                
            print(varname, ":", nsnode.sprint_node(car(cdr(var))), argslist)
                
        
        # Get FSM and stuff
        glob.compute_model()
        fsm = glob.prop_database().master.bddFsm
        self.assertIsNotNone(fsm)
        
        flatH = nscompile.cvar.mainFlatHierarchy
        st = nscompile.Compile_get_global_symb_table()
        self.assertIsNotNone(flatH)
        
        
        print("--- Check arguments instances")
        for instance in instances_args:
            print("INSTANCE", instance)
            for arg in instances_args[instance]:                
                arg, err = nscompile.FlattenSexp(st, arg, None)
                self.assertEqual(err, 0)
                isVar = nssymb_table.SymbTable_is_symbol_var(st, arg)
                if isVar:
                    print("VAR", nsnode.sprint_node(arg))
                else:
                    print("NOT VAR", nsnode.sprint_node(arg))
        
        
        print("--- All vars")
        varset = nscompile.FlatHierarchy_get_vars(flatH)
        self.assertIsNotNone(varset)
        varlist = nsset.Set_Set2List(varset)
        self.assertIsNotNone(varlist)
        
        ite = nsutils.NodeList_get_first_iter(varlist)
        while not nsutils.ListIter_is_end(ite):
            var = nsutils.NodeList_get_elem_at(varlist, ite)
            isInput = nssymb_table.SymbTable_is_symbol_input_var(st, var)
            isVar = nssymb_table.SymbTable_is_symbol_var(st, var)
            if isInput:
                print("IVAR", "\t",
                      "IN", "'" + nsnode.sprint_node(car(var)) + "'", "\t",
                      nsnode.sprint_node(var))
            elif isVar:
                print("VAR", "\t",
                      "IN", "'" + nsnode.sprint_node(car(var)) + "'", "\t",
                      nsnode.sprint_node(var))
            else:
                print("[ERROR] Unknown type:", nsnode.sprint_node(var))
            ite = nsutils.ListIter_get_next(ite)
            
        print("------------------------------------------------------")
        
        
    def test_get_instances_after_flattening(self):
        car = nsnode.car
        cdr = nsnode.cdr
        
        glob.load_from_file("tests/tools/ctlk/dining-crypto.smv")
        
        # Flatten
        glob.flatten_hierarchy()
        
        # Get parsed tree
        tree = nsparser.cvar.parsed_tree
        self.assertIsNotNone(tree)
        self.assertIsNotNone(car(tree))
        
        print(tree.type) # 145 = CONS
        print(car(tree).type) # 117 = MODULE
        print(nsnode.sprint_node(car(car(car(tree))))) # main
        print(cdr(car(tree)).type) # 145 = CONS
        print(car(cdr(car(tree))).type) # 113 = DEFINE
        print(cdr(cdr(car(tree)))) # None 
        print(cdr(tree).type) # 145 = CONS
        print(car(cdr(tree)).type) # 117 = MODULE
        print(nsnode.sprint_node(car(car(car(cdr(tree)))))) # cryptograph
        
        print("----- Instances after flattening")
        # main module is car(tree)
        main_vars = self.get_instances_for_module(car(tree))
        # Variables are removed from parsed_tree when flattening
        self.assertEqual(len(main_vars), 0)
        
        instances_args = {}
        
        for var in main_vars:
            # var = COLON(ATOM, MODTYPE(ATOM, CONS))
            
            varname = nsnode.sprint_node(car(var))
            instances_args[varname] = []
            
            args = cdr(cdr(var))
            argslist = []
            while args is not None:
                arg = car(args)
                instances_args[varname].append(arg)
                argslist.append(nsnode.sprint_node(arg))
                args = cdr(args)
                
            print(varname, ":", nsnode.sprint_node(car(cdr(var))), argslist)
            
        print("------------------------------------------------------")

            
    def test_dincry(self):
        self.print_instances("tests/tools/ctlk/dining-crypto.smv")
            
        
    def test_bitCounter(self):
        self.print_instances("tests/tools/ctlk/bitCounter.smv")
       
        
    def test_counters(self):
        self.print_instances("tests/tools/ctlk/counters.smv")