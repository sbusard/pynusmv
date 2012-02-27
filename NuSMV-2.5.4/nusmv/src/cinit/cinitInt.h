/**CHeaderFile*****************************************************************

  FileName    [cinitInt.h]

  PackageName [cinit]

  Synopsis    [Internal declarations for the main package.]

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

#ifndef _CINIT_INT
#define _CINIT_INT

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#if NUSMV_STDC_HEADERS
#  include <stdlib.h>
#else
   char * getenv();
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#if NUSMV_HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "cinit/cinit.h"
#include "util.h"
#include "utils/utils.h"
#include "node/node.h"
#include "dd/dd.h"
#include "parser/symbols.h"
#include "parser/parser.h"
#include "rbc/rbc.h"
#include "set/set.h"
#include "compile/compile.h"
#include "mc/mc.h"
#include "simulate/simulate.h"
#include "ltl/ltl.h"
#include "cmd/cmd.h"
#include "opt/opt.h"
#include "bmc/bmc.h"
#include "fsm/FsmBuilder.h"
#include "trace/TraceManager.h"
#include "trace/pkg_trace.h"
#include "hrc/hrc.h"


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void cinitBatchMain ARGS((void));

EXTERN void print_usage ARGS((FILE *));
EXTERN void cinit_AddCmd ARGS((void));

EXTERN void CInit_init ARGS((void));
EXTERN void CInit_end ARGS((void));
EXTERN void CInit_reset_first ARGS((void));
EXTERN void CInit_reset_last ARGS((void));

/*---------------------------------------------------------------------------*/
/* Variable declaration                                                      */
/*---------------------------------------------------------------------------*/

EXTERN FsmBuilder_ptr global_fsm_builder; 
EXTERN TraceManager_ptr global_trace_manager;

extern int longjmp_on_err;
extern DdManager * dd_manager;
extern FILE * nusmv_stderr;
extern FILE * nusmv_stdout;
extern FILE * nusmv_stdin;
extern FILE* def_nusmv_stderr;
extern FILE* def_nusmv_stdout;
extern cmp_struct_ptr cmps;
extern node_ptr parsed_tree;

#endif /* _CINIT_INT */
