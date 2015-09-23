import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from pynusmv.nusmv.dd import dd as nsdd

from tools.mas import glob

from tools.atlkPO.eval import split, cex_si, nfair_gamma_si, nfair_gamma
from tools.atlkPO.evalPartial import reach, split_reach, split as psplit

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
        
    def tree(self):
        glob.load_from_file("tests/tools/atlkPO/models/tree.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def collapsed_tree(self):
        glob.load_from_file("tests/tools/atlkPO/models/collapsed-tree.smv")
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
        glob.load_from_file(
                        "tests/tools/atlkPO/models/transmission-post-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def trans2_fair(self):
        glob.load_from_file("tests/tools/atlkPO/models/2-transmission-fair.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def nfair_model(self):
        glob.load_from_file("tests/tools/atlkPO/models/nfair.smv")
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
        daak = eval_simple_expression(fsm, "dealer.action = dealAK")
        
        win = eval_simple_expression(fsm, "win")
        lose = eval_simple_expression(fsm, "lose")
        
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        dealercube = fsm.inputs_cube_for_agents({'dealer'})
        playercube = fsm.inputs_cube_for_agents({'player'})
        
        
        self.assertTrue(pk & dq & s2 & pan & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win))
        self.assertTrue(pk & dq & s2 & pan & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pk & dq & pan))
        self.assertTrue(pk & dq & s1 & pak & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))
        self.assertFalse(pk & dq & s1 & pas & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))
        self.assertTrue(pa & dq & s1 & pas & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win))
        self.assertFalse(pa & dq & s1 & pas & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= cex_si(fsm, {'player'}, fsm.reachable_states & win, pak))
        
        strat = (s0 & daak & fsm.protocol({'dealer'})) | ((s2) & fsm.protocol({'dealer'}))
        self.assertTrue(nfair_gamma_si(fsm, {'dealer'}, strat).is_false())
        self.assertEqual(pa, nfair_gamma_si(fsm, {'dealer'}, strat) | pa)
        cexsi = cex_si(fsm, {'dealer'}, fsm.reachable_states & pa, strat)
        self.assertTrue(fsm.init <= cexsi.forsome(fsm.bddEnc.inputsCube))
        
        
    
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
        
        
    def test_nfair_gamma_si_transmission_post_fair(self):
        fsm = self.transmission_post_fair()
        
        transmit = eval_simple_expression(fsm, "transmitter.action = transmit")
        block = eval_simple_expression(fsm, "transmitter.action = block")
        wait = eval_simple_expression(fsm, "sender.action = wait")
        send = eval_simple_expression(fsm, "sender.action = send")
        
        received = eval_simple_expression(fsm, "received")
        sent = eval_simple_expression(fsm, "sent")
        waited = eval_simple_expression(fsm, "waited")
        transmitted = eval_simple_expression(fsm, "transmitted")
        blocked = eval_simple_expression(fsm, "blocked")
        
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        nfgsit = nfair_gamma_si(fsm, {'transmitter'})
        self.assertTrue(fsm.protocol({'transmitter'}) <= nfgsit)
        
        nfgsis = nfair_gamma_si(fsm, {'sender'})
        self.assertEqual(false, nfgsis)
        
    
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
        
        self.assertTrue(fsm.reachable_states & fsm.bddEnc.statesInputsMask <= nfair_gamma(fsm, {'dealer'}))
        
        strats = split(fsm, fsm.protocol({'dealer'}), {'dealer'})
        for strat in strats:
            self.assertTrue(fsm.reachable_states & fsm.bddEnc.statesInputsMask <= nfair_gamma(fsm, {'dealer'}, strat))
            
            
    def test_nfair_si_nfair_model(self):
        fsm = self.nfair_model()
        
        s0 = eval_simple_expression(fsm, "state = s0")
        s1 = eval_simple_expression(fsm, "state = s1")
        s2 = eval_simple_expression(fsm, "state = s2")
        
        a0 = eval_simple_expression(fsm, "a.a = 0")
        a1 = eval_simple_expression(fsm, "a.a = 1")
        a2 = eval_simple_expression(fsm, "a.a = 2")
        
        b0 = eval_simple_expression(fsm, "b.a = 0")
        b1 = eval_simple_expression(fsm, "b.a = 1")
        
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        nfgsia = nfair_gamma_si(fsm, {'a'})
        nfgsib  = nfair_gamma_si(fsm, {'b'})
        
        self.assertTrue(s0 & a1 & fsm.protocol({'a'}) <= nfgsia)
        self.assertTrue(s1 & a0 & fsm.protocol({'a'}) <= nfgsia)
        self.assertEqual((s0 & a1 & fsm.protocol({'a'})) |
                         (s1 & a0 & fsm.protocol({'a'})), nfgsia)
                         
        self.assertEqual(s1 & b0 & fsm.protocol({'b'}), nfgsib)
        
        
    def test_splitreach_tree(self):
        fsm = self.tree()
        agents = {"a"}
        
        s0 = eval_simple_expression(fsm, "a.state = 0")
        s1 = eval_simple_expression(fsm, "a.state = 1")
        s2 = eval_simple_expression(fsm, "a.state = 2")
        s3 = eval_simple_expression(fsm, "a.state = 3")
        s4 = eval_simple_expression(fsm, "a.state = 4")
        s5 = eval_simple_expression(fsm, "a.state = 5")
        s6 = eval_simple_expression(fsm, "a.state = 6")
        
        aa = eval_simple_expression(fsm, "a.action = a")
        ab = eval_simple_expression(fsm, "a.action = b")
        
        splitted_init = psplit(fsm, fsm.init & fsm.protocol(agents), agents)
        strats = {strat for pustrat in splitted_init
                        for strat in split_reach(fsm, agents, pustrat)}
        self.assertEqual(len(strats), 4)
        self.assertTrue(((s0 & aa) | (s1 & aa) | (s3 & aa))
                         & fsm.bddEnc.statesMask in strats)
        self.assertTrue(((s0 & aa) | (s1 & ab) | (s4 & aa))
                        & fsm.bddEnc.statesMask in strats)
        self.assertTrue(((s0 & ab) | (s2 & aa) | (s5 & aa))
                        & fsm.bddEnc.statesMask in strats)
        self.assertTrue(((s0 & ab) | (s2 & ab) | (s6 & aa))
                        & fsm.bddEnc.statesMask in strats)
                        
                        
    def test_splitreach_collapsed_tree(self):
        fsm = self.collapsed_tree()
        agents = {"a"}
        
        sa0 = eval_simple_expression(fsm, "a.state = 0")
        sa1 = eval_simple_expression(fsm, "a.state = 1")
        
        s0 = eval_simple_expression(fsm, "s = 0")
        s1 = eval_simple_expression(fsm, "s = 1")
        s2 = eval_simple_expression(fsm, "s = 2")
        
        aa = eval_simple_expression(fsm, "a.action = a")
        ab = eval_simple_expression(fsm, "a.action = b")
        
        splitted_init = psplit(fsm, fsm.init & fsm.protocol(agents), agents)
        strats = {strat for pustrat in splitted_init
                        for strat in split_reach(fsm, agents, pustrat)}
        self.assertEqual(len(strats), 2)
        
        
    def test_reach_cardgame(self):
        fsm = self.cardgame()
        self.assertEqual(fsm.reachable_states, reach(fsm, fsm.init))