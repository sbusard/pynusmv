import unittest

from tools.multimodal import glob
from pynusmv.glob import glob as superGlob
from pynusmv.init.init import init_nusmv, deinit_nusmv
from pynusmv.mc.mc import eval_simple_expression

from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.compile import compile as nscompile
from pynusmv.nusmv.parser import parser as nsparser

class TestFlattening(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
        
    def test_flattening(self):
        superGlob.load_from_file("tests/pynusmv/models/counters.smv")
        
        translist = glob.flatten_and_remove_trans()
        self.assertIsNotNone(translist)
        for tr in translist:
            self.assertIsNotNone(tr)
            self.assertEqual(tr.type, nsparser.CONTEXT)
        
        superGlob.compute_model()
        
        
        fsm = superGlob.prop_database().master.bddFsm
        c1c0 = eval_simple_expression(fsm, "c1.c = 0")
        c1c1 = eval_simple_expression(fsm, "c1.c = 1")
        c2c0 = eval_simple_expression(fsm, "c2.c = 0")
        true = eval_simple_expression(fsm, "TRUE")
        
        # Test on INIT
        self.assertEqual(c1c0 & c2c0, fsm.init)
        
        # Test on inner trans
        self.assertEqual(true, fsm.post(fsm.init))
        
        
        