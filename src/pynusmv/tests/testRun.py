import unittest
import sys

class TestRun(unittest.TestCase):
	
	def test_run_checkctlspec(self):
		from ..nusmv.cinit import cinit
		from ..nusmv.cmd import cmd
		
		cinit.NuSMVCore_init_data()
		cinit.NuSMVCore_init(None, 0)
		
		ret = cmd.Cmd_SecureCommandExecute("read_model -i pynusmv/tests/admin.smv")
		self.assertEqual(ret, 0)
		ret = cmd.Cmd_SecureCommandExecute("go")
		self.assertEqual(ret, 0)
		ret = cmd.Cmd_SecureCommandExecute("check_ctlspec")
		self.assertEqual(ret, 0)
		
		cinit.NuSMVCore_quit()