/**CHeaderFile*****************************************************************

  FileName    [rbc.h]

  PackageName [rbc]

  Synopsis    [Formula handling with Reduced Boolean Circuits (RBCs).]

  Description [External functions and data structures of the rbc package.]

  SeeAlso     []

  Author      [Armando Tacchella, Marco Roveri]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by University of Genova.

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

  Revision    [$Id: rbc.h,v 1.4.6.4.2.3.2.4.6.11 2010-02-18 10:00:03 nusmv Exp $]

******************************************************************************/
#ifndef _RBC
#define _RBC


#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

/* Standard includes. */
#if NUSMV_HAVE_MALLOC_H
# if NUSMV_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <malloc.h>
#elif NUSMV_HAVE_SYS_MALLOC_H
# if NUSMV_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <sys/malloc.h>
#elif NUSMV_HAVE_STDLIB_H
# include <stdlib.h>
#endif

#include <stdio.h>

/* Submodule includes. */
#include "dag/dag.h"
#include "utils/list.h"
#include "utils/Slist.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define RBC_TSEITIN_CONVERSION_NAME  "tseitin"

#define RBC_SHERIDAN_CONVERSION_NAME "sheridan"
#define RBC_INVALID_CONVERSION_NAME  "invalid"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Enum**********************************************************************
  Synopsis      [RBC CNF conversion algorithm.]
  Description   [RBC CNF conversion algorithm.]
  SeeAlso       []
******************************************************************************/
typedef enum _Rbc_2CnfAlgorithm {
  RBC_INVALID_CONVERSION = 0,
  RBC_TSEITIN_CONVERSION,
  RBC_SHERIDAN_CONVERSION
} Rbc_2CnfAlgorithm;

/**Enum**********************************************************************
  Synopsis      [RBC boolean values.]
  Description   [RBC boolean values.]
  SeeAlso       []
******************************************************************************/
typedef enum Rbc_Bool {
  RBC_FALSE = DAG_ANNOTATION_BIT,
  RBC_TRUE = 0
} Rbc_Bool_c;

typedef struct RbcManager Rbc_Manager_t;
typedef Dag_Vertex_t      Rbc_t;
typedef Dag_DfsFunctions_t RbcDfsFunctions_t;
typedef void (*Rbc_ProcPtr_t)();
typedef int (*Rbc_IntPtr_t)();

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*---------------------------------------------------------------------------*/
struct RbcDfsFunctions {
  Rbc_IntPtr_t   Set;
  Rbc_ProcPtr_t  FirstVisit;
  Rbc_ProcPtr_t  BackVisit;
  Rbc_ProcPtr_t  LastVisit;
};

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define RBC_INVALID_SUBST_VALUE \
   INT_MAX

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void Rbc_pkg_init ARGS((void));
EXTERN void Rbc_pkg_quit ARGS((void));

EXTERN int Rbc_Convert2Cnf
ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f, int polarity,
      Slist_ptr clauses, Slist_ptr vars, int* literalAssignedToWholeFormula));

EXTERN int Rbc_CnfVar2RbcIndex ARGS((Rbc_Manager_t* rbcManager, int cnfVar));
EXTERN int Rbc_RbcIndex2CnfVar ARGS((Rbc_Manager_t* rbcManager, int rbcIndex));

EXTERN Rbc_t* Rbc_GetOne ARGS((Rbc_Manager_t* rbcManager));
EXTERN Rbc_t* Rbc_GetZero ARGS((Rbc_Manager_t* rbcManager));
EXTERN boolean Rbc_IsConstant ARGS((Rbc_Manager_t* manager, Rbc_t* f));

EXTERN Rbc_t* Rbc_GetIthVar ARGS((Rbc_Manager_t* rbcManager, int varIndex));
EXTERN Rbc_t* Rbc_MakeNot ARGS((Rbc_Manager_t* rbcManager, Rbc_t* left));

EXTERN Rbc_t* Rbc_MakeAnd
ARGS((Rbc_Manager_t* rbcManager, Rbc_t* left, Rbc_t* right, Rbc_Bool_c sign));

EXTERN Rbc_t* Rbc_MakeOr
ARGS((Rbc_Manager_t* rbcManager, Rbc_t* left, Rbc_t* right, Rbc_Bool_c sign));

EXTERN Rbc_t* Rbc_MakeIff
ARGS((Rbc_Manager_t* rbcManager, Rbc_t* left, Rbc_t* right, Rbc_Bool_c sign));

EXTERN Rbc_t* Rbc_MakeXor
ARGS((Rbc_Manager_t* rbcManager, Rbc_t* left, Rbc_t* right, Rbc_Bool_c sign));

EXTERN Rbc_t* Rbc_MakeIte
ARGS((Rbc_Manager_t* rbcManager, Rbc_t* c, Rbc_t* t, Rbc_t* e,
      Rbc_Bool_c sign));

EXTERN Rbc_t* Rbc_GetLeftOpnd ARGS((Rbc_t* f));
EXTERN Rbc_t* Rbc_GetRightOpnd ARGS((Rbc_t* f));
EXTERN int Rbc_GetVarIndex ARGS((Rbc_t* f));
EXTERN void Rbc_Mark ARGS((Rbc_Manager_t* rbc, Rbc_t* f));
EXTERN void Rbc_Unmark ARGS((Rbc_Manager_t* rbc, Rbc_t* f));
EXTERN Rbc_Manager_t* Rbc_ManagerAlloc ARGS((int varCapacity));

EXTERN void Rbc_ManagerReserve
ARGS((Rbc_Manager_t* rbcManager, int newVarCapacity));

EXTERN void Rbc_ManagerReset
ARGS((Rbc_Manager_t* rbcManager));

EXTERN int Rbc_ManagerCapacity ARGS((Rbc_Manager_t* rbcManager));
EXTERN void Rbc_ManagerFree ARGS((Rbc_Manager_t* rbcManager));
EXTERN void Rbc_ManagerGC ARGS((Rbc_Manager_t* rbcManager));

EXTERN void
Rbc_OutputDaVinci ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f, FILE* outFile));

EXTERN void
Rbc_OutputSexpr ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f, FILE* outFile));

EXTERN void
Rbc_OutputGdl ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f, FILE* outFile));

EXTERN Rbc_t*
Rbc_Subst ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f, int* subst));

EXTERN Rbc_t* Rbc_LogicalSubst ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f,
                                     int* subst, const int* log2phy,
                                     const int* phy2log));

EXTERN Rbc_t* Rbc_Shift ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f, int shift));

EXTERN Rbc_t*
Rbc_LogicalShift ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f,
                       int shift, const int* log2phy, const int* phy2log));
EXTERN Rbc_t*
Rbc_SubstRbc ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f, Rbc_t** substRbc));

EXTERN Rbc_t*
Rbc_LogicalSubstRbc ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f,
                          Rbc_t** substRbc, int* phy2log));
EXTERN void
Rbc_PrintStats ARGS((Rbc_Manager_t* rbcManager, int clustSz, FILE* outFile));

EXTERN Slist_ptr
RbcUtils_get_dependencies ARGS((Rbc_Manager_t* rbcManager, Rbc_t* f,
                                boolean reset_dag));

EXTERN struct InlineResult_TAG*
RbcInline_apply_inlining ARGS((Rbc_Manager_t* rbcm, Rbc_t* f));

EXTERN Rbc_2CnfAlgorithm
Rbc_CnfConversionAlgorithmFromStr ARGS((const char* str));

EXTERN const char *
Rbc_CnfConversionAlgorithm2Str ARGS((Rbc_2CnfAlgorithm algo));

EXTERN const char *
Rbc_CnfGetValidRbc2CnfAlgorithms ARGS((void));

EXTERN boolean Rbc_is_top ARGS((Rbc_t* rbc));

EXTERN boolean Rbc_is_var ARGS((Rbc_t* rbc));

EXTERN boolean Rbc_is_and ARGS((Rbc_t* rbc));

EXTERN boolean Rbc_is_iff ARGS((Rbc_t* rbc));

EXTERN boolean Rbc_is_ite ARGS((Rbc_t* rbc));

EXTERN void Rbc_Dfs_exported ARGS((Rbc_t* dfsRoot,
                                   RbcDfsFunctions_t* dfsFun,
                                   void* dfsData,
                                   Rbc_Manager_t* manager));

EXTERN void Rbc_Dfs_clean_exported ARGS((Rbc_t* dfsRoot,
                                         Rbc_Manager_t* manager));

/**AutomaticEnd***************************************************************/

#endif /* _RBC */

