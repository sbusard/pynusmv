/**CHeaderFile*****************************************************************

  FileName    [pslNode.h]

  PackageName [parser.psl]

  Synopsis    [PslNode interface]

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

  Revision    [$Id: pslNode.h,v 1.1.4.9.4.11 2010-01-04 23:01:16 nusmv Exp $]

******************************************************************************/

#ifndef __PSL_NODE_H__
#define __PSL_NODE_H__

#include "node/node.h"
#include "utils/utils.h"
#include "utils/error.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef node_ptr PslNode_ptr;

typedef short int PslOp;

typedef enum PslOpConvType_TAG {
  TOK2PSL, 
  TOK2SMV, 
  PSL2SMV, 
  PSL2PSL, 
  PSL2TOK
} PslOpConvType;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro**********************************************************************
  Synopsis     [This value represents a null PslNode]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PSL_NULL ((PslNode_ptr) NULL)

/**Macro**********************************************************************
  Synopsis     [Casts the given node to an int]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PSLNODE_TO_INT(x) \
    ((int) (nusmv_ptrint) x)

/**Macro**********************************************************************
  Synopsis     [Casts the given int to a PslNode_ptr]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define PSLNODE_FROM_INT(x) \
    ((PslNode_ptr) (nusmv_ptrint) x)


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


EXTERN node_ptr PslNode_convert_psl_to_core ARGS((PslNode_ptr expr));

EXTERN PslNode_ptr psl_new_node ARGS((PslOp op,
                                      PslNode_ptr left, PslNode_ptr right));
EXTERN PslNode_ptr psl_node_get_left ARGS((PslNode_ptr n));
EXTERN PslNode_ptr psl_node_get_right ARGS((PslNode_ptr n));
EXTERN PslOp psl_node_get_op ARGS((PslNode_ptr n));
EXTERN void psl_node_set_left ARGS((PslNode_ptr n, PslNode_ptr l));
EXTERN void psl_node_set_right ARGS((PslNode_ptr n, PslNode_ptr r));

EXTERN PslNode_ptr PslNode_convert_from_node_ptr ARGS((node_ptr expr));
EXTERN node_ptr PslNode_convert_to_node_ptr ARGS((PslNode_ptr expr));

EXTERN PslNode_ptr psl_node_make_true ARGS(()); 
EXTERN PslNode_ptr psl_node_make_false ARGS(()); 

EXTERN boolean psl_node_is_true ARGS((PslNode_ptr e)); 
EXTERN boolean psl_node_is_false ARGS((PslNode_ptr e)); 

EXTERN PslNode_ptr psl_node_prune ARGS((PslNode_ptr tree, PslNode_ptr branch));

EXTERN boolean psl_node_is_sere ARGS((PslNode_ptr expr));
EXTERN PslNode_ptr psl_node_sere_star_get_count ARGS((const PslNode_ptr e));
EXTERN boolean psl_node_is_handled_star ARGS((PslNode_ptr expr, boolean toplevel));
EXTERN boolean psl_node_sere_is_propositional ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_repeated ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_star ARGS((PslNode_ptr e));

EXTERN PslNode_ptr psl_node_sere_star_get_starred ARGS((PslNode_ptr e));

EXTERN boolean psl_node_sere_is_stareq ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_starminusgt ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_standalone_star ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_plus ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_standalone_plus ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_star_count ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_star_count_zero ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_concat_holes_free ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_concat_fusion ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_concat_fusion_holes_free ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_2ampersand ARGS((PslNode_ptr e));

EXTERN boolean psl_node_is_suffix_implication ARGS((PslNode_ptr expr));
EXTERN boolean psl_node_is_suffix_implication_weak ARGS((PslNode_ptr expr));
EXTERN boolean psl_node_is_suffix_implication_strong ARGS((PslNode_ptr expr));

EXTERN boolean psl_node_is_propstar ARGS((PslNode_ptr e));

EXTERN boolean psl_node_is_ite ARGS((PslNode_ptr _ite));
EXTERN PslNode_ptr psl_node_get_ite_cond ARGS((PslNode_ptr _ite));
EXTERN PslNode_ptr psl_node_get_ite_then ARGS((PslNode_ptr _ite));
EXTERN PslNode_ptr psl_node_get_ite_else ARGS((PslNode_ptr _ite));

EXTERN boolean psl_node_is_case ARGS((PslNode_ptr _case));
EXTERN PslNode_ptr psl_node_get_case_cond ARGS((PslNode_ptr _case));
EXTERN PslNode_ptr psl_node_get_case_then ARGS((PslNode_ptr _case));
EXTERN PslNode_ptr psl_node_get_case_next ARGS((PslNode_ptr _case));

EXTERN boolean psl_node_is_serebrackets ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_concat ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_fusion ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_concat_get_left ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_concat_get_right ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_concat_get_leftmost ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_concat_get_rightmost ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_concat_cut_leftmost ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_fusion_get_left ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_fusion_get_right ARGS((PslNode_ptr e));
EXTERN boolean psl_node_sere_is_or ARGS((PslNode_ptr e));

EXTERN PslNode_ptr psl_node_sere_propositional_get_expr ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_compound_get_left ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_compound_get_right ARGS((PslNode_ptr e));

EXTERN PslNode_ptr psl_node_make_sere_propositional ARGS((PslNode_ptr expr));
EXTERN PslNode_ptr psl_node_make_sere_concat ARGS((PslNode_ptr seq1, PslNode_ptr seq2));
EXTERN PslNode_ptr psl_node_make_sere_2ampersand ARGS((PslNode_ptr seq1, PslNode_ptr seq2));
EXTERN PslNode_ptr psl_node_make_sere_star ARGS((PslNode_ptr seq));

EXTERN PslNode_ptr psl_node_make_sere_compound ARGS((PslNode_ptr seq1, PslOp op,
                                                     PslNode_ptr seq2));
EXTERN PslNode_ptr psl_node_make_sere_compound ARGS((PslNode_ptr seq1, PslOp op,
                                                     PslNode_ptr seq2));
EXTERN boolean psl_node_is_sere_compound_binary ARGS((PslNode_ptr e));

EXTERN PslNode_ptr psl_node_make_cons ARGS((PslNode_ptr elem, PslNode_ptr next));
EXTERN PslNode_ptr psl_node_make_cons_new ARGS((PslNode_ptr elem, PslNode_ptr next));

EXTERN boolean psl_node_is_boolean_type ARGS((PslNode_ptr expr));

EXTERN boolean psl_node_is_leaf ARGS((PslNode_ptr expr));
EXTERN boolean psl_node_is_infinite ARGS((PslNode_ptr expr));
EXTERN boolean psl_node_is_id ARGS((PslNode_ptr expr)); 
EXTERN boolean psl_node_is_id_equal ARGS((PslNode_ptr _id1, PslNode_ptr _id2)); 

EXTERN boolean psl_node_is_number ARGS((PslNode_ptr e)); 
EXTERN boolean psl_node_is_word_number ARGS((PslNode_ptr e)); 

EXTERN PslNode_ptr psl_node_make_number ARGS((int value)); 
EXTERN int psl_node_number_get_value ARGS((PslNode_ptr e)); 
EXTERN boolean psl_node_is_num_equal ARGS((PslNode_ptr _id1, PslNode_ptr _id2)); 

EXTERN PslNode_ptr 
psl_node_make_failure ARGS((const char* msg, FailureKind kind));

EXTERN PslNode_ptr 
psl_node_make_case ARGS((PslNode_ptr _cond, 
                         PslNode_ptr _then, PslNode_ptr _next));

EXTERN boolean psl_node_is_range ARGS((PslNode_ptr expr)); 
EXTERN PslNode_ptr psl_node_range_get_low ARGS((PslNode_ptr expr)); 
EXTERN PslNode_ptr psl_node_range_get_high ARGS((PslNode_ptr expr)); 

EXTERN boolean psl_node_is_cons ARGS((PslNode_ptr e)); 
EXTERN PslNode_ptr psl_node_cons_get_element ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_cons_get_next ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_cons_reverse ARGS((PslNode_ptr e));

EXTERN PslNode_ptr psl_node_suffix_implication_get_premise ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_suffix_implication_get_consequence ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_repeated_get_expr ARGS((PslNode_ptr e));
EXTERN PslNode_ptr psl_node_sere_repeated_get_count ARGS((PslNode_ptr e));
EXTERN PslOp psl_node_sere_repeated_get_op ARGS((PslNode_ptr e));

EXTERN boolean psl_node_is_repl_prop ARGS((PslNode_ptr _prop));
EXTERN PslNode_ptr psl_node_repl_prop_get_property ARGS((PslNode_ptr _prop));
EXTERN PslNode_ptr psl_node_repl_prop_get_replicator ARGS((PslNode_ptr _prop));

EXTERN boolean psl_node_is_replicator ARGS((PslNode_ptr _repl));
EXTERN PslNode_ptr psl_node_get_replicator_value_set ARGS((PslNode_ptr _repl));
EXTERN PslOp psl_node_get_replicator_join_op ARGS((PslNode_ptr _repl));
EXTERN PslNode_ptr 
psl_node_get_replicator_normalized_value_set ARGS((PslNode_ptr rep));

EXTERN PslNode_ptr psl_node_get_replicator_range ARGS((PslNode_ptr _repl));
EXTERN PslNode_ptr psl_node_get_replicator_id ARGS((PslNode_ptr _repl));

EXTERN PslNode_ptr psl_node_context_to_main_context ARGS((PslNode_ptr context));

EXTERN PslNode_ptr PslNode_new_context ARGS((PslNode_ptr ctx, PslNode_ptr node));

EXTERN PslNode_ptr psl_node_make_extended_next ARGS((PslOp op, PslNode_ptr expr,
                                                     PslNode_ptr when,
                                                     PslNode_ptr condition));
EXTERN boolean psl_node_is_extended_next ARGS((PslNode_ptr e)); 
EXTERN PslNode_ptr psl_node_extended_next_get_expr ARGS((PslNode_ptr next));
EXTERN PslNode_ptr psl_node_extended_next_get_when ARGS((PslNode_ptr next));

EXTERN PslNode_ptr 
psl_node_extended_next_get_condition ARGS((PslNode_ptr next));



/* Predicates */
EXTERN boolean PslNode_is_handled_psl ARGS((PslNode_ptr e));
EXTERN boolean PslNode_is_propositional ARGS((const PslNode_ptr expr));
EXTERN boolean PslNode_is_trans_propositional ARGS((const PslNode_ptr expr));
EXTERN boolean PslNode_propositional_contains_next ARGS((const PslNode_ptr expr));
EXTERN boolean PslNode_is_obe ARGS((const PslNode_ptr expr));
EXTERN boolean PslNode_is_ltl ARGS((const PslNode_ptr expr));

/* convert */
PslNode_ptr PslNode_convert_id ARGS((PslNode_ptr id, PslOpConvType type));
PslNode_ptr PslNode_pslobe2ctl ARGS((PslNode_ptr expr, PslOpConvType type)); 
PslNode_ptr PslNode_pslltl2ltl ARGS((PslNode_ptr expr, PslOpConvType type));
PslNode_ptr PslNode_remove_sere ARGS((PslNode_ptr e));
PslNode_ptr PslNode_remove_forall_replicators ARGS((PslNode_ptr e));


#endif /* __PSL_NODE_H__ */
