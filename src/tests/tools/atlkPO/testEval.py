import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from pynusmv.nusmv.dd import dd as nsdd

from tools.mas import glob

from tools.atlkPO.eval import split, cex_si


class TestEval(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
    
    
    def little(self):
        glob.load_from_file("tests/tools/atlkPO/little.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def cardgame(self):
        glob.load_from_file("tests/tools/atlkPO/cardgame.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def show_si(self, fsm, bdd):
        sis = fsm.pick_all_states_inputs(bdd)
        print("SI count:", len(sis))
        for si in sis:
            print(si.get_str_values())
            
    def show_s(self, fsm, bdd):
        for s in fsm.pick_all_states(bdd):
            print(s.get_str_values())
            
    def show_i(self, fsm, bdd):
        for i in fsm.pick_all_inputs(bdd):
            print(i.get_str_values())


    def test_split(self):
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
        
        ngamma_cube = (fsm.bddEnc.inputsCube -
                       fsm.inputs_cube_for_agents({"player"}))
        
        strats = split(fsm, fsm.protocol({"player"}), {"player"})
        
        self.assertEqual(len(strats), 8)
    
    
    def test_cex_si(self):
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
        
        pan = eval_simple_expression(fsm, "player.action = none")
        pak = eval_simple_expression(fsm, "player.action = keep")
        pas = eval_simple_expression(fsm, "player.action = swap")
        
        dan = eval_simple_expression(fsm, "dealer.action = none")
        
        win = eval_simple_expression(fsm, "win")
        lose = eval_simple_expression(fsm, "lose")
        
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        
        self.assertTrue(pk & dq & s2 & pan & fsm.reachable_states <= cex_si(fsm, {'player'}, fsm.reachable_states & win))
        self.assertTrue(pk & dq & s2 & pan & fsm.reachable_states <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pk & dq & pan))
        self.assertTrue(pk & dq & s1 & pak & fsm.reachable_states <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))
        self.assertFalse(pk & dq & s1 & pas & fsm.reachable_states <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))
        self.assertTrue(pa & dq & s1 & pas & fsm.reachable_states <= cex_si(fsm, {'player'}, fsm.reachable_states & win))
        self.assertFalse(pa & dq & s1 & pas & fsm.reachable_states <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))