import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from tools.multimodal import glob
from tools.multimodal.bddTrans import BddTrans
from pynusmv.mc.mc import eval_simple_expression
from pynusmv.glob import glob as pyglob

from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.parser import parser as nsparser

class TestFlattening(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
        
        
    def test_flattening(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        
        translist = glob._flatten_and_remove_trans()
        self.assertIsNotNone(translist)
        for tr in translist:
            self.assertIsNotNone(tr)
            self.assertEqual(tr.type, nsparser.CONTEXT)
        
        glob.compute_model()
        
        
        fsm = pyglob.prop_database().master.bddFsm
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        true = eval_simple_expression(fsm, "TRUE")
        
        # Test on INIT
        self.assertEqual(c1c0 & c2c0, fsm.init)
        
        # Test on inner trans
        self.assertEqual(true, fsm.post(fsm.init))
        
    
    def test_sort_trans_by_context(self):
        glob.load_from_file("tests/tools/multimodal/bitCounter.smv")
        
        translist = glob._flatten_and_remove_trans()
        self.assertIsNotNone(translist)
        
        glob.compute_model()
        st = pyglob.symb_table()
        
        transbymod = {}
        for trans in translist:
            context = nsnode.sprint_node(nsnode.car(trans))
            if context not in transbymod:
                transbymod[context] = None
            transbymod[context] = nsnode.find_node(nsparser.AND,
                                                   transbymod[context],
                                                   trans)
                     
        bddtrans = {}                              
        for cont in transbymod:
            bddtrans[cont] = BddTrans.from_trans(st, transbymod[cont], None)
            
        
        uptrans = bddtrans['up']
        lowtrans = bddtrans['low']
        maintrans = bddtrans['']
        
        fsm = pyglob.prop_database().master.bddFsm
        ll = eval_simple_expression(fsm, "low.low")
        lu = eval_simple_expression(fsm, "low.up")
        ul = eval_simple_expression(fsm, "up.low")
        uu = eval_simple_expression(fsm, "up.up")
        upinc = eval_simple_expression(fsm, "upinc")
        
        self.assertEqual(lowtrans.post(ll & lu), ~ll & ~lu)
        self.assertEqual(uptrans.post(ul & uu & upinc), ~ul & ~uu)
        self.assertEqual(maintrans.post(upinc), lu.imply(~upinc) | upinc)