import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser

from pynusmv.dd import BDD
from pynusmv.dd import BDDList
from pynusmv.prop import PropDb
from pynusmv.mc import eval_ctl_spec, explainEX, explainEU, explainEG, explain
from pynusmv.prop import Spec
from pynusmv.prop import (true as sptrue, false as spfalse, imply, iff,
                               ex, eg, ef, eu, ew, ax, ag, af, au, aw, atom)
                              
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import glob
from pynusmv.parser import parse_ctl_spec

class TestExplain(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
        
        
    def init_model(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                           "tests/pynusmv/models/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        return fsm
        
        
    def test_init(self):
        fsm = self.init_model()
        manager = fsm.bddEnc.DDmanager
        init = fsm.init
        
        initState = fsm.pick_one_state(init)
        
        self.assertTrue(BDD.false(manager) <= init <= BDD.true(manager))
        self.assertTrue(BDD.false(manager) < initState <= init)
        
        
    def test_eval_atom(self):
        fsm = self.init_model()
        manager = fsm.bddEnc.DDmanager
        init = fsm.init
        
        initState = fsm.pick_one_state(init)
        
        adminNone = eval_ctl_spec(fsm, atom("admin = none"))
        self.assertTrue((initState & adminNone).isnot_false())
        self.assertTrue(init <= adminNone)
        
        
    def test_explain_ex(self):
        fsm = self.init_model()
        manager = fsm.bddEnc.DDmanager
        init = fsm.init
        
        initState = fsm.pick_one_state(init)
        
        adminNone = eval_ctl_spec(fsm, atom("admin = none"))
        exAdminNone = eval_ctl_spec(fsm, ex(atom("admin = none")))
        self.assertTrue(initState <= exAdminNone)
        
        path = explainEX(fsm, initState, adminNone)
        self.assertEqual(initState, path[0])
        self.assertTrue(path[2] <= adminNone)
        

    def test_explain_eu(self):
        fsm = self.init_model()
        manager = fsm.bddEnc.DDmanager
        init = fsm.init
        
        initState = fsm.pick_one_state(init)
        
        adminNone = eval_ctl_spec(fsm, atom("admin = none"))
        adminAlice = eval_ctl_spec(fsm, atom("admin = alice"))
        euNoneUAlice = eval_ctl_spec(fsm, eu(atom("admin = none"),
                                             atom("admin = alice")))
        self.assertTrue(initState <= euNoneUAlice)
        
        path = explainEU(fsm, initState, adminNone, adminAlice)
        self.assertEqual(initState, path[0])
        for i in range(2,len(path)-2,2):
            self.assertTrue(path[i] <= adminNone)
        self.assertTrue(path[-1] <= adminAlice)
        
        
    def test_explain_eg(self):
        fsm = self.init_model()
        manager = fsm.bddEnc.DDmanager
        init = fsm.init
        
        adminAlice = eval_ctl_spec(fsm, atom("admin = alice"))
        egAlice = eval_ctl_spec(fsm, eg(atom("admin = alice")))
        self.assertTrue(egAlice <= adminAlice)
        
        state = fsm.pick_one_state(egAlice)
        self.assertTrue(BDD.false(manager) < state <= adminAlice)
        self.assertTrue(state <= egAlice)
        
        path, (inloop, loop) = explainEG(fsm, state, adminAlice)
        self.assertEqual(state, path[0])
        self.assertIn(loop, path)
        for i in range(0,len(path), 2):
            self.assertTrue(path[i] <= adminAlice)
        self.assertTrue(path[-1] <= adminAlice)
    
    def test_explain(self):
        fsm = self.init_model()
        
        initState = fsm.pick_one_state(fsm.init)
        adminNone = eval_ctl_spec(fsm, atom("admin = none"))
        adminAlice = eval_ctl_spec(fsm, atom("admin = alice"))
        egAlice = eval_ctl_spec(fsm, eg(atom("admin = alice")))
        euNoneUAlice = eval_ctl_spec(fsm, eu(atom("admin = none"),
                                             atom("admin = alice")))
        
        spec = Spec(parse_ctl_spec("EX admin = none"))
        path = explain(fsm, initState, spec)
        self.assertEqual(initState, path[0])
        self.assertTrue(path[2] <= adminNone)
        
        state = fsm.pick_one_state(egAlice)
        spec = Spec(parse_ctl_spec("EG admin = alice"))
        path = explain(fsm, state, spec)
        self.assertEqual(state, path[0])
        self.assertIn(path[-1], path[:-1])
        for i in range(0,len(path), 2):
            self.assertTrue(path[i] <= adminAlice)
        self.assertTrue(path[-1] <= adminAlice)
        self.assertTrue(initState <= euNoneUAlice)
        
        spec = Spec(parse_ctl_spec("E[admin = none U admin = alice]"))
        path = explain(fsm, initState, spec)
        self.assertEqual(initState, path[0])
        for i in range(2,len(path)-2,2):
            self.assertTrue(path[i] <= adminNone)
        self.assertTrue(path[-1] <= adminAlice)