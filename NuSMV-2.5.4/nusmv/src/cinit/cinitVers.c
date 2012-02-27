/**CFile***********************************************************************

  FileName    [cinitVers.c]

  PackageName [cinit]

  Synopsis    [Supplies the compile date and version information.]

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``cinit'' package of NuSMV version 2.
  Copyright (C) 1998-2001 by CMU and FBK-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "cinitInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#ifndef NUSMV_PACKAGE_BUILD_DATE
#define NUSMV_PACKAGE_BUILD_DATE  "<compile date not supplied>"
#endif

#ifndef NUSMV_PACKAGE_STRING
#define NUSMV_PACKAGE_STRING         "NuSMV 2.5.x"
#endif

#if NUSMV_HAVE_SOLVER_ZCHAFF
# define NUSMV_PACKAGE_STRING_POSTFIX " zchaff"
#else
# define NUSMV_PACKAGE_STRING_POSTFIX ""
#endif

#ifndef NUSMV_SHARE_PATH
# ifdef DATADIR
#  define NUSMV_SHARE_PATH DATADIR "/nusmv"
# else
#  define NUSMV_SHARE_PATH "/usr/share/nusmv"
# endif
#endif


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void cinit_banner_print ARGS((FILE* file, boolean is_linked));
static char * DateReadFromDateString ARGS((char * datestr));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Returns the current NuSMV version.]

  Description [Returns a static string giving the NuSMV version and compilation
  timestamp.  The user should not free this string.]

  SideEffects []

  SeeAlso     [CInit_NuSMVObtainLibrary]

******************************************************************************/
char* CInit_NuSMVReadVersion()
{
  static char version[1024];
  int c;

  c = snprintf(version, sizeof(version), "%s %s (compiled on %s)",
              NuSMVCore_get_tool_name(), NuSMVCore_get_tool_version(),
              NuSMVCore_get_build_date());
  SNPRINTF_CHECK(c, sizeof(version));
  return version;
}


/**Function********************************************************************

  Synopsis    [Returns the NuSMV library path.]

  Description [Returns a string giving the directory which contains
               the standard NuSMV library.  Used to find things like
               the default .nusmvrc, the on-line help files, etc. It
               is the responsibility of the user to free the returned
               string.]

  SideEffects []

  SeeAlso     [CInit_NuSMVReadVersion]

******************************************************************************/
char* CInit_NuSMVObtainLibrary()
{
#if NUSMV_HAVE_GETENV
  char * nusmv_lib_path;

  nusmv_lib_path = getenv("NuSMV_LIBRARY_PATH");
  if (nusmv_lib_path) {
    return util_tilde_expand(nusmv_lib_path);
  } else {
    return util_tilde_expand(NUSMV_SHARE_PATH);
  }
#else
#warning "Support of CInit_NuSMVObtainLibrary is poor"
  return util_tilde_expand(NUSMV_SHARE_PATH);
#endif
}

/**Function********************************************************************

  Synopsis           [Start piping stdout through the "more" command]

  Description        [This function is  called to initialize piping
                      stdout through "more". It is important to call
                      CInit_NuSMVEndPrintMore before returning from
                      your function and after calling
                      CInit_NuSMVInitPrintMore (preferably at the end
                      of your printing; failing to do so will cause
                      the stdin lines not to appear).]

  SideEffects        []

  SeeAlso            [ CInit_NuSMVEndPrintMore]

******************************************************************************/
void CInit_NuSMVInitPrintMore()
{
    fflush(nusmv_stdout);
#if NUSMV_HAVE_POPEN
    nusmv_stdpipe = popen("more","w");
#else
#warning "Pipes are not supported"
#endif
}
/**Function********************************************************************

  Synopsis           [Stop piping stdout through the "more" command]

  Description        [This function is  called to terminate piping
                      stdout through "more". It is important to call
                      CInit_NuSMVEndPrintMore before exiting your
                      function (preferably at the end of your
                      printing; failing to do so will cause the stdin
                      lines not to appear). The function returns a 0
                      if it fails.]

  SideEffects        []

  SeeAlso            [ CInit_NuSMVInitPrintMore]

******************************************************************************/
int CInit_NuSMVEndPrintMore()
{
#if NUSMV_HAVE_POPEN
    if (nusmv_stdpipe != NIL(FILE)) {
      (void) pclose(nusmv_stdpipe);
      return 1;
    }
    return 0;
#else
    return 1;
#endif
}


/**Function********************************************************************

  Synopsis    [Prints the banner of NuSMV.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void CInit_BannerPrint(FILE * file)
{
  cinit_banner_print(file, false);
}


/**Function********************************************************************

  Synopsis    [Prints the COMPLETE banner of the NuSMV library.]

  Description [To be used by addons linking against the NuSMV library.
               You can use this as banner print function if you don't
               need a special banner print function and you are
               linking against NuSMV]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void CInit_BannerPrintLibrary(FILE * file)
{
  cinit_banner_print(file, true);
}


/**Function********************************************************************

  Synopsis    [Prints the banner of the NuSMV library.]

  Description [To be used by tools linking against the NuSMV library
               and using custom banner function]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void CInit_BannerPrint_nusmv_library(FILE * file)
{
  fprintf(file, "*** This version of %s is linked to %s %s.\n",
          NuSMVCore_get_tool_name(), NuSMVCore_get_library_name(),
          NuSMVCore_get_library_version());

  fprintf(file, "*** For more information on %s see <%s>\n",
          NuSMVCore_get_library_name(), NuSMVCore_get_library_website());

  fprintf(file, "*** or email to <%s>.\n", NuSMVCore_get_library_email());

  fprintf(file, "*** Copyright (C) 2010 by Fondazione Bruno Kessler\n\n");
}

/**Function********************************************************************

  Synopsis    [Prints the banner of cudd.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void CInit_BannerPrint_cudd(FILE * file)
{
  fprintf(file,
          "*** This version of %s is linked to the CUDD library version %s\n",
          NuSMVCore_get_tool_name(), CUDD_VERSION);
  fprintf(file,
          "*** Copyright (c) 1995-2004, Regents of the University of Colorado\n\n");

  fflush(NULL); /* to flush all the banner before any other output */
}


/**Function********************************************************************

  Synopsis    [Prints the banner of minisat.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void CInit_BannerPrint_minisat(FILE * file)
{
  fprintf(file,
          "*** This version of %s is linked to the MiniSat SAT solver. \n",
          NuSMVCore_get_tool_name());
  fprintf(file,
          "*** See http://www.cs.chalmers.se/Cs/Research/FormalMethods/MiniSat\n");
  fprintf(file,
          "*** Copyright (c) 2003-2005, Niklas Een, Niklas Sorensson \n\n");

  fflush(NULL); /* to flush all the banner before any other output */
}


/**Function********************************************************************

  Synopsis    [Prints the banner of zchaff.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void CInit_BannerPrint_zchaff(FILE * file)
{
  int i;

  fprintf(file,
          "WARNING *** This version of %s is linked to the zchaff SAT",
          NuSMVCore_get_tool_name());
  for (i = 0; i < 13 - strlen(NuSMVCore_get_tool_name()); i++) {
    fprintf(file, " ");
  }
  fprintf(file, " ***\n");
  fprintf(file,"WARNING *** solver (see http://www.princeton.edu/~chaff/zchaff.html). ***\n");
  fprintf(file, "WARNING *** Zchaff is used in Bounded Model Checking when the         ***\n");
  fprintf(file, "WARNING *** system variable \"sat_solver\" is set to \"zchaff\".          ***\n");
  fprintf(file, "WARNING *** Notice that zchaff is for non-commercial purposes only.   ***\n");
  fprintf(file, "WARNING *** NO COMMERCIAL USE OF ZCHAFF IS ALLOWED WITHOUT WRITTEN    ***\n");
  fprintf(file, "WARNING *** PERMISSION FROM PRINCETON UNIVERSITY.                     ***\n");
  fprintf(file, "WARNING *** Please contact Sharad Malik (malik@ee.princeton.edu)      ***\n");
  fprintf(file, "WARNING *** for details.                                              ***\n\n");

  fflush(NULL); /* to flush all the banner before any other output */
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Prints the banner of NuSMV. ]

  Description [Prints the banner of NuSMV. If is_linked is true,
               also the NuSMV library banner is output]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void cinit_banner_print(FILE* file, boolean is_linked)
{
  fprintf(file, "*** This is %s %s (compiled on %s)\n",
          NuSMVCore_get_tool_name(),
          NuSMVCore_get_tool_version(),
          NuSMVCore_get_build_date());
# ifdef NUSMV_LINKED_CORE_ADDONS
  /* linked addons are printed only if not empty */
  if (strcmp(NuSMVCore_get_linked_addons(), "") != 0) {
    fprintf(file, "*** Enabled addons are: %s\n",
            NuSMVCore_get_linked_addons());
  }
#endif
  fprintf(file, "*** For more information on %s see <%s>\n",
          NuSMVCore_get_tool_name(), NuSMVCore_get_website());

  fprintf(file, "*** or email to <%s>.\n",
          NuSMVCore_get_email());
  fprintf(file, "*** %s\n\n",
          NuSMVCore_get_bug_report_message());

  fprintf(file, "*** Copyright (c) 2010, Fondazione Bruno Kessler\n\n");

  if (is_linked) {
    CInit_BannerPrint_nusmv_library(file);
  }

  /* Cudd license */
  CInit_BannerPrint_cudd(file);

# if NUSMV_HAVE_SOLVER_MINISAT
  CInit_BannerPrint_minisat(file);
# endif

# if NUSMV_HAVE_SOLVER_ZCHAFF
  CInit_BannerPrint_zchaff(file);
# endif

  fflush(NULL); /* to flush all the banner before any other output */
}


/**Function********************************************************************

  Synopsis    [Returns the date in a brief format assuming its coming from
  the program `date'.]

  Description [optional]

  SideEffects []

******************************************************************************/
static char *
DateReadFromDateString(
  char * datestr)
{
  static char result[25];
  char        day[10];
  char        month[10];
  char        zone[10];
  char       *at;
  int         date;
  int         hour;
  int         minute;
  int         second;
  int         year;
  int c;

  if (sscanf(datestr, "%s %s %2d %2d:%2d:%2d %s %4d",
             day, month, &date, &hour, &minute, &second, zone, &year) == 8) {
    if (hour >= 12) {
      if (hour >= 13) hour -= 12;
      at = "PM";
    }
    else {
      if (hour == 0) hour = 12;
      at = "AM";
    }
    c = snprintf(result, sizeof(result), "%d-%3s-%02d at %d:%02d %s",
                 date, month, year % 100, hour, minute, at);
    SNPRINTF_CHECK(c, sizeof(result));
    return result;
  }
  else {
    return datestr;
  }
}





