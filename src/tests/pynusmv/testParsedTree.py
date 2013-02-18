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

car = nsnode.car
cdr = nsnode.cdr

class TestParsedTree(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
        
    def transs(self, tree):
        def _transs(tree, ltrans):
            if tree.type == nsparser.TRANS:
                ltrans.append(tree)
            else:
                if tree.type not in {
                    nsparser.NUMBER_SIGNED_WORD,
                    nsparser.NUMBER_UNSIGNED_WORD,
                    nsparser.NUMBER_FRAC,
                    nsparser.NUMBER_EXP,
                    nsparser.NUMBER_REAL,
                    nsparser.NUMBER,
                    nsparser.ATOM,
                    nsparser.FAILURE
                }:
                    if car(tree) is not None:
                        _transs(car(tree), ltrans)
                    if cdr(tree) is not None:
                        _transs(cdr(tree), ltrans)
        ltrans = []
        _transs(tree, ltrans)
        return ltrans
        
    
    def print_transs(self, filepath):
        # Parse a model
        nsparser.ReadSMVFromFile(filepath)
        parsed_tree = nsparser.cvar.parsed_tree
        
        print("----------------------------------")
        print(filepath)
        print("----------------------------------")
            
        # Get the trans
        trs = self.transs(parsed_tree)
        for tr in trs:
            self.assertEqual(tr.type, nsparser.TRANS)
            
            print("----- TRANS ----------------------")
            print(nsnode.sprint_node(car(tr)))
            
            
    def test_counters(self):
        self.print_transs("tests/pynusmv/models/counters.smv")
        
    
    def test_counters_assign(self):
        self.print_transs("tests/pynusmv/models/counters-assign.smv")
        
            
    def test_admin(self):
        self.print_transs("tests/pynusmv/models/admin.smv")
        
    
    def test_mutex(self):
        self.print_transs("tests/pynusmv/models/mutex.smv")
        
            
    def test_simple_scheduler(self):
        self.print_transs("tests/pynusmv/models/simple-scheduler.smv")