#! /usr/bin/env python3.2

import os


# For each directory in pynusmv/nusmv,
# create a SWIG module with
#	- the module name
# 	- the package name
#	- the list of included headers, taken from nusmv/src corresponding dir

# This script assumes that the subtree of directories is the same on both hands


NuSMVsourceDir = "nusmv/src"
PyNuSMVdestDir = "pynusmv/nusmv"
PyNuSMVPackage = "pynusmv.nusmv"



def build_modules():
	"""
	Build all modules from root.
	"""
	
	def _build_modules(directory, submodulepath):
		"""
		Build all modules from directory.
		"""
		
		smp = ""
		if len(submodulepath) > 0:
			smp = submodulepath + "."
		build_module(directory, smp + os.path.basename(directory))
		dirpath = os.path.join(NuSMVsourceDir, directory)
		for f in os.listdir(dirpath):
			if os.path.isdir(os.path.join(dirpath,f)) and not f.startswith('.'):
				_build_modules(os.path.join(directory, f),
							smp + os.path.basename(directory))
							
	_build_modules("", "")	


def build_module(directory, submodulepath):
	"""
	Given a directory, build the SWIG module for this directory.
	This directory must be in the hierarchy of PyNuSMV.
	directory is the path from the root of the hierarchy of PyNuSMV
	to the directory.
	submodulepath is the path of modules to the directory (in Python style).
	"""
	
	files = os.listdir(os.path.join(NuSMVsourceDir, directory))
	files = filter_files(files)
	if len(files) > 0:
		modulename = os.path.basename(directory)
		module = swig_module_from_files(files, directory, submodulepath,
					modulename)
		dirpath = os.path.join(PyNuSMVdestDir, directory)
		os.makedirs(dirpath, exist_ok=True)
		modulefile = open(os.path.join(dirpath,modulename) + ".i", "w")
		modulefile.write(module)
		modulefile.close()


def filter_files(files):
	"""
	Returns the subset of files that must be included.
	This means .h files, but not Int.h files.
	"""
	return [f for f in files	if f.endswith('.h')
								if not f.endswith('Int.h')
								if not f.endswith('_private.h')
								if not f.endswith('_int.h')]


def swig_module_from_files(files, path, submodulepath, module):
	"""
	Returns the SWIG module content of a module including all files in files.
	The path from NuSMV source dir is path and the PyNuSMV submodule path 
	is submodulepath. module is the name of the module.
	"""
	
	dirpath = os.path.join(PyNuSMVdestDir, path)
	relpath = os.path.relpath(os.path.join(NuSMVsourceDir, path), dirpath)
	
	content = \
	"""%module(package="{PyNuSMVPackage}.{submodule}") {module}

%{{""".format(	PyNuSMVPackage=PyNuSMVPackage,
				submodule=submodulepath,
				module=module)	
	for f in files:
		content += \
"""
#include "{path}/{file}" """.format(path=relpath, file=f)
	content += """
%}
"""
	for f in files:
		content += \
"""
%include {path}/{file}""".format(path=relpath, file=f)
	return content
	
	
build_modules()