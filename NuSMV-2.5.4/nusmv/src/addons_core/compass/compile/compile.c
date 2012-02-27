/**CFile*****************************************************************

  FileName    [compile.c]

  PackageName [compass.compile]

  Synopsis    []

  Description []

  SeeAlso     [compass.h]

  Author      [Roberto Cavada and Marco Roveri]

  Copyright   [
  This file is part of the ``compass'' package of NuSMV version 2.
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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "ProbAssign.h"

#include "addons_core/compass/compass.h"
#include "addons_core/compass/compassInt.h"

#include "utils/error.h"


/**Function********************************************************************

  Synopsis           [Checks semantics of given probabilistic list]

  Description        []

  SideEffects        []

******************************************************************************/
void Compass_check_prob_list(TypeChecker_ptr tc, NodeList_ptr list)
{
  ListIter_ptr iter;
  NODE_LIST_FOREACH(list, iter) {
    ProbAssign_ptr ta = PROB_ASSIGN(NodeList_get_elem_at(list, iter));
    node_ptr asgns = ProbAssign_get_assigns_expr(ta);
    /*    node_ptr val = ProbAssign_get_prob(ta); */

    if (!TypeChecker_is_expression_wellformed(tc, asgns, Nil)) {
      rpterr("An error was found in probabilistic list\n");
    }
  }
}


/**Function********************************************************************

  Synopsis           [Checks semantics of given atomic proposition list]

  Description        []

  SideEffects        []

******************************************************************************/
void Compass_check_ap_list(TypeChecker_ptr tc, NodeList_ptr list)
{
  ListIter_ptr iter;

  NODE_LIST_FOREACH(list, iter) {
    node_ptr ap_el = (node_ptr)NodeList_get_elem_at(list, iter);
    node_ptr ap = cdr(ap_el);

    if (!TypeChecker_is_expression_wellformed(tc, ap, Nil)) {
      fprintf(nusmv_stderr, "This AP is wrong: ");
      print_node(nusmv_stderr, ap);
      fprintf(nusmv_stderr, "\n");
      rpterr("An error was found in atomic proposition list\n");
    }
  }
}


/* internal service of function Compass_process_prob_list */
static node_ptr compass_add_mul(node_ptr n1, node_ptr n2)
{
  extern node_ptr one_number;
  extern node_ptr zero_number;

  if (n1 == one_number) return n2;
  if (n1 == zero_number) return n1;
  if (n2 == one_number) return n1;
  if (n2 == zero_number) return n2;
  error_unreachable_code(); /* no other possibilities here: one of the
                          two must be 1 or 0 */
}

/* internal service of function Compass_process_prob_list */
static node_ptr compass_add_sum(node_ptr n1, node_ptr n2)
{
  extern node_ptr one_number;
  extern node_ptr zero_number;

  if (n1 == zero_number) return n2;
  if (n2 == zero_number) return n1;
  error_unreachable_code(); /* no other possibilities here: one of the
                          two must be 0 */
}

/**Function********************************************************************

  Synopsis           []

  Description        [list is assumed to be already checked here. Returned add
  is referenced]

  SideEffects        []

******************************************************************************/
add_ptr Compass_process_prob_list(BddEnc_ptr enc, NodeList_ptr list, add_ptr trans)
{
  DdManager* dd = BddEnc_get_dd_manager(enc);
  ListIter_ptr iter;
  add_ptr res_add = add_zero(dd);
  bdd_ptr cube = BddEnc_get_state_frozen_vars_cube(enc);
  bdd_ptr ncube = BddEnc_get_next_state_vars_cube(enc);

  bdd_and_accumulate(dd, &cube, ncube);
  bdd_free(dd, ncube);
/* #define DEBUG_PROB */
#ifdef DEBUG_PROB
    printf("========================================\n");
    printf("TRANS\n");
    printf("========================================\n");
    dd_printminterm(dd, trans);
    printf("========================================\n");
    dd_printminterm(dd, BddEnc_get_input_vars_cube(enc));
    printf("========================================\n");
#endif

  NODE_LIST_FOREACH(list, iter) {
    bdd_ptr tmp_bdd;
    add_ptr tmp_add, asgns_add, val_add, asgns_prob_add;
    ProbAssign_ptr pa = PROB_ASSIGN(NodeList_get_elem_at(list, iter));
    node_ptr asgns = ProbAssign_get_assigns_expr(pa);
    node_ptr val = ProbAssign_get_prob(pa);
    bdd_ptr asgns_bdd = BddEnc_expr_to_bdd(enc, asgns, Nil);


    tmp_bdd = bdd_and_abstract(dd, asgns_bdd, trans, cube);
    bdd_free(dd, asgns_bdd);

    /* This is probably redundant, however we do it as to ensure there
       are no incomplete assignments */
    asgns_bdd = BddEnc_apply_input_vars_mask_bdd(enc, tmp_bdd);
    bdd_free(dd, tmp_bdd);

#ifdef DEBUG_PROB
    printf("========================================\n");
    printf("ASSIGN\n");
    printf("========================================\n");
    dd_printminterm(dd, asgns_bdd);
    printf("========================================\n");

#endif

    asgns_add = bdd_to_01_add(dd, asgns_bdd);
    bdd_free(dd, asgns_bdd);

    val_add = add_leaf(dd, val);
    asgns_prob_add = add_apply(dd, &compass_add_mul,
                               asgns_add, val_add);
    tmp_add = add_apply(dd, &compass_add_sum, res_add, asgns_prob_add);

    add_free(dd, res_add);
    res_add = tmp_add;

    add_free(dd, asgns_prob_add);
    add_free(dd, val_add);
    add_free(dd, asgns_add);
  } /* loop over prob list */

#ifdef DEBUG_PROB
    printf("========================================\n");
    printf("PROB\n");
    printf("========================================\n");
    dd_printminterm(dd, res_add);
    printf("========================================\n");
#endif

  bdd_free(dd, ncube);
  return res_add;
}
