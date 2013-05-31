import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from tools.mas import glob
from tools.atlkFO.eval import (evalATLK, cax, cag, cau, caw, fair_gamma_states)


class TestEval(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
    
    
    def transmission(self):
        glob.load_from_file("tests/tools/atlkFO/models/transmission.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    
    def transmission_fair(self):
        glob.load_from_file("tests/tools/atlkFO/models/transmission-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    
    def cardgame(self):
        glob.load_from_file("tests/tools/atlkFO/models/cardgame.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    
    def cardgame_fair(self):
        glob.load_from_file("tests/tools/atlkFO/models/cardgame-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def test_cax(self):
        fsm = self.cardgame()
        
        s0 = eval_simple_expression(fsm, "step = 0")
        s1 = eval_simple_expression(fsm, "step = 1")
        s2 = eval_simple_expression(fsm, "step = 2")
        pa = eval_simple_expression(fsm, "pcard = Ac")
        pk = eval_simple_expression(fsm, "pcard = K")
        pq = eval_simple_expression(fsm, "pcard = Q")
        da = eval_simple_expression(fsm, "dcard = Ac")
        dk = eval_simple_expression(fsm, "dcard = K")
        dq = eval_simple_expression(fsm, "dcard = Q")
        dda = eval_simple_expression(fsm, "ddcard = Ac")
        ddk = eval_simple_expression(fsm, "ddcard = K")
        ddq = eval_simple_expression(fsm, "ddcard = Q")
        win = eval_simple_expression(fsm, "win")
        lose = eval_simple_expression(fsm, "lose")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertTrue(s0 & fsm.reachable_states <= cax(fsm, {"player"}, s1))
        self.assertTrue(s1 & fsm.reachable_states <= cax(fsm, {"dealer"}, s2))
        
        self.assertFalse(s1 & pk & fsm.reachable_states <= cax(fsm, {"player"},
                                                               s2 & pk))
        self.assertTrue(s1 & dk <= cax(fsm, {"dealer"}, s2 & dk))