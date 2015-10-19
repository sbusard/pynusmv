import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from pynusmv.nusmv.dd import dd as nsdd

from tools.mas import glob

from tools.atlkPO.check import check
from tools.atlkFO.parsing import parseATLK
from tools.atlkPO import config

from pynusmv.model import *
from tools.mas.mas import Agent, Group


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
        glob.load_from_file("tests/tools/atlkPO/models/"
                            "2-transmission-fair.smv")
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
        glob.load_from_file("tests/tools/atlkPO/models/"
                            "transmission-post-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
    
    def coins(self):
        # Build model
        class main(Module):
            c1 = Var(Scalar(('none', 'head', 'tail')))
            c2 = Var(Scalar(('none', 'head', 'tail')))
            result = Var(Scalar(('none', 'win', 'lose')))

            act1 = IVar(Scalar(("different", "equal")))
            act2 = IVar(Scalar(("different", "equal")))

            INIT = [(c1 == 'none') & (c2 == 'none') & (result == 'none')]

            TRANS = []

            TRANS += [((c1 == 'none') & (c2 == 'none')).implies(
                       c1.next().in_(Set(('head', 'tail'))) &
                       c2.next().in_(Set(('head', 'tail'))) &
                       (result.next() == result))]
            TRANS += [((c1 != 'none') & (c2 != 'none') &
                       (result == 'none')).implies(
                      (c1.next() == c1) & (c2.next() == c2) &
                      (result.next() ==
                       (((c1 == c2) & (act1 == 'equal') & (act2 == 'equal')) |
                        ((c1 != c2) & (act1 == 'different') &
                         (act2 == 'different'))
                       ).ite('win', 'lose')))]
            TRANS += [(result != 'none').implies((result.next() == result) &
                       (c1.next( ) == c1) & (c2.next() == c2))]

        # Define agents
        a1 = Agent("a1", {main.c1}, {main.act1})
        a2 = Agent("a2", {main.c2}, {main.act2})
        g = Group("g", a1, a2)

        # Load model
        glob.load(main)

        # Get MAS
        mas = glob.mas({a1, a2, g})
        
        self.assertIsNotNone(mas)
        return mas
    
    def counters(self):
        # Build model
        class main(Module):
            v1 = Var(Boolean())
            v2 = Var(Boolean())

            act1 = IVar(Scalar(("skip", "inc")))
            act2 = IVar(Scalar(("skip", "inc")))

            TRANS = [v1.next() == (act1 == "skip").ite(v1, ~v1)]
            TRANS += [v2.next() == (act2 == "skip").ite(v2, ~v2)]

        # Define agents
        a1 = Agent("a1", {main.v1}, {main.act1})
        a2 = Agent("a2", {main.v2}, {main.act2})
        g = Group("g", a1, a2)

        # Load model
        glob.load(main)

        # Get MAS
        mas = glob.mas({a1, a2, g})
        
        self.assertIsNotNone(mas)
        return mas
        
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
    
    
    def test_transmission_early_filtering(self):
        fsm = self.transmission()
        
        config.partial.filtering = True
        
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        # False because transmitter cannot win if received is already true
        # and he has no clue about it
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        config.partial.filtering = False
        
    
    def test_transmission_with_know_early_filtering(self):
        fsm = self.transmission_with_knowledge()
        
        config.partial.filtering = True
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertFalse(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        config.partial.filtering = False
        
    
    def test_transmission_post_fair_early_filtering(self):
        fsm = self.transmission_post_fair()
        
        config.partial.filtering = True
        
        self.assertTrue(check(fsm, parseATLK("<'sender'> F 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> G ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> X ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'transmitter'> F 'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        # False because the sender does not know if the bit is already
        # transmitted (and in this case, no strategy can avoid 'received')
        self.assertFalse(check(fsm, parseATLK("<'sender'> G ~'received'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        config.partial.filtering = False
        
        
    def test_cardgame_early_filtering(self):
        fsm = self.cardgame()
        
        config.partial.filtering = True
        self.assertFalse(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="FS", implem="partialSI", semantics="individual"))        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        config.partial.filtering = False
        
        
    def test_cardgame_post_fair_early_filtering(self):
        fsm = self.cardgame_post_fair()
        
        config.partial.filtering = True
        
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~<'player'> X 'win')")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        self.assertTrue(check(fsm, parseATLK("K<'player'>'pcard=none' & K<'player'>'dcard=none'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("AG('step = 1' -> ~(K<'player'> 'dcard=Ac' | K<'player'> 'dcard=K' | K<'player'> 'dcard=Q'))")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        self.assertFalse(check(fsm, parseATLK("['dealer'] F 'win'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0], variant="FS", implem="partialSI", semantics="individual"))
        self.assertTrue(check(fsm, parseATLK("<'dealer'> G ~'win'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        # Player can win
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0], variant="FS", implem="partialSI", semantics="individual"))
        # Dealer can avoid fairness
        self.assertTrue(check(fsm, parseATLK("<'dealer'> F 'FALSE'")[0], variant="FS", implem="partialSI", semantics="individual"))
        
        config.partial.filtering = False