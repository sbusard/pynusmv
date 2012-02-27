# FileName	[ Makefile ]
#
# PackageName	[ NuSMV ]
#
# Synopsis	[ Package-wide Makefile ]
#
# Description	[ Makefile template for the NuSMV library ]
#
# Author	[ Marco Roveri ]
#
# Copyright	[Copyright (C) 2007 by FBK-irst ]
#
# NuSMV version 2 is free software; you can redistribute it and/or 
# modify it under the terms of the GNU Lesser General Public 
# License as published by the Free Software Foundation; either 
# version 2 of the License, or (at your option) any later version.
#
# NuSMV version 2 is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public 
# License along with this library; if not, write to the Free Software 
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.
#
# For more information on NuSMV see <http://nusmv.fbk.eu>
# or email to <nusmv-users@fbk.eu>.
# Please report bugs to <nusmv-users@fbk.eu>.
#
# To contact the NuSMV development board, email to <nusmv@fbk.eu>.]
#
# Revision	[1.1.2.4 2003/08/04 11:29:51]

AUTOMAKE_OPTIONS = subdir-objects

## enable or disable the compass package 
if HAVE_COMPASS
 compass_libs = compass/libcompass.la
 _COMPASS = compass
else
 compass_libs = 
endif


## -------------------------------------------------------
SUBDIRS = $(_COMPASS) 

#SUBDIRS = $(ADDONS_PKGS)
# This is done to force distribution of only configured addons
DIST_SUBDIRS = $(SUBDIRS) 

CUDD_NAME = $(cuddname)
CUDD_DIR = ../$(CUDD_NAME)

INCLUDES = -I$(top_srcdir)/src -I$(top_srcdir)/$(CUDD_DIR)/include

PKG_SOURCES = addonsCore.c
addonsdir = $(includedir)/nusmv/addons_core
addons_HEADERS = addonsCore.h

## -------------------------------------------------------
## The NuSMV's addons core library:

# lib_LTLIBRARIES = libaddonscore.la	
noinst_LTLIBRARIES = libaddonscore.la
libaddonscore_la_SOURCES = $(PKG_SOURCES)
libaddonscore_la_LIBADD =  \
	$(compass_libs)      
