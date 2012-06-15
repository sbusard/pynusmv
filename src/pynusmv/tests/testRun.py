import unittest
import sys

from ..nusmv.cinit import cinit
from ..nusmv.cmd import cmd
from ..nusmv.mc import mc
from ..nusmv.prop import prop

class TestRun(unittest.TestCase):
	
	def test_run_checkctlspec(self):
		cinit.NuSMVCore_init_data()
		cinit.NuSMVCore_init(None, 0)
		ret = cmd.Cmd_CommandExecute("read_model -i pynusmv/tests/admin.smv")
		self.assertEqual(ret, 0)
		cmd.Cmd_CommandExecute("go")
		self.assertEqual(ret, 0)
		cmd.Cmd_CommandExecute("check_ctlspec")
		self.assertEqual(ret, 0)