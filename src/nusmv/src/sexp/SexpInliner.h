/**CHeaderFile*****************************************************************

  FileName    [SexpInliner.h]

  PackageName [utils]

  Synopsis    [The SexpInliner API]

  Description [Class SexpInliner declaration]

  SeeAlso     [SexpInliner.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``sexp'' package of NuSMV version 2.
  Copyright (C) 2008 by FBK-irst.

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

#ifndef __FSM_SEXP_INLINER_H__
#define __FSM_SEXP_INLINER_H__

#include "fsm/sexp/Expr.h"
#include "compile/symb_table/SymbTable.h"
#include "set/set.h"
#include "utils/utils.h"


/**Type***********************************************************************

  Synopsis     [The SexpInliner type ]

  Description  [The SexpInliner type ]

  Notes        []

******************************************************************************/
typedef struct SexpInliner_TAG* SexpInliner_ptr;


#define SEXP_INLINER(x) \
         ((SexpInliner_ptr) x)

#define SEXP_INLINER_CHECK_INSTANCE(x) \
         (nusmv_assert(SEXP_INLINER(x) != SEXP_INLINER(NULL)))


/**Type***********************************************************************

  Synopsis     [Inliner result type ]

  Description  [Inliner result type ]

  Notes        []

******************************************************************************/
typedef struct InlineRes_TAG* InlineRes_ptr;


#define INLINE_RES(x) \
         ((InlineRes_ptr) x)

#define INLINE_RES_CHECK_INSTANCE(x) \
         (nusmv_assert(INLINE_RES(x) != INLINE_RES(NULL)))


/*---------------------------------------------------------------------------*/
/* Public Function Interface                                                 */
/*---------------------------------------------------------------------------*/


/* ===================  SexpInliner  =================== */
EXTERN SexpInliner_ptr SexpInliner_create ARGS((SymbTable_ptr st,
                                                const size_t fixpoint_limit));

EXTERN SexpInliner_ptr SexpInliner_copy ARGS((const SexpInliner_ptr self));
EXTERN void SexpInliner_destroy ARGS((SexpInliner_ptr self));

EXTERN SymbTable_ptr
SexpInliner_get_symb_table ARGS((const SexpInliner_ptr self));

EXTERN boolean
SexpInliner_force_equivalence ARGS((SexpInliner_ptr self,
                                    node_ptr var, Expr_ptr expr));

EXTERN boolean
SexpInliner_force_equivalences ARGS((SexpInliner_ptr self, Set_t equivs));

EXTERN boolean
SexpInliner_force_invariant ARGS((SexpInliner_ptr self,
                                  node_ptr var, Expr_ptr expr));

EXTERN boolean
SexpInliner_force_invariants ARGS((SexpInliner_ptr self, Set_t invars));

EXTERN void
SexpInliner_blacklist_name ARGS((SexpInliner_ptr self, node_ptr var));

EXTERN void
SexpInliner_clear_equivalences ARGS((SexpInliner_ptr self));

EXTERN void
SexpInliner_clear_invariants ARGS((SexpInliner_ptr self));

EXTERN void
SexpInliner_clear_blacklist ARGS((SexpInliner_ptr self));

EXTERN InlineRes_ptr
SexpInliner_inline ARGS((SexpInliner_ptr self, Expr_ptr expr,
                         boolean* changed));

EXTERN Expr_ptr
SexpInliner_inline_no_learning ARGS((SexpInliner_ptr self, Expr_ptr expr,
                                     boolean* changed));

EXTERN hash_ptr
SexpInliner_get_var2expr_hash ARGS((SexpInliner_ptr self));

EXTERN hash_ptr
SexpInliner_get_var2invar_hash ARGS((SexpInliner_ptr self));

/* ===================  InlineRes  =================== */
EXTERN void InlineRes_destroy ARGS((InlineRes_ptr self));

EXTERN Expr_ptr
InlineRes_get_original_expr ARGS((const InlineRes_ptr self));

EXTERN Expr_ptr
InlineRes_get_result ARGS((const InlineRes_ptr self));

EXTERN Expr_ptr
InlineRes_get_result_unique ARGS((const InlineRes_ptr self));

EXTERN Expr_ptr
InlineRes_get_inlined_expr ARGS((const InlineRes_ptr self));

EXTERN Expr_ptr
InlineRes_get_equivalences_expr ARGS((const InlineRes_ptr self));

EXTERN Set_t
InlineRes_get_equivalences ARGS((const InlineRes_ptr self));

EXTERN Expr_ptr
InlineRes_get_invariant_expr ARGS((const InlineRes_ptr self));

EXTERN Set_t
InlineRes_get_invariants ARGS((const InlineRes_ptr self));

#endif /* __SEXP_INLINER_H__ */
