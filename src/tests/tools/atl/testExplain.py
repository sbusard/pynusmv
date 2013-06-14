import unittest

from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.mc import eval_simple_expression

from pynusmv.utils import fixpoint as fp

from tools.mas import glob

from tools.atl.check import check
from tools.atl.eval import evalATL, cex
from tools.atl.parsing import parseATL
from tools.atl.explain import (explain_cex, explain_cax, explain_ceu,
                               explain_cau, explain_ceg, explain_cag,
                               explain_cew, explain_caw)


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
            ss = fsm.pick_all_states(bdd)
            print("S count:", len(ss))
            for s in ss:
                print(s.get_str_values())
            
    def show_i(self, fsm, bdd):
        if bdd.isnot_false():
            ii = fsm.pick_all_inputs(bdd)
            print("I count:", len(ii))
            for i in ii:
                print(i.get_str_values())
                
                
    def show_cex(self, explanation, spec):
        """Show the explanation of spec satisfied by explanation.state."""
        print()
        print("Explaining", spec)
        print(explanation.state.get_str_values())
        for action, succ in explanation.successors:
            print("action:", action.get_str_values())
            print("successor:", succ.state.get_str_values())
    
    
    def check_cex(self, fsm, explanation, agents, phi):
        """Check that the explanation is correct."""
        # Get the cubes
        gamma_cube = fsm.inputs_cube_for_agents(agents)
        ngamma_cube = fsm.bddEnc.inputsCube - gamma_cube
        
        # The first state satisfies the spec
        self.assertTrue(explanation.state <= cex(fsm, agents, phi))
        acts = BDD.false(fsm.bddEnc.DDmanager)
        states = BDD.false(fsm.bddEnc.DDmanager)
        for (act, succ) in explanation.successors:
            ag_action = act.forsome(ngamma_cube)
            # The successor satisfies phi
            self.assertTrue(succ.state <= phi)
            # The action is effectively possible
            self.assertTrue(act <= fsm.get_inputs_between_states(
                                                 explanation.state, succ.state))
            # Accumulate states and actions
            acts = acts | act
            states = states | succ.state
        
        # The reached states are effectively the reachable states
        # through the action
        self.assertTrue(states <= fsm.post(explanation.state, ag_action))
        self.assertTrue(states >= fsm.post(explanation.state, ag_action)
                                                        & fsm.bddEnc.statesMask)
                                                        
        # The actions are effectively all the possible actions completing ag_act
        self.assertTrue(acts <= ag_action)
        self.assertTrue(acts >= ag_action & 
                      fsm.get_inputs_between_states(explanation.state, states) &
                      fsm.bddEnc.inputsMask)
                      
                      
    def check_cax(self, fsm, explanation, agents, phi):
        """Check that the explanation is correct."""
        # Get the cubes
        gamma_cube = fsm.inputs_cube_for_agents(agents)
        ngamma_cube = fsm.bddEnc.inputsCube - gamma_cube
        
        # The first state satisfies the spec
        self.assertTrue(explanation.state <= ~cex(fsm, agents, ~phi))
        acts = BDD.false(fsm.bddEnc.DDmanager)
        states = BDD.false(fsm.bddEnc.DDmanager)
        for (act, succ) in explanation.successors:
            # The successor satisfies phi
            self.assertTrue(succ.state <= phi)
            # The action is effectively possible
            self.assertTrue(act <= fsm.get_inputs_between_states(
                                                 explanation.state, succ.state))
            # Accumulate states and actions
            acts = acts | act
            states = states | succ.state
        
        # The actions are effectively all the possible ones
        self.assertEqual((fsm.protocol(agents) &
                          explanation.state).forsome(fsm.bddEnc.statesCube).
                          forsome(ngamma_cube) &
                         fsm.bddEnc.statesMask,
                         acts.forsome(ngamma_cube) & fsm.bddEnc.statesMask)
    
    
    def check_ceu(self, fsm, explanation, agents, phi, psi):
        # For all states, check that
        # it is in phi | psi,
        # if it's in phi, it has a least one successor
        # if it's in psi, it has no successor
        extract = {explanation}
        states = set()
        while len(extract) > 0:
            expl = extract.pop()
            states.add(expl)
            self.assertTrue(expl.state <= phi | psi)
            if expl.state <= psi:
                self.assertTrue(len(expl.successors) == 0)
            else:
                self.assertTrue(len(expl.successors) > 0)
            for action, succ in expl.successors:
                if succ not in states:
                    extract.add(succ)
                
    
    def check_ceg(self, fsm, explanation, agents, phi):
        # For all states, check that
        # it is in phi,
        # it has at least one successor
        extract = {explanation}
        states = set()
        while len(extract) > 0:
            expl = extract.pop()
            states.add(expl)
            self.assertTrue(expl.state <= phi)
            self.assertTrue(len(expl.successors) > 0)
            for action, succ in expl.successors:
                if succ not in states:
                    extract.add(succ)
    
    
    def test_transmission_cex(self):
        fsm = self.transmission()
        
        spec = parseATL("<'transmitter'>X ~'received'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cex(fsm, first, agents, phi)
        #self.show_cex(explanation, spec)
        self.check_cex(fsm, explanation, agents, phi)
    
    
    def test_cardgame_cex(self):
        fsm = self.cardgame()
        
        spec = parseATL("<'dealer'>X 'pcard = Ac'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cex(fsm, first, agents, evalATL(fsm, spec.child))
        #self.show_cex(explanation, spec)
        self.check_cex(fsm, explanation, agents, phi)
        
        spec = parseATL("<'player'>X 'step = 1'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cex(fsm, first, agents, evalATL(fsm, spec.child))
        #self.show_cex(explanation, spec)
        self.check_cex(fsm, explanation, agents, phi)
        

    def test_tictactoe_cex(self):
        fsm = self.tictactoe()
        
        spec = parseATL("'run = circle' -> <'circlep'>X 'run = cross'")[0]
        
        agents = {atom.value for atom in spec.right.group}
        phi = evalATL(fsm, spec.right.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cex(fsm, first, agents, phi)
        #self.show_cex(explanation, spec.right)
        self.check_cex(fsm, explanation, agents, phi)
        
        spec = parseATL("'run = circle' -> <'circlep'>X 'board[1] = circle'")[0]
        agents = {atom.value for atom in spec.right.group}
        phi = evalATL(fsm, spec.right.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cex(fsm, first, agents, phi)
        #self.show_cex(explanation, spec.right)
        self.check_cex(fsm, explanation, agents, phi)
        

    def test_transmission_cax(self):
        fsm = self.transmission()
        
        spec = parseATL("['sender']X ~'received'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cax(fsm, first, agents, phi)
        #self.show_cex(explanation, spec)
        self.check_cax(fsm, explanation, agents, phi)
        
    
    def test_cardgame_cax(self):
        fsm = self.cardgame()
        
        spec = parseATL("['player']X 'pcard = Ac'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cax(fsm, first, agents, evalATL(fsm, spec.child))
        #self.show_cex(explanation, spec)
        self.check_cax(fsm, explanation, agents, phi)
        
        spec = parseATL("['dealer']X 'win'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        # False since we do not want initial states
        #self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = (sat & eval_simple_expression(fsm, 'step = 1') &
                   fsm.reachable_states)
        first = fsm.pick_one_state(initsat)
        explanation = explain_cax(fsm, first, agents, evalATL(fsm, spec.child))
        #self.show_cex(explanation, spec)
        self.check_cax(fsm, explanation, agents, phi)
        

    def test_tictactoe_cax(self):
        fsm = self.tictactoe()
        
        spec = parseATL("'run = circle' -> ['circlep']X 'run = cross'")[0]
        
        agents = {atom.value for atom in spec.right.group}
        phi = evalATL(fsm, spec.right.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cax(fsm, first, agents, phi)
        #self.show_cex(explanation, spec.right)
        self.check_cax(fsm, explanation, agents, phi)
        
        spec = parseATL("'run = circle' -> ['crossp']X 'board[1] = circle'")[0]
        agents = {atom.value for atom in spec.right.group}
        phi = evalATL(fsm, spec.right.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cax(fsm, first, agents, phi)
        #self.show_cex(explanation, spec.right)
        self.check_cax(fsm, explanation, agents, phi)
    
    
    def test_transmission_ceu(self):
        fsm = self.transmission()
        
        spec = parseATL("<'sender','transmitter'>['TRUE' U 'received']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceu(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
    
    
    def test_cardgame_ceu(self):
        fsm = self.cardgame()
        
        spec = parseATL("<'player'>['TRUE' U 'win']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceu(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
        
    
    @unittest.skip("Test too long")
    def test_tictactoe_ceu(self):
        fsm = self.tictactoe()
        
        spec = parseATL("<'circlep'> ['TRUE' U  'winner != circle' & 'run = stop']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceu(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
        
        spec = parseATL("<'circlep', 'crossp'> ['TRUE' U 'winner = empty' & 'run = stop']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceu(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
        
    
    def test_transmission_cau(self):
        fsm = self.transmission()
        
        spec = parseATL("['transmitter']['TRUE' U ~'received']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cau(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
    
    
    def test_cardgame_cau(self):
        fsm = self.cardgame()
        
        spec = parseATL("['dealer']['TRUE' U 'win']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cau(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
        
        spec = parseATL("['dealer']['TRUE' U 'lose']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cau(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
        
    
    @unittest.skip("Test too long")
    def test_tictactoe_cau(self):
        fsm = self.tictactoe()
        
        spec = parseATL("['circlep']['TRUE' U  ('winner != circle' & 'run = stop')]")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        
        self.assertEqual(sat, fp(lambda Y : psi | (phi & fsm.pre_nstrat(Y, agents)), BDD.false(fsm.bddEnc.DDmanager)))
        
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cau(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)

        
    def test_transmission_ceg(self):
        fsm = self.transmission()
        
        spec = parseATL("<'transmitter'> G ~'received'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceg(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
        
        
        spec = parseATL("<'sender'> G ~'received'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceg(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
    
    
    def test_cardgame_ceg(self):
        fsm = self.cardgame()
        
        spec = parseATL("<'player'> G ~'win'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceg(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
        
        spec = parseATL("<'player','dealer'> G ~'win'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceg(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
        
    
    @unittest.skip("Test too long.")
    def test_tictactoe_ceg(self):
        fsm = self.tictactoe()
        
        spec = parseATL("<'circlep', 'crossp'> G 'winner = empty'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceg(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
        
        spec = parseATL("<'circlep'> G 'winner != circle'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_ceg(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
        
    
    def test_transmission_cag(self):
        fsm = self.transmission()
        
        spec = parseATL("['sender'] G ~'received'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cag(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
        
        
        spec = parseATL("['transmitter'] G ~'received'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cag(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
    
    
    def test_cardgame_cag(self):
        fsm = self.cardgame()
        
        spec = parseATL("['dealer'] G ~'win'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cag(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
        
    
    @unittest.skip("Test too long.")
    def test_tictactoe_cag(self):
        fsm = self.tictactoe()
        
        spec = parseATL("['circlep'] G 'winner != circle'")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.child)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cag(fsm, first, agents, phi)
        self.check_ceg(fsm, explanation, agents, phi)
    
    
    def test_cardgame_cew(self):
        fsm = self.cardgame()
        
        spec = parseATL("<'player'>[~'lose' W 'win']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cew(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
        
        
        spec = parseATL("<'player'>[~'lose' W 'step = 3']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_cew(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
        
        
    def test_cardgame_caw(self):
        fsm = self.cardgame()
        
        spec = parseATL("['dealer'][~'lose' W 'win']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_caw(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)
        
        
        spec = parseATL("['dealer'][~'lose' W 'step = 3']")[0]
        agents = {atom.value for atom in spec.group}
        phi = evalATL(fsm, spec.left)
        psi = evalATL(fsm, spec.right)
        self.assertTrue(check(fsm, spec))
        sat = evalATL(fsm, spec)
        initsat = sat & fsm.init
        first = fsm.pick_one_state(initsat)
        explanation = explain_caw(fsm, first, agents, phi, psi)
        self.check_ceu(fsm, explanation, agents, phi, psi)