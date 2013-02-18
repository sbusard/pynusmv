import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser

from pynusmv.dd import BDD
from pynusmv.dd import BDDList
from pynusmv.prop import PropDb
from pynusmv import glob

from pynusmv.init import init_nusmv, deinit_nusmv

class TestBDDList(unittest.TestCase):
    
    def setUp(self):
        init_nusmv()
        
    def tearDown(self):
        deinit_nusmv()
    
    
    def test_bddlist(self):
        ln = BDDList.from_tuple((None, None, None))
        self.assertEqual(len(ln), 3)
        for n in ln:
            self.assertIsNone(n)
            
            
    def test_slice(self):
        ln = BDDList.from_tuple((None, None, None, None, None))
        self.assertEqual(len(ln), 5)
        with self.assertRaises(NotImplementedError):
            s = ln[2::]
            
            
    def test_elements(self):
        # Initialize the model
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                     "tests/pynusmv/models/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = glob.prop_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        
        init = fsm.init
        
        ln = BDDList.from_tuple((init, BDD.true(init._manager), init))
        self.assertEqual(len(ln), 3)
        
        self.assertSequenceEqual((init, BDD.true(init._manager), init),
                                 ln.to_tuple())
        del ln