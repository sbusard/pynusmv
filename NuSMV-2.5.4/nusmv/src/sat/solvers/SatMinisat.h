/**CHeaderFile*****************************************************************

  FileName    [SatMinisat.h]

  PackageName [SatMinisat]

  Synopsis    [The header file for the SatMinisat class.]

  Description [Minisat is an incremental SAT solver.
  SatMinisat inherits the SatIncSolver (interface) class]

  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``sat'' package of NuSMV version 2.
  Copyright (C) 2004 by FBK-irst.

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

  Revision    [$Id: SatMinisat.h,v 1.1.2.2.2.2 2005-03-08 16:17:26 nusmv Exp $]

******************************************************************************/

#ifndef __SAT_SOLVER_SAT_MINISAT__H
#define __SAT_SOLVER_SAT_MINISAT__H

#include "sat/SatIncSolver.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct SatMinisat_TAG* SatMinisat_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define SAT_MINISAT(x)                          \
  ((SatMinisat_ptr) x)

#define SAT_MINISAT_CHECK_INSTANCE(x)                   \
  (nusmv_assert(SAT_MINISAT(x) != SAT_MINISAT(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* SatMinisat Constructor/Destructors */
EXTERN SatMinisat_ptr SatMinisat_create ARGS((const char* name,
                                              boolean enable_proof_logging));
EXTERN void SatMinisat_destroy ARGS((SatMinisat_ptr self));

/**AutomaticEnd***************************************************************/

#endif /* __SAT_SOLVER_SAT_MINISAT__H */
