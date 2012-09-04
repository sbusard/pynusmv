import unittest

from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.mc.mc import eval_simple_expression

from tools.multimodal import glob
from tools.multimodal.trans import BddTrans

from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.utils import utils as nsutils

class TestTrans(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
        
    def test_flattening(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        
        translist = glob.flatten_and_remove_trans()
        self.assertTrue(len(translist) > 0)
        
        #trans = translist[-2]
        #print(nsnode.sprint_node(trans))
        #print(trans.type) # 130 = CONTEXT
        #print(nsnode.car(trans).type) # 208 = DOT
        #print(nsnode.car(nsnode.car(trans))) # None
        #print(nsnode.cdr(nsnode.car(trans)).type) # 161 = ATOM
        #print(nsnode.sprint_node(nsnode.cdr(nsnode.car(trans)))) # c2
        #print(nsnode.cdr(trans).type) # 192 = EQUAL
                
        st = glob.symb_table()
        self.assertIsNotNone(st)
        
        glob.encode_variables()
        bddtranslist = []
        for trans in translist:
            context = nsnode.car(trans)
            
            #print(nsnode.sprint_node(context))
            #print(context.type)
            #print(nsnode.car(context))
            #print("context extra data:", context.extra_data)
            #print("context left node:", context.left.nodetype)
            #print("context left bdd:", context.left.bddtype)
            #print("context left int:", context.left.inttype)
            #print("context left str:", context.left.strtype)
            #print(nsnode.cdr(context).type)
            #print(nsutils.get_text(nsnode.node2string(nsnode.car(nsnode.cdr(context)))))            
            
            trans = nsnode.cdr(trans)
            bddtranslist.append(BddTrans.from_trans(st, trans, context))
            #bddtranslist.append(BddTrans.from_trans(st, trans))
            self.assertIsNotNone(bddtranslist[-1])
            
        self.assertEqual(len(translist), len(bddtranslist))
        
        glob.compute_model()
        
        fsm = glob.prop_database().master.bddFsm
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c2c1 = eval_simple_expression(fsm, "c2.c = 1")
        
        bddtrans = bddtranslist[-2] # c2.c TRANS
        next = bddtrans.post(fsm.init)
        self.assertEqual(next, c2c0 | c2c1)
        
        bddtrans = bddtranslist[-1] # c1.c TRANS
        prev = bddtrans.pre(c1c1)
        self.assertEqual(c1c0 | c1c1, prev)
        
        
    def test_from_string_without_context(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        
        glob.flatten_hierarchy()
        glob.compute_model()
        
        st = glob.symb_table()
        trans = "(next(c1.c) = c1.c)"
        bddtrans = BddTrans.from_string(st, trans)
        
        propDb = glob.prop_database() 
        master = propDb.master
        self.assertIsNotNone(master)
        self.assertIsNotNone(master._ptr)
        fsm = master.bddFsm
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c2c1 = eval_simple_expression(fsm, "c2.c = 1")
        
        next = bddtrans.post(c1c0)
        self.assertEqual(c1c0, next)
        
    
    def test_from_string_with_embedded_context(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        
        glob.flatten_hierarchy()
        glob.compute_model()
        
        st = glob.symb_table()
        trans = "(next(c) = c) IN c1"
        bddtrans = BddTrans.from_string(st, trans)
        
        propDb = glob.prop_database() 
        master = propDb.master
        self.assertIsNotNone(master)
        self.assertIsNotNone(master._ptr)
        fsm = master.bddFsm
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c2c1 = eval_simple_expression(fsm, "c2.c = 1")
        
        next = bddtrans.post(c1c0)
        self.assertEqual(c1c0, next)
        
        
    def test_from_string_with_context(self):
        glob.load_from_file("tests/pynusmv/models/counters.smv")
        
        glob.flatten_hierarchy()
        glob.compute_model()
        
        st = glob.symb_table()
        trans = "(next(c) = c)"
        context = "c1"
        
        bddtrans = BddTrans.from_string(st, trans, context)
        
        propDb = glob.prop_database() 
        master = propDb.master
        self.assertIsNotNone(master)
        self.assertIsNotNone(master._ptr)
        fsm = master.bddFsm
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c2c1 = eval_simple_expression(fsm, "c2.c = 1")
        
        next = bddtrans.post(c1c0)
        self.assertEqual(c1c0, next)
        