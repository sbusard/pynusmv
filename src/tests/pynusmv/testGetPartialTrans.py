import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.fsm import BddFsm
from pynusmv.fsm import BddTrans
from pynusmv.dd import BDD
from pynusmv.mc import eval_simple_expression as evalSexp

from pynusmv.prop import PropDb
from pynusmv.fsm import BddTrans
from pynusmv import glob


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

class TestGetPartialTrans(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
    def model(self, modelpath):
        fsm = BddFsm.from_filename(modelpath)
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def trans_for_module(self, sexpfsm_ptr, module):
        var_list = nssexp.SexpFsm_get_vars_list(sexpfsm_ptr)
        
        trans = None
        
        var_list_iter = nsutils.NodeList_get_first_iter(var_list)
        while var_list_iter is not None:
            var = nsutils.NodeList_get_elem_at(var_list, var_list_iter)
            
            if nsnode.sprint_node(nsnode.car(var)) == module:
                var_trans = nssexp.SexpFsm_get_var_trans(sexpfsm_ptr, var)
                trans = nssexp.Expr_and_nil(trans, var_trans)
            
            var_list_iter = nsutils.ListIter_get_next(var_list_iter)
            
        return trans
        
        
    def print_states(self, fsm, states):
        while states.isnot_false():
            s = fsm.pick_one_state(states)
            print(s.get_str_values())
            states -= s
        
        
    def test_get_trans_counters_assign(self):    
        
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
        
        propDb = glob.prop_database()
        master = propDb.master
        
        sexpfsm_ptr = nsprop.Prop_get_scalar_sexp_fsm(master._ptr)
        
        trans = self.trans_for_module(sexpfsm_ptr, "c2")
        
        
        # ANALYSE THE TRANS TO UNDERSTAND HOW TO CREATE NUMBERS
        #car = nsnode.car
        #cdr = nsnode.cdr
        #print("TRANS--------------------------------")
        #print(nsnode.sprint_node(cdr(trans)))
        #print("TRANS--------------------------------")
        # cdr(trans) = 
        #case
        #run = rc2 : case
        #c2.c + 1 >= stop : start;
        #c2.c + 1 < stop : c2.c + 1;
        #esac;
        #!(run = rc2) : c2.c;
        #esac
        
        # car(cdr(trans)) = 
        #run = rc2 : case
        #c2.c + 1 >= stop : start;
        #c2.c + 1 < stop : c2.c + 1;
        #esac;
        
        # cdr(car(cdr(trans))) = 
        #case
        #c2.c + 1 >= stop : start;
        #c2.c + 1 < stop : c2.c + 1;
        #esac
        
        # car(cdr(car(cdr(trans)))) = 
        #c2.c + 1 >= stop : start
        
        # car(car(cdr(car(cdr(trans))))) = 
        #c2.c + 1 >= stop
        
        # car(car(car(cdr(car(cdr(trans)))))) = 
        #c2.c + 1
        
        # car(car(car(car(cdr(car(cdr(trans))))))) = 
        #c2.c
        
        # car(car(car(car(car(cdr(car(cdr(trans)))))))) = 
        #c2
        
        # cdr(car(car(car(car(cdr(car(cdr(trans)))))))) = 
        #c
        
        # cdr(car(car(car(cdr(car(cdr(trans))))))) = 
        #1
        
        #print(cdr(cdr(car(car(car(car(cdr(car(cdr(trans))))))))))
        #print(nsnode.sprint_node(cdr(car(car(car(car(cdr(car(cdr(trans))))))))))
        
        
        
        
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
        self.assertEqual(c2c1bdd | c2c0bdd, fsm.post(fsm.init))
        