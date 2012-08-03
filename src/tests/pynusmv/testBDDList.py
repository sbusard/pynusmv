import unittest

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd
from pynusmv.nusmv.node import node as nsnode
from pynusmv.nusmv.parser import parser

from pynusmv.dd.bdd import BDD
from pynusmv.dd.bddlist import BDDList
from pynusmv.prop.propDb import PropDb

class TestBDDList(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
    
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
    
    
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
                                     "tests/pynusmv/admin.smv")
        self.assertEqual(ret, 0, "cannot read the model")
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0, "cannot build the model")
        
        propDb = PropDb.get_global_database()
        master = propDb.master
        fsm = propDb.master.bddFsm
        
        init = fsm.init
        
        ln = BDDList.from_tuple((init, BDD.true(init._manager), init))
        self.assertEqual(len(ln), 3)
        
        self.assertSequenceEqual((init, BDD.true(init._manager), init),
                                 ln.to_tuple())
        del ln