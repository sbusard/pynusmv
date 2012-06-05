 /**CHeaderFile*****************************************************************

  FileName    [simulateInt.h]

  PackageName [simulate]

  Synopsis    [Internal Header File for the simulate package]

  Description [Internal Header File for the simulate package]

  SeeAlso     [simulate.c]

  Author      [Andrea Morichetti]

  Copyright   [
  This file is part of the ``simulate'' package of NuSMV version 2. 
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

  Revision    [$Id: simulateInt.h,v 1.4.4.12.4.6.4.3 2010-01-29 12:50:45 nusmv Exp $]

******************************************************************************/

#ifndef _SIMULATEINT
#define _SIMULATEINT

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif 

#include <stdio.h>
#include <stdlib.h>

#include "simulate.h"

#include "utils/utils.h"
#include "dd/dd.h"
#include "opt/opt.h"

#include "fsm/FsmBuilder.h"
#include "fsm/bdd/BddFsm.h"

#include "compile/compile.h"
#include "trace/Trace.h"
#include "trace/TraceManager.h"


#if NUSMV_HAVE_SYS_SIGNAL_H
#  include <sys/signal.h>
#endif
#if NUSMV_HAVE_SIGNAL_H
#  include <signal.h>
#endif

#include <setjmp.h>
#include <assert.h>



/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/* Length of the string used for the choice entered in interac. sim.*/
#define CHOICE_LENGTH 8


EXTERN DdManager* dd_manager;
EXTERN cmp_struct_ptr cmps;
EXTERN int trace_number;

EXTERN FILE* nusmv_stdin;
EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;

EXTERN TraceManager_ptr global_trace_manager;
EXTERN FsmBuilder_ptr global_fsm_builder; 

EXTERN char* simulation_buffer;
EXTERN size_t simulation_buffer_size;

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN int 
Simulate_CmdPickOneState ARGS((BddFsm_ptr, Simulation_Mode, int, char *));
EXTERN int Simulate_CmdShowTraces ARGS((int, int, boolean, char *));

EXTERN 
bdd_ptr simulate_get_constraints_from_string ARGS((const char* constr_str, 
                                                   BddEnc_ptr enc, 
                                                   boolean allow_nexts,
                                                   boolean allow_inputs));


EXTERN bdd_ptr current_state_bdd_get ARGS((void));
EXTERN void current_state_set ARGS((bdd_ptr state, TraceLabel label)); 

#endif /* _SIMULATEINT */
