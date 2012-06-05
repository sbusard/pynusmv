#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.58)

AC_INIT(NuSMV, 2.5.4, nusmv-users@fbk.eu)

# where to look for install.sh and other helpers
AC_CONFIG_AUX_DIR(helpers)

AM_INIT_AUTOMAKE([1.9 tar-ustar no-define])

AC_CANONICAL_HOST

AM_CONFIG_HEADER([config.h])
AX_PREFIX_CONFIG_H([nusmv-config.h])

AC_DEFINE_UNQUOTED(PACKAGE_BUILD_DATE, "`date -u`", Build date)

# Library informations
AC_DEFINE_UNQUOTED(LIBRARY_NAME, "NuSMV", Library Name)
AC_DEFINE_UNQUOTED(LIBRARY_VERSION, "2.5.4", Library Version)
AC_DEFINE_UNQUOTED(LIBRARY_BUILD_DATE, "`date -u`", Library build date)
AC_DEFINE_UNQUOTED(LIBRARY_WEBSITE, "http://nusmv.fbk.eu", Library Website)
AC_DEFINE_UNQUOTED(LIBRARY_EMAIL, "nusmv-users@list.fbk.eu", Library Email)
AC_DEFINE_UNQUOTED(LIBRARY_BUGREPORT, "Please report bugs to <nusmv-users@fbk.eu>", Library bug message)

cuddnamedef=cudd-2.4.1.1
if test "x$cuddname" = "x"; then
cuddname=$cuddnamedef
fi
AC_SUBST(cuddname)

cudddir=`cd $srcdir/../$cuddname && pwd`
AC_SUBST(cudddir)

# version
NUSMV_MAJ_VER=`echo $PACKAGE_VERSION | cut -d. -f"1,2"`
AC_SUBST(NUSMV_MAJ_VER)

# Arguments

# libraries are static by default. User can override by passing 
# the flag --enable-dynamic
AC_DISABLE_SHARED


# Extra packages that must be built and distributed
extra_pkgs=


# default addons
def_addons=compass
AC_ARG_ENABLE(addons, 
  [  --enable-addons  Builds given addons. Enabled addon names must be colon-separated (:)],
  [ if test "x$enableval" = "xno"; then 
     addons=
    else 
      if test "x$enableval" = "xyes"; then
        addons=all
      else addons=$enableval
      fi
    fi],
  [ addons=$def_addons ])


# If the LTL2SMV program must be generated:
AC_ARG_ENABLE(ltl2smv,
  [  --enable-ltl2smv  Builds the executable ltl2smv (default is yes)], 
  [if test "x$enableval" = "xno"; then 
    enable_ltl2smv=no
fi], [enable_ltl2smv=yes])
AM_CONDITIONAL(HAVE_LTL2SMV, test "x$enable_ltl2smv" != "xno")


# Executables are statically created
opt_static_link=no
AC_ARG_ENABLE(static-link, AC_HELP_STRING([--enable-static-link], [link all executables statically]), opt_static_link=$enableval)

# If the PSL parser must be generated:
AC_ARG_ENABLE(pslparser,
  [  --enable-pslparser  Builds the executable pslparser to parse PSL properties (default is no)], 
  [if test "x$enableval" = "xyes"; then 
    enable_psl=yes
fi], [enable_psl=no])

AM_CONDITIONAL(HAVE_PSL_PARSER, test "x$enable_psl" = "xyes")


expatdefpath=/usr

# readline (linking with readline may be disabled by the user)
AC_ARG_ENABLE(expat,
[  --enable-expat    Links NuSMV with expat library (default is yes when found)],
  [if test "x$enableval" = "xyes"; then
    enable_expat=yes
   fi
   if test "x$enableval" = "xno"; then 
    enable_expat=no
   fi], [enable_expat=yes])

# expat library dir:
AC_ARG_WITH(expat-libdir,
[  --with-expat-libdir=<dir> The directory which the expat library is contained in. 
			      (default is /usr/lib)],
[expatlibdir=-L$withval], [expatlibdir=])

# expat library header dir:
AC_ARG_WITH(expat-incdir,
  [  --with-expat-incdir=<dir> The directory which the expat.h header is contained in. 
			      (default are standard locations)],
  [ expatincdir=$withval
    if ! test -f $expatincdir/expat.h; then
        AC_MSG_ERROR(The expat.h header file has not been found in "$expatincdir".
See configure option --with-expat-incdir to specify where the header file is located.)
    else 
     expat_h_found=yes
     expatuserinc="-L $expatincdir"
    fi
  ], 
  [ expatuserinc=""
    AC_CHECK_HEADER(expat.h, 
      [expat_h_found=yes],
      [])
  ])
AC_SUBST(expatuserinc)


# readline (linking with readline may be disabled by the user)
AC_ARG_ENABLE(readline,
[  --enable-readline    Links NuSMV with readline library (default is yes)],
  [if test "x$enableval" = "xyes"; then
    enable_readline=yes
   fi
   if test "x$enableval" = "xno"; then 
    enable_readline=no
   fi], [enable_readline=yes])


# ZChaff configuration
zchaff_nusmv=$srcdir/../zchaff
if [ test -f  $zchaff_nusmv/zchaff-default]; then
. $zchaff_nusmv/zchaff-default
else 
. $zchaff_nusmv/zchaff-default.in
fi
AC_SUBST(zchaff_ver)
AC_SUBST(zchaff_nusmv)
AC_SUBST(zchaff_patch)
AC_SUBST(zchaff_libname)
AC_SUBST(zchaff_ifcname)
zchaffdefpath=$srcdir/../$zchaff_link

# ZChaff library dir:
AC_SUBST(zchafflibdir)
AC_ARG_WITH(zchaff-libdir,
[  --with-zchaff-libdir=<dir> The directory which the zchaff library is contained in. 
			  Depends on --enable-zchaff. (default is ../zchaff.<version>)],
[ if [ test -x $withval ]; then 
    zchafflibdir=`cd $withval && pwd`
  else
    zchafflibdir=$withval
  fi],
[ if [ test -x $zchaffdefpath ]; then 
    zchafflibdir=`cd $zchaffdefpath && pwd`
  else
    zchafflibdir=$zchaffdefpath
  fi]
)

# ZChaff include dir:
AC_SUBST(zchaffincdir)
AC_ARG_WITH(zchaff-incdir,
[  --with-zchaff-incdir=<dir> The directory the SAT_C.h header is contained in.
			  Depends on --enable-zchaff. (default is ../zchaff.<version>)],
[ if [ test -x $withval ]; then 
    zchaffincdir=`cd $withval && pwd`
  else
    zchaffincdir=$withval
  fi],
[ if [ test -x $zchaffdefpath ]; then 
    zchaffincdir=`cd $zchaffdefpath && pwd`
  else
    zchaffincdir=$zchaffdefpath
  fi]
)

# If zchaff must be linked:
AC_ARG_ENABLE(zchaff,
  [  --enable-zchaff  Makes the ZChaff SAT solver usable from within NuSMV (default is no)], 
  [if test "x$enableval" = "xyes"; then 
    enable_zchaff=yes
fi], [enable_zchaff=no])


# MiniSat configuration (tries user-settings first)
minisat_nusmv=$srcdir/../MiniSat
if [ test -f  $minisat_nusmv/minisat-default]; then
. $minisat_nusmv/minisat-default
else 
. $minisat_nusmv/minisat-default.in
fi

AC_SUBST(minisat_ver)
AC_SUBST(minisat_nusmv)
AC_SUBST(minisat_patch)
AC_SUBST(minisat_libname)
AC_SUBST(minisat_ifcname)
minisatdefpath=$srcdir/../$minisat_link

# MiniSat library dir:
AC_SUBST(minisatlibdir)
AC_ARG_WITH(minisat-libdir,
[  --with-minisat-libdir=<dir> The directory which the MiniSat library 
	                  (lib$$minisat_libname.a) is contained in. 
			  Depends on --enable-minisat. (default is ../MiniSat_<version>)],
[ if [ test -x $withval ]; then
    minisatlibdir=`cd $withval && pwd`
  else 
    minisatlibdir=$withval
  fi], 
[ if [ test -x $minisatdefpath ]; then
    minisatlibdir=`cd $minisatdefpath && pwd`
  else 
    minisatlibdir=$minisatdefpath
  fi]
)

# MiniSat include dir:
AC_SUBST(minisatincdir)
AC_ARG_WITH(minisat-incdir,
[  --with-minisat-incdir=<dir> The directory the interface files (Solver_C.h) is contained in.
			  Depends on --enable-minisat. (default is ../MiniSat_<version>)],
[ if [ test -x $withval ]; then
    minisatincdir=`cd $withval && pwd`
  else
    minisatincdir=$withval
  fi], 
[ if  [ test -x $minisatdefpath ]; then 
    minisatincdir=`cd $minisatdefpath && pwd`
  else 
    minisatincdir=$minisatdefpath
  fi]
)

# If MiniSat must be linked:
AC_ARG_ENABLE(minisat,
  [  --enable-minisat  Makes the MiniSat SAT solver usable from within NuSMV (default is yes)], 
  [if test "x$enableval" = "xyes"; then 
    enable_minisat=yes
   fi
   if test "x$enableval" = "xno"; then 
    enable_minisat=no
   fi], [enable_minisat=auto])


excflags=""
AC_ARG_ENABLE(profiling,
  [  --enable-profiling  Builds the system with profiling information (default no)], 
  [if test "x$enableval" = "xyes"; then 
    excflags="-pg "
fi], [])

AC_ARG_ENABLE(pedantic,
  [  --enable-pedantic  Builds the system with pedantic warning (default no)], 
  [if test "x$enableval" = "xyes"; then 
    excflags="-Wall -pedantic $excflags"
fi], [])


# Checks for programs.
oldcflags=$CFLAGS
AC_PROG_CC
AM_PROG_CC_C_O
QEF_C_NORETURN
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
CFLAGS="-fno-strict-aliasing $excflags$oldcflags"

if test "x$ac_cv_prog_CPP" != "x"; then
AC_DEFINE(HAVE_CPP, 1, Have preprocessor)
AC_DEFINE_UNQUOTED(PROG_CPP, "$ac_cv_prog_CPP -x c", Preprocessor call string)
else
AC_DEFINE(HAVE_CPP, 0, Have preprocessor)
fi

AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AC_PROG_YACC
AM_PROG_LEX

# checks that bison was indeed found
AC_CHECK_PROG(_have_bison, bison, yes)
if test "x$_have_bison" != "xyes"; then
  echo "bison is required, found $YACC"
  exit 1
fi

dnl Check whether there is a substitute for flex
if test "$LEX" != flex; then
  LEX="$SHELL $missing_dir/missing flex"
  AC_SUBST(LEX_OUTPUT_ROOT, lex.yy)
  AC_SUBST(LEXLIB, '')
  if test "x$LEX" == "x"; then
    AC_MSG_ERROR("Working lex or flex are required to build NuSMV.")
  fi
fi

dnl Set legacy flag when we are not sure that we are going to use flex
AM_CONDITIONAL(USE_LEGACY_FLEX, test "$LEX" != flex)
AC_SUBST(USE_LEGACY_FLEX)


AC_PATH_PROG(PERL, perl, perl_is_missing)
AC_CHECK_PROGS(HTMLDUMP, lynx links, lynx_or_links_is_missing)

AC_PATH_PROG(LATEX, latex)
AM_CONDITIONAL(HAVE_LATEX, test "x$LATEX" != "x")

AC_PATH_PROG(PDFLATEX, pdflatex)
AC_PATH_PROG(EPSTOPDF, epstopdf)
AC_PATH_PROG(PDFTOPS, pdftops)
AM_CONDITIONAL(HAVE_PDFLATEXETAL, test "x$PDFLATEX" != "x" -a "x$EPSTOPDF" != "x" -a "x$PDFTOPS" != "x")
if test "x$PDFLATEX" != "x" -a "x$EPSTOPDF" != "x" -a "x$PDFTOPS" != "x"; then
IF_USE_PACKAGE_HYPERREF='\iftrue'
else
IF_USE_PACKAGE_HYPERREF='\iffalse'
fi
AC_SUBST(IF_USE_PACKAGE_HYPERREF)

AC_CHECK_TOOL(STRIP, strip)

# features
AC_CPP_FUNC

# portability issues
AC_CHECK_SIZEOF([void *])
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long])
AC_CHECK_SIZEOF([long long])


# Checks for libraries.

AC_CHECK_LIB([m], [exp])

# For readline:
# if readline is enabled, then:
# 1. chooses either ncurses, curses or termcap to be linked to readline
# 2. checks readline with the library selected in 1
# 3. uses the internal readline if 2 fails
if test "x$enable_readline" = "xyes"; then
AC_CHECK_LIB([ncurses], [main], [ac_termcap_lib=-lncurses], \
  [AC_CHECK_LIB([curses], [main], [ac_termcap_lib=-lcurses], \
     [AC_CHECK_LIB([termcap], [main], [ac_termcap_lib=-ltermcap], \
        [ac_termcap_lib=])])])
fi

if test "x$enable_readline" = "xyes"; then
if test x$ac_termcap_lib == x; then
   AC_MSG_WARN(cannot find a library to link with readline (next step might fail))
fi
fi

if test "x$enable_readline" = "xyes"; then
AC_CHECK_LIB([readline], [readline], \
   [AC_DEFINE(HAVE_LIBREADLINE, 1, Uses the system readline)
    readline_libs="-lreadline $ac_termcap_lib"], \
   [AC_DEFINE(HAVE_LIBREADLINE, 0, Uses the private readline)
    readline_libs="";\
    AC_MSG_WARN(cannot link readline: NuSMV will use an internal version)], \
   [$ac_termcap_lib])	
else
   readline_libs=
fi
AC_SUBST(readline_libs)

# expat library
if test x$expat_h_found == xyes -a x$enable_expat ==  xyes; then
LIBS_old=$LIBS
AC_CHECK_LIB(expat, XML_ParserCreate, 
   [ac_have_expat=yes
    AC_DEFINE(HAVE_LIBEXPAT, 1, Enables expat usage)
    expat_libs="$expatlibdir -lexpat"
    ],
   [ac_have_expat=no
    AC_DEFINE(HAVE_LIBEXPAT, 0, Disables expat usage)
    expat_libs=
    ], 
   [$expatlibdir])    
LIBS=$LIBS_old	

else 
  expat_libs=
  ac_have_expat=no
fi

AC_SUBST(expat_libs)		      

AM_CONDITIONAL(HAVE_LIBEXPAT, test "x$ac_have_expat" = "xyes")
AC_SUBST(HAVE_LIBEXPAT)

# regex library for mingw
if [[ x$host_os == xmingw32 ]]; then
   regex_libs="-lregex"
else
   regex_libs=
fi

AC_SUBST(regex_libs)

# checks for the cudd package:

LIBS_old=$LIBS
AC_CHECK_LIB(cudd_util, util_cpu_time, 
   [ac_have_cudd=yes], \
   [ac_have_cudd=no], \
   [-L$srcdir/../$cuddname/lib])

if test "x$ac_have_cudd" = "xyes"; then
   AC_CHECK_LIB(mtr, Mtr_AllocNode,
     [ac_have_cudd=yes], \
     [ac_have_cudd=no], \
     [-L$srcdir/../$cuddname/lib -lcudd_util])

   if test "x$ac_have_cudd" = "xyes"; then
     AC_CHECK_LIB(st, st_init_table,
       [ac_have_cudd=yes], \
       [ac_have_cudd=no], \
       [-L$srcdir/../$cuddname/lib -lmtr -lcudd_util])

     if test "x$ac_have_cudd" = "xyes"; then
       AC_CHECK_LIB(cudd, Cudd_Ref,
         [ac_have_cudd=yes], \
	 [ac_have_cudd=no], \
	 [-L$srcdir/../$cuddname/lib -lst -lmtr -lcudd_util])

     fi 
   fi 
fi
AM_CONDITIONAL(HAVE_NOT_CUDD, test "x$ac_have_cudd" = "xno")
LIBS=$LIBS_old	


# checks cudd version
cuddver=`echo "$cuddname" | cut -d- -f2- | sed 's/\([[0-9]][[0-9]]*\)\.\([[0-9]][[0-9]]*\)\.\([[0-9]][[0-9]]*\).*/\1\2\3/'`
if test $cuddver -ge 240; then
  AC_DEFINE(HAVE_CUDD_24, 1, If cudd-2.4 or greater is available)
else
  AC_DEFINE(HAVE_CUDD_24, 0, If cudd-2.4 or greater is available)
fi


#zchaff header:
if test "x$enable_zchaff" = "xyes"; then 
  if ! test -f $zchaffincdir/$zchaff_ifcname; then
      	AC_MSG_ERROR(Header file "$zchaff_ifcname" has not been found in "$zchaffincdir".
See configure option --with-zchaff-incdir to specify where the header file is located.)
  fi
fi

ac_have_sat_solver=no

# zchaff library
ac_have_zchaff=no
if test "x$enable_zchaff" = "xyes"; then 
LIBS=$LIBS_old
AC_CHECK_LIB([$zchaff_libname], [SAT_AddClause], [ac_have_zchaff=yes], \
	[AC_MSG_ERROR(Library $zchaff_libname not found in $zchafflibdir. 
 Library's location can be specified with the option --with-zchaff-libdir=<dir>
 WARNING: If you are using gcc-3.x and you are trying to link with zchaff 
 version 2001.6.15 or previous you need to install a newer version of zchaff.
 Also you can configure NuSMV to be built with gcc-2.9.x. For example 
 by setting variable CC=gcc296 when invoking configure.)],\
	[-L$zchafflibdir -lstdc++])
LIBS=$LIBS_old
fi


AM_CONDITIONAL(HAVE_ZCHAFF, test "x$ac_have_zchaff" = "xyes")
if test "x$ac_have_zchaff" = "xyes"; then 
   AC_DEFINE(HAVE_SOLVER_ZCHAFF, 1, Enables ZChaff)

   # checks zchaff's version:
   CXXFLAGS_old=$CXXFLAGS
   CXXFLAGS="-I. -I$zchaffincdir"
   AC_LANG_PUSH(C++)
   zchaff_libs="-L$zchafflibdir -l$zchaff_libname -lstdc++"
   LIBS=$zchaff_libs
   AC_MSG_CHECKING(for sat library version)
   AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM([[#include "SAT.h"]], 
		[[void* mng = SAT_InitManager();
                  int lits[] = {1,2,3}; 
                  SAT_AddClause(mng, lits, 3, 0); 
 	          SAT_ReleaseManager(mng);]])], 
      [AC_MSG_RESULT(>= 2003.6.16 (new interface))
       AC_DEFINE(HAVE_OLD_ZCHAFF, 0, Uses the new version of zchaff)],

      [ #tries the old interface:
	AC_MSG_RESULT(Failed with newer interface)
	AC_MSG_CHECKING(for old interface of zchaff)
 	AC_COMPILE_IFELSE(
	  [AC_LANG_PROGRAM([[#include "SAT.h"]], 
		[[void* mng = SAT_InitManager();
                  int lits[] = {1,2,3}; 
                  SAT_AddClause(mng, lits, 3); 
 	          SAT_ReleaseManager(mng);]])],
	  [AC_MSG_RESULT(< 2003.6.16 (old interface))
           AC_DEFINE(HAVE_OLD_ZCHAFF, 1, Uses the old version of zchaff)], 
	  [ AC_MSG_RESULT(Failed the older as well)
	    AC_MSG_ERROR([Could not find a compilable version of zchaff. 
See config.log for details. 
If you are trying to link the an old version of zchaff, it might the case you 
use an old version of gcc like gcc296 for example. To specify the C compiler to be 
used, pass for example CC=gcc296 to configure when you invoke it. 
If you cannot manage with this, contact NuSMV development team for futher help.]) ])
     ])
   LIBS=$LIBS_old
   CXXFLAGS=$CXXFLAGS_old
   AC_LANG_POP(C++)

else # if not ac_have_zchaff
   AC_DEFINE(HAVE_SOLVER_ZCHAFF, 0, Disables ZChaff)
   zchaff_libs=
fi # if ac_have_zchaff

AC_SUBST(zchaff_libs)
AC_SUBST(HAVE_ZCHAFF)

# MiniSat header:
if test "x$enable_minisat" = "xyes"; then 
  if ! test -f $minisatincdir/$minisat_ifcname; then
      	AC_MSG_ERROR(Header file "$minisat_ifcname" has not been found in "$minisatincdir".
See configure option --with-minisat-incdir to specify where the header file is located.)
  fi
fi

if test "x$enable_minisat" = "xauto"; then 
  if ! test -f $minisatincdir/$minisat_ifcname; then
      	AC_MSG_WARN(Header file "$minisat_ifcname" has not been found in "$minisatincdir".
See configure option --with-minisat-incdir to specify where the header file is located.
Minisat will not be linked to the system.)
        enable_minisat=no
  fi
fi

# MiniSat library
ac_have_minisat=no
if test "x$enable_minisat" = "xyes"; then 
LIBS=$LIBS_old
AC_CHECK_LIB([$minisat_libname], [MiniSat_Create],\
        [ac_have_minisat=yes], \
	[AC_MSG_ERROR($minisat_libname library not found in $minisatlibdir
The library  location can be specified with the option --with-minisat-libdir=<dir>.)],\
	[-L$minisatlibdir -lstdc++])
LIBS=$LIBS_old
fi

if test "x$enable_minisat" = "xauto"; then 
LIBS=$LIBS_old
AC_CHECK_LIB([$minisat_libname], [MiniSat_Create],\
        [ac_have_minisat=yes], \
	[AC_MSG_WARN($minisat_libname library not found in $minisatlibdir 
 The library location can be specified with the option --with-minisat-libdir=<dir>.
 Minisat will NOT be linked to NuSMV.)
         enable_minisat=no
         ac_have_minisat=no],\
	[-L$minisatlibdir -lstdc++])
LIBS=$LIBS_old
fi


AM_CONDITIONAL(HAVE_MINISAT, test "x$ac_have_minisat" = "xyes")
if test "x$ac_have_minisat" = "xyes"; then 
   AC_DEFINE(HAVE_SOLVER_MINISAT, 1, Enables Minisat)
   minisat_libs="-L$minisatlibdir -l$minisat_libname  -lstdc++"
else # if not ac_have_minisat
   AC_DEFINE(HAVE_SOLVER_MINISAT, 0, Disables Minisat)
   minisat_libs=
fi # if ac_have_minisat

AC_SUBST(minisat_libs)
AC_SUBST(HAVE_MINISAT)

# have any sat solver?
if [ test "x$ac_have_minisat" = "xyes" ] || [ test "x$ac_have_zchaff" = "xyes" ];  then
   AC_DEFINE(HAVE_SAT_SOLVER, 1, Have at least one sat solver)
   have_sat_solver="yes"
else
   AC_DEFINE(HAVE_SAT_SOLVER, 0, No sat solvers available)
   have_sat_solver="no"
fi
AC_SUBST(have_sat_solver)


# This is used to identify which makefile should be used to build cudds
AC_MSG_CHECKING(Guessing a suitable cudd builder)
case $host_os in
  mingw* | pw32* | os2*)
    cudd_mkf="Makefile_mingw"
    ;;
  darwin* | rhapsody*)
    cudd_mkf="Makefile_os_x"
    ;;
  *)
    cudd_mkf="Makefile"
    ;;
  esac
if test "x$cudd_mkf" = "x"; then
   AC_MSG_ERROR("Unexpected fail")
fi
if test "$ac_cv_sizeof_void_p" = "8"; then
   cudd_mkf="${cudd_mkf}_64bit"
fi

AC_MSG_RESULT(assumed $cudd_mkf)
AC_SUBST(cudd_mkf)

# ======================================================================
#                          Addons checking
#                              -------
AC_MSG_CHECKING(for Addons)

# The list of all available addons
all_addons="compass"

# The list of addons that when required should be configured but
# not built. This rules does not apply when the addon is required
# as a dependency.
not_built_addons=""

# The list of meta-addons, that do not need to have a
# corresponding entry in src/addons, i.e. they are not actually
# required to exist in a separate directory inside src/addons like
# normal addons.
# Still meta-addons need to be configured as the source code
# contains sections that are conditioned to them, or they can be
# used as aliases that trigger the building of a set of
# other dependecies. 
meta_addons=""


# ----------------------------------
# Dependencies among addons:
compass_deps=
# ----------------------------------


# ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
_bs_addons=`echo "$addons" | $SED -e 's/:/ /g'` # blank-separated addons
found_addons=
notfound_addons=
alias_addons=

# check for 'all'
if test "x$_bs_addons" = "xall"; then
  _bs_addons=$all_addons
fi

# checking distributed addons are available
for addon in $_bs_addons
do
  if test -d src/addons_core/$addon; then
    found_addons="$addon $found_addons"
  else
    if test "x$(echo $all_addons | grep -w $addon -)" == "x"; then
      AC_MSG_ERROR(Addon '$addon' does not exist)
    else
      if test "x(echo $meta_addons | grep -w $addon -)" == "x"; then
         notfound_addons="$addon $notfound_addons"
      else
         alias_addons="$addon $alias_addons"
      fi
    fi
  fi
done

dep_addons=
function addon_calc_dep() # calculates dependencies of given addon
{
  addon=$1
  __addon_deps=${addon}_deps
  deps=${!__addon_deps}
  
  for dep in $deps; do
    _tmp=$(echo "$dep_addons $found_addons $alias_addons" | grep -w $dep -) # already taken?
    if test "x$_tmp" == "x"; then
        dep_addons="$dep $dep_addons"
    fi
    addon_calc_dep $dep    
  done
}

# Dependency tracking
for addon in $found_addons $alias_addons; do
  addon_calc_dep $addon
done

# All other possible specific checkings for addons:
for addon in $found_addons $dep_addons $alias_addons; do
  case $addon in
  compass) ;;
  *) 
    AC_MSG_ERROR("Addon $addon is not supported");;
  esac
done

if test "x$notfound_addons" == "x"; then
  AC_MSG_RESULT(ok)
else
  AC_MSG_RESULT(warnings)
fi


# filters out all not built addons, but only if not required as dependecies
all_built_addons=$dep_addons
all_not_built_addons=
for addon in $found_addons; do
    if test "x$(echo $not_built_addons | grep -w $addon -)" == "x"; then
       all_built_addons="$addon $all_built_addons"
    else
       all_not_built_addons="$addon $all_not_built_addons"
    fi
done
AC_SUBST(ADDONS_PKGS, $all_built_addons)


# declares and assigns a set of have_<addon> vars to be used later
for addon in $all_addons; do
  if test "x$(echo "$found_addons $dep_addons $alias_addons" | grep -w $addon -)" == "x"; then
    eval have_addon_${addon}=0
  else eval have_addon_${addon}=1
  fi
done

# Outputs 
for addon in $all_addons; do
  _vname=have_addon_${addon}
  _have_addon=${!_vname}
  case $addon in
  compass) 
    if test $_have_addon == 1; then 
       AC_CONFIG_FILES([src/addons_core/compass/Makefile]) 
       AC_CONFIG_FILES([src/addons_core/compass/parser/Makefile]) 
       AC_CONFIG_FILES([src/addons_core/compass/parser/prob/Makefile])
       AC_CONFIG_FILES([src/addons_core/compass/parser/ap/Makefile])
    fi
    AC_DEFINE_UNQUOTED(HAVE_COMPASS, $_have_addon, Enables the named addon)
    AM_CONDITIONAL(HAVE_COMPASS, test $_have_addon = 1);;

  *) 
    AC_MSG_ERROR("Addon $addon is not supported");;
  esac  
done

AC_DEFINE_UNQUOTED(LINKED_CORE_ADDONS, "${found_addons}${alias_addons}${dep_addons}", 
  Names of linked addons)


#                              -------
#                       End of Addons checking
# ======================================================================



# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_CHECK_HEADERS([float.h limits.h memory.h stddef.h stdlib.h string.h \
  sys/ioctl.h sys/param.h sys/time.h sys/resource.h unistd.h signal.h sys/signal.h \
  errno.h regex.h inttypes.h])

# This is for malloc:
AC_CHECK_HEADER(sys/types.h)
AC_CHECK_HEADER(malloc.h, 
	[ AC_DEFINE(HAVE_MALLOC_H, 1, Defined to 1 if the system provides malloc.h)],
	[ AC_CHECK_HEADER(sys/malloc.h, 
	    [AC_DEFINE(HAVE_SYS_MALLOC_H, 1, Defined to 1 if the system provides sys/malloc.h)],
	    [],
	    [
	      #if HAVE_SYS_TYPES_H
	      # include <sys/types.h>
	      #endif
	    ]  
	    )],
	[
	 #if HAVE_SYS_TYPES_H
	 # include <sys/types.h>
	 #endif
	])


# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_PROG_GCC_TRADITIONAL
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_STRTOD
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([floor memmove memset pow strcasecmp strchr \
		strrchr strstr strtol, random srandom getpid \
		mkstemp mktemp tmpnam getenv setvbuf system popen isatty])

# Executable file names suffix:
AC_DEFINE_UNQUOTED(EXEEXT, "$EXEEXT", Executable file names suffix)

AC_SUBST(EXTRA_PKGS, $extra_pkgs)
                     
AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/be/Makefile
                 src/bmc/Makefile
                 src/bmc/sbmc/Makefile
                 src/cmd/Makefile
                 src/compile/Makefile
                 src/compile/symb_table/Makefile
                 src/compile/type_checking/Makefile
                 src/dag/Makefile
                 src/dd/Makefile
                 src/enc/Makefile
                 src/fsm/Makefile
                 src/ltl/Makefile
                 src/ltl/ltl2smv/Makefile
                 src/mc/Makefile
                 src/node/Makefile
                 src/node/printers/Makefile
                 src/node/normalizers/Makefile
                 src/opt/Makefile
                 src/parser/Makefile
                 src/parser/ord/Makefile
                 src/parser/idlist/Makefile
                 src/parser/psl/Makefile
                 src/prop/Makefile
                 src/rbc/Makefile
                 src/sat/Makefile
                 src/sat/solvers/Makefile
                 src/set/Makefile
                 src/simulate/Makefile
                 src/cinit/Makefile
                 src/trace/Makefile
                 src/trans/Makefile
                 src/utils/Makefile
                 src/sexp/Makefile
                 src/hrc/Makefile
                 src/wff/Makefile
                 src/wff/w2w/Makefile
                 src/addons_core/Makefile
                 doc/Makefile
                 doc/user-man/Makefile
                 doc/user-man/nusmv.sty
                 doc/user-man/main.tex
                 doc/tutorial/Makefile
                 doc/tutorial/nusmvtut.sty
                 doc/tutorial/main.tex
                 doc/html/Makefile
                 examples/Makefile
                 share/Makefile
                 contrib/Makefile
                 helpers/extract_doc
                 nusmv-2.pc])

if test "$opt_static_link" = yes; then
		LDFLAGS="-all-static ${LDFLAGS}"
fi

# ----------------------------------------------------------------------
# All conditionally distributed packages go here:
# ----------------------------------------------------------------------

AC_OUTPUT

echo

if test "x${found_addons}${notfound_addons}${alias_addons}" != "x"; then
echo ------------------------------------------------------------------ 
echo "Addons"
if test "x${found_addons}${alias_addons}" != "x"; then
  printf "  configured:\t${found_addons}${alias_addons}\n"
fi
if test "x${dep_addons}" != "x"; then
  printf "  dependencies:\t${dep_addons}\n"
fi
if test "x${all_not_built_addons}" != "x"; then
  printf "  not built:\t${all_not_built_addons}\t (although configured)\n"
fi
if test "x${notfound_addons}" != "x"; then
  printf "  unsupported:\t${notfound_addons}\n"
fi
echo ------------------------------------------------------------------ 
fi

if test "x$extra_pkgs" != "x"; then
echo ------------------------------------------------------------------ 
echo "Extra packages: $extra_pkgs"
echo ------------------------------------------------------------------ 
fi

echo ------------------------------------------------------------------ 
if test "x$ac_have_zchaff" = "xyes"; then 
echo "ZCHAFF v.$zchaff_ver will be embedded into the NuSMV executable."
else 
echo "The ZCHAFF sat solver will NOT be linked to NuSMV."
echo "If you want to link it, please use configure options"
echo "--enable-zchaff, --with-zchaff-libdir, --with-zchaff-incdir."
echo "For further help, try \"./configure --help\"."
fi
echo ------------------------------------------------------------------ 
echo

echo ------------------------------------------------------------------ 
if test "x$ac_have_minisat" = "xyes"; then 
echo "MiniSat v.$minisat_ver will be embedded into the NuSMV executable."
else 
echo "The MINISAT sat solver will NOT be linked to NuSMV."
echo "If you want to link it, please use configure options"
echo "--enable-minisat, --with-minisat-libdir, --with-minisat-incdir."
echo "For further help, try \"./configure --help\"."
fi
echo ------------------------------------------------------------------ 
echo

if  [ test "x$ac_have_minisat" = "xno" ] && [ test "x$ac_have_zchaff" = "xno" ]; then
echo ------------------------------------------------------------------ 
echo "WARNING: No SAT solver will be linked to NuSMV."
echo "WARNING: All NuSMV commands and options that depend on a SAT "
echo "WARNING: solver will not be available."
echo "WARNING: If you want to use SAT solver NuSMV features, please "
echo "WARNING: compile and link the zchaff SAT solver or the MiniSat"
echo "WARNING: SAT solver. "
echo "WARNING: For further help, try \"./configure --help\"."
echo ------------------------------------------------------------------ 
fi

if test "x$expat_h_found" = "xno"; then
echo ------------------------------------------------------------------ 
echo "The expat library has not been found.                            "
echo "See configure options --with-expat-{inc,lib}dir to help "
echo "configure finding expat if it is installed in non-standard paths." 
echo ------------------------------------------------------------------ 
echo
fi

if test "x$ac_have_expat" = "xno"; then
echo ------------------------------------------------------------------ 
echo "The expat library has not been found.                            "
echo "It might be the case that either libexpat is not installed on    "
echo "your system, or it is actually installed, but configure could not"
echo "find it. See configure options --with-expat-{inc,lib}dir to help "
echo "configure finding expat if it is installed in non-standard paths."
echo "**  If you run 'make' now, NuSMV would still build correctly   **"
echo "**  but the functionality to load saved traces from a file     **"
echo "**  will not be available.                                     **"
echo ------------------------------------------------------------------ 
echo
fi

if test "x$ac_have_cudd" = "xno"; then
echo "Warning: Cannot find the cudd libraries in '$srcdir/../$cuddname/lib'". 
echo "         Did you build cudd?"
echo "         If you did not, make will search for cudd dir into"
echo "         the parent dir, and will make cudd before making"
echo "         the NuSMV executable"
echo "         Cudd will be built with \"make -f $cudd_mkf\""
echo
else
echo ------------------------------------------------------------------ 
echo "Using CUDDs in $cuddname"
echo ------------------------------------------------------------------ 
echo
fi

if test "x$enable_ltl2smv" = "xno"; then
echo ------------------------------------------------------------------ 
echo "As you requested, the ltl2smv executable will be not produced."
echo ------------------------------------------------------------------ 
echo
fi

if test "x$enable_psl" = "xyes"; then
echo ------------------------------------------------------------------ 
echo "As you requested, the PSL parser will be generated as well."
echo ------------------------------------------------------------------ 
echo
fi


if test "$opt_static_link" = yes; then
echo ------------------------------------------------------------------ 
echo "As you requested, all executables will be statically linked."
echo ------------------------------------------------------------------ 
echo
fi
