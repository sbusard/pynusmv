import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from tools.mas import glob

from tools.atlkFO.check import check
from tools.atlkFO.parsing import parseATLK


class TestCheck(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
        
    def cardgame(self):
        glob.load_from_file("tests/tools/atlkFO/models/cardgame.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def transmission(self):
        glob.load_from_file("tests/tools/atlkFO/models/transmission.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
    
    def tictactoe(self):
        glob.load_from_file("tests/tools/atlkFO/models/tictactoe.smv")
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
        
        self.assertTrue(check(fsm, parseATLK("<'dealer'> X 'pcard=Ac'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'dealer'> G ~'win'")[0]))
        self.assertTrue(check(fsm, parseATLK("['player'] X 'pcard=Ac'")[0]))
        self.assertTrue(check(fsm, parseATLK("['dealer'] F 'win'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'player'> F 'win'")[0]))
        

    def test_transmission(self):
        fsm = self.transmission()
        
        self.assertFalse(check(fsm, parseATLK("<'sender'> F 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK(
                                           "<'transmitter'> G ~'received'")[0]))
        self.assertFalse(check(fsm, parseATLK("<'sender'> X 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK(
                                           "<'transmitter'> X ~'received'")[0]))
        self.assertFalse(check(fsm, parseATLK(
                                            "<'transmitter'> F 'received'")[0]))
        self.assertTrue(check(fsm, parseATLK("<'sender'> G ~'received'")[0]))

                             
                             
    def test_tictactoe(self):
        fsm = self.tictactoe()
        
        self.assertTrue(check(fsm, parseATLK(
                     "'run = circle' -> <'circlep'> X 'board[1] = circle'")[0]))
        self.assertTrue(check(fsm, parseATLK(
                           "'run = circle' -> ['circlep'] X 'run = cross'")[0]))
        self.assertTrue(check(fsm, parseATLK(
                             "<'circlep'> F 'run = stop'")[0]))
        self.assertTrue(check(fsm, parseATLK(
                        "<'circlep', 'crossp'> G 'winner = empty'")[0]))
        self.assertTrue(check(fsm, parseATLK(
                        "<'circlep'> G 'winner != circle'")[0]))