from distutils.core import setup
from distutils.extension import Extension

cinit_extension = Extension(	'_cinit',
								[	'pynusmv/nusmv/cinit/cinit.i'],
								depends = [
									'nusmv/nusmv-config.h',
									'nusmv/src/utils/defs.h',
									'nusmv/src/cinit/cinit.h',
									'lib/libnusmv.dylib'
									],
								swig_opts=['-py3'],
								include_dirs = ['nusmv',
												'nusmv/src',
												'cudd-2.4.1.1/include'],
								libraries=['nusmv'],
								library_dirs=['lib'])
								
node_extension = Extension(		'_node',
								['pynusmv/nusmv/node/node.i'],
								swig_opts=['-py3'],
								include_dirs = ['nusmv',
												'nusmv/src',
												'cudd-2.4.1.1/include'],
								libraries=['nusmv'],
								library_dirs=['lib'])

setup(	name = "PyNuSMV",
		version = "0.1.0",
		author = "Simon Busard",
		author_email = "simon.busard@uclouvain.be",
		url = "http://lvl.info.ucl.ac.be/",
		description = "Python interface for NuSMV.",
		
		ext_modules = [cinit_extension, node_extension])