import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from tools.mas import glob

from tools.atl.check import check
from tools.atl.parsing import parseATL


class TestCheck(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
    
    
    def small(self):
        glob.load_from_file("tests/tools/atl/models/small-game.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def cardgame(self):
        glob.load_from_file("tests/tools/atl/models/cardgame.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def cardgamenew(self):
        glob.load_from_file("tests/tools/atl/models/card-game3.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def transmission(self):
        glob.load_from_file("tests/tools/atl/models/transmission.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
    
    def tictactoe(self):
        glob.load_from_file("tests/tools/atl/models/tictactoe.smv")
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
        
        self.assertTrue(check(fsm, parseATL("<'dealer'> X 'pcard=Ac'")[0]))
        self.assertFalse(check(fsm, parseATL("<'dealer'> G ~'win'")[0]))
        self.assertTrue(check(fsm, parseATL("['player'] X 'pcard=Ac'")[0]))
        self.assertTrue(check(fsm, parseATL("['dealer'] F 'win'")[0]))
        self.assertTrue(check(fsm, parseATL("<'player'> F 'win'")[0]))
        
    
    def test_cardgame_new(self):
        fsm = self.cardgamenew()
        
        self.assertTrue(check(fsm, parseATL("<'dealer'>F 'player.state=1'")[0]))
        self.assertTrue(check(fsm, parseATL("<'dealer'>F 'player.state=2'")[0]))
        self.assertTrue(check(fsm, parseATL("<'dealer'>F 'player.state=3'")[0]))
        self.assertTrue(check(fsm, parseATL("['player']F 'player.state=1'")[0]))
        self.assertTrue(check(fsm, parseATL("<'player'>F 'playerwins'")[0]))
        self.assertFalse(check(fsm, parseATL("<'dealer'>F 'dealerwins'")[0]))
        

    def test_transmission(self):
        fsm = self.transmission()
        
        self.assertFalse(check(fsm, parseATL("<'sender'> F 'received'")[0]))
        self.assertTrue(check(fsm, parseATL(
                                           "<'transmitter'> G ~'received'")[0]))
        self.assertFalse(check(fsm, parseATL("<'sender'> X 'received'")[0]))
        self.assertTrue(check(fsm, parseATL(
                                           "<'transmitter'> X ~'received'")[0]))
        self.assertFalse(check(fsm, parseATL(
                                            "<'transmitter'> F 'received'")[0]))
        self.assertTrue(check(fsm, parseATL("<'sender'> G ~'received'")[0]))
        
    
    def test_small(self):
        fsm = self.small()
        
        self.assertFalse(check(fsm, parseATL(
                     "'run = p1' -> <'player1'> X 'FALSE'")[0]))
        self.assertTrue(check(fsm, parseATL(
                     "'run = p1' -> <'player1'> G ~'winner = p1'")[0]))
        self.assertTrue(check(fsm, parseATL(
                     "'run = p1' -> <'player1'> X 'board[1] = circle'")[0]))
        self.assertTrue(check(fsm, parseATL(
                     "'run = p1' -> <'player1'> X 'board[1] = cross'")[0]))
        self.assertTrue(check(fsm, parseATL(
                     "'run = p1' -> ['player2'] F 'winner = p1'")[0]))
        self.assertTrue(check(fsm, parseATL(
                     "'run = p1' -> <'player1'> F 'winner = p1'")[0]))
        self.assertTrue(check(fsm, parseATL(
                     "'run = p1' -> <'player1'> F 'winner = p2'")[0]))
        self.assertFalse(check(fsm, parseATL(
                     "'run = p2' -> <'player1'> F 'winner = p2'")[0]))
                             
                             
    def test_tictactoe(self):
        fsm = self.tictactoe()
        
        self.assertTrue(check(fsm, parseATL(
                     "'run = circle' -> <'circlep'> X 'board[1] = circle'")[0]))
        self.assertTrue(check(fsm, parseATL(
                           "'run = circle' -> ['circlep'] X 'run = cross'")[0]))
        self.assertTrue(check(fsm, parseATL(
                             "<'circlep'> F 'run = stop'")[0]))
        self.assertTrue(check(fsm, parseATL(
                        "<'circlep', 'crossp'> G 'winner = empty'")[0]))
        self.assertTrue(check(fsm, parseATL(
                        "<'circlep'> G 'winner != circle'")[0]))