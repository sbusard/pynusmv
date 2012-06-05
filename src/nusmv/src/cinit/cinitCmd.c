/**CFile***********************************************************************

  FileName    [cinitCmd.c]

  PackageName [cinit]

  Synopsis    [Interface of the cinit package with the shell.]

  Description [Interface of the cinit package with the shell.]

  SeeAlso     []

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

#include "cinitInt.h"

#include "utils/error.h"
#include "enc/enc.h"
#include "trace/pkg_trace.h"
#include "simulate/simulateInt.h"

int CommandCmdReset(int argc, char **argv);
int CommandPrintUsage(int argc, char **argv);
/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int UsageCmdReset ARGS((void));
static int UsagePrintUsage ARGS((void));
/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
void cinit_AddCmd(void){
  Cmd_CommandAdd("reset", CommandCmdReset, 0, false);
  Cmd_CommandAdd("print_usage", CommandPrintUsage, 0, true);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Implements the reset command.]

  CommandName        [reset]

  CommandSynopsis    [Resets the whole system.]

  CommandArguments   [\[-h\]]

  CommandDescription [Resets the whole system, in order to read in
  another model and to perform verification on it.
  <p>
  Command options:<p>
  <dl>
    <dt> -h
       <dd> Prints the command usage.
  </dl>]

  SideEffects        []

******************************************************************************/
int CommandCmdReset(int argc, char ** argv)
{
  int c;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c){
    case 'h': return(UsageCmdReset());
    default:  return(UsageCmdReset());
    }
  }
  if (argc != util_optind) return(UsageCmdReset());

  NuSMVCore_reset();

  return 0;
}


static int UsageCmdReset()
{
  fprintf(nusmv_stderr, "usage: reset [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return(1);
}


/**Function********************************************************************

  Synopsis           [Implements the print_usage command.]

  CommandName        [print_usage]

  CommandSynopsis    [Prints processor and BDD statistics.]

  CommandArguments   [\[-h\]]

  CommandDescription [Prints a formatted dump of processor-specific
  usage statistics, and BDD usage statistics. For Berkeley Unix, this
  includes all of the information in the <tt>getrusage()</tt> structure.
  <p>
  Command options:<p>
  <dl>
    <dt> -h
       <dd> Prints the command usage.
  </dl>]

  SideEffects        [required]

******************************************************************************/
int CommandPrintUsage(int argc, char **argv)
{
  int c;

  util_getopt_reset();
  while((c = util_getopt(argc,argv,"h")) != EOF){
    switch(c) {
    case 'h': return(UsagePrintUsage());
    default:  return(UsagePrintUsage());
    }
  }
  /* Reporting of statistical information. */
  print_usage(nusmv_stdout);
  return(0);
}

static int UsagePrintUsage()
{
  fprintf(nusmv_stderr, "usage: print_usage [-h]\n");
  fprintf(nusmv_stderr, "   -h \t\tPrints the command usage.\n");
  return(1);
}



