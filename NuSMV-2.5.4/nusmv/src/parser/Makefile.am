# FileName	[ Makefile ]
#
# PackageName	[ NuSMV ]
#
# Synopsis	[ Package-wide Makefile ]
#
# Description	[ Makefile template for the NuSMV executable ]
#
# Author	[ Roberto Cavada, Viktor Schuppan ]
#
# Copyright	[ Copyright (C) 2010 by FBK-irst ]
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
# Revision	[1.1.2.3 2003/08/07 15:43:31]


CUDD_NAME = $(cuddname)
CUDD_DIR = ../$(CUDD_NAME)

INCLUDES = -I$(top_srcdir)/src -I$(top_builddir)/src -I$(top_srcdir)/$(CUDD_DIR)/include

SUBDIRS = ord psl idlist

GRAMMAR_START = begin # initial smv grammar non-terminal symbol


if HAVE_NOT_CUDD
BUILT_SOURCES = grammar.h grammar.c grammar.y \
                input.c input.l input.l.1 input.l.2 input.l.3 \
	        			$(top_srcdir)/$(CUDD_DIR)/include/cudd.h
else
BUILT_SOURCES = grammar.h grammar.c grammar.y \
		 	        	grammar.y.1 grammar.y.2 grammar.y.3 \
                input.c input.l input.l.1 input.l.2 input.l.3 
endif

MOSTLYCLEANFILES = grammar.h grammar.c grammar.y \
		   grammar.y.1 grammar.y.2 grammar.y.3 \
       input.c input.l input.l.1 input.l.2 input.l.3

AM_YFLAGS = -d

if USE_LEGACY_FLEX
AM_LFLAGS = -l
endif

PKG_SOURCES = parser.h parserInt.h parserCmd.c parserUtil.c symbols.h 
GRAMMAR_SOURCES = grammar.y input.l

EXTRA_DIST = 	input.l.1.50 input.l.2.25 input.l.2.50 input.l.2.75 input.l.3.50 \
						 	grammar.y.1.50 grammar.y.2.50 grammar.y.2.51 grammar.y.3.50

parserdir = $(includedir)/nusmv/parser
parser_HEADERS =  parser.h symbols.h 

# -------------------------------------------------------

lib_LTLIBRARIES = libsmvgrammar.la
noinst_LTLIBRARIES = libparser.la

nodist_libsmvgrammar_la_SOURCES = $(GRAMMAR_SOURCES)
libparser_la_SOURCES = $(PKG_SOURCES)

libparser_la_LIBADD =        \
	ord/libparserOrd.la        \
	idlist/libparseridlist.la  \
  psl/libparserpsl.la  


$(top_srcdir)/$(CUDD_DIR)/include/cudd.h:
	cd $(top_srcdir)/$(CUDD_DIR) && $(SHELL) setup.sh


# -------------------------------------------------------
# rules to compose grammar out of its parts

#
# [VS] Ideally the following would be used below. Not doing this for
# now as compatibility with auto tools is not positively clarified.
#
#define mergesubsections
#rm -f $@.tmp;
#for f in $^; do                           \
#  echo "  /* BEGINS: $${f} */" >> $@.tmp; \
#  cat $$f >> $@.tmp;                      \
#  echo "  /* ENDS:   $${f} */" >> $@.tmp; \
#done; 
#mv $@.tmp $@;
#endef

input.l.1 : input.l.1.50
	rm -f $@.tmp;
	for f in $^; do                           \
	  echo "  /* BEGINS: $${f} */" >> $@.tmp; \
	  cat $$f >> $@.tmp;                      \
	  echo "  /* ENDS:   $${f} */" >> $@.tmp; \
	done; 
	mv $@.tmp $@;

input.l.2 : input.l.2.25 \
	    input.l.2.50 \
	    input.l.2.75
	rm -f $@.tmp;
	for f in $^; do                           \
	  echo "  /* BEGINS: $${f} */" >> $@.tmp; \
	  cat $$f >> $@.tmp;                      \
	  echo "  /* ENDS:   $${f} */" >> $@.tmp; \
	done; 
	mv $@.tmp $@;

input.l.3 : input.l.3.50
	rm -f $@.tmp;
	for f in $^; do                           \
	  echo "  /* BEGINS: $${f} */" >> $@.tmp; \
	  cat $$f >> $@.tmp;                      \
	  echo "  /* ENDS:   $${f} */" >> $@.tmp; \
	done; 
	mv $@.tmp $@;

input.l : input.l.1 input.l.2 input.l.3 
	rm -f $@.tmp;
	cat input.l.1 >> $@.tmp;
	echo "%%" >> $@.tmp;
	cat input.l.2 >> $@.tmp;
	echo "%%" >> $@.tmp;
	cat input.l.3 >> $@.tmp;
	mv $@.tmp $@;

grammar.y.1: grammar.y.1.50
	rm -f $@.tmp;
	for f in $^; do                           \
	  echo "  /* BEGINS: $${f} */" >> $@.tmp; \
	  cat $$f >> $@.tmp;                      \
	  echo "  /* ENDS:   $${f} */" >> $@.tmp; \
	done; 
	mv $@.tmp $@;

grammar.y.2: grammar.y.2.50 \
	     grammar.y.2.51
	rm -f $@.tmp;
	for f in $^; do                           \
	  echo "  /* BEGINS: $${f} */" >> $@.tmp; \
	  cat $$f >> $@.tmp;                      \
	  echo "  /* ENDS:   $${f} */" >> $@.tmp; \
	done; 
	mv $@.tmp $@;

grammar.y.3: grammar.y.3.50
	rm -f $@.tmp;
	for f in $^; do                           \
	  echo "  /* BEGINS: $${f} */" >> $@.tmp; \
	  cat $$f >> $@.tmp;                      \
	  echo "  /* ENDS:   $${f} */" >> $@.tmp; \
	done; 
	mv $@.tmp $@;

grammar.y : grammar.y.1 grammar.y.2 grammar.y.3
	rm -f $@.tmp;
	cat grammar.y.1 >> $@.tmp;
	echo "%%" >> $@.tmp;
	cat grammar.y.2 >> $@.tmp;
	echo "%%" >> $@.tmp;
	cat grammar.y.3 >> $@.tmp;
	${SED} -i -e 's/@start@/${GRAMMAR_START}/' $@.tmp;
	mv $@.tmp $@;
