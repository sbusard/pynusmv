import unittest
import sys

from pynusmv.nusmv.cinit import cinit
from pynusmv.nusmv.cmd import cmd

from pynusmv.prop.propDb import PropDb

class TestPropDb(unittest.TestCase):
    
    def setUp(self):
        cinit.NuSMVCore_init_data()
        cinit.NuSMVCore_init(None, 0)
        ret = cmd.Cmd_SecureCommandExecute("read_model -i"
                                           " tests/pynusmv/admin.smv")
        self.assertEqual(ret, 0)
        ret = cmd.Cmd_SecureCommandExecute("go")
        self.assertEqual(ret, 0)
        
    def tearDown(self):
        cinit.NuSMVCore_quit()
        
            
    def test_propDb(self):
        propDb = PropDb.get_global_database()
        self.assertTrue(propDb.get_size() >= 1, "propDb misses some props")
        prop = propDb.get_prop_at_index(0)
        self.assertIsNotNone(prop, "prop should not be None")
        
        self.assertEqual(len(propDb), propDb.get_size())
        self.assertEqual(propDb[0]._ptr, propDb.get_prop_at_index(0)._ptr)
        
        for prop in propDb:
            self.assertIsNotNone(prop, "prop should not be None")
        