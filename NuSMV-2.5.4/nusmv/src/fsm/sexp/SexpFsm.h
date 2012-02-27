/**CHeaderFile*****************************************************************

  FileName    [SexpFsm.h]

  PackageName [fsm.sexp]

  Synopsis    [The SexpFsm API]

  Description [Class SexpFsm declaration]

  SeeAlso     [SexpFsm.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.sexp'' package of NuSMV version 2.
  Copyright (C) 2003 by FBK-irst.

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

#ifndef __FSM_SEXP_SEXP_FSM_H__
#define __FSM_SEXP_SEXP_FSM_H__

#include "fsm/sexp/sexp.h"
#include "fsm/sexp/Expr.h"

#include "set/set.h"
#include "compile/FlatHierarchy.h"
#include "compile/symb_table/SymbLayer.h"
#include "compile/PredicateNormaliser.h"
#include "enc/bdd/BddEnc.h"


/**Type***********************************************************************

  Synopsis     [The SexpFsm type ]

  Description  [The SexpFsm type ]

  Notes        []

******************************************************************************/
typedef struct SexpFsm_TAG* SexpFsm_ptr;


#define SEXP_FSM(x) \
         ((SexpFsm_ptr) x)

#define SEXP_FSM_CHECK_INSTANCE(x) \
         (nusmv_assert(SEXP_FSM(x) != SEXP_FSM(NULL)))

/*---------------------------------------------------------------------------*/
/* Public Function Interface                                                 */
/*---------------------------------------------------------------------------*/

/* constructors */
EXTERN SexpFsm_ptr
SexpFsm_create ARGS((const FlatHierarchy_ptr hierarchy,
                     const Set_t vars_set));

EXTERN SexpFsm_ptr SexpFsm_copy ARGS((const SexpFsm_ptr self));

/* convertion to predicate-normalised FSM */
EXTERN SexpFsm_ptr SexpFsm_create_predicate_normalised_copy
ARGS((const SexpFsm_ptr self,
      PredicateNormaliser_ptr normaliser));

/* deconstructors */
EXTERN void SexpFsm_destroy ARGS((SexpFsm_ptr self));

EXTERN SymbTable_ptr SexpFsm_get_symb_table ARGS((const SexpFsm_ptr self));

EXTERN boolean SexpFsm_is_boolean ARGS((const SexpFsm_ptr self));

/* access functions */
EXTERN FlatHierarchy_ptr
SexpFsm_get_hierarchy ARGS((const SexpFsm_ptr self));

EXTERN boolean SexpFsm_is_boolean ARGS((const SexpFsm_ptr self));

EXTERN Expr_ptr SexpFsm_get_init ARGS((const SexpFsm_ptr self));

EXTERN Expr_ptr SexpFsm_get_invar ARGS((const SexpFsm_ptr self));

EXTERN Expr_ptr SexpFsm_get_trans ARGS((const SexpFsm_ptr self));

EXTERN Expr_ptr SexpFsm_get_input ARGS((const SexpFsm_ptr self));

EXTERN node_ptr SexpFsm_get_justice ARGS((const SexpFsm_ptr self));

EXTERN node_ptr SexpFsm_get_compassion ARGS((const SexpFsm_ptr self));

EXTERN NodeList_ptr SexpFsm_get_vars_list ARGS((const SexpFsm_ptr self));

EXTERN NodeList_ptr SexpFsm_get_symbols_list ARGS((const SexpFsm_ptr self));

EXTERN Set_t SexpFsm_get_vars ARGS((const SexpFsm_ptr self));

EXTERN void
SexpFsm_apply_synchronous_product ARGS((SexpFsm_ptr self,
                                        SexpFsm_ptr other));

EXTERN boolean
SexpFsm_is_syntactically_universal ARGS((SexpFsm_ptr self));

EXTERN Expr_ptr
SexpFsm_get_var_init ARGS((const SexpFsm_ptr self, node_ptr v));

EXTERN Expr_ptr
SexpFsm_get_var_invar ARGS((const SexpFsm_ptr self, node_ptr v));

EXTERN Expr_ptr
SexpFsm_get_var_trans ARGS((const SexpFsm_ptr self, node_ptr v));

EXTERN Expr_ptr
SexpFsm_get_var_input ARGS((const SexpFsm_ptr self, node_ptr v));

EXTERN void SexpFsm_self_check ARGS((const SexpFsm_ptr self));


#endif /* __FSM_SEXP_SEXP_FSM_H__ */
