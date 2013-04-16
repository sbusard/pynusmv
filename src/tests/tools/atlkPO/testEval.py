import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from pynusmv.nusmv.dd import dd as nsdd

from tools.mas import glob

from tools.atlkPO.eval import split, cex_si, nfair_gamma_si, nfair_gamma

from pynusmv.utils import fixpoint as fp


class TestEval(unittest.TestCase):
    
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
        
    def transmission_post_fair(self):
        glob.load_from_file("tests/tools/atlkPO/models/transmission-post-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def trans2_fair(self):
        glob.load_from_file("tests/tools/atlkPO/models/2-transmission-fair.smv")
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
            ss = fsm.pick_all_states(bdd)
            print("S count:", len(ss))
            for s in ss:
                print(s.get_str_values())
            
    def show_i(self, fsm, bdd):
        if bdd.isnot_false():
            iss = fsm.pick_all_inputs(bdd)
            print("I count:", len(iss))
            for i in iss:
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
        
        
        self.assertTrue(pk & dq & s2 & pan & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win))
        self.assertTrue(pk & dq & s2 & pan & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pk & dq & pan))
        self.assertTrue(pk & dq & s1 & pak & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))
        self.assertFalse(pk & dq & s1 & pas & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))
        self.assertTrue(pa & dq & s1 & pas & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win))
        self.assertFalse(pa & dq & s1 & pas & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))
        
    
    def test_nfair_gamma_si(self):
        fsm = self.cardgame_post_fair()
        
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
        
        
        agents = {'dealer'}
        strats = split(fsm, fsm.protocol(agents), agents)
        strat = strats.pop()
        nf = ~fsm.fairness_constraints[0] & fsm.bddEnc.statesInputsMask
        
        self.assertEqual(nf & fsm.pre_strat_si(BDD.true(fsm.bddEnc.DDmanager), agents, strat), nf & fsm.pre_strat_si(BDD.true(fsm.bddEnc.DDmanager), agents, strat) & fsm.bddEnc.statesInputsMask)
        
        nfp = nfair_gamma_si(fsm, {'player'})
        nfd = nfair_gamma_si(fsm, {'dealer'})
        
        self.assertTrue(nfp.is_false())
        self.assertTrue(fsm.protocol({'dealer'}) <= nfd)
        
        
    def test_nfair_gamma_si_trans2_fair(self):
        fsm = self.trans2_fair()
        
        #print("nfair_[transmitter] SI without strategies")
        #self.show_si(fsm, nfair_gamma_si(fsm, {'transmitter'}) &
        #                  fsm.protocol({'transmitter','sender'}))
        
        #print("nfair_[sender] SI without strategies")
        #self.show_si(fsm, nfair_gamma_si(fsm, {'sender'}))
        
        strats = split(fsm, fsm.protocol({'transmitter'}), {'transmitter'})
        strat = strats.pop()
        #print("nfair_[transmitter] SI in a uniform strategy")
        #self.show_si(fsm, nfair_gamma_si(fsm, {'transmitter'}, strat))
        
        # TODO Write a test
        
    
    def test_nfair_gamma(self):
        fsm = self.transmission_post_fair()
        
        transmit = eval_simple_expression(fsm, "transmitter.action = transmit")
        false = BDD.false(fsm.bddEnc.DDmanager)
        
        self.assertTrue(fsm.reachable_states <= nfair_gamma(fsm, {'transmitter'}))
        
        self.assertEqual(false, nfair_gamma(fsm, {'sender'}))
        
        strats = split(fsm, fsm.protocol({'transmitter'}), {'transmitter'})
        for strat in strats:
            if (strat & transmit).isnot_false():
                self.assertTrue(nfair_gamma(fsm, {'transmitter'}, strat).is_false())
            else:
                self.assertTrue(fsm.reachable_states <= nfair_gamma(fsm, {'transmitter'}, strat))
                
                
    def test_nfair_gamma_cardgame_post_fair(self):
        fsm = self.cardgame_post_fair()
        
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
        
        self.assertEqual(false, nfair_gamma(fsm, {'player'}))
        
        self.assertTrue(fsm.reachable_states  & fsm.bddEnc.statesInputsMask <= nfair_gamma(fsm, {'dealer'}))
        
        strats = split(fsm, fsm.protocol({'dealer'}), {'dealer'})
        for strat in strats:
            self.assertTrue(fsm.reachable_states & fsm.bddEnc.statesInputsMask <= nfair_gamma(fsm, {'dealer'}, strat))