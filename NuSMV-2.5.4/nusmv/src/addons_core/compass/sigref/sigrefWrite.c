/**CFile*****************************************************************

  FileName    [sigrefWrite.c]

  PackageName [compass.sigref]

  Synopsis    []

  Description []

  SeeAlso     [compass.h]

  Author      [Roberto Cavada]

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


#include <stdarg.h>
#include <stdio.h>

#include "util.h"
#include "utils/utils.h"
#include "utils/ustring.h"

#include "addons_core/compass/compass.h"
#include "addons_core/compass/compassInt.h"

#include "opt/opt.h"
#include "enc/bdd/BddEnc.h"
#include "utils/assoc.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: sigrefWrite.c,v 1.1.2.13 2009-09-17 11:49:43 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define _INDENT(file, c, cond) \
  if (cond) {int _i_; for (_i_=indent; _i_>0; --_i_) fprintf(file, " ");}

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int compass_sigref_dump_dd(FILE* file, DdManager* dd, add_ptr add,
                                  const char* dd_title, const char * dd_label,
                                  boolean do_indent);

static int
compass_print_add_sigref_format(DdManager* dd, add_ptr add, FILE* file,
                                int indent, hash_ptr hash, boolean do_indent);

static void compass_write_sigref_adds(FILE* file,
                                      BddEnc_ptr enc, /* language */
                                      add_ptr init,   /* init & invar states */
                                      add_ptr trans,  /* trans & invar */
                                      add_ptr prob,   /* probabilities */
                                      add_ptr tau,     /* tau value */
                                      NodeList_ptr ap_list, /* ap list (optional) */
                                      boolean do_indent /* beautify XML output */
                                      );

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Compass_write_sigref(FILE* file, BddFsm_ptr fsm,
                          NodeList_ptr probs_list, /* can be NULL */
                          Expr_ptr tau, /* can be NULL */
                          NodeList_ptr ap_list, /* can be NULL */
                          boolean do_indent /* Beautify the XML output */
                          )
{
  NodeList_ptr ap_list_add = NODE_LIST(NULL);
  BddEnc_ptr enc = BddFsm_get_bdd_encoding(fsm);
  DdManager* dd = BddEnc_get_dd_manager(enc);

  /* collects all required pieces */
  add_ptr prob_add;
  add_ptr init_add;
  add_ptr trans_add;
  add_ptr tau_add;
  bdd_ptr bdd_trans;
  bdd_ptr state_mask = BddEnc_get_state_frozen_vars_mask_bdd(enc);
  bdd_ptr input_mask = BddEnc_get_input_vars_mask_bdd(enc);
  bdd_ptr next_state_mask = BddEnc_state_var_to_next_state_var(enc, state_mask);
  bdd_ptr next_and_curr_state_mask = bdd_and(dd, state_mask, next_state_mask);

  { /* calculates the initial states */
    bdd_ptr bdd_init = BddFsm_get_init(fsm);
    bdd_ptr bdd_sinvar = BddFsm_get_state_constraints(fsm);
    bdd_ptr bdd_iinvar = BddFsm_get_input_constraints(fsm);
    bdd_and_accumulate(dd, &bdd_init, bdd_sinvar);
    bdd_and_accumulate(dd, &bdd_init, bdd_iinvar);
    bdd_and_accumulate(dd, &bdd_init, state_mask);

    init_add = bdd_to_add(dd, bdd_init);

    bdd_free(dd, bdd_iinvar);
    bdd_free(dd, bdd_sinvar);
    bdd_free(dd, bdd_init);
  }


  { /* to control dynamic reordering */
    dd_reorderingtype method;
    int dd_reord_status;

    /* Dynamic reordering during monolithic trans can improve performances */
    dd_reord_status = dd_reordering_status(dd, &method);
    dd_autodyn_enable(dd, method);

    { /* calculates the transition relation */
      bdd_ptr bdd_sinvar = BddFsm_get_state_constraints(fsm);
      bdd_ptr bdd_nsinvar = BddEnc_state_var_to_next_state_var(enc, bdd_sinvar);
      bdd_ptr bdd_iinvar = BddFsm_get_input_constraints(fsm);

      bdd_trans = BddFsm_get_monolithic_trans_bdd(fsm);
      bdd_and_accumulate(dd, &bdd_trans, bdd_sinvar);
      bdd_and_accumulate(dd, &bdd_trans, bdd_nsinvar);
      bdd_and_accumulate(dd, &bdd_trans, bdd_iinvar);
      bdd_and_accumulate(dd, &bdd_trans, next_and_curr_state_mask);
      bdd_and_accumulate(dd, &bdd_trans, input_mask);

/* #define DEBUG_TRANS */
#ifdef DEBUG_TRANS
      FILE * p = stdout; // fopen("TRANS.txt", "w");
      fprintf(p, "==================================================\n");
      fprintf(p, "TRANS\n");
      fprintf(p, "==================================================\n");
      // print_trans(enc, bdd_trans, p);
      dd_printminterm(dd, bdd_trans);
      fprintf(p, "==================================================\n");
      dd_printminterm(dd, input_mask);
      fprintf(p, "==================================================\n");
      // fclose(p);
#endif

      trans_add = bdd_to_add(dd, bdd_trans);

      bdd_free(dd, bdd_iinvar);
      bdd_free(dd, bdd_nsinvar);
      bdd_free(dd, bdd_sinvar);
    }

    /* Dynamic reordering is disabled after monolitic
       construction. Later it will be re-enabled if it was enable
       before entering this block */
    dd_autodyn_disable(dd);

    /* probability and tau are optional */
    if (probs_list != NODE_LIST(NULL)) {
      prob_add = Compass_process_prob_list(enc, probs_list, bdd_trans);
    }
    else prob_add = (add_ptr) NULL;

    bdd_free(dd, bdd_trans);

    if (tau != (Expr_ptr) NULL) {
      tau_add = BddEnc_expr_to_add(enc, tau, Nil);

#if 1

#else
      mask_tau_add = BddEnc_apply_input_vars_mask_add(enc, tau_add);
      add_free(dd, tau_add);
      tau_add = mask_tau_add;
#endif
    }
    else tau_add = (add_ptr) NULL;

    if (NODE_LIST(NULL) != ap_list) {
      add_ptr expr_add, expr_add_mask;
      ListIter_ptr iter;
      ap_list_add = NodeList_create();

      NODE_LIST_FOREACH(ap_list, iter) {
        node_ptr ap_el = (node_ptr)NodeList_get_elem_at(ap_list, iter);
        node_ptr lab = car(ap_el);
        node_ptr expr = cdr(ap_el);

        expr_add = BddEnc_expr_to_add(enc, expr, Nil);
        expr_add_mask = BddEnc_apply_state_frozen_vars_mask_add(enc, expr_add);
        NodeList_append(ap_list_add, cons(lab, (node_ptr)expr_add_mask));
        add_free(dd, expr_add);
        /* The expr_add_mask will be freed later when the ap_list_add
           will be destroyed */
      }
    }

    /* dump the whole fsm */
    compass_write_sigref_adds(file, enc,
                              init_add, trans_add,
                              prob_add, tau_add,
                              ap_list_add, do_indent);

    if ((add_ptr) NULL != tau_add) add_free(dd, tau_add);
    if ((add_ptr) NULL != prob_add) add_free(dd, prob_add);
    add_free(dd, trans_add);
    add_free(dd, init_add);

    if (NODE_LIST(NULL) != ap_list) {
      ListIter_ptr iter;

      NODE_LIST_FOREACH(ap_list_add, iter) {
        node_ptr ap_el = (node_ptr)NodeList_get_elem_at(ap_list_add, iter);
        add_ptr lab_add = (add_ptr)cdr(ap_el);

        add_free(dd, lab_add);
        free_node(ap_el);
      }
      NodeList_destroy(ap_list_add);
    }

    bdd_free(dd, next_and_curr_state_mask);
    bdd_free(dd, next_state_mask);
    bdd_free(dd, state_mask);

    /* re-enable dynamic reordering if it was already enabled */
    if (dd_reord_status == 1) { dd_autodyn_enable(dd, method); }
  } /* end of block for dynamic reordering */
}



/**Function********************************************************************

Synopsis        [Handles the piece of sigref format regarding the language
(<variables> ... </variables>) ]

Description     [Returns 0 if successful, a negative number if an occurs]

SideEffects     []

SeeAlso         []

******************************************************************************/
int Compass_write_language_sigref(BddEnc_ptr enc, FILE* file)
{
  static const char* TAG_VARS = "variables";
  static const char* SVAR_FM = \
    "<var index=\"%d\" name=\"%s\" type=\"%s\" corr=\"%d\"/>\n";
  static const char* IVAR_FM = "<var index=\"%d\" name=\"%s\" type=\"%s\"/>\n";
  static const char* SVAR_TYPE = "ps";
  static const char* NVAR_TYPE = "ns";
  static const char* IVAR_TYPE = "in";

  int res;

  res = fprintf(file, "<%s>\n", TAG_VARS);
  if (res <= 0) return -1;

  { /* loop over variables */
    SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));
    NodeList_ptr layers = BaseEnc_get_committed_layers(BASE_ENC(enc));
    ListIter_ptr lay_it;
    NODE_LIST_FOREACH(layers, lay_it) {
      SymbLayer_ptr lay = SYMB_LAYER(NodeList_get_elem_at(layers, lay_it));
      SymbLayerIter lay_it;

      SYMB_LAYER_FOREACH_FILTER(lay, lay_it, STT_VAR,
                                SymbLayer_iter_filter_bool_vars, NULL) {
        node_ptr cvar = SymbLayer_iter_get_symbol(lay, &lay_it);

        boolean is_ivar = SymbTable_is_symbol_input_var(st, cvar);
        int cidx = BddEnc_get_var_index_from_name(enc, cvar);
        char* cvarn = sprint_node(cvar);

        if (is_ivar) { /* input variable */
          fprintf(file, IVAR_FM, cidx, cvarn, IVAR_TYPE);
        }
        else { /* state variables */
          node_ptr nvar = Expr_next(cvar, st);
          int nidx = BddEnc_get_var_index_from_name(enc, nvar);
          char* nvarn = sprint_node(nvar);
          fprintf(file, SVAR_FM, cidx, cvarn, SVAR_TYPE, nidx);
          fprintf(file, SVAR_FM, nidx, nvarn, NVAR_TYPE, cidx);
          FREE(nvarn);
        }
        FREE(cvarn);
      }
    }
  }

  res = fprintf(file, "</%s>\n", TAG_VARS);
  if (res <= 0) return -1;
  return 0;
}



/**Function********************************************************************

Synopsis        [Prints recursively an ADD node.
<dd_node ... </dd_node>]

Description     [Returns 0 if successful, a negative number if an occurs]

SideEffects     []

SeeAlso         []

******************************************************************************/
int Compass_print_add_sigref_format(DdManager* dd, add_ptr add, FILE* file, boolean do_indent)
{
  int res;
  hash_ptr hash = new_assoc();
  res = compass_print_add_sigref_format(dd, add, file, 1, hash, do_indent);
  free_assoc(hash);
  return res;
}



/* service for temporary command sigref_dumper */
static int compass_sigref_dump_dd(FILE* file, DdManager* dd,
                                  add_ptr add, const char* dd_title,
                                  const char * dd_lab,
                                  boolean do_indent)
{
  static const char* DD_TAG = "dd";
  static const char* DD_FM = "type=\"%s\"";
  static const char* DD_LAB = " label=\"%s\"";
  int res;

  fprintf(file, "<%s ", DD_TAG);
  fprintf(file, DD_FM, dd_title);
  if ((char *)NULL != dd_lab) {
    fprintf(file, DD_LAB, dd_lab);
  }
  fprintf(file, ">\n");

  res = Compass_print_add_sigref_format(dd, add, file, do_indent);

  fprintf(file, "</%s>\n", DD_TAG);
  fflush(file);
  return res;
}



/**Function********************************************************************

Synopsis        [Private service to a node child, used by
print_add_sigref_format]

Description     [Returns 0 if successful, a negative number if an occurs]

SideEffects     []

SeeAlso         []

******************************************************************************/
static int compass_print_add_child_sigref(DdManager* dd, add_ptr add,
                                          FILE* file,
                                          int indent, const char* child_tag,
                                          hash_ptr hash, boolean do_indent)
{
  static const char* LEAF_FM = "const_value=\"%s\"";
  static const char* REF_FM = "node_ref=\"%#lu\"";
  int res;

  _INDENT(file, indent, do_indent);
  res = fprintf(file, "<%s", child_tag); if (res<0) return -1;

  if (add_isleaf(add)) {
    const char* val;
    res = fprintf(file, " "); if (res<0) return -1;

    if (add_is_false(dd, add)) val = util_strsav("0.0");
    else if (add_is_true(dd, add)) val = util_strsav("1.0");
    else val = sprint_node((node_ptr) add_get_leaf(dd, add));

    res = fprintf(file, LEAF_FM, val);
    FREE(val);
    if (res<0) return -1;
    res = fprintf(file, "/>\n"); if (res<0) return -1;
  }
  else { /* not a leaf, recurse if needed */
    if (find_assoc(hash, (node_ptr) add) == NULL) {
      res = fprintf(file, ">\n"); if (res<0) return -1;
      res = compass_print_add_sigref_format(dd, add, file, indent+1, hash, do_indent);
      if (res<0) return -1;
      _INDENT(file, indent, do_indent);
      res = fprintf(file, "</%s>\n", child_tag); if (res<0) return -1;

      /* stores the node as printed */
      insert_assoc(hash, (node_ptr) add, PTR_FROM_INT(node_ptr, 1));
    }
    else { /* this node has already been dumped */
      res = fprintf(file, " "); if (res<0) return -1;
      res = fprintf(file, REF_FM, add); if (res<0) return -1;
      res = fprintf(file, "/>\n"); if (res<0) return -1;
    }
  }

  return 0;
}


static int compass_print_add_sigref_format(DdManager* dd, add_ptr add, FILE* file,
                                           int indent, hash_ptr hash, boolean do_indent)
{
  static const char* DD_NODE_FM = "<dd_node index=\"%d\" id=\"%#lu\">\n";
  static const char* TAG_THEN = "dd_then";
  static const char* TAG_ELSE = "dd_else";
  static const char* TAG_NODE = "dd_node";
  int res;

  if (add_isleaf(add)) {
    /* term 'child' here is not used properly */
    res = compass_print_add_child_sigref(dd, add, file, indent, TAG_NODE, hash, do_indent);
    if (res<0) return -1;
  }
  else {
    add_ptr _then = add_then(dd, add);
    add_ptr _else = add_else(dd, add);

    _INDENT(file,indent,do_indent);
    res = fprintf(file, DD_NODE_FM, add_index(dd, add), add);
    if (res<0) return -1;

    res = compass_print_add_child_sigref(dd, _then, file,
                                         indent+1, TAG_THEN, hash, do_indent);
    if (res<0) return -1;
    res = compass_print_add_child_sigref(dd, _else, file,
                                         indent+1, TAG_ELSE, hash, do_indent);
    if (res<0) return -1;
    _INDENT(file,indent,do_indent);
    res = fprintf(file, "</%s>\n",TAG_NODE);
    if (res<0) return -1;
  }
  return 0;
}


static void compass_write_sigref_adds(FILE* file,
                                      BddEnc_ptr enc, /* language */
                                      add_ptr init,   /* init & invar states */
                                      add_ptr trans,  /* trans & invar */
                                      add_ptr prob,   /* probabilities (optional) */
                                      add_ptr tau,    /* tau value (optional) */
                                      NodeList_ptr ap_list, /* ap list (optional) */
                                      boolean do_indent /* Enable indent */
                                      )
{
  /* ---------------------------------------------------------------------- */
  static const char* SIGREF_HEADER = "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>";
  static const char* MODEL_TAG = "model";
  static const char* MODEL_FM = "type=\"%s\"";
  static const char* MODEL_TYPE = "ctmc";

  /* labels for ADDs */
  static const char* SIGREF_INIT_STATE_LBL = "initial_state";
  static const char* SIGREF_LTS_STATE_LBL = "trans";
  static const char* SIGREF_TAU_LBL = "tau";
  static const char* SIGREF_PROB_MAP_LBL = "label_map";

  /* labels for AP */
  static const char* SIGREF_AP_LBL = "atomic_prop";
  /* ---------------------------------------------------------------------- */

  DdManager* dd = BddEnc_get_dd_manager(enc);

  fprintf(file, SIGREF_HEADER);
  fprintf(file, "\n<%s ", MODEL_TAG);
  fprintf(file, MODEL_FM, MODEL_TYPE);
  fprintf(file, ">\n");

  /* language */
  Compass_write_language_sigref(enc, file);
  fprintf(file, "\n");

  /* init and trans */
  compass_sigref_dump_dd(file, dd, init, SIGREF_INIT_STATE_LBL, NULL, do_indent);
  fprintf(file, "\n");
  compass_sigref_dump_dd(file, dd, trans, SIGREF_LTS_STATE_LBL, NULL, do_indent);

  if (prob != (add_ptr) NULL) {
#ifdef DEBUG_PROBS
    fprintf(file, "\n");
    printf("==================\n");
    dd_printminterm(dd, prob);
    printf("==================\n");
#endif
    compass_sigref_dump_dd(file, dd, prob, SIGREF_PROB_MAP_LBL, NULL, do_indent);
  }

  if (tau != (add_ptr) NULL) {
    fprintf(file, "\n");
    compass_sigref_dump_dd(file, dd, tau, SIGREF_TAU_LBL, NULL, do_indent);
  }

  if (NODE_LIST(NULL) != ap_list) {
    ListIter_ptr iter;

    NODE_LIST_FOREACH(ap_list, iter) {
      node_ptr ap_el = (node_ptr)NodeList_get_elem_at(ap_list, iter);
      char * ap_lab = get_text((string_ptr)car(car(ap_el)));
      add_ptr ap_bdd = (add_ptr)cdr(ap_el);

      nusmv_assert(node_get_type(car(ap_el)) == ATOM);

      fprintf(file, "\n");
      compass_sigref_dump_dd(file, dd, ap_bdd, SIGREF_AP_LBL, ap_lab, do_indent);
    }
  }

  fprintf(file, "\n</%s>\n", MODEL_TAG);
  fflush(file);
}


void print_trans(BddEnc_ptr enc, bdd_ptr bdd_to_print, FILE *f) {
  bdd_ptr bdd;
  NodeList_ptr vars_list;
  NodeList_ptr sv_list;
  SymbTable_ptr st = BaseEnc_get_symb_table(BASE_ENC(enc));
  DdManager * dd = BddEnc_get_dd_manager(enc);

  if (bdd_is_false(dd, bdd_to_print)) {
    fprintf(f, "-- The BDD is the constant 0\n");
    return;
  }

  vars_list = SymbTable_get_layers_sf_i_vars(st,
                SymbTable_get_class_layer_names(st, (const char*) NULL));

  sv_list = SymbTable_get_layers_sf_vars(st,
                SymbTable_get_class_layer_names(st, (const char*) NULL));

  { /* We append next variables */
    ListIter_ptr iter;

    NODE_LIST_FOREACH(sv_list, iter) {
      node_ptr v = (node_ptr)NodeList_get_elem_at(sv_list, iter);

      if (SymbTable_is_symbol_state_var(st, v)) {
        NodeList_append(vars_list, find_node(NEXT, v, Nil));
      }
    }
  }

  bdd = bdd_dup(bdd_to_print);

  do {
    bdd_ptr bdd_of_assigns, tmp;
    node_ptr varValueList = BddEnc_assign_symbols(enc, bdd, vars_list,
                                                  true, &bdd_of_assigns);

    {
      node_ptr l;
      for (l = varValueList; Nil != l; l = cdr(l)) {
        print_node(f, car(car(l)));
        fprintf(f, " = ");
        print_node(f, cdr(car(l)));
        fprintf(f, "\t");
      }
      fprintf(f, "\n");
    }
    free_list(varValueList);

    tmp = bdd_not(dd, bdd_of_assigns);
    bdd_and_accumulate(dd, &bdd, tmp);
    bdd_free(dd, tmp);
    bdd_free(dd, bdd_of_assigns);
  } while (bdd_isnot_false(dd, bdd));
}
