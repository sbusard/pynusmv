import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from pynusmv.nusmv.dd import dd as nsdd

from tools.mas import glob

from tools.atlkPO.check import check
from tools.atlkFO.parsing import parseATLK


class TestCheck(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
    
    
    def little(self):
        glob.load_from_file("tests/tools/atlkPO/models/little.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def cardgame(self):
        glob.load_from_file("tests/tools/atlkPO/models/cardgame.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def cardgame_fair(self):
        glob.load_from_file("tests/tools/atlkPO/models/cardgame-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def cardgame_post_fair(self):
        glob.load_from_file("tests/tools/atlkPO/models/cardgame-post-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def trans2_fair(self):
        glob.load_from_file("tests/tools/atlkPO/models/2-transmission-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
    
    def transmission_post_fair(self):
        glob.load_from_file("tests/tools/atlkPO/models/transmission-post-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def show_si(self, fsm, bdd):
        if bdd.isnot_false():
            sis = fsm.pick_all_states_inputs(bdd)
            print("SI count:", len(sis))
            for si in sis:
                print(si.get_str_values())
            
    def show_s(self, fsm, bdd):
        if bdd.isnot_false():
            for s in fsm.pick_all_states(bdd):
                print(s.get_str_values())
            
    def show_i(self, fsm, bdd):
        if bdd.isnot_false():
            for i in fsm.pick_all_inputs(bdd):
                print(i.get_str_values())


    def test_cardgame(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0]))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0]))
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0]))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0]))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0]))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0]))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0]))
        
        
    
    def test_cardgame_post_fair(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0]))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0]))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0]))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0]))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0]))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0]))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0]))