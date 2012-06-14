import unittest
import sys

from ..nusmv.cinit import cinit

class TestCinit(unittest.TestCase):
	
	def test_cinit_NuSMV_init(self):
		cinit.NuSMVCore_init_data()
		cinit.NuSMVCore_init(None, 0)
		self.assertEqual(cinit.NuSMVCore_get_tool_name(), "NuSMV")
		self.assertEqual(cinit.NuSMVCore_get_tool_version(), "2.5.4")