import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from pynusmv.nusmv.dd import dd as nsdd

from tools.mas import glob

from tools.atlkPO.eval import split
from tools.atlkPO.evalPartial import split as split_partial


class TestSplit(unittest.TestCase):
    
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
        
    def cardgame3(self):
        glob.load_from_file("tests/tools/atlkPO/models/cardgame3.smv")
        fsm = glob.mas()
        self.assertIsNotNone(fsm)
        return fsm
        
    def show_si(self, fsm, bdd):
        for si in fsm.pick_all_states_inputs(bdd):
            print(si.get_str_values())
            
    def show_s(self, fsm, bdd):
        for s in fsm.pick_all_states(bdd):
            print(s.get_str_values())
            
    def show_i(self, fsm, bdd):
        for i in fsm.pick_all_inputs(bdd):
            print(i.get_str_values())
    
    @unittest.skip    
    def test_split_by_picking(self):
        fsm = self.little()
        
        aa = eval_simple_expression(fsm, "a.a = 1")
        ap = eval_simple_expression(fsm, "a.p = 1")
        ba = eval_simple_expression(fsm, "b.a = 1")
        bq = eval_simple_expression(fsm, "b.q = 1")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        excluded = BDD.false(fsm.bddEnc.DDmanager)
        
        strats = fsm.protocol({"a"})
        print("protocol of a -- strats to consider")
        self.show_si(fsm, strats)
        si = fsm.pick_one_state_inputs(strats)
        self.assertEqual(len(fsm.pick_all_states(
                                            si.forsome(fsm.bddEnc.inputsCube))),
                         1)
        s = fsm.pick_one_state(si.forsome(fsm.bddEnc.inputsCube))
        print("si:", si.get_str_values())
        print("s:", s.get_str_values())
        eqs = fsm.equivalent_states(s, {"a"})
        print("equivalent to", s.get_str_values())
        self.show_s(fsm, eqs)
        eqcl = strats & eqs
        print("equivalence class (" + str(s.get_str_values()) + ")")
        self.show_si(fsm, eqcl)
        act = eqcl.forsome(fsm.bddEnc.statesCube)
        print("actions of eq class (" + str(fsm.count_inputs(act)) + ")")
        self.show_i(fsm, act)
        
        
        gamma_inputs = [var
                        for agent in {"a"}
                        for var in fsm.agents_inputvars[agent]]
        gamma_cube = fsm.bddEnc.cube_for_inputs_vars(gamma_inputs)
        ngamma_cube = fsm.bddEnc.inputsCube - gamma_cube
        
        
        nact = ~act.forsome(ngamma_cube) # Useless here
        print("the other actions")
        self.show_i(fsm, nact)
        
        # There is more than one action if act contains other actions than
        # si
        self.assertTrue((act -
                         si.forsome(fsm.bddEnc.statesCube).
                            forsome(ngamma_cube)).is_false())
        print("Are there other actions than si's one ?",
              (act - si.forsome(fsm.bddEnc.statesCube).
                                  forsome(ngamma_cube)).isnot_false())
        
        # remove act from Strats
        excluded = excluded | (strats & eqcl)
        print("excluded")
        self.show_si(fsm, excluded)
        strats = strats - excluded
        print("new strats")
        self.show_si(fsm, strats)
        
        # Restart the process
        si = fsm.pick_one_state_inputs(strats)
        self.assertEqual(len(fsm.pick_all_states(
                                            si.forsome(fsm.bddEnc.inputsCube))),
                         1)
        s = fsm.pick_one_state(si.forsome(fsm.bddEnc.inputsCube))
        print("si:", si.get_str_values())
        print("s:", s.get_str_values())
        eqs = fsm.equivalent_states(s, {"a"})
        print("equivalent to", s.get_str_values())
        self.show_s(fsm, eqs)
        eqcl = strats & eqs
        print("equivalence class (" + str(s.get_str_values()) + ")")
        self.show_si(fsm, eqcl)
        act = eqcl.forsome(fsm.bddEnc.statesCube)
        print("actions of eq class (" + str(fsm.count_inputs(act)) + ")")
        self.show_i(fsm, act)
        
        print("Are there other actions than si's one ?",
              (act - si.forsome(fsm.bddEnc.statesCube).
                                  forsome(ngamma_cube)).isnot_false())
        self.show_i(fsm, (act - si.forsome(fsm.bddEnc.statesCube).
                                   forsome(ngamma_cube)))
                                   
        # Need to split eqcl now !
        # Keep eqcl to split strats later
        fulleqcl = eqcl
        ncss = (eqcl & si.forsome(fsm.bddEnc.statesCube).
                          forsome(ngamma_cube))
        eqcls = [ncss]
        eqcl = eqcl - ncss
        while eqcl.isnot_false():
            si = fsm.pick_one_state_inputs(eqcl)
            ncss = (eqcl & si.forsome(fsm.bddEnc.statesCube).
                              forsome(ngamma_cube))
            eqcls.append(ncss)
            eqcl = eqcl - ncss
        for ncss in eqcls:
            print("new non-conflicting subset")
            self.show_si(fsm, ncss)
            
        splitted = []
        for ncss in eqcls:
            splitted.append(strats - fulleqcl + ncss + excluded)
        print("show splitted strats")
        for substrat in splitted:
            print("new sub strategy")
            self.show_si(fsm, substrat)
            
            
    def test_split(self):
        fsm = self.little()
        
        aa = eval_simple_expression(fsm, "a.a = 1")
        ap = eval_simple_expression(fsm, "a.p = 1")
        ba = eval_simple_expression(fsm, "b.a = 1")
        bq = eval_simple_expression(fsm, "b.q = 1")
        true = eval_simple_expression(fsm, "TRUE")
        false = eval_simple_expression(fsm, "FALSE")
        
        gamma_inputs = [var
                        for agent in {"a"}
                        for var in fsm.agents_inputvars[agent]]
        gamma_cube = fsm.bddEnc.cube_for_inputs_vars(gamma_inputs)
        ngamma_cube = fsm.bddEnc.inputsCube - gamma_cube
        
        strats = split(fsm, fsm.protocol({"a"}), {"a"})
            
        commstrat = (((~ap & bq & ~aa) | (~ap & ~bq & ~aa)).forsome(ngamma_cube) &
                     fsm.protocol({"a"}))
        firststrat = (((ap & bq & aa) | (ap & ~bq & aa)).forsome(ngamma_cube) &
                      fsm.protocol({"a"}))
        secstrat = (((ap & bq & ~aa) | (ap & ~bq & ~aa)).forsome(ngamma_cube) &
                    fsm.protocol({"a"}))
        
        self.assertTrue((commstrat | firststrat) in strats)
        self.assertTrue((commstrat | secstrat) in strats)
        
        self.assertSetEqual({commstrat | firststrat, commstrat | secstrat},
                            strats)
    
    def test_split_cardgame3(self):
        fsm = self.cardgame3()
        agents = {'player'}
        
        strats = set()
        nbstrats = 0
        for strat in split(fsm, fsm.protocol(agents), agents):
            nbstrats += 1
            self.assertTrue(strat not in strats)
            strats.add(strat)
        self.assertEqual(nbstrats, 8)
        self.assertEqual(len(strats), nbstrats)
        