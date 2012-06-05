# FileName	[ Makefile ]
#
# PackageName	[ NuSMV ]
#
# Synopsis	[ Package-wide Makefile ]
#
# Description	[ Makefile template for the psl parser ]
#
# Author	[ Roberto Cavada ]
#
# Copyright	[Copyright (C) 2005 by ITC-irst ]
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
# Revision	[v 1.0.0.0 2005/5/3 15:06:47]

CUDD_NAME = $(cuddname)
CUDD_DIR = ../$(CUDD_NAME)

INCLUDES = -I. -I../.. -I$(top_srcdir)/src -I$(top_srcdir)/$(CUDD_DIR)/include

BUILT_SOURCES = psl_grammar.h psl_grammar.c psl_input.c 

MOSTLYCLEANFILES = psl_grammar.h psl_grammar.c psl_input.c

AM_YFLAGS = -d -p psl_yy 

if USE_LEGACY_FLEX
AM_LFLAGS = -l -Ppsl_yy -o$(LEX_OUTPUT_ROOT).c
else
AM_LFLAGS = -Ppsl_yy -o$(LEX_OUTPUT_ROOT).c
endif

PKG_SOURCES = psl_grammar.y psl_input.l psl_grammar.h psl_symbols.h \
	      pslExpr.c pslNode.c pslConv.c \
	      pslExpr.h   pslInt.h  pslNode.h 

parserpsldir = $(includedir)/nusmv/parser/psl
parserpsl_HEADERS = pslExpr.h pslNode.h psl_grammar.h psl_symbols.h

# -------------------------------------------------------

noinst_LTLIBRARIES = libparserpsl.la
libparserpsl_la_SOURCES = $(PKG_SOURCES)

