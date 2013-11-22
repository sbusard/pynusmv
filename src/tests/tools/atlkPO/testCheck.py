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
        
    def transmission(self):
        glob.load_from_file("tests/tools/atlkPO/models/transmission.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def transmission_with_knowledge(self):
        glob.load_from_file(
                        "tests/tools/atlkPO/models/transmission-knowledge.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def transmission_fair(self):
        glob.load_from_file("tests/tools/atlkPO/models/transmission-fair.smv")
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
        
        
    def test_cardgame_not_improved(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0]))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0]))
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0]))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0]))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0]))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0]))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0]))
        
        self.assertTrue(check(fsm, parseATLK("EG ~'win'")[0]))
        self.assertTrue(check(fsm, parseATLK("EF 'win'")[0]))
        self.assertFalse(check(fsm, parseATLK("AF 'win'")[0]))
        
    
    @unittest.expectedFailure # MC algo does not deal with fairness on action for now
    def test_cardgame_fair_not_improved(self):
        fsm = self.cardgame_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0]))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0]))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0]))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0]))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0]))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0]))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0]))
        
    
    def test_cardgame_post_fair_not_improved(self):
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
        

    def test_transmission_not_improved(self):
        fsm = self.transmission()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0]))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0]))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0]))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0]))
        
    
    def test_transmission_with_know_not_improved(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0]))
    
    
    @unittest.expectedFailure # MC algo does not deal with fairness on action for now
    def test_transmission_fair_not_improved(self):
        fsm = self.transmission_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0]))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0]))
    
    
    def test_transmission_post_fair_not_improved(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0]))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0]))
        
        
    def test_transmission_improved(self):
        fsm = self.transmission()
        
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="SF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="SF"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="SF"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="SF"))
        
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="SF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="SF"))
        
    
    def test_transmission_with_know_improved(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="SF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="SF"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="SF"))
        
    
    def test_transmission_post_fair_improved(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="SF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="SF"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="SF"))
        
        
    def test_cardgame_improved(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="SF"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="SF"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="SF"))
        
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="SF"))
        
        
    @unittest.skip("Model checking takes too long.")
    def test_cardgame_post_fair_improved(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], variant="SF"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="SF"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="SF"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="SF"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="SF"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], variant="SF"))
        
    
    def test_transmission_FSF(self):
        fsm = self.transmission()
        
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF"))
        
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF"))
        
    def test_transmission_with_know_FSF(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF"))
        
    def test_transmission_post_fair_FSF(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF"))
        
        
    def test_cardgame_FSF(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="FSF"))
        
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="FSF"))
        
        
    @unittest.skip("Model checking takes too long.")
    def test_cardgame_post_fair_FSF(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], variant="FSF"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="FSF"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="FSF"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], variant="FSF"))
        

    def test_cardgame_not_improved_gen(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="generator"))
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="generator"))
        
    
    @unittest.expectedFailure # MC algo does not deal with fairness on action for now
    def test_cardgame_fair_not_improved_gen(self):
        fsm = self.cardgame_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="generator"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="generator"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="generator"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], implem="generator"))
        
    
    def test_cardgame_post_fair_not_improved_gen(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="generator"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="generator"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="generator"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], implem="generator"))
        

    def test_transmission_not_improved_gen(self):
        fsm = self.transmission()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="generator"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="generator"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="generator"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="generator"))
        
    
    def test_transmission_with_know_not_improved_gen(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="generator"))
    
    
    @unittest.expectedFailure # MC algo does not deal with fairness on action for now
    def test_transmission_fair_not_improved_gen(self):
        fsm = self.transmission_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="generator"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="generator"))
    
    
    def test_transmission_post_fair_not_improved_gen(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="generator"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="generator"))
        
        
    def test_transmission_improved_gen(self):
        fsm = self.transmission()
        
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="SF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="SF", implem="generator"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="SF", implem="generator"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="SF", implem="generator"))
        
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="SF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="SF", implem="generator"))
        
    
    def test_transmission_with_know_improved_gen(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="SF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="SF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="SF", implem="generator"))
        
    
    def test_transmission_post_fair_improved_gen(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="SF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="SF", implem="generator"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="SF", implem="generator"))
        
        
    def test_cardgame_improved_gen(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="SF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="SF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="SF", implem="generator"))
        
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="SF", implem="generator"))
        
        
    @unittest.skip("Model checking takes too long.")
    def test_cardgame_post_fair_improved_gen(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], variant="SF", implem="generator"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="SF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="SF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="SF", implem="generator"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="SF", implem="generator"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], variant="SF", implem="generator"))
        
    
    def test_transmission_FSF_gen(self):
        fsm = self.transmission()
        
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF", implem="generator"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF", implem="generator"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF", implem="generator"))
        
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF", implem="generator"))
        
    def test_transmission_with_know_FSF_gen(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF", implem="generator"))
        
    def test_transmission_post_fair_FSF_gen(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF", implem="generator"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF", implem="generator"))
        
        
    def test_cardgame_FSF_gen(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="FSF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="FSF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="FSF", implem="generator"))
        
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="FSF", implem="generator"))
        
        
    @unittest.skip("Model checking takes too long.")
    def test_cardgame_post_fair_FSF_gen(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], variant="FSF", implem="generator"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="FSF", implem="generator"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="FSF", implem="generator"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="FSF", implem="generator"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="FSF", implem="generator"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], variant="FSF", implem="generator"))
        
        
        
    def test_cardgame_not_improved_opt(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="optimized"))
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="optimized"))
        
    
    @unittest.expectedFailure # MC algo does not deal with fairness on action for now
    def test_cardgame_fair_not_improved_opt(self):
        fsm = self.cardgame_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="optimized"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="optimized"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="optimized"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], implem="optimized"))
        
    
    def test_cardgame_post_fair_not_improved_opt(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="optimized"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="optimized"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="optimized"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], implem="optimized"))
        

    def test_transmission_not_improved_opt(self):
        fsm = self.transmission()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="optimized"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="optimized"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="optimized"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="optimized"))
        
    
    def test_transmission_with_know_not_improved_opt(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="optimized"))
    
    
    @unittest.expectedFailure # MC algo does not deal with fairness on action for now
    def test_transmission_fair_not_improved_opt(self):
        fsm = self.transmission_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="optimized"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="optimized"))
    
    
    def test_transmission_post_fair_not_improved_opt(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="optimized"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="optimized"))
        
    
    def test_transmission_FSF_opt(self):
        fsm = self.transmission()
        
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF", implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF", implem="optimized"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF", implem="optimized"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF", implem="optimized"))
        
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF", implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF", implem="optimized"))
        
    def test_transmission_with_know_FSF_opt(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF", implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF", implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF", implem="optimized"))
        
    def test_transmission_post_fair_FSF_opt(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FSF", implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FSF", implem="optimized"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FSF", implem="optimized"))
        
        
    def test_cardgame_FSF_opt(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="FSF", implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="FSF", implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="FSF", implem="optimized"))
        
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="FSF", implem="optimized"))
        
        
    @unittest.skip("Model checking takes too long.")
    def test_cardgame_post_fair_FSF_opt(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], variant="FSF", implem="optimized"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="FSF", implem="optimized"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="FSF", implem="optimized"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="FSF", implem="optimized"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="FSF", implem="optimized"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], variant="FSF", implem="optimized"))
        
    
    def test_cardgame_not_improved_mem(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="memory"))
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="memory"))
        
    
    @unittest.expectedFailure # MC algo does not deal with fairness on action for now
    def test_cardgame_fair_not_improved_mem(self):
        fsm = self.cardgame_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="memory"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="memory"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="memory"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], implem="memory"))
        
    
    def test_cardgame_post_fair_not_improved_mem(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="memory"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="memory"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="memory"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], implem="memory"))
        

    def test_transmission_not_improved_mem(self):
        fsm = self.transmission()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="memory"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="memory"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="memory"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="memory"))
        
    
    def test_transmission_with_know_not_improved_mem(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="memory"))
    
    
    @unittest.expectedFailure # MC algo does not deal with fairness on action for now
    def test_transmission_fair_not_improved_mem(self):
        fsm = self.transmission_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="memory"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="memory"))
    
    
    def test_transmission_post_fair_not_improved_mem(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="memory"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="memory"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="memory"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="memory"))