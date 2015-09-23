import unittest

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression
from pynusmv.dd import BDD
from pynusmv import model as md

from tools.mas import glob
from tools.mas.mas import Agent, Group

class TestMAS(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        glob.reset_globals()
        deinit_nusmv()
        
    def model(self):
        glob.load_from_file("tests/tools/mas/dining-crypto.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    
    def cardgame(self):
        glob.load_from_file("tests/tools/mas/cardgame.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def simple(self):
        glob.load_from_file("tests/tools/mas/simple.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def premod(self):
        glob.load_from_file("tests/tools/mas/pre.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
        
    def test_pre(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        unknown = eval_simple_expression(fsm, "countsay = unknown")
        odd = eval_simple_expression(fsm, "countsay = odd")
        even = eval_simple_expression(fsm, "countsay = even")
        true = eval_simple_expression(fsm, "TRUE")
        
        self.assertTrue(odd & fsm.bddEnc.statesMask <= fsm.pre(odd))
        self.assertTrue(fsm.init <= unknown)
        self.assertTrue(fsm.pre(unknown).is_false())
        
        
    def test_simple_pre(self):
        glob.load_from_file("tests/tools/ctlk/agents.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertEqual(fsm.pre(lt & ~lf & ~g),
                         (lt & lf & ~g) | (~lt & ~lf & g))
        self.assertEqual(fsm.pre(g), true)
        self.assertEqual(fsm.pre(lf), lf.iff(g))
        
        
    def test_count(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        c3p = eval_simple_expression(fsm, "c3.payer")
        c1ch = eval_simple_expression(fsm, "c1.coin = head")
        c2ch = eval_simple_expression(fsm, "c2.coin = head")
        c3ch = eval_simple_expression(fsm, "c3.coin = head")
        c1se = eval_simple_expression(fsm, "c1.say = equal")
        c2se = eval_simple_expression(fsm, "c2.say = equal")
        c3se = eval_simple_expression(fsm, "c3.say = equal")        
        unknown = eval_simple_expression(fsm, "countsay = unknown")
        odd = eval_simple_expression(fsm, "countsay = odd")
        even = eval_simple_expression(fsm, "countsay = even")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertEqual(1, fsm.count_states(fsm.pick_one_state(c1p)))
        
        self.assertEqual(1, fsm.count_states(c1p & ~c2p & ~c3p & c1ch & c2ch & c3ch & unknown))
        self.assertEqual(1, fsm.count_states(c1p & ~c2p & ~c3p & c1ch & c2ch & c3ch & odd))
        
        
    def test_constraints_post(self):
        glob.load_from_file("tests/tools/ctlk/constraints.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        false = eval_simple_expression(fsm, "FALSE")
        false = eval_simple_expression(fsm, "TRUE")
        p = eval_simple_expression(fsm, "p")
        q = eval_simple_expression(fsm, "q")
        a = eval_simple_expression(fsm, "a")
        
        self.assertEqual(1, fsm.count_states(fsm.init))
        self.assertEqual(2, fsm.count_states(fsm.post(fsm.init)))
        self.assertEqual(1, fsm.count_states(p & q))
        self.assertEqual(1, fsm.count_states(p & ~q))
        self.assertEqual(1, fsm.count_states(~p & q))
        self.assertEqual(1, fsm.count_states(~p & ~q))
        
        self.assertEqual(2, fsm.count_states(fsm.post(p & q)))
        self.assertEqual(1, fsm.count_states(fsm.post(p & ~q)))
        self.assertEqual(1, fsm.count_states(fsm.post(~p & q)))
        self.assertEqual(1, fsm.count_states(fsm.post(p & q, a)))
        self.assertEqual(0, fsm.count_states(fsm.post(p & ~q, ~a)))
        self.assertEqual(0, fsm.count_states(fsm.post(~p & q, a)))
        self.assertEqual(1, fsm.count_states(fsm.post(~p & q, ~a)))
                
        
    def test_simple_post(self):
        glob.load_from_file("tests/tools/ctlk/agents.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        lt = eval_simple_expression(fsm, "at.local")
        lf = eval_simple_expression(fsm, "af.local")
        g = eval_simple_expression(fsm, "global")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertEqual(fsm.post(lt & lf & g), ~lt & lf & g | ~lt & lf & ~g)
        self.assertEqual(fsm.post(lt & g), ~lt)
        self.assertEqual(fsm.post(lt & ~g), lt)
        self.assertEqual(fsm.post(lt & lf), lt.iff(~lf))
        
        
    def test_equivalent_states(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        true = eval_simple_expression(fsm, "TRUE")
        
        self.assertEqual(fsm.equivalent_states(c1p, {"c1"}), c1p)
        self.assertEqual(fsm.equivalent_states(c1p, {"c2"}), true)
        
        
    def test_reachable_states_for_simple_model(self):
        glob.load_from_file("tests/tools/ctlk/agents.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        
        true = eval_simple_expression(fsm, "TRUE")
        self.assertEqual(fsm.reachable_states, true)
        
        
    def test_reachable_states(self):
        fsm = self.model()
        
        c1p = eval_simple_expression(fsm, "c1.payer")
        c2p = eval_simple_expression(fsm, "c2.payer")
        odd = eval_simple_expression(fsm, "countsay = odd")
        true = eval_simple_expression(fsm, "TRUE")
        
        self.assertTrue(fsm.reachable_states.isnot_true())
        self.assertTrue(fsm.reachable_states.isnot_false())
        self.assertTrue((fsm.init & c1p & c2p).is_false())
        self.assertTrue((fsm.post(fsm.init) & c1p & c2p).is_false())
        
        tmp = fsm.reachable_states & (c1p & c2p)
        while tmp.isnot_false():
            s = fsm.pick_one_state(tmp)
            print(s.get_str_values())
            tmp -= s
        
        self.assertTrue((fsm.reachable_states & (c1p & c2p)).is_false())
        
        
    def test_agents(self):
        fsm = self.model()
        self.assertSetEqual({'c1','c2','c3'}, fsm.agents)
        
        
    def test_observed_variables(self):
        fsm = self.model()
        
        self.assertSetEqual({'c1','c2','c3'},
                            set(fsm.agents_observed_variables.keys()))
        
        self.assertSetEqual(fsm.agents_observed_variables['c1'],
                            {'c1.coin', 'c1.payer', 'c3.coin', 'countsay'})
        self.assertSetEqual(fsm.agents_observed_variables['c2'],
                            {'c2.coin', 'c2.payer', 'c1.coin', 'countsay'})
        self.assertSetEqual(fsm.agents_observed_variables['c3'],
                            {'c3.coin', 'c3.payer', 'c2.coin', 'countsay'})
        
        
    def test_input_variables(self):
        fsm = self.model()
        
        self.assertSetEqual({'c1','c2','c3'}, set(fsm.agents_inputvars.keys()))
        
        self.assertSetEqual(fsm.agents_inputvars['c1'], {'c1.say'})
        self.assertSetEqual(fsm.agents_inputvars['c2'], {'c2.say'})
        self.assertSetEqual(fsm.agents_inputvars['c3'], {'c3.say'})
        
        
    def test_protocol(self):
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
        dak = eval_simple_expression(fsm, "dealer.action = dealAK")
        dn = eval_simple_expression(fsm, "dealer.action = none")
        ps = eval_simple_expression(fsm, "player.action = swap")
        pk = eval_simple_expression(fsm, "player.action = keep")
        pn = eval_simple_expression(fsm, "player.action = none")
        
        self.assertTrue((s0 & dak & fsm.protocol({"dealer"})).isnot_false())
        self.assertTrue((s0 & dak & fsm.reachable_states &
                         fsm.bddEnc.statesInputsMask) <= fsm.protocol({"dealer"}))
        self.assertFalse((s0 & dn & fsm.reachable_states &
                         fsm.bddEnc.statesInputsMask) <= fsm.protocol({"dealer"}))
        
        self.assertTrue((s0 & pn & fsm.reachable_states &
                         fsm.bddEnc.statesInputsMask) <= fsm.protocol({"player"}))
        self.assertTrue((s1 & pk & fsm.reachable_states) &
                         fsm.bddEnc.statesInputsMask <= fsm.protocol({"player"}))
        self.assertTrue((s2 & pn & fsm.reachable_states &
                         fsm.bddEnc.statesInputsMask) <= fsm.protocol({"player"}))
        self.assertTrue((s1 & ps & fsm.reachable_states &
                         fsm.bddEnc.statesInputsMask) <= fsm.protocol({"player"}))
        self.assertFalse((s1 & pn & fsm.reachable_states &
                         fsm.bddEnc.statesInputsMask) <= fsm.protocol({"player"}))
    
    
    def test_pre_strat(self):
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
        
        self.assertTrue(pa & dq & s2 & fsm.reachable_states  & fsm.bddEnc.statesInputsMask <= fsm.pre_strat(pa & dq & s2 & fsm.reachable_states, {'player'}))
        self.assertTrue(pa & dq & s1 & fsm.reachable_states  & fsm.bddEnc.statesInputsMask<= fsm.pre_strat(pa & dq & s2 & fsm.reachable_states, {'player'}))
        self.assertTrue(pk & dq & s1 & fsm.reachable_states  & fsm.bddEnc.statesInputsMask<= fsm.pre_strat(pa & dq & s2 & fsm.reachable_states, {'player'}))
        self.assertFalse(pk & da & s1 & fsm.reachable_states  & fsm.bddEnc.statesInputsMask<= fsm.pre_strat(pa & dq & s2 & fsm.reachable_states, {'player'}))
        
        self.assertTrue(s1 & fsm.reachable_states  & fsm.bddEnc.statesInputsMask<=
                        fsm.pre_strat(win & fsm.reachable_states, {'player'}))
        self.assertTrue(s1 & fsm.reachable_states  & fsm.bddEnc.statesInputsMask<=
                                                fsm.pre_strat(lose, {'player'}))
        
        self.assertTrue(fsm.init & fsm.bddEnc.statesInputsMask <= fsm.pre_strat(s1, {'dealer'}))
        self.assertTrue(fsm.init & fsm.bddEnc.statesInputsMask <= fsm.pre_strat(s1 & pq & da, {'dealer'}))
        
        self.assertFalse(fsm.init & fsm.bddEnc.statesInputsMask <= fsm.pre_strat(s1 & pq & da, {'player'}))
        
    
    def test_simple(self):
        mas = self.simple()
        s4 = eval_simple_expression(mas, "s = 4")
        ac = mas.bddEnc.cube_for_inputs_vars("a")
        bc = mas.bddEnc.cube_for_inputs_vars("b")
        self.assertTrue((~(mas.weak_pre(~s4).forsome(bc)) &     
                        mas.weak_pre(s4)).forsome(mas.bddEnc.inputsCube))
                        
                        
    def test_pre_nstat(self):
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
        
        self.assertTrue(s0 & fsm.reachable_states <=
                        fsm.pre_nstrat(s1 & pa, {"player"}))
        self.assertTrue(s1 & fsm.reachable_states <=
                        fsm.pre_nstrat(win, {"dealer"}))
        self.assertFalse(s1 & fsm.reachable_states <=
                        fsm.pre_nstrat(win, {"player"}))
                        
                        
    def test_pre_stat_si(self):
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
        daqa = eval_simple_expression(fsm, "dealer.action = dealQA")
        win = eval_simple_expression(fsm, "win")
        lose = eval_simple_expression(fsm, "lose")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        self.assertTrue(pa & dq & s2 & pan & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= fsm.pre_strat_si(pa & dq & s2 & fsm.reachable_states, {'player'}))
        self.assertTrue(pa & dq & s1 & pak & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= fsm.pre_strat_si(pa & dq & s2 & fsm.reachable_states, {'player'}))
        self.assertTrue(pk & dq & s1 & pas & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= fsm.pre_strat_si(pa & dq & s2 & fsm.reachable_states, {'player'}))
        self.assertFalse(pk & dq & s1 & pak & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= fsm.pre_strat_si(pa & dq & s2 & fsm.reachable_states, {'player'}))
        self.assertFalse(pk & da & s1 & fsm.reachable_states & fsm.bddEnc.statesInputsMask <= fsm.pre_strat_si(pa & dq & s2 & fsm.reachable_states, {'player'}))
        
        self.assertFalse(s1 & fsm.reachable_states & fsm.bddEnc.statesInputsMask <=
                         fsm.pre_strat_si(win & fsm.reachable_states, {'player'}))
        self.assertFalse(s1 & fsm.reachable_states & fsm.bddEnc.statesInputsMask <=
                         fsm.pre_strat_si(lose, {'player'}))
                         
        self.assertTrue(win & fsm.reachable_states & fsm.bddEnc.statesInputsMask & fsm.protocol({'player'}) <=
                        fsm.pre_strat_si(win & fsm.reachable_states, {'player'}))
        self.assertTrue(s1 & pk & dq & pas & fsm.reachable_states & fsm.bddEnc.statesInputsMask <=
                        fsm.pre_strat_si(lose, {'player'}))
        
        self.assertTrue(fsm.init & fsm.protocol({'dealer'}) & fsm.bddEnc.statesInputsMask <= fsm.pre_strat_si(s1, {'dealer'}))
        self.assertTrue(fsm.init & daqa & fsm.bddEnc.statesInputsMask <= fsm.pre_strat_si(s1 & pq & da, {'dealer'}))
        
        
    def test_premod_pre_strat_si(self):
        fsm = self.premod()
        
        p = eval_simple_expression(fsm, "a.p = 1")
        q = eval_simple_expression(fsm, "b.q = 1")
        pa = eval_simple_expression(fsm, "a.a = 1")
        qa = eval_simple_expression(fsm, "b.a = 1")
        
        aa = {'a'}
        bb = {'b'}
        ag = {'a', 'b'}
        
        self.assertEqual((~p & q) | (p & ~q & ~pa), fsm.pre_strat_si(~p & q, aa))
        self.assertTrue(fsm.pre_strat_si(p, ag).is_false())
        self.assertEqual(fsm.pre_strat_si(~p & q, bb), (~p & q) | (p & q & ~qa))
        
        strat = (pa)
        
        self.assertEqual((~p & q & pa), fsm.pre_strat_si(~p & q, aa, strat))
        self.assertEqual(fsm.pre_strat_si(~p & q, bb, strat),
                         ((~p & q) | (p & q & ~qa)) & strat)
        self.assertEqual(fsm.pre_strat_si(~p & q, bb, qa),
                         (~p & q & qa))
    
    def test_post(self):
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
        pan = eval_simple_expression(fsm, "player.action = none")
        pak = eval_simple_expression(fsm, "player.action = keep")
        pas = eval_simple_expression(fsm, "player.action = swap")
        daqa = eval_simple_expression(fsm, "dealer.action = dealQA")
        
        self.assertEqual(fsm.post(s0 & fsm.reachable_states) &
                         fsm.bddEnc.statesMask,
                         s1 & fsm.reachable_states & fsm.bddEnc.statesMask)
        self.assertEqual(fsm.post(s0 & fsm.reachable_states, inputs=daqa) &
                         fsm.bddEnc.statesMask,
                         s1 & pq & da & fsm.reachable_states &
                         fsm.bddEnc.statesMask)
        self.assertEqual(fsm.post(s0 & fsm.reachable_states,
                                  subsystem=s0 & daqa) & fsm.bddEnc.statesMask,
                         s1 & pq & da & fsm.reachable_states &
                         fsm.bddEnc.statesMask)
        self.assertEqual(fsm.post(fsm.reachable_states,
                                  subsystem=(s0 & daqa) | (s1 & pak)) &
                         fsm.bddEnc.statesMask,
                         ((s1 & pq & da) | (s2)) & fsm.reachable_states &
                         fsm.bddEnc.statesMask)
        self.assertEqual(fsm.post(s1 & fsm.reachable_states,
                        subsystem=(s1 & pa & dq & pak) | (s1 & pk & da & pas))
                         & fsm.bddEnc.statesMask,
                         ((s2 & pa & dq) | (s2 & da & pq)) &
                         fsm.reachable_states &
                         fsm.bddEnc.statesMask)
    
    def test_cardgame_with_agents(self):
        glob.load_from_file("tests/tools/mas/cardgame.smv")
        agents = (Agent("player", {"step", "pcard", "ddcard"},
                        {"player.action"}),
                  Agent("dealer", {"step", "pcard", "dcard"},
                        {"dealer.action"}))
        fsm = glob.mas(agents=agents)
        self.assertIsNotNone(fsm)
        return fsm
    
    def test_check_mas_free_choice_ok(self):
        class agent(md.Module):
            v = md.Var(md.Boolean())
            a = md.IVar(md.Boolean())
            INIT = v
            TRANS = (v.next() == a)
        class main(md.Module):
            a1 = md.Var(agent())
            a2 = md.Var(agent())
        
        agents = {Agent("a1", {main.a1.v, main.a2.v}, {main.a1.a}),
                  Agent("a2", {main.a1.v, main.a2.v}, {main.a2.a})}
        
        glob.load(agent, main)
        mas = glob.mas(agents=agents)
        self.assertIsNotNone(mas)
        self.assertTrue(mas.check_free_choice().is_false())
    
    def test_check_mas_free_choice_ko(self):
        class agent(md.Module):
            v = md.Var(md.Boolean())
            a = md.IVar(md.Boolean())
            INIT = v
            TRANS = (v.next() == a)
        class main(md.Module):
            a1 = md.Var(agent())
            a2 = md.Var(agent())
            TRANS = a2.a.implies(a1.a)
        
        agents = {Agent("a1", {main.a1.v, main.a2.v}, {main.a1.a}),
                  Agent("a2", {main.a1.v, main.a2.v}, {main.a2.a})}
        
        glob.load(agent, main)
        mas = glob.mas(agents=agents)
        self.assertIsNotNone(mas)
        
        free_choice = mas.check_free_choice()
        self.assertTrue(free_choice.isnot_false())
    
    def test_check_mas_uniform_choice_ok(self):
        class agent(md.Module):
            v = md.Var(md.Boolean())
            a = md.IVar(md.Boolean())
            INIT = v
            TRANS = (v.next() == a)
        class main(md.Module):
            a1 = md.Var(agent())
            a2 = md.Var(agent())
        
        agents = {Agent("a1", {main.a1.v}, {main.a1.a}),
                  Agent("a2", {main.a2.v}, {main.a2.a})}
        
        glob.load(agent, main)
        mas = glob.mas(agents=agents)
        self.assertIsNotNone(mas)
        self.assertTrue(mas.check_uniform_choice().is_false())
    
    def test_check_mas_uniform_choice_ko(self):
        class agent(md.Module):
            v = md.Var(md.Boolean())
            a = md.IVar(md.Boolean())
            INIT = v
            TRANS = (v.next() == a)
        class main(md.Module):
            a1 = md.Var(agent())
            a2 = md.Var(agent())
            TRANS = (a1.a == a2.v)
        
        agents = {Agent("a1", {main.a1.v}, {main.a1.a}),
                  Agent("a2", {main.a2.v}, {main.a2.a})}
        
        glob.load(agent, main)
        mas = glob.mas(agents=agents)
        self.assertIsNotNone(mas)
        
        uniform_choice = mas.check_uniform_choice()
        self.assertTrue(uniform_choice.isnot_false())