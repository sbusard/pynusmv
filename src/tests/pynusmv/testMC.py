import unittest

from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.prop import prop as nsprop
from pynusmv.nusmv.mc import mc as nsmc

from pynusmv.init import init_nusmv, deinit_nusmv
from pynusmv import mc
from pynusmv import glob
from pynusmv import prop


class TestMC(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
    
    
    def test_nsmc(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                           " tests/pynusmv/models/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        # Check CTL specs
        propDb = nsprop.PropPkg_get_prop_database()
        for i in range(nsprop.PropDb_get_size(propDb)):
            p = nsprop.PropDb_get_prop_at_index(propDb, i)
            if nsprop.Prop_get_type(p) == nsprop.Prop_Ctl:
                nsmc.Mc_CheckCTLSpec(p)
    
    def test_mc(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                           " tests/pynusmv/models/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
        ret = {"(EF admin = alice -> AG (admin != none -> admin = alice))":
               False,
               "(EF admin = alice & EF admin = bob)": True}
        
        propDb = glob.prop_database()
        fsm = propDb.master.bddFsm
        
        for p in propDb:
            if p.type == prop.propTypes["CTL"]:
                spec = p.expr
                self.assertEqual(mc.check_ctl_spec(fsm, spec), ret[str(spec)])