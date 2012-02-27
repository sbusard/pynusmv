/**CHeaderFile*****************************************************************

  FileName    [Prop.h]

  PackageName [prop]

  Synopsis    [Public interface of class 'Prop']

  Description [This file is responsible of manipulate all the
  informations associated to a given property, i.e. the kind of
  property, the property itself, its cone of influence, if the
  property is not satisfied the associated copunter-example, the
  associated FSM in different formats (flatten sexp, flatten boolean
  sexp, bdd, and BE).]

  SeeAlso     [Prop.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``prop'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

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

  Revision    [$Id: $]

******************************************************************************/


#ifndef __PROP_H__
#define __PROP_H__

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "fsm/FsmBuilder.h"
#include "fsm/sexp/Expr.h"
#include "set/set.h"
#include "fsm/sexp/SexpFsm.h"
#include "fsm/sexp/BoolSexpFsm.h"
#include "fsm/bdd/BddFsm.h"
#include "fsm/be/BeFsm.h"

#include "utils/object.h"
#include "utils/utils.h"


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/**Enum************************************************************************

  Synopsis    [The status of a property]

  Description [The status of a property, i.e. If it is checked,
  unchecked, satisifed or unsatisfied.]

  SeeAlso     [optional]

******************************************************************************/
enum _Prop_Status {Prop_NoStatus, Prop_Unchecked, Prop_True, Prop_False,
                   Prop_Number};

#define PROP_NOSTATUS_STRING "NoStatus"
#define PROP_UNCHECKED_STRING "Unchecked"
#define PROP_TRUE_STRING "True"
#define PROP_FALSE_STRING "False"
#define PROP_NUMBER_STRING "Number"


/**Enum************************************************************************

  Synopsis    [Enumerates the different types of a specification]

  Description [Enumerates the different types of a specification]

******************************************************************************/
enum _Prop_Type {
  Prop_Prop_Type_First = 100, /* Do not touch this */
  /* ---------------------------------------------------------------------- */
  Prop_NoType,
  Prop_Ctl,
  Prop_Ltl,
  Prop_Psl,
  Prop_Invar,
  Prop_Compute,
  Prop_CompId, /* For properties names comparison */
  /* ---------------------------------------------------------------------- */
  Prop_Prop_Type_Last /* Do not touch this */
};


/**Enum************************************************************************

  Synopsis    [Format used when printing]

  Description []

  SeeAlso     []

******************************************************************************/
enum _PropDb_PrintFmt {
  PROPDB_PRINT_FMT_TABULAR, 
  PROPDB_PRINT_FMT_DEFAULT = PROPDB_PRINT_FMT_TABULAR,
  PROPDB_PRINT_FMT_XML, 
};

enum _Prop_PrintFmt {
  PROP_PRINT_FMT_FORMULA,
  PROP_PRINT_FMT_FORMULA_TRUNC,
  PROP_PRINT_FMT_INDEX,
  PROP_PRINT_FMT_NAME,
  PROP_PRINT_FMT_DEFAULT = PROP_PRINT_FMT_FORMULA
};

#define PROP_NOTYPE_STRING "NoType"
#define PROP_CTL_STRING "CTL"
#define PROP_LTL_STRING "LTL"
#define PROP_PSL_STRING "PSL"
#define PROP_INVAR_STRING "Invar"
#define PROP_COMPUTE_STRING "Quantitative"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef enum _Prop_Status Prop_Status;
typedef enum _Prop_Type Prop_Type;
typedef enum _PropDb_PrintFmt PropDb_PrintFmt;
typedef enum _Prop_PrintFmt Prop_PrintFmt;


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class Prop]

  Description []

******************************************************************************/
typedef struct Prop_TAG*  Prop_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class Prop]

  Description [These macros must be used respectively to cast and to check
  instances of class Prop]

******************************************************************************/
#define PROP(self) \
         ((Prop_ptr) self)

#define PROP_CHECK_INSTANCE(self) \
         (nusmv_assert(PROP(self) != PROP(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Prop_ptr Prop_create ARGS((void));
EXTERN Prop_ptr Prop_create_partial ARGS((Expr_ptr expr, Prop_Type type));

EXTERN void Prop_destroy ARGS((Prop_ptr self));



EXTERN Expr_ptr Prop_get_expr ARGS((const Prop_ptr self));
EXTERN Expr_ptr Prop_get_expr_core ARGS((const Prop_ptr self));
EXTERN Expr_ptr Prop_get_expr_core_for_coi ARGS((const Prop_ptr self));
EXTERN Set_t    Prop_get_cone ARGS((const Prop_ptr self));

EXTERN Prop_Type Prop_get_type ARGS((const Prop_ptr self));
EXTERN const char* Prop_get_type_as_string ARGS((Prop_ptr self));

EXTERN node_ptr Prop_get_name ARGS((const Prop_ptr self));
EXTERN void Prop_set_name ARGS((Prop_ptr self, node_ptr name));

EXTERN Prop_Status Prop_get_status ARGS((const Prop_ptr self));
EXTERN const char* Prop_get_status_as_string ARGS((const Prop_ptr self));

EXTERN int Prop_get_number ARGS((const Prop_ptr self));
EXTERN char* Prop_get_number_as_string ARGS((const Prop_ptr self));

EXTERN int Prop_get_trace ARGS((const Prop_ptr self));


EXTERN void Prop_set_cone ARGS((Prop_ptr self, Set_t cone));
EXTERN void Prop_set_status ARGS((Prop_ptr self, Prop_Status s));
EXTERN void Prop_set_number ARGS((Prop_ptr self, int n));
EXTERN void Prop_set_number_infinite ARGS((Prop_ptr self));
EXTERN void Prop_set_number_undefined ARGS((Prop_ptr self));
EXTERN void Prop_set_trace ARGS((Prop_ptr self, int t));

EXTERN int Prop_get_index ARGS((const Prop_ptr self));
EXTERN void Prop_set_index ARGS((Prop_ptr self, const int index));

EXTERN node_ptr Prop_get_name ARGS((const Prop_ptr self));
EXTERN void Prop_set_name ARGS((Prop_ptr self, const node_ptr name));

EXTERN char* Prop_get_name_as_string ARGS((const Prop_ptr self));

EXTERN boolean Prop_needs_rewriting ARGS((const Prop_ptr self));

EXTERN SexpFsm_ptr
Prop_compute_ground_sexp_fsm ARGS((const Prop_ptr self,
                                   const FsmBuilder_ptr builder,
                                   const SymbTable_ptr symb_table));
EXTERN BddFsm_ptr
Prop_compute_ground_bdd_fsm ARGS((const Prop_ptr self,
                                  const FsmBuilder_ptr builder));
EXTERN BeFsm_ptr
Prop_compute_ground_be_fsm ARGS((const Prop_ptr self,
                                 const FsmBuilder_ptr builder));

EXTERN SexpFsm_ptr Prop_get_scalar_sexp_fsm ARGS((const Prop_ptr self));
EXTERN BoolSexpFsm_ptr Prop_get_bool_sexp_fsm ARGS((const Prop_ptr self));
EXTERN BddFsm_ptr  Prop_get_bdd_fsm ARGS((const Prop_ptr self));
EXTERN BeFsm_ptr Prop_get_be_fsm ARGS((const Prop_ptr self));

EXTERN void Prop_set_scalar_sexp_fsm ARGS((Prop_ptr self, SexpFsm_ptr fsm));
EXTERN void Prop_set_bool_sexp_fsm ARGS((Prop_ptr self, BoolSexpFsm_ptr fsm));
EXTERN void Prop_set_bdd_fsm ARGS((Prop_ptr self, BddFsm_ptr fsm));
EXTERN void Prop_set_be_fsm ARGS((Prop_ptr self, BeFsm_ptr fsm));

EXTERN Set_t  Prop_compute_cone ARGS((const Prop_ptr self,
                                      FlatHierarchy_ptr hierarchy,
                                      SymbTable_ptr symb_table));

EXTERN void
Prop_apply_coi_for_scalar ARGS((Prop_ptr self, FsmBuilder_ptr helper,
                                FlatHierarchy_ptr hierarchy,
                                SymbTable_ptr symb_table));

EXTERN void
Prop_apply_coi_for_bdd ARGS((Prop_ptr self, FsmBuilder_ptr helper));

EXTERN void
Prop_apply_coi_for_bmc ARGS((Prop_ptr self, FsmBuilder_ptr helper));

EXTERN void Prop_destroy_coi_for_bmc ARGS((Prop_ptr self));

EXTERN void Prop_verify ARGS((Prop_ptr self));

EXTERN void Prop_print ARGS((Prop_ptr self, FILE*, Prop_PrintFmt fmt));
EXTERN void Prop_print_db ARGS((Prop_ptr self, FILE*, PropDb_PrintFmt));

EXTERN int Prop_check_type ARGS((const Prop_ptr self, Prop_Type type));

EXTERN char* Prop_get_text ARGS((const Prop_ptr self));
EXTERN char* Prop_get_context_text ARGS((const Prop_ptr self));

EXTERN boolean Prop_is_psl_ltl ARGS((const Prop_ptr self));
EXTERN boolean Prop_is_psl_obe ARGS((const Prop_ptr self));

EXTERN const char* PropType_to_string ARGS((const Prop_Type type));


/**AutomaticEnd***************************************************************/



#endif /* __PROP_H__ */
