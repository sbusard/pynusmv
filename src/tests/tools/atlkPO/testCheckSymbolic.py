import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from pynusmv.nusmv.dd import dd as nsdd

from tools.mas import glob

from tools.atlkPO.check import check
from tools.atlkFO.parsing import parseATLK

from tools.atlkPO import config


class TestCheckSymbolic(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        #config.debug = True
        #config.garbage.type = "step"
        #config.garbage.step = 4
    
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
        
        
    def test_cardgame_not_improved_symbolic(self):
        fsm = self.cardgame()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="symbolic"))
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="symbolic"))
        
        self.assertTrue(check(fsm, parseATLK("EG ~'win'")[0],
                              implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("EF 'win'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("AF 'win'")[0], implem="symbolic"))
        
        self.assertFalse(check(fsm, parseATLK("<'player'> G <'player'> F 'win'")[0], implem="symbolic"))
        
        self.assertFalse(check(fsm, parseATLK("<'player'> F <'player'>[~'lose' U 'win']")[0], implem="symbolic"))
        
        self.assertFalse(check(fsm, parseATLK("AG <'player'> X 'pcard = Ac'")[0], implem="symbolic"))
    
    
    def test_cardgame_post_fair_not_improved_symbolic(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="symbolic"))
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="symbolic"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="symbolic"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], implem="symbolic"))
        
        self.assertTrue(check(fsm, parseATLK("EG ~'win'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("EF 'win'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("AF 'win'")[0], implem="symbolic"))
        
        self.assertTrue(check(fsm, parseATLK("<'player'> G <'player'> F 'win'")[0], implem="symbolic"))
        
        self.assertTrue(check(fsm, parseATLK("<'player'> F <'player'>[~'lose' U 'win']")[0], implem="symbolic"))
        

    def test_transmission_not_improved_symbolic(self):
        fsm = self.transmission()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="symbolic"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="symbolic"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="symbolic"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="symbolic"))
        
        self.assertTrue(check(fsm, parseATLK("EF 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("EG ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("AG ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("AF 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("AF ~'received'")[0], implem="symbolic"))
        
    
    def test_transmission_with_know_not_improved_symbolic(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="symbolic"))
    
    
    def test_transmission_post_fair_not_improved_symbolic(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="symbolic"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="symbolic"))
        
        self.assertTrue(check(fsm, parseATLK("EF 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("EG ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("AG ~'received'")[0], implem="symbolic"))
        self.assertFalse(check(fsm, parseATLK("AF 'received'")[0], implem="symbolic"))
        self.assertTrue(check(fsm, parseATLK("AF ~'received'")[0], implem="symbolic"))
    
    
    def test_cardgame_improved_symbolic(self):
        fsm = self.cardgame()
        
        self.assertFalse(check(fsm, parseATLK("<'player'> X 'win'")[0],
                         implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="symbolic", variant="FSF"))
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="symbolic", variant="FSF"))
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="symbolic", variant="FSF"))
        
        self.assertTrue(check(fsm, parseATLK("EG ~'win'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("EF 'win'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("AF 'win'")[0], implem="symbolic", variant="FSF"))
        
        self.assertFalse(check(fsm, parseATLK("<'player'> G <'player'> F 'win'")[0], implem="symbolic", variant="FSF"))
        
        self.assertFalse(check(fsm, parseATLK("<'player'> F <'player'>[~'lose' U 'win']")[0], implem="symbolic", variant="FSF"))
        
        self.assertFalse(check(fsm, parseATLK("AG <'player'> X 'pcard = Ac'")[0], implem="symbolic", variant="FSF"))
        
    
    def test_cardgame_post_fair_improved_symbolic(self):
        fsm = self.cardgame_post_fair()
        
        self.assertFalse(check(fsm, parseATLK("<'player'> X 'win'")[0], implem="symbolic", variant="FSF"))
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], implem="symbolic", variant="FSF"))
        
    def test_cardgame_post_fair_improved_symbolic2(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], implem="symbolic", variant="FSF"))
    
    def test_cardgame_post_fair_improved_symbolic3(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], implem="symbolic", variant="FSF"))
    
    def test_cardgame_post_fair_improved_symbolic4(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], implem="symbolic", variant="FSF"))
    
    def test_cardgame_post_fair_improved_symbolic5(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], implem="symbolic", variant="FSF"))
    
    def test_cardgame_post_fair_improved_symbolic6(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], implem="symbolic", variant="FSF"))
        
    def test_cardgame_post_fair_improved_symbolic7(self):
        fsm = self.cardgame_post_fair()
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], implem="symbolic", variant="FSF"))
    
    def test_cardgame_post_fair_improved_symbolic8(self):
        fsm = self.cardgame_post_fair()
        
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], implem="symbolic", variant="FSF"))
        
    def test_cardgame_post_fair_improved_symbolic9(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("EG ~'win'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("EF 'win'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("AF 'win'")[0], implem="symbolic", variant="FSF"))
        
    def test_cardgame_post_fair_improved_symbolic10(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'player'> G <'player'> F 'win'")[0], implem="symbolic", variant="FSF"))
        
    def test_cardgame_post_fair_improved_symbolic11(self):
        fsm = self.cardgame_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'player'> F <'player'>[~'lose' U 'win']")[0], implem="symbolic", variant="FSF"))
        
    
    def test_cardgame_post_fair_improved_symbolic12(self):
        fsm = self.cardgame_post_fair()
        
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], implem="symbolic", variant="FSF"))
        

    def test_transmission_improved_symbolic(self):
        fsm = self.transmission()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="symbolic", variant="FSF"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="symbolic", variant="FSF"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="symbolic", variant="FSF"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="symbolic", variant="FSF"))
        
        self.assertTrue(check(fsm, parseATLK("EF 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("EG ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("AG ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("AF 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("AF ~'received'")[0], implem="symbolic", variant="FSF"))
        
    
    def test_transmission_with_know_improved_symbolic(self):
        fsm = self.transmission_with_knowledge()
        
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="symbolic", variant="FSF"))
    
    
    def test_transmission_post_fair_improved_symbolic(self):
        fsm = self.transmission_post_fair()
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], implem="symbolic", variant="FSF"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], implem="symbolic", variant="FSF"))
        
        self.assertTrue(check(fsm, parseATLK("EF 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("EG ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("AG ~'received'")[0], implem="symbolic", variant="FSF"))
        self.assertFalse(check(fsm, parseATLK("AF 'received'")[0], implem="symbolic", variant="FSF"))
        self.assertTrue(check(fsm, parseATLK("AF ~'received'")[0], implem="symbolic", variant="FSF"))