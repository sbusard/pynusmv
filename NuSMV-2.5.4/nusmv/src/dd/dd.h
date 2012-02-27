/**CHeaderFile*****************************************************************

  FileName    [dd.h]

  PackageName [dd]

  Synopsis    [Header file for Decisison Diagram Package.]

  Description [External functions and data strucures of the DD
  package. The BDD or ADD returned as a result of an operation are
  always referenced (see the CUDD User Manual for more details about
  this), and need to be dereferenced when the result is no more
  necessary to computation, in order to release the memory associated
  to it when garbage collection occurs.]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``dd'' package of NuSMV version 2. 
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

  Revision    [$Id: dd.h,v 1.4.6.5.2.1.2.7.4.12 2009-11-30 14:53:13 nusmv Exp $]

******************************************************************************/

#ifndef _DD_H
#define _DD_H

#include "utils/utils.h"
#include "utils/array.h"
#include "utils/avl.h"
#include "node/node.h"
#include "cudd.h"
#include "opt/OptsHandler.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct DdNode * add_ptr;
typedef struct DdNode * bdd_ptr;
typedef struct DdNode * dd_ptr; /* represents both add_ptr and bdd_ptr */

typedef struct MtrNode dd_block;
typedef Cudd_ReorderingType dd_reorderingtype;

typedef void (*VPFDD)(DdManager *, bdd_ptr);
typedef node_ptr (*NPFDD)(DdManager *, bdd_ptr);
typedef void (*VPFCVT)(CUDD_VALUE_TYPE);
typedef node_ptr (*NPFCVT)(CUDD_VALUE_TYPE);

typedef add_ptr (*FP_A_DA)(DdManager*, add_ptr);
typedef add_ptr (*FP_A_DAA)(DdManager*, add_ptr, add_ptr);

typedef DdGen dd_gen;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#ifndef MAX_VAR_INDEX
#define MAX_VAR_INDEX            CUDD_MAXINDEX
#endif

/* initial size of the unique tables */
#define UNIQUE_SLOTS             CUDD_UNIQUE_SLOTS

/* initial size of the cache */
#define CACHE_SLOTS              CUDD_CACHE_SLOTS

/* use value currently stored in the manager. */
#define REORDER_SAME             CUDD_REORDER_SAME
/* no reardering at all */
#define REORDER_NONE             CUDD_REORDER_NONE

#define REORDER_RANDOM           CUDD_REORDER_RANDOM
#define REORDER_RANDOM_PIVOT     CUDD_REORDER_RANDOM_PIVOT
#define REORDER_SIFT             CUDD_REORDER_SIFT
#define REORDER_SIFT_CONV        CUDD_REORDER_SIFT_CONVERGE
#define REORDER_SYMM_SIFT        CUDD_REORDER_SYMM_SIFT
#define REORDER_SYMM_SIFT_CONV   CUDD_REORDER_SYMM_SIFT_CONV
#define REORDER_WINDOW2          CUDD_REORDER_WINDOW2
#define REORDER_WINDOW3          CUDD_REORDER_WINDOW3
#define REORDER_WINDOW4          CUDD_REORDER_WINDOW4
#define REORDER_WINDOW2_CONV     CUDD_REORDER_WINDOW2_CONV
#define REORDER_WINDOW3_CONV     CUDD_REORDER_WINDOW3_CONV
#define REORDER_WINDOW4_CONV     CUDD_REORDER_WINDOW4_CONV
#define REORDER_GROUP_SIFT       CUDD_REORDER_GROUP_SIFT
#define REORDER_GROUP_SIFT_CONV  CUDD_REORDER_GROUP_SIFT_CONV
#define REORDER_ANNEALING        CUDD_REORDER_ANNEALING
#define REORDER_GENETIC          CUDD_REORDER_GENETIC
#define REORDER_LINEAR           CUDD_REORDER_LINEAR
#define REORDER_LINEAR_CONV      CUDD_REORDER_LINEAR_CONVERGE
#define REORDER_EXACT            CUDD_REORDER_EXACT

#define DEFAULT_REORDER          REORDER_SIFT /* The default value in the CUDD package */
#define DEFAULT_MINSIZE          10 /* 10 = whatever (Verbatim from file cuddTable.c) */

#define ADD_FOREACH_NODE(manager, f, gen, node) \
  Cudd_ForeachNode(manager, f, gen, node)

#define BDD_FOREACH_NODE(manager, f, gen, node) \
  ADD_FOREACH_NODE(manager, f, gen, node)

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN DdManager * init_dd_package      ARGS((void));
EXTERN void     quit_dd_package         ARGS((DdManager *));
EXTERN add_ptr  add_true                 ARGS((DdManager *));
EXTERN add_ptr  add_then                ARGS((DdManager *, add_ptr));
EXTERN add_ptr  add_else                ARGS((DdManager *, add_ptr));
EXTERN int      add_index               ARGS((DdManager *, add_ptr));
EXTERN add_ptr  add_false                ARGS((DdManager *));
EXTERN int      add_is_true              ARGS((DdManager *, add_ptr));
EXTERN int      add_is_false             ARGS((DdManager *, add_ptr));
EXTERN add_ptr  add_one                 ARGS((DdManager *));
EXTERN add_ptr  add_zero                ARGS((DdManager *));
EXTERN int      add_is_one              ARGS((DdManager *, add_ptr));
EXTERN int      add_is_zero             ARGS((DdManager *, add_ptr));
EXTERN void     add_ref                 ARGS((add_ptr));
EXTERN void     add_deref               ARGS((add_ptr));
EXTERN add_ptr  add_dup                 ARGS((add_ptr));
EXTERN void     add_free                ARGS((DdManager *, add_ptr));
EXTERN add_ptr  add_new_var_with_index  ARGS((DdManager *, int));
EXTERN add_ptr  add_build               ARGS((DdManager *, int, add_ptr , add_ptr));
EXTERN add_ptr  add_new_var_at_level    ARGS((DdManager *, int));
EXTERN int      add_isleaf              ARGS((add_ptr));
EXTERN int      bdd_isleaf              ARGS((add_ptr));
EXTERN add_ptr  add_leaf                ARGS((DdManager *, node_ptr));
EXTERN node_ptr add_get_leaf            ARGS((DdManager *, add_ptr));
EXTERN bdd_ptr  add_to_bdd              ARGS((DdManager *, add_ptr));
EXTERN bdd_ptr  add_to_bdd_strict_threshold ARGS((DdManager *, add_ptr, int));
EXTERN add_ptr  bdd_to_add              ARGS((DdManager *, bdd_ptr));
EXTERN add_ptr  bdd_to_01_add           ARGS((DdManager *, bdd_ptr));
EXTERN add_ptr  add_and                 ARGS((DdManager *, add_ptr, add_ptr));
EXTERN void     add_and_accumulate      ARGS((DdManager *, add_ptr *, add_ptr));
EXTERN add_ptr  add_or                  ARGS((DdManager *, add_ptr, add_ptr));
EXTERN void     add_or_accumulate       ARGS((DdManager *, add_ptr *, add_ptr));

EXTERN add_ptr  add_not                 ARGS((DdManager *, add_ptr));
EXTERN add_ptr  add_implies             ARGS((DdManager *, add_ptr, add_ptr));
EXTERN add_ptr  add_iff                 ARGS((DdManager *, add_ptr, add_ptr));
EXTERN add_ptr  add_xor                 ARGS((DdManager *, add_ptr, add_ptr));
EXTERN add_ptr  add_xnor                ARGS((DdManager *, add_ptr, add_ptr));

EXTERN add_ptr  add_apply               ARGS((DdManager *, NPFNN, add_ptr, add_ptr));
EXTERN add_ptr  add_monadic_apply       ARGS((DdManager *, NPFNN/*NPFCVT*/, add_ptr));
EXTERN add_ptr  add_exist_abstract      ARGS((DdManager* dd, add_ptr a, bdd_ptr b));
EXTERN add_ptr  add_ifthenelse          ARGS((DdManager *, add_ptr, add_ptr, add_ptr));
EXTERN add_ptr  add_cube_diff           ARGS((DdManager *, add_ptr , add_ptr));
EXTERN add_ptr  add_simplify_assuming   ARGS((DdManager *, add_ptr, add_ptr));
EXTERN add_ptr  add_permute             ARGS((DdManager *, add_ptr, int *));
EXTERN add_ptr  add_support             ARGS((DdManager *, add_ptr));
EXTERN void     add_walkleaves          ARGS((VPFCVT, add_ptr));
EXTERN int      add_size                ARGS((DdManager *, add_ptr));
EXTERN double   add_count_minterm       ARGS((DdManager *, add_ptr, int));
EXTERN int      get_dd_nodes_allocated  ARGS((DdManager *));
EXTERN node_ptr add_value               ARGS((DdManager *, add_ptr));
EXTERN add_ptr  add_if_then             ARGS((DdManager *, add_ptr, add_ptr));

EXTERN node_ptr map_dd                  ARGS((DdManager *, NPFDD, node_ptr));
EXTERN void     walk_dd                 ARGS((DdManager *, VPFDD, node_ptr));
EXTERN dd_block* dd_new_var_block       ARGS((DdManager *, int, int));
EXTERN int      dd_free_var_block       ARGS((DdManager*, dd_block*));
EXTERN int      dd_get_index_at_level   ARGS((DdManager *, int));
EXTERN int      dd_get_level_at_index   ARGS((DdManager*, int));
EXTERN int      dd_get_size             ARGS((DdManager *));
EXTERN int      dd_set_order            ARGS((DdManager *, int *permutation));
EXTERN void     dd_autodyn_enable       ARGS((DdManager *, dd_reorderingtype));
EXTERN void     dd_autodyn_disable      ARGS((DdManager *));
EXTERN int      dd_reordering_status    ARGS((DdManager *, dd_reorderingtype *));
EXTERN int      dd_reorder              ARGS((DdManager *, int, int));
EXTERN int      dd_get_reorderings      ARGS((DdManager *));
EXTERN dd_reorderingtype dd_get_ordering_method  ARGS((DdManager *));
EXTERN int      StringConvertToDynOrderType  ARGS((char *string));
EXTERN char *   DynOrderTypeConvertToString  ARGS((int method));
EXTERN int      dd_checkzeroref         ARGS((DdManager *));

EXTERN void     bdd_ref                 ARGS((bdd_ptr));
EXTERN void     bdd_deref               ARGS((bdd_ptr));
EXTERN bdd_ptr  bdd_dup                 ARGS((bdd_ptr));
EXTERN bdd_ptr  bdd_true                 ARGS((DdManager *));
EXTERN bdd_ptr  bdd_false                ARGS((DdManager *));
EXTERN int      bdd_is_true              ARGS((DdManager *, bdd_ptr));
EXTERN int      bdd_is_false             ARGS((DdManager *, bdd_ptr));
EXTERN int      bdd_isnot_true           ARGS((DdManager *, bdd_ptr));
EXTERN int      bdd_isnot_false          ARGS((DdManager *, bdd_ptr));
EXTERN void     bdd_free                ARGS((DdManager *, bdd_ptr));
EXTERN bdd_ptr  bdd_not                 ARGS((DdManager *, bdd_ptr));
EXTERN bdd_ptr  bdd_and                 ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN void     bdd_and_accumulate      ARGS((DdManager *, bdd_ptr *, bdd_ptr));
EXTERN bdd_ptr  bdd_or                  ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN void     bdd_or_accumulate       ARGS((DdManager *, bdd_ptr *, bdd_ptr));
EXTERN bdd_ptr  bdd_xor                 ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_iff                 ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_imply               ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_forsome             ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_forall              ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_permute             ARGS((DdManager *, bdd_ptr, int *));
EXTERN bdd_ptr  bdd_and_abstract        ARGS((DdManager *, bdd_ptr, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_simplify_assuming   ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_minimize            ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_cofactor            ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_between             ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN int      bdd_entailed            ARGS((DdManager * dd, bdd_ptr f, bdd_ptr g));
EXTERN int      bdd_intersected         ARGS((DdManager * dd, bdd_ptr f, bdd_ptr g));
EXTERN bdd_ptr  bdd_then                ARGS((DdManager *, bdd_ptr));
EXTERN bdd_ptr  bdd_else                ARGS((DdManager *, bdd_ptr));
EXTERN bdd_ptr  bdd_ite                 ARGS((DdManager *, bdd_ptr, bdd_ptr, bdd_ptr));
EXTERN int      bdd_iscomplement        ARGS((DdManager *, bdd_ptr));
EXTERN int      bdd_readperm            ARGS((DdManager *, bdd_ptr));
EXTERN int      bdd_index               ARGS((DdManager *, bdd_ptr));
EXTERN bdd_ptr  bdd_pick_one_minterm    ARGS((DdManager *, bdd_ptr, bdd_ptr *, int));
EXTERN bdd_ptr  bdd_pick_one_minterm_rand  ARGS((DdManager *, bdd_ptr, bdd_ptr *, int));
EXTERN int      bdd_pick_all_terms      ARGS((DdManager *, bdd_ptr,  bdd_ptr *, int, bdd_ptr *, int));
EXTERN bdd_ptr  bdd_support             ARGS((DdManager *, bdd_ptr));
EXTERN int      bdd_size                ARGS((DdManager *, bdd_ptr));
EXTERN double   bdd_count_minterm       ARGS((DdManager *, bdd_ptr, int));
EXTERN bdd_ptr  bdd_new_var_with_index  ARGS((DdManager *, int));
EXTERN bdd_ptr bdd_get_one_sparse_sat   ARGS((DdManager *, bdd_ptr));
EXTERN int      dd_set_parameters       ARGS((DdManager *, OptsHandler_ptr, FILE *));
EXTERN void     dd_print_stats          ARGS((DdManager *, FILE *));
EXTERN bdd_ptr  bdd_cube_diff           ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_cube_union          ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN bdd_ptr  bdd_cube_intersection   ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN int      bdd_get_lowest_index    ARGS((DdManager *, bdd_ptr));
EXTERN int      dd_printminterm         ARGS((DdManager *, dd_ptr));
EXTERN int      dd_dump_dot             ARGS((DdManager *, int, dd_ptr *, const char **, const char **, FILE *));
EXTERN int      dd_dump_davinci         ARGS((DdManager *, int, dd_ptr *, const char **, const char **, FILE *));
EXTERN void     dd_AddCmd               ARGS((void));
EXTERN bdd_ptr  bdd_largest_cube        ARGS((DdManager *, bdd_ptr, int *));
EXTERN bdd_ptr  bdd_compute_prime_low   ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN array_t * bdd_compute_primes_low ARGS((DdManager *, bdd_ptr, bdd_ptr));
EXTERN array_t * bdd_compute_primes     ARGS((DdManager * dd, bdd_ptr b));
EXTERN bdd_ptr  bdd_make_prime          ARGS((DdManager *dd, bdd_ptr cube, bdd_ptr b));
EXTERN bdd_ptr  bdd_compute_essentials  ARGS((DdManager *dd, bdd_ptr b));
EXTERN int      bdd_DumpBlif            ARGS((DdManager *dd, int n, bdd_ptr *f, char **inames, char **onames, char *mname, FILE *fp));
EXTERN int      bdd_DumpBlifBody        ARGS((DdManager *dd, int n, bdd_ptr *f, char **inames, char **onames, FILE *fp));
EXTERN int      bdd_leq                 ARGS((DdManager *dd, bdd_ptr f, bdd_ptr g));
EXTERN bdd_ptr  bdd_swap_variables      ARGS((DdManager *dd, bdd_ptr f, bdd_ptr *x_varlist, bdd_ptr *y_varlist, int n));
EXTERN bdd_ptr  bdd_compose             ARGS((DdManager *dd, bdd_ptr f, bdd_ptr g, int v));
EXTERN int      bdd_ref_count           ARGS((bdd_ptr n));
EXTERN int      calculate_bdd_value     ARGS((DdManager* mgr, bdd_ptr f, int* values));

#endif /* _DD_H */
