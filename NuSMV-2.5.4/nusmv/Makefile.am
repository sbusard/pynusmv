# FileName	[ Makefile ]
#
# PackageName	[ NuSMV ]
#
# Synopsis	[ Package-wide Makefile ]
#
# Description	[ Makefile template for the NuSMV executable ]
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
# Revision	[1.1.2.14 2003/11/10 14:15:42]

CUDD_NAME = $(cuddname)
CUDD_DIR = ../$(CUDD_NAME)

INCLUDES = -I. -I./src -I$(top_srcdir)/src -I$(top_srcdir)/$(CUDD_DIR)/include $(expatuserinc)

SUBDIRS = src doc examples contrib share

TESTS = helpers/runtests.sh

PACKAGE = $(PACKAGE_NAME)
VERSION = $(PACKAGE_VERSION)

AM_CPPFLAGS = -DDATADIR=\"$(datadir)\" @CPPFLAGS@
CUDD_MAKEFILE = @cudd_mkf@

READMES = LGPL-2.1 NEWS README README_PLATFORMS README.zChaff README.MiniSat

MINISAT_ALL_PATCHES = MiniSat_v070721_nusmv.patch 
MINISAT_PATCHES = $(filter-out $(minisat_nusmv)/${minisat_patch},\
	$(addprefix $(minisat_nusmv)/,$(MINISAT_ALL_PATCHES))) \
	$(minisat_nusmv)/${minisat_patch}
MINISAT_DIST = $(MINISAT_PATCHES) \
        $(minisat_nusmv)/minisat-default.in $(minisat_nusmv)/build.sh \
	$(minisat_nusmv)/gen_maker.sh $(minisat_nusmv)/clean.sh \
	$(minisat_nusmv)/README

ZCHAFF_ALL_PATCHES = zchaff.2003.12.04_nusmv.patch  zchaff.2004.11.15_nusmv.patch  \
	zchaff.64bit.2007.3.12_nusmv.patch
ZCHAFF_PATCHES = $(filter-out $(zchaff_nusmv)/${zchaff_patch},\
	$(addprefix $(zchaff_nusmv)/,$(ZCHAFF_ALL_PATCHES))) \
	$(zchaff_nusmv)/${zchaff_patch}
ZCHAFF_DIST = $(ZCHAFF_PATCHES) \
	$(zchaff_nusmv)/zchaff-default.in $(zchaff_nusmv)/build.sh \
	$(zchaff_nusmv)/clean.sh $(zchaff_nusmv)/README


HELPERS = helpers/compile helpers/missing helpers/config.guess \
	helpers/depcomp helpers/install-sh helpers/config.sub helpers/ltmain.sh


CUSTOM_CMDS = helpers/extdoc helpers/extindex helpers/extproto \
	helpers/extract_doc.in helpers/regression_test

OTHER_DIST_FILES = nusmv-2.pc.in 

EXTERNAL_DIST_DIRS = $(CUDD_DIR) $(MINISAT_DIST) $(ZCHAFF_DIST) 

# ----------------------------------------------------------------------
# Programs to be built
bin_PROGRAMS = NuSMV
if HAVE_LTL2SMV
bin_PROGRAMS += src/ltl/ltl2smv/ltl2smv
endif
if HAVE_PSL_PARSER
bin_PROGRAMS += pslparser
endif
# ----------------------------------------------------------------------

lib_LTLIBRARIES = libnusmv.la  libnusmvcore.la

# ----------------------------------------------------------------------
# The NuSMV executable
NuSMV_SOURCES = src/main.c 
NuSMV_LDADD = libnusmv.la
NuSMV_DEPENDENCIES = libnusmv.la
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
# The ltl2smv program as stand-alone executable.
src_ltl_ltl2smv_ltl2smv_SOURCES = src/ltl/ltl2smv/ltl2smvMain.c
src_ltl_ltl2smv_ltl2smv_LDADD = libnusmv.la 
src_ltl_ltl2smv_ltl2smv_DEPENDENCIES = libnusmv.la
# ----------------------------------------------------------------------

# ----------------------------------------------------------------------
# The PSL parser as stand-alone program, mainly for testing.
# This target is built only if explicitly required during configuration
pslparser_SOURCES = src/parser/psl/psl.c
pslparser_LDADD = libnusmv.la 
pslparser_DEPENDENCIES = libnusmv.la
# ----------------------------------------------------------------------


# lib for external linking:
libnusmv_la_SOURCES = src/cinit/cinitDummyMac.c  # to make 'ar' work on mac osx. See cinit/cinitDummyMac.c for more details.
libnusmv_la_LIBADD = \
	$(top_builddir)/src/libnusmvsrc.la \
	$(top_builddir)/src/parser/libsmvgrammar.la \
	$(top_builddir)/src/librbcdag.la \
	-L$(top_srcdir)/$(CUDD_DIR)/lib \
	-lcudd -lcudd_util -lmtr -lst   \
        $(readline_libs) $(zchaff_libs) $(minisat_libs) $(expat_libs) $(regex_libs)

libnusmv_la_DEPENDENCIES = $(top_builddir)/src/libnusmvsrc.la	\
	$(top_builddir)/src/librbcdag.la			\
	$(top_builddir)/src/cinit/libcinit.la			\
	$(top_builddir)/src/parser/libsmvgrammar.la		\
	$(CUDD_DEPENDENCY)

# core lib for external linking:
libnusmvcore_la_SOURCES = src/cinit/cinitDummyMac.c  # to make 'ar' work on mac osx. See cinit/cinitDummyMac.c for more details.
libnusmvcore_la_LIBADD = \
	$(top_builddir)/src/libnusmvsrc.la \
	-L$(top_srcdir)/$(CUDD_DIR)/lib \
	-lcudd -lcudd_util -lmtr -lst   \
        $(readline_libs) $(zchaff_libs) $(minisat_libs) $(expat_libs) $(regex_libs)

libnusmvcore_la_DEPENDENCIES = $(top_builddir)/src/libnusmvsrc.la	\
$(CUDD_DEPENDENCY)


externalfiles = $(filter-out $(shell find $(EXTERNAL_DIST_DIRS) -wholename '*/.svn*') %.o %.a %~, \
	$(shell find $(EXTERNAL_DIST_DIRS)))

EXTRA_DIST = $(HELPERS) $(TESTS) $(READMES) $(CUSTOM_CMDS) $(OTHER_DIST_FILES)

CLEANFILES = _configs.sed

# ----------------------------------------------------------------------
nusmvincdir = $(includedir)/nusmv
nusmvinc_HEADERS = nusmv-config.h

readmedir = $(datadir)/nusmv
readme_DATA = $(READMES)

cuddincdir = $(includedir)/$(CUDD_NAME)
cuddinc_HEADERS = $(CUDD_DIR)/include/cudd.h  $(CUDD_DIR)/include/dddmp.h \
	$(CUDD_DIR)/include/mnemosyne.h  $(CUDD_DIR)/include/mtr.h  \
	$(CUDD_DIR)/include/st.h $(CUDD_DIR)/include/util.h 

pkgconfdir = $(libdir)/pkgconfig
pkgconf_DATA = nusmv-2.pc

# ----------------------------------------------------------------------
dist-hook:	
	(test ! -d __tmp_nusmv__ || rm -rf __tmp_nusmv__)
	mkdir __tmp_nusmv__ 
	mv $(distdir)/*  __tmp_nusmv__
	mv __tmp_nusmv__ $(distdir)/nusmv
	mv $(CUDD_NAME) $(distdir)/
	$(AMTAR) cz $(externalfiles) | $(AMTAR) xz -C $(distdir) # copies external files
	cd $(distdir)/$(CUDD_NAME) && $(MAKE) -f $(CUDD_MAKEFILE) distclean
#mv $(distdir)/$(minisat_nusmv) $(distdir)
#mv $(distdir)/$(zchaff_nusmv) $(distdir)
	rm -rf `find $(distdir) -name CVS`
	rm -rf `find $(distdir) -name ".svn"`
	rm -rf `find $(distdir) -name '*~'`	
	cd $(distdir)/nusmv/src/parser && rm -f grammar.h grammar.c input.c
	cd $(distdir)/nusmv/src/parser/ord && rm -f ord_grammar.h ord_grammar.c ord_input.c
	cd $(distdir)/nusmv/src/parser/psl && rm -f psl_grammar.h psl_grammar.c psl_input.c
	cd $(distdir)/nusmv/src/parser/idlist && rm -f idlist_grammar.h idlist_grammar.c idlist_input.c
	cd $(distdir)/nusmv/src/addons_core/compass/parser/ap && rm -f ap_grammar.h ap_grammar.c ap_input.c
	cd $(distdir)/nusmv/src/addons_core/compass/parser/prob && rm -f prob_grammar.h prob_grammar.c prob_input.c
	cd $(distdir)/nusmv/src/ltl/ltl2smv && rm -f lex.c yacc.c yacc.h
	rm -rf $(distdir)/nusmv/examples/TipSuite

distcleancheck_listfiles = \
	find -type f -exec sh -c 'test -f $(srcdir)/{} || echo {}' ';'

distcheck: dist
	$(am__remove_distdir)
	GZIP=$(GZIP_ENV) gunzip -c $(distdir).tar.gz | $(AMTAR) xf -
	chmod a+w $(distdir)	
	mkdir $(distdir)/=build
	mkdir $(distdir)/=inst
	cp -r $(distdir)/nusmv/helpers $(distdir)/=build
	cp -r $(distdir)/nusmv/examples $(distdir)/=build

	dc_install_base=`$(am__cd) $(distdir)/=inst && pwd` \
	  && dc_src_base=`$(am__cd) $(distdir)/nusmv && pwd` \
	  && $(am__cd) $(distdir)/=build \
	  && ../nusmv/configure --srcdir=$(dc_src_base) --prefix=$$dc_install_base \
	    $(DISTCHECK_CONFIGURE_FLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) \
	  && $(MAKE) $(AM_MAKEFLAGS) check \
	  && $(MAKE) $(AM_MAKEFLAGS) install \
	  && $(MAKE) $(AM_MAKEFLAGS) installcheck \
	  && $(MAKE) $(AM_MAKEFLAGS) uninstall \
	  && (test `find $$dc_install_base -type f -print | wc -l` -le 1 \
	      || { echo "ERROR: files left after uninstall:" ; \
	           find $$dc_install_base -type f -print ; \
	           exit 1; } >&2 ) \
	  && $(MAKE) $(AM_MAKEFLAGS) dist-gzip \
	  && rm -f $(distdir).tar.gz \
	  && $(MAKE) $(AM_MAKEFLAGS) distcleancheck
	$(am__remove_distdir)
	@echo "$(distdir).tar.gz is ready for distribution" | \
	  sed 'h;s/./=/g;p;x;p;x'


# To build ps and pdf versions of the info file
docs:
	cd $(top_builddir)/doc && $(MAKE) docs

# To extract commands/helps and source code documentation
allDocs: docs


# The used did not build the cudd package, or the distcheck 
# target is performing its work
if HAVE_NOT_CUDD
CUDD_DEPENDENCY = cudd
endif
cudd:
	cd $(top_srcdir)/$(CUDD_DIR) && $(MAKE) -f $(CUDD_MAKEFILE)



# ----------------------------------------------------------------------
# This is for target 'distbin'
if HAVE_ZCHAFF
distbindir =  $(PACKAGE)-zchaff-$(VERSION)-$(host_triplet)
else
distbindir =  $(PACKAGE)-$(VERSION)-$(host_triplet)
endif

distbin: distbin_gzip

remove_distbindir = \
  { test ! -d $(distbindir) || { rm -fr $(distbindir); }; }

do_distbindir: 
	$(remove_distbindir)
	mkdir $(distbindir)
	chmod a+w $(distbindir)

# creates the gzip distribution and strips executables
distbin_gzip: do_distbindir
	$(MAKE) prefix=$(shell cd $(distbindir) && pwd) install-strip
	tardir=$(distbindir) && $(am__tar) | GZIP=$(GZIP_ENV) gzip -c >$(distbindir).tar.gz
	$(remove_distbindir)
# ----------------------------------------------------------------------
