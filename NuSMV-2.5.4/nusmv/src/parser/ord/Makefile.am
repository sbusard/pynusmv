# FileName	[ Makefile ]
#
# PackageName	[ NuSMV ]
#
# Synopsis	[ Package-wide Makefile ]
#
# Description	[ Makefile template for the ordering parser ]
#
# Author	[ Roberto Cavada ]
#
# Copyright	[Copyright (C) 2003 by ITC-irst ]
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
# Revision	[v 1.1.2.1 2003/11/27 09:11:47]

CUDD_NAME = $(cuddname)
CUDD_DIR = ../$(CUDD_NAME)

INCLUDES = -I. -I../.. -I$(top_srcdir)/src -I$(top_srcdir)/$(CUDD_DIR)/include

BUILT_SOURCES = ord_grammar.h ord_grammar.c ord_input.c 

MOSTLYCLEANFILES = ord_grammar.h ord_grammar.c ord_input.c

AM_YFLAGS = -d -p parser_ord_

if USE_LEGACY_FLEX
AM_LFLAGS = -l
endif

PKG_SOURCES = ord_grammar.y ord_input.l ordInt.h  \
              ParserOrd.h  ParserOrd_private.h  ParserOrd.c

parserorddir = $(includedir)/nusmv/parser/ord
parserord_HEADERS = ParserOrd.h

# -------------------------------------------------------

noinst_LTLIBRARIES = libparserOrd.la
libparserOrd_la_SOURCES = $(PKG_SOURCES)

