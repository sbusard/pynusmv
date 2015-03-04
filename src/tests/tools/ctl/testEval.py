import unittest

from pynusmv.fsm import BddFsm
from pynusmv.dd import BDD
from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv.parser import parse_simple_expression as parseSexp
from pynusmv import prop

from tools.ctl.eval import eval_ctl


class TestEval(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
    
    def tearDown(self):
        deinit_nusmv()
    
    
    def init_model(self):
        fsm = BddFsm.from_filename("tests/tools/ctl/admin.smv")
        self.assertIsNotNone(fsm)
        return fsm
        

    def test_atom(self):
        fsm = self.init_model()
        
        anspec = prop.Spec(parseSexp("admin = none"))
        an = eval_ctl(fsm, anspec)
        self.assertIsNotNone(an)
        self.assertTrue(fsm.init <= an)
        
        aaspec = prop.Spec(parseSexp("admin = alice"))
        aa = eval_ctl(fsm, aaspec)
        self.assertIsNotNone(aa)
        self.assertTrue((fsm.init & aa).is_false())
        self.assertTrue((aa & an).is_false())
    
    def test_ctl(self):
        fsm = self.init_model()
        # EF admin = alice is True
        efaa = prop.ef(prop.atom("admin = alice"))
        self.assertTrue(fsm.init <= eval_ctl(fsm, efaa))
        
        # EG admin != alice is True
        egana = prop.eg(prop.atom("admin != alice"))
        self.assertTrue(fsm.init <= eval_ctl(fsm, egana))
        
        # AF admin != none is True
        afann = prop.af(prop.atom("admin != none"))
        self.assertTrue(fsm.init <= eval_ctl(fsm, afann))
        
        # AG admin in {alice, bob, none} is True
        aga = prop.ag(prop.atom("admin in {alice, bob, none}"))
        self.assertTrue(fsm.init <= eval_ctl(fsm, aga))
        
        # AG (admin != none -> AG admin != none) is True
        aganniann = prop.ag(prop.imply(prop.atom("admin != none"),
                                       prop.ag(prop.atom("admin != none"))))
        self.assertTrue(fsm.init <= eval_ctl(fsm, aganniann))
        
        # AG (admin = alice -> AX admin = alice) is True
        agaaiaxaa = prop.ag(prop.imply(prop.atom("admin = alice"),
                                       prop.ax(prop.atom("admin = alice"))))
        self.assertTrue(fsm.init <= eval_ctl(fsm, agaaiaxaa))
        
        
        # AX admin = alice is False
        axaa = prop.ax(prop.atom("admin = alice"))
        self.assertFalse(fsm.init <= eval_ctl(fsm, axaa))
        
        # EX admin != none is False
        exann = prop.ex(prop.atom("admin != none"))
        self.assertFalse(fsm.init <= eval_ctl(fsm, exann))
        
        # AG admin = none is False
        agan = prop.ag(prop.atom("admin = none"))
        self.assertFalse(fsm.init <= eval_ctl(fsm, agan))
        
        # EG admin = none is False
        egan = prop.eg(prop.atom("admin = none"))
        self.assertFalse(fsm.init <= eval_ctl(fsm, egan))