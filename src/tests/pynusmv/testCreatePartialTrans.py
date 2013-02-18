import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.fsm import BddFsm
from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression as evalSexp

from pynusmv.prop import PropDb
from pynusmv import glob
from pynusmv.fsm import BddTrans


from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.prop import prop as nsprop
from pynusmv.nusmv.fsm import fsm as nsfsm
from pynusmv.nusmv.fsm.sexp import sexp as nssexp
from pynusmv.nusmv.utils import utils as nsutils
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.enc import enc as nsenc
from pynusmv.nusmv.enc.base import base as nsbaseenc
from pynusmv.nusmv.enc.bdd import bdd as nsbddenc
from pynusmv.nusmv.trans.bdd import bdd as nsbddtrans
from pynusmv.nusmv.opt import opt as nsopt
from pynusmv.nusmv.parser import parser

class TestCreatePartialTrans(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def model(self, modelpath):
        fsm = BddFsm.from_filename(modelpath)
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def print_states(self, fsm, states):
        while states.isnot_false():
            s = fsm.pick_one_state(states)
            print(s.get_str_values())
            states -= s
            
            
    def get_variable_from_string(self, sexpfsm_ptr, var_string):
        var_list = nssexp.SexpFsm_get_vars_list(sexpfsm_ptr)
        
        var_list_iter = nsutils.NodeList_get_first_iter(var_list)
        while var_list_iter is not None:
            var = nsutils.NodeList_get_elem_at(var_list, var_list_iter)
            
            if nsnode.sprint_node(var) == var_string:
                return var
            
            var_list_iter = nsutils.ListIter_get_next(var_list_iter)
            
        return None
        
        
    def test_create_trans_counters_assign(self):    
        
        fsm = self.model("tests/pynusmv/models/counters-assign.smv")
        
        c1c0bdd = evalSexp(fsm, "c1.c = 0")
        c2c0bdd = evalSexp(fsm, "c2.c = 0")
        c1c1bdd = evalSexp(fsm, "c1.c = 1")
        c2c1bdd = evalSexp(fsm, "c2.c = 1")
        
        self.assertEqual(c1c0bdd & c2c0bdd, fsm.init)
        self.assertEqual(c1c0bdd & c2c1bdd | c1c1bdd & c2c0bdd,
                         fsm.post(fsm.init))
        
        fsmbuilder = nscompile.Compile_get_global_fsm_builder()
        enc = nsenc.Enc_get_bdd_encoding()
        ddmanager = nsbddenc.BddEnc_get_dd_manager(enc)
        base_enc = nsbddenc.bddenc2baseenc(enc)
        symb_table = nsbaseenc.BaseEnc_get_symb_table(base_enc)
        
        propDb = glob.prop_database()
        master = propDb.master
        
        sexpfsm_ptr = nsprop.Prop_get_scalar_sexp_fsm(master._ptr)
        
        
        # Create a new expr trans
        c2c = self.get_variable_from_string(sexpfsm_ptr, "c2.c")
        self.assertIsNotNone(c2c)
        # trans = next(c2c) = (c2.c + 1) % 4
        nextc2c = nssexp.Expr_next(c2c, symb_table)
        one = nsnode.create_node(parser.NUMBER, None, None)
        one.left.nodetype = nsnode.int2node(1)
        self.assertEqual(nsnode.sprint_node(one), "1")
        four = nsnode.create_node(parser.NUMBER, None, None)
        four.left.nodetype = nsnode.int2node(4)
        self.assertEqual(nsnode.sprint_node(four), "4")
        c2cp1 = nssexp.Expr_plus(c2c, one)
        c2cp1m4 = nssexp.Expr_mod(c2cp1, four)
        trans = nssexp.Expr_equal(nextc2c, c2cp1m4, symb_table)
        
        
        clusters = nsfsm.FsmBuilder_clusterize_expr(fsmbuilder, enc, trans)
        cluster_options = nsbddtrans.ClusterOptions_create(
                            nsopt.OptsHandler_get_instance())
                            
        bddTrans = BddTrans(
                    nsbddtrans.BddTrans_create(
                        ddmanager,
                        clusters,
                        nsbddenc.BddEnc_get_state_vars_cube(enc),
                        nsbddenc.BddEnc_get_input_vars_cube(enc),
                        nsbddenc.BddEnc_get_next_state_vars_cube(enc),
                        nsopt.get_partition_method(nsopt.OptsHandler_get_instance()),
                        cluster_options))
                        
        fsm.trans = bddTrans
        
        
        self.assertEqual(c1c0bdd & c2c0bdd, fsm.init)
        self.assertEqual(c2c1bdd, fsm.post(fsm.init))
        