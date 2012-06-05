/**CHeaderFile*****************************************************************

  FileName    [Expr.h]

  PackageName [fsm.sexp]

  Synopsis    [Interface for Expr type]

  Description []

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

  Revision    [$Id: Expr.h,v 1.1.2.2.4.1.6.14 2010/01/26 14:36:27 nusmv Exp $]

******************************************************************************/


#ifndef __FSM_SEXP_EXPR_H__
#define __FSM_SEXP_EXPR_H__

#include "utils/utils.h"
#include "node/node.h"
#include "compile/symb_table/SymbTable.h"

/**Type***********************************************************************

  Synopsis     [The Expr type ]

  Description  [An Expr is any expression represented as a sexpr object]

******************************************************************************/
typedef node_ptr Expr_ptr;

#define EXPR(x) \
      ((Expr_ptr) x)

#define EXPR_CHECK_INSTANCE(x) \
      (nusmv_assert(EXPR(x) != EXPR(NULL)))

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define Expr_get_type(t) \
  node_get_type(t)

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN Expr_ptr Expr_true ARGS((void));
EXTERN Expr_ptr Expr_false ARGS((void));
EXTERN boolean Expr_is_true ARGS((const Expr_ptr expr));
EXTERN boolean Expr_is_false ARGS((const Expr_ptr expr));

#ifdef SYMB_TABLE_STATIC
EXTERN Expr_ptr Expr_next ARGS((const Expr_ptr a, SymbTable_ptr st));
EXTERN Expr_ptr Expr_ite ARGS((const Expr_ptr cond,
                               const Expr_ptr t,
                               const Expr_ptr e,
                               SymbTable_ptr st));
EXTERN Expr_ptr Expr_equal ARGS((const Expr_ptr a,
                                 const Expr_ptr b,
                                 SymbTable_ptr st));
EXTERN Expr_ptr Expr_notequal ARGS((const Expr_ptr a,
                                    const Expr_ptr b,
                                    SymbTable_ptr st));
EXTERN Expr_ptr Expr_le ARGS((const Expr_ptr a,
                              const Expr_ptr b,
                              SymbTable_ptr st));
EXTERN Expr_ptr Expr_ge ARGS((const Expr_ptr a,
                              const Expr_ptr b,
                              SymbTable_ptr st));
EXTERN Expr_ptr Expr_simplify_word_extend ARGS((SymbTable_ptr st,
                                                Expr_ptr w, Expr_ptr i));
EXTERN Expr_ptr Expr_attime ARGS((Expr_ptr e, int time, SymbTable_ptr st));
EXTERN Expr_ptr Expr_word_constant ARGS((SymbTable_ptr st,
                                                  int type,
                                                  Expr_ptr w,
                                                  Expr_ptr i));
EXTERN Expr_ptr Expr_and_from_list ARGS((node_ptr list, SymbTable_ptr st));
#else
EXTERN Expr_ptr Expr_next ARGS((const Expr_ptr a, const SymbTable_ptr st));
EXTERN Expr_ptr Expr_ite ARGS((const Expr_ptr cond,
                               const Expr_ptr t,
                               const Expr_ptr e,
                               const SymbTable_ptr st));
EXTERN Expr_ptr Expr_equal ARGS((const Expr_ptr a,
                                 const Expr_ptr b,
                                 const SymbTable_ptr st));
EXTERN Expr_ptr Expr_notequal ARGS((const Expr_ptr a,
                                    const Expr_ptr b,
                                    const SymbTable_ptr st));
EXTERN Expr_ptr Expr_le ARGS((const Expr_ptr a,
                              const Expr_ptr b,
                              const SymbTable_ptr st));
EXTERN Expr_ptr Expr_ge ARGS((const Expr_ptr a,
                              const Expr_ptr b,
                              const SymbTable_ptr st));
EXTERN Expr_ptr Expr_simplify_word_extend ARGS((const SymbTable_ptr st,
                                                Expr_ptr w, Expr_ptr i));
EXTERN Expr_ptr Expr_attime ARGS((Expr_ptr e, int time,
                                  const SymbTable_ptr st));
EXTERN Expr_ptr Expr_word_constant ARGS((const SymbTable_ptr st,
                                                  int type,
                                                  Expr_ptr w,
                                                  Expr_ptr i));
EXTERN Expr_ptr Expr_and_from_list ARGS((node_ptr list,
                                         const SymbTable_ptr st));
#endif
/* endif SYMB_TABLE_STATIC */


EXTERN Expr_ptr Expr_and ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_and_nil ARGS((const Expr_ptr a, const Expr_ptr b));


EXTERN Expr_ptr Expr_not ARGS((const Expr_ptr expr));
EXTERN Expr_ptr Expr_or ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_xor ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_xnor ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_iff ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_implies ARGS((const Expr_ptr a, const Expr_ptr b));

EXTERN Expr_ptr Expr_lt ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_simplify_lt ARGS((const SymbTable_ptr st,
                                       const Expr_ptr a,
                                       const Expr_ptr b));

EXTERN Expr_ptr Expr_gt ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_simplify_gt ARGS((const SymbTable_ptr st,
                                       const Expr_ptr a,
                                       const Expr_ptr b));

EXTERN Expr_ptr Expr_plus ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_minus ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_times ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_divide ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_mod ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_unary_minus ARGS((const Expr_ptr a));

EXTERN Expr_ptr Expr_word_left_shift ARGS((const Expr_ptr a,
                                           const Expr_ptr b));
EXTERN Expr_ptr Expr_word_right_shift ARGS((const Expr_ptr a,
                                            const Expr_ptr b));
EXTERN Expr_ptr Expr_word_left_rotate ARGS((const Expr_ptr a,
                                            const Expr_ptr b));
EXTERN Expr_ptr Expr_word_right_rotate ARGS((const Expr_ptr a,
                                             const Expr_ptr b));
EXTERN Expr_ptr Expr_word_bit_select ARGS((const Expr_ptr w,
                                           const Expr_ptr r));
EXTERN Expr_ptr Expr_simplify_word_bit_select ARGS((const SymbTable_ptr st,
                                                    const Expr_ptr w,
                                                    const Expr_ptr r));
EXTERN Expr_ptr Expr_word_concatenate ARGS((const Expr_ptr a,
                                            const Expr_ptr b));

EXTERN Expr_ptr Expr_word1_to_bool ARGS((Expr_ptr w));
EXTERN Expr_ptr Expr_bool_to_word1 ARGS((Expr_ptr a));
EXTERN Expr_ptr Expr_signed_word_to_unsigned ARGS((Expr_ptr w));
EXTERN Expr_ptr Expr_unsigned_word_to_signed ARGS((Expr_ptr w));
EXTERN Expr_ptr Expr_word_extend ARGS((Expr_ptr w,
                                       Expr_ptr i,
                                       const SymbTable_ptr st));

EXTERN int Expr_attime_get_time ARGS((Expr_ptr e));
EXTERN Expr_ptr Expr_attime_get_untimed ARGS((Expr_ptr e));

EXTERN Expr_ptr Expr_union ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_setin ARGS((const Expr_ptr a, const Expr_ptr b, const SymbTable_ptr st));
EXTERN Expr_ptr Expr_range ARGS((const Expr_ptr a, const Expr_ptr b));
EXTERN Expr_ptr Expr_function ARGS((const Expr_ptr name,
                                    const Expr_ptr params));

EXTERN Expr_ptr Expr_resolve ARGS((SymbTable_ptr st,
                                   int type, Expr_ptr left, Expr_ptr right));
EXTERN Expr_ptr Expr_simplify ARGS((SymbTable_ptr st, Expr_ptr expr));

EXTERN boolean Expr_is_timed ARGS((Expr_ptr expr, hash_ptr cache));

EXTERN int Expr_get_time ARGS((SymbTable_ptr st, Expr_ptr expr));

EXTERN Expr_ptr Expr_untimed ARGS((SymbTable_ptr st, Expr_ptr expr));

EXTERN Expr_ptr Expr_untimed_explicit_time ARGS((SymbTable_ptr st,
                                                 Expr_ptr expr,
                                                 int time));

EXTERN Expr_ptr Expr_wsizeof ARGS((Expr_ptr l, Expr_ptr r));

EXTERN Expr_ptr Expr_cast_toint ARGS((Expr_ptr l, Expr_ptr r));
#endif /* __FSM_SEXP_EXPR_H__ */
