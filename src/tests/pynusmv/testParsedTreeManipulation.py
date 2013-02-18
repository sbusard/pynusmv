import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression as evalSexp

from pynusmv.prop import PropDb
from pynusmv.fsm import BddTrans


from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.prop import prop as nsprop
from pynusmv.nusmv.fsm import fsm as nsfsm
from pynusmv.nusmv.fsm.sexp import sexp as nssexp
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.enc import enc as nsenc
from pynusmv.nusmv.enc.bdd import bdd as nsbddenc
from pynusmv.nusmv.trans.bdd import bdd as nsbddtrans
from pynusmv.nusmv.opt import opt as nsopt
from pynusmv.nusmv.set import set as nsset
from pynusmv.nusmv.parser import parser as nsparser
from pynusmv.nusmv.cmd import cmd as nscmd
from pynusmv.nusmv.compile.symb_table import symb_table as nssymb_table

car = nsnode.car
cdr = nsnode.cdr

class TestParsedTreeManipulation(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()

    
    def test_trans(self):
        # Parse a model
        #nsparser.ReadSMVFromFile("tests/pynusmv/models/modules.smv")
        #parsed_tree = nsparser.cvar.parsed_tree
        nscmd.Cmd_SecureCommandExecute("read_model -i tests/pynusmv/models/modules.smv")
        nscmd.Cmd_SecureCommandExecute("flatten_hierarchy")
        
        
        st = nscompile.Compile_get_global_symb_table()
        
        
#       main = nsnode.find_node(
#               nsparser.ATOM,
#               nsnode.string2node(nsutils.find_string("main")),
#                None)
#       
#       # Flatten
#       nscompile.CompileFlatten_init_flattener()
#       st = nscompile.Compile_get_global_symb_table()
#       layer = nssymb_table.SymbTable_create_layer(st, "model",
#                   nssymb_table.SYMB_LAYER_POS_BOTTOM)
#       nssymb_table.SymbTable_layer_add_to_class(st, "model", "Model Class")
#       nssymb_table.SymbTable_set_default_layers_class_name(st, "Model Class")
#       
#       #hierarchy = nscompile.Compile_FlattenHierarchy(
#       #                st, layer, main, None, None, 1, 0, None)
#       
#       hierarchy = nscompile.FlatHierarchy_create(st)
#       instances = nsutils.new_assoc()
#       
#       nscompile.Compile_ConstructHierarchy(st, layer, main, None, None,
#                       hierarchy, None, instances)
#       
#       fhtrans = nscompile.FlatHierarchy_get_trans(hierarchy)
#       print("NON FLATTENED")
#       print(nsnode.sprint_node(fhtrans))
#       
#       print(fhtrans.type)                 #169 = AND
#       print(car(fhtrans))                 #None
#       print(cdr(fhtrans).type)            #130 = CONTEXT
#       print(car(cdr(fhtrans)).type)       #208 = DOT
#       print(car(car(cdr(fhtrans))))       #None
#       print(cdr(car(cdr(fhtrans))).type)  #161 = ATOM
#       print(nsnode.sprint_node(cdr(car(cdr(fhtrans))))) #m
#       
#       print(cdr(cdr(fhtrans)).type)       #192 = EQUAL
#       
#       
#       
#       trans = nscompile.Compile_FlattenSexp(st, cdr(fhtrans), None)
#       print("FLATTENED")
#       print(nsnode.sprint_node(trans))
        
        
        
        
        
        
        
        layers = nssymb_table.SymbTable_get_class_layer_names(st, None)
        variables = nssymb_table.SymbTable_get_layers_sf_i_vars(st, layers)
        ite = nsutils.NodeList_get_first_iter(variables)
        while not nsutils.ListIter_is_end(ite):
            variable = nsutils.NodeList_get_elem_at(variables, ite)
            print(nsnode.sprint_node(variable))
            ite = nsutils.ListIter_get_next(ite)
        
        
        top = nsnode.find_node(nsparser.ATOM,
                                 nsnode.string2node(nsutils.find_string("top")),
                                 None)
                                 
        trans = nssexp.Expr_equal(
                    nssexp.Expr_next(top, st),
                    top,
                    st)
        
        flattrans = nscompile.Compile_FlattenSexp(st, trans, None)
        
        
        inmod = nsnode.find_node(
                nsparser.ATOM,
                nsnode.string2node(nsutils.find_string("inmod")),
                 None)
                 
        t = nsnode.find_node(
                nsparser.ATOM,
                nsnode.string2node(nsutils.find_string("t")),
                 None)
                 
        m = nsnode.find_node(
                nsparser.ATOM,
                nsnode.string2node(nsutils.find_string("m")),
                 None)
    
        my = nsnode.find_node(
                nsparser.ATOM,
                nsnode.string2node(nsutils.find_string("my")),
                 None)
                 
        n = nsnode.find_node(
                nsparser.ATOM,
                nsnode.string2node(nsutils.find_string("n")),
                 None)
                 
        mymod = nsnode.find_node(
                nsparser.ATOM,
                nsnode.string2node(nsutils.find_string("mymod")),
                 None)
                 
        main = nsnode.find_node(
                nsparser.ATOM,
                nsnode.string2node(nsutils.find_string("main")),
                 None)
                 
        minmod = nsnode.find_node(
                    nsparser.DOT,
                    nsnode.find_node(
                        nsparser.DOT,
                        None,
                        m),
                    inmod)
        
        ftrans = nssexp.Expr_equal(
                    nssexp.Expr_next(inmod, st),
                    nssexp.Expr_or(inmod,
                        nsnode.find_node(nsparser.DOT, my, inmod)),
                    st)
        print(nsnode.sprint_node(ftrans))
                    
        res, err = nsparser.ReadNextExprFromString("next(inmod) = (inmod | my.inmod) IN n")
        self.assertEqual(err, 0)
        trans = car(res)
        print(nsnode.sprint_node(trans))
                    
        conttrans = nsnode.find_node(
                        nsparser.CONTEXT,
                        nsnode.find_node(nsparser.DOT, None, n),
                        ftrans)
        print(nsnode.sprint_node(conttrans))
        
                
        fflattrans = nscompile.Compile_FlattenSexp(st, conttrans, None)
        flattrans = nscompile.Compile_FlattenSexp(st, trans, None)
        
        print(nsnode.sprint_node(fflattrans))
        print(nsnode.sprint_node(flattrans))
        
            
            