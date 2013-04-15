import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob
from pynusmv.mc import eval_simple_expression


class TestEval(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def model(self):
        glob.load_from_file("tests/pynusmv/models/cardgame-post-fair.smv")
        glob.compute_model()
        fsm = glob.prop_database().master.bddFsm
        self.assertIsNotNone(fsm)
        return fsm
        
    def show_si(self, fsm, bdd):
        for si in fsm.pick_all_states_inputs(bdd):
            print(si.get_str_values())
            
    def show_s(self, fsm, bdd):
        for s in fsm.pick_all_states(bdd):
            print(s.get_str_values())
            
    def show_i(self, fsm, bdd):
        for i in fsm.pick_all_inputs(bdd):
            print(i.get_str_values())


    def test_eval_and_mask(self):
        fsm = self.model()
        
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
        
        pan = eval_simple_expression(fsm, "player.action = none")
        pak = eval_simple_expression(fsm, "player.action = keep")
        pas = eval_simple_expression(fsm, "player.action = swap")
        
        dan = eval_simple_expression(fsm, "dealer.action = none")
        
        win = eval_simple_expression(fsm, "win")
        lose = eval_simple_expression(fsm, "lose")
        
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertNotEqual(pan, pan & fsm.bddEnc.statesInputsMask)
        self.assertNotEqual(s1, s1 & fsm.bddEnc.statesInputsMask)
        self.assertNotEqual(true, true & fsm.bddEnc.statesInputsMask)
        
        self.assertEqual((pan | s1) & fsm.bddEnc.statesInputsMask,
                         (pan & fsm.bddEnc.statesInputsMask) |
                         (s1 & fsm.bddEnc.statesInputsMask))
        
        pan_si = pan & fsm.bddEnc.statesInputsMask
        s1_si = s1 & fsm.bddEnc.statesInputsMask
        
        self.assertEqual(pan_si & s1_si, pan_si & s1_si & fsm.bddEnc.statesInputsMask)
        self.assertEqual(pan_si | s1_si, (pan_si | s1_si) & fsm.bddEnc.statesInputsMask)
        self.assertNotEqual(~pan_si, ~pan_si & fsm.bddEnc.statesInputsMask)
        self.assertNotEqual(~s1_si, ~s1_si & fsm.bddEnc.statesInputsMask)
        