import unittest
import sys

class TestCinit(unittest.TestCase):
	
	def test_cinit_NuSMV_init(self):
		
		from pynusmv.nusmv.cinit import cinit
		
		cinit.NuSMVCore_init_data()
		cinit.NuSMVCore_init(None, 0)
		
		self.assertEqual(cinit.NuSMVCore_get_tool_name(), "NuSMV")
		self.assertEqual(cinit.NuSMVCore_get_tool_version(), "2.5.4")
		
		cinit.NuSMVCore_quit()