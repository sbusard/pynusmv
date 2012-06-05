/**CHeaderFile*****************************************************************

  FileName    [pslExpr.h]

  PackageName [parser.psl]

  Synopsis    [PSL parser interface]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.psl'' package of NuSMV version 2. 
  Copyright (C) 2005 by FBK-irst. 

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

  Revision    [$Id: pslExpr.h,v 1.1.4.3.6.4 2009-11-02 17:50:12 nusmv Exp $]

******************************************************************************/

#ifndef __PSL_EXPR_H__
#define __PSL_EXPR_H__

#include "pslNode.h"
#include "utils/utils.h"
#include "utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef enum SyntaxClass_TAG 
{
  SC_NUM_EXPR,       /* numerical or id */
  SC_BOOL_EXPR,      /* boolean or id */
  SC_WORD_EXPR,
  SC_IDENTIFIER,     /* only id */
  SC_NUM_BOOL_WORD_EXPR,  /* boolean or numerical or word or id */
  SC_NUM_BOOL_EXPR,  /* boolean or numerical or id */
  
  SC_BOOL_WORD_EXPR, /* Boolean or word or id */

  SC_NUM_WORD_EXPR, /* numerical or word or id operation */

  SC_PROPERTY, 
  SC_FL_PROPERTY, 
  SC_OBE_PROPERTY, 

  SC_SEQUENCE, 
  SC_REPLICATOR, 
  SC_NONE, 
  SC_RANGE, 
  SC_LIST, 
  SC_NUM_RANGE       /* number, id or range */  
} SyntaxClass ;


typedef struct PslExpr_TAG 
{
  SyntaxClass klass;
  PslNode_ptr psl_node;
} PslExpr;




/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* Shortcuts for unary operators
   Legend ------------------------------  
    B : boolean (or identifier)
    N : numeric (or identifier)
    W : Word    (or identifier)
    NBW: B or N or W
    NW: N or W
    BW: B or W
    T: the same type of the operand 
    F: fl property
    O: obe property
    ------------------------------------ */
#define PSL_EXPR_MAKE_W2W_OP(res, right, op)                            \
  psl_expr_make_unary_op(&res, &right, op, SC_WORD_EXPR, SC_WORD_EXPR)

#define PSL_EXPR_MAKE_B2W_OP(res, right, op)                            \
  psl_expr_make_unary_op(&res, &right, op, SC_BOOL_EXPR, SC_WORD_EXPR)

#define PSL_EXPR_MAKE_W2B_OP(res, right, op)                            \
  psl_expr_make_unary_op(&res, &right, op, SC_WORD_EXPR, SC_BOOL_EXPR)

#define PSL_EXPR_MAKE_W2N_OP(res, right, op)                            \
  psl_expr_make_unary_op(&res, &right, op, SC_WORD_EXPR, SC_BOOL_EXPR)

#define PSL_EXPR_MAKE_NW2NW_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_NUM_WORD_EXPR, SC_NUM_WORD_EXPR)

#define PSL_EXPR_MAKE_BW2BW_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_BOOL_WORD_EXPR, SC_BOOL_WORD_EXPR)

#define PSL_EXPR_MAKE_N2N_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_NUM_EXPR, SC_NUM_EXPR)

#define PSL_EXPR_MAKE_N2B_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_NUM_EXPR, SC_BOOL_EXPR)

#define PSL_EXPR_MAKE_B2B_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_BOOL_EXPR, SC_BOOL_EXPR)

#define PSL_EXPR_MAKE_NBW2N_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_NUM_BOOL_WORD_EXPR, SC_NUM_EXPR)

#define PSL_EXPR_MAKE_NBW2B_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_NUM_BOOL_WORD_EXPR, SC_BOOL_EXPR)

#define PSL_EXPR_MAKE_F2F_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_FL_PROPERTY, SC_FL_PROPERTY)

#define PSL_EXPR_MAKE_B2F_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, SC_BOOL_EXPR, SC_FL_PROPERTY)

/* this preserves the right's klass */
#define PSL_EXPR_MAKE_T2T_OP(res, right, op) \
  psl_expr_make_unary_op(&res, &right, op, right.klass, right.klass)

/* Shortcuts for binary operators: */
#define PSL_EXPR_MAKE_W_N2W_OP(res, left, op, right)            \
  psl_expr_make_binary_mixed_op(&res, &left, op, &right,        \
                                SC_WORD_EXPR, SC_NUM_EXPR, SC_WORD_EXPR)
/* Shortcuts for binary operators: */
#define PSL_EXPR_MAKE_N_N2W_OP(res, left, op, right)            \
  psl_expr_make_binary_op(&res, &left, op, &right,              \
                          SC_NUM_EXPR, SC_WORD_EXPR)

#define PSL_EXPR_MAKE_N_N2N_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_NUM_EXPR, SC_NUM_EXPR)

#define PSL_EXPR_MAKE_N_N2B_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_NUM_EXPR, SC_BOOL_EXPR)


#define PSL_EXPR_MAKE_NB_NB2B_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_NUM_BOOL_EXPR, SC_BOOL_EXPR)


#define PSL_EXPR_MAKE_BW_BW2BW_OP(res, left, op, right)         \
  psl_expr_make_binary_op(&res, &left, op, &right,              \
                          SC_BOOL_WORD_EXPR, SC_BOOL_WORD_EXPR)

#define PSL_EXPR_MAKE_B_B2B_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_BOOL_EXPR, SC_BOOL_EXPR)

#define PSL_EXPR_MAKE_W_W2W_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_WORD_EXPR, SC_WORD_EXPR)

#define PSL_EXPR_MAKE_W_N2W_OP(res, left, op, right)                    \
  psl_expr_make_binary_mixed_op(&res, &left, op, &right,                \
                                SC_WORD_EXPR, SC_NUM_EXPR, SC_WORD_EXPR)

#define PSL_EXPR_MAKE_W_NW2W_OP(res, left, op, right)                    \
  psl_expr_make_binary_mixed_op(&res, &left, op, &right,                \
                                SC_WORD_EXPR, SC_NUM_WORD_EXPR, SC_WORD_EXPR)

#define PSL_EXPR_MAKE_N_W2W_OP(res, left, op, right)                    \
  psl_expr_make_binary_mixed_op(&res, &left, op, &right,                \
                                SC_NUM_EXPR, SC_WORD_EXPR, SC_WORD_EXPR)

#define PSL_EXPR_MAKE_NB_NB2N_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_NUM_BOOL_EXPR, SC_NUM_EXPR)

#define PSL_EXPR_MAKE_NW_NW2NW_OP(res, left, op, right)                  \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_NUM_WORD_EXPR, SC_NUM_WORD_EXPR)

#define PSL_EXPR_MAKE_NW_NW2B_OP(res, left, op, right)                  \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_NUM_WORD_EXPR, SC_BOOL_EXPR)

#define PSL_EXPR_MAKE_NBW_NBW2N_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_NUM_BOOL_WORD_EXPR, SC_NUM_EXPR)

#define PSL_EXPR_MAKE_NBW_NBW2B_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_NUM_BOOL_WORD_EXPR, SC_BOOL_EXPR)

#define PSL_EXPR_MAKE_F_F2F_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_FL_PROPERTY, SC_FL_PROPERTY)

#define PSL_EXPR_MAKE_B_B2F_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, SC_BOOL_EXPR, SC_FL_PROPERTY)

#define PSL_EXPR_MAKE_T_T2T_OP(res, left, op, right) \
  psl_expr_make_binary_op(&res, &left, op, &right, left.klass, left.klass)

#define PSL_EXPR_MAKE_EXT_NEXT_OP_BOOL(res, operator, fl_property, bool_expr) \
   psl_expr_make_extended_next_op(operator, &fl_property, NULL, &bool_expr, &res);

#define PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN(res, operator, fl_property, when) \
   psl_expr_make_extended_next_op(operator, &fl_property, &when, NULL, &res);

#define PSL_EXPR_MAKE_EXT_NEXT_OP_WHEN_BOOL(res, operator, fl_property, \
                    when, bool_expr)            \
   psl_expr_make_extended_next_op(operator, &fl_property, &when, &bool_expr, &res);


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


EXTERN void psl_expr_make_unary_op ARGS((PslExpr* res, 
                                         const PslExpr* right, 
                                         const PslOp op_id, 
                                         const SyntaxClass right_req_klass, 
                                         const SyntaxClass res_klass));

EXTERN void psl_expr_make_binary_op ARGS((PslExpr* res, 
                                          const PslExpr* left, 
                                          const PslOp op_id, 
                                          const PslExpr* right, 
                                          const SyntaxClass ops_req_klass, 
                                          const SyntaxClass res_klass));

EXTERN void psl_expr_make_binary_mixed_op ARGS((PslExpr* res, 
                                                const PslExpr* left, 
                                                const PslOp op_id, 
                                                const PslExpr* right, 
                                                const SyntaxClass left_req_klass, 
                                                const SyntaxClass right_req_klass, 
                                                const SyntaxClass res_klass));


EXTERN void psl_expr_make_extended_next_op ARGS((PslOp op_id, 
                                                 const PslExpr* fl_property, 
                                                 const PslExpr* when, 
                                                 const PslExpr* bool_expr, 
                                                 PslExpr* res));

EXTERN PslExpr psl_expr_make_replicator ARGS((PslOp op_id, 
                                              PslExpr id, PslExpr range, 
                                              PslExpr value_set));

EXTERN PslExpr 
psl_expr_make_replicated_property ARGS((PslExpr replicator, PslExpr expr));

EXTERN PslExpr psl_expr_make_atom ARGS((const char* str));
EXTERN PslExpr psl_expr_make_id ARGS((PslExpr left, PslExpr right));
EXTERN PslExpr psl_expr_make_id_array ARGS((PslExpr id, PslExpr num));
EXTERN PslExpr psl_expr_make_context ARGS((PslExpr ctx, PslExpr node));
EXTERN PslExpr psl_expr_make_empty ARGS(());
EXTERN PslExpr psl_expr_make_true ARGS(());
EXTERN PslExpr psl_expr_make_false ARGS(());
EXTERN PslExpr psl_expr_make_inf ARGS(());
EXTERN PslExpr psl_expr_make_boolean_type ARGS(());
EXTERN PslExpr psl_expr_make_boolean_value ARGS((int val));
EXTERN PslExpr psl_expr_make_failure ARGS((const char* msg, FailureKind kind));

EXTERN PslExpr psl_expr_make_number ARGS((int val));
EXTERN PslExpr psl_expr_make_base_number ARGS((char* base_num));
EXTERN PslExpr psl_expr_make_real_number ARGS((char* fval));
EXTERN PslExpr psl_expr_make_word_number ARGS((char* wval));
EXTERN PslExpr psl_expr_make_range ARGS((PslExpr low, PslExpr high));

EXTERN PslExpr 
psl_expr_make_case ARGS((PslExpr cond, PslExpr _then, PslExpr _list));

EXTERN PslExpr 
psl_expr_make_ite ARGS((PslExpr cond, PslExpr _then, PslExpr _else));

EXTERN PslExpr 
psl_expr_make_suffix_implication_weak ARGS((PslExpr seq, PslOp op, 
                                            PslExpr expr));

EXTERN PslExpr 
psl_expr_make_suffix_implication_strong ARGS((PslExpr seq, PslOp op, 
                                              PslExpr expr));

EXTERN PslExpr 
psl_expr_make_within ARGS((PslOp op, PslExpr begin, PslExpr end, 
                           PslExpr seq));

EXTERN PslExpr 
psl_expr_make_whilenot ARGS((PslOp op, PslExpr expr, PslExpr seq));

EXTERN PslExpr psl_expr_make_abort ARGS((PslExpr fl_prop, PslExpr cond));
EXTERN PslExpr psl_expr_make_sere ARGS((PslExpr expr));
EXTERN PslExpr psl_expr_make_sere_concat ARGS((PslExpr seq1, PslExpr seq2));
EXTERN PslExpr psl_expr_make_sere_fusion ARGS((PslExpr seq1, PslExpr seq2));

EXTERN PslExpr 
psl_expr_make_sere_compound_binary_op ARGS((PslExpr seq1, PslOp op, 
                                            PslExpr seq2));

EXTERN PslExpr 
psl_expr_make_repeated_sere ARGS((PslOp op, PslExpr sere, PslExpr count));

EXTERN PslExpr psl_expr_make_cons ARGS((PslExpr a, PslExpr b));

EXTERN PslExpr psl_expr_make_cons_new ARGS((PslExpr a, PslExpr b));

EXTERN PslExpr psl_expr_make_concatenation ARGS((PslExpr expr_list));

EXTERN PslExpr 
psl_expr_make_multiple_concatenation ARGS((PslExpr expr, PslExpr expr_list));

EXTERN PslExpr psl_expr_make_obe_unary ARGS((PslOp op, PslExpr expr));

EXTERN PslExpr 
psl_expr_make_obe_binary ARGS((PslExpr left, PslOp op, PslExpr right));

EXTERN PslExpr 
psl_expr_make_bit_selection ARGS((PslExpr word_expr, PslExpr left, PslExpr right));

EXTERN PslExpr 
psl_expr_make_word_concatenation ARGS((PslExpr left, PslExpr right));


#endif /* __PSL_EXPR_H__ */
