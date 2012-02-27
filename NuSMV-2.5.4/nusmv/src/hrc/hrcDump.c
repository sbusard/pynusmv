/**CFile***********************************************************************

  FileName    [hrcDump.c]

  PackageName [hrc]

  Synopsis    [Creation of an SMV file of an Hrc structure]

  Description [Creates a SMV file from the hrc
  structure.

  The exported function Hrc_WriteModel allows to print a HrcNode_ptr
  structure on a file.

  The file contains static functions needed to print an SMV file given
  the hrc structure.]

  SeeAlso     []

  Author      [Sergio Mover]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK.

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


#include "hrc.h"
#include "HrcNode.h"

#include "hrc/dumpers/HrcDumper.h"

#include "parser/symbols.h"
#include "parser/parser.h"
#include "utils/Slist.h"
#include "utils/assoc.h"
#include "utils/ustring.h"
#include "compile/compile.h" /* for Compile_print_array_define */


static char rcsid[] UTIL_UNUSED = "$Id: $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define _DUMP_SNIPPET_BEGIN(snippet, dumper, info)  \
  (info)->stage = HRC_STAGE_BEGIN;                  \
  HrcDumper_dump_snippet(dumper, snippet, info)

#define _DUMP_SNIPPET_BEGIN_INDENT(snippet, dumper, info)  \
  (info)->stage = HRC_STAGE_BEGIN;                         \
  HrcDumper_dump_snippet(dumper, snippet, info);           \
  HrcDumper_inc_indent(dumper)

#define _DUMP_SNIPPET_END(snippet, dumper, info)    \
  (info)->stage = HRC_STAGE_END;                    \
  HrcDumper_dump_snippet(dumper, snippet, info)

#define _DUMP_SNIPPET_END_INDENT(snippet, dumper, info)    \
  HrcDumper_dec_indent(dumper);                            \
  (info)->stage = HRC_STAGE_END;                           \
  HrcDumper_dump_snippet(dumper, snippet, info)

#define _DUMP_SNIPPET_BEGIN_END(snippet, dumper, info)  \
  (info)->stage = HRC_STAGE_BEGIN | HRC_STAGE_END;      \
  HrcDumper_dump_snippet(dumper, snippet, info)


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void hrc_dump_module_instance ARGS((HrcNode_ptr hrcNode,
                                           HrcDumper_ptr dumper,
                                           HrcDumperInfo* info,
                                           hash_ptr printed_module_map));

static void hrc_dump_compile_info ARGS((HrcNode_ptr hrcNode,
                                        HrcDumper_ptr dumper,
                                        HrcDumperInfo* info));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Prints the SMV module for the hrcNode.]

  Description        [Prints the SMV module for the hrcNode. If the
  flag append_suffix is true then the suffix HRC_WRITE_MODULE_SUFFIX
  is appended when a module type is printed. So
  HRC_WRITE_MODULE_SUFFIX is appended to the module name in module
  declarations and to the module name in a module instantiation. The
  feature is needed for testing to avoid name clash among modules
  names when the original model and the model generated from hrc are
  merged.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Hrc_DumpModel(HrcNode_ptr hrcNode, HrcDumper_ptr dumper)
{
  HrcDumperInfo info;
  hash_ptr printed_module_map; /* hash table used to keep track of
                                  previously printed modules. */

  HRC_NODE_CHECK_INSTANCE(hrcNode);

  printed_module_map = new_assoc();

  /* top-level */
  info.hrcNode = hrcNode;
  _DUMP_SNIPPET_BEGIN(HDS_HRC_TOP, dumper, &info);

  /* list of modules */
  info.list_is_empty = ((HrcNode_ptr) NULL == hrcNode);
  _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_MODS, dumper, &info);

  /* call the recursive creation of the modules */
  hrc_dump_module_instance(hrcNode, dumper, &info, printed_module_map);

  info.hrcNode = hrcNode;
  info.list_is_empty = ((HrcNode_ptr) NULL == hrcNode);
  _DUMP_SNIPPET_END_INDENT(HDS_LIST_MODS, dumper, &info);

  /* compiler info */
  hrc_dump_compile_info(hrcNode, dumper, &info);

  _DUMP_SNIPPET_END(HDS_HRC_TOP, dumper, &info);

  free_assoc(printed_module_map);
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Writes the SMV translation of the instance
  module contained in hrcNode on file.]

  Description        [Writes the SMV translation of the instance
  module contained in hrcNode on file.]

  SideEffects        [printed_module_map is changed to keep track of
  printed modules.]

  SeeAlso            []

******************************************************************************/
static void hrc_dump_module_instance(HrcNode_ptr hrcNode,
                                     HrcDumper_ptr dumper,
                                     HrcDumperInfo* info,
                                     hash_ptr printed_module_map)
{
  node_ptr module_name = HrcNode_get_name(hrcNode);
  Slist_ptr rev_child_stack = \
    Slist_copy_reversed(HrcNode_get_child_hrc_nodes(hrcNode));

  /* Set the module as printed  */
  insert_assoc(printed_module_map, module_name, PTR_FROM_INT(node_ptr, 1));

  /* sets the currently processed node */
  info->hrcNode = hrcNode;

  /* ---------------------------------------------------------------------- */
  { /* module prototype */

    info->n1.name = HrcNode_get_crude_name(hrcNode);
    info->n2.lineno = HrcNode_get_lineno(hrcNode);
    _DUMP_SNIPPET_BEGIN_INDENT(HDS_MOD, dumper, info);
    /* name */
    _DUMP_SNIPPET_BEGIN_END(HDS_MOD_NAME, dumper, info);

    { /* formal params */
      node_ptr params = HrcNode_get_formal_parameters(hrcNode);

      info->list_is_empty = (Nil == params);
      _DUMP_SNIPPET_BEGIN(HDS_LIST_MOD_FORMAL_PARAMS, dumper, info);

      while (Nil != params) {
        info->n1.name = caar(params);
        info->n2.type = cdar(params);
        info->last_in_list = (Nil == cdr(params));
        _DUMP_SNIPPET_BEGIN_END(HDS_MOD_FORMAL_PARAM, dumper, info);

        params = cdr(params);
      }

      _DUMP_SNIPPET_END(HDS_LIST_MOD_FORMAL_PARAMS, dumper, info);
    } /* end of formal parameters */
  }

  /* ---------------------------------------------------------------------- */
  { /* Iterates over all children of this node, creating variables
     and assigning module names. Children stack is reversed in
     order to preserve order. */
    Siter iter;

    info->symb_cat = SYMBOL_STATE_VAR;
    info->list_is_empty = Slist_is_empty(rev_child_stack);
    _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_MOD_INSTANCES, dumper, info);

    SLIST_FOREACH (rev_child_stack, iter) {
      HrcNode_ptr child = HRC_NODE(Siter_element(iter));

      info->last_in_list = Siter_is_end(Siter_next(iter));
      info->n1.name = HrcNode_get_instance_name(child);
      info->n2.type = HrcNode_get_name(child);
      _DUMP_SNIPPET_BEGIN(HDS_MOD_INSTANCE, dumper, info);

      _DUMP_SNIPPET_BEGIN_END(HDS_MOD_INSTANCE_VARNAME, dumper, info);

      info->n1.name = HrcNode_get_name(child);
      _DUMP_SNIPPET_BEGIN_END(HDS_MOD_INSTANCE_MODNAME, dumper, info);

      { /* actual parameters */
        node_ptr actuals = HrcNode_get_actual_parameters(child);

        info->list_is_empty  = (Nil == actuals);
        _DUMP_SNIPPET_BEGIN(HDS_LIST_MOD_INSTANCE_ACTUAL_PARAMS, dumper, info);

        for (; Nil != actuals; actuals=cdr(actuals)) {
          info->n1.value = caar(actuals);
          info->last_in_list = (Nil == cdr(actuals));
          _DUMP_SNIPPET_BEGIN_END(HDS_MOD_INSTANCE_ACTUAL_PARAM, dumper, info);
        }

        _DUMP_SNIPPET_END(HDS_LIST_MOD_INSTANCE_ACTUAL_PARAMS, dumper, info);
      } /* end of actual params */

      info->last_in_list = Siter_is_end(Siter_next(iter));
      _DUMP_SNIPPET_END(HDS_MOD_INSTANCE, dumper, info);
    }

    info->list_is_empty = Slist_is_empty(rev_child_stack);
    _DUMP_SNIPPET_END_INDENT(HDS_LIST_MOD_INSTANCES, dumper, info);
  } /* end of mod instances */

  /* ---------------------------------------------------------------------- */
  { /* data (non-instance) variables */
    struct {
      node_ptr list;
      SymbCategory cat;
    } lists[] = {
      { HrcNode_get_state_variables(hrcNode), SYMBOL_STATE_VAR },
      { HrcNode_get_input_variables(hrcNode), SYMBOL_INPUT_VAR },
      { HrcNode_get_frozen_variables(hrcNode), SYMBOL_FROZEN_VAR },
    };
    int idx;
    for (idx=0; idx < sizeof(lists)/sizeof(lists[0]); ++idx) {
      node_ptr liter;

      info->symb_cat = lists[idx].cat;
      info->list_is_empty = (Nil == lists[idx].list);
      _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_SYMBOLS, dumper, info);

      for (liter=lists[idx].list; Nil != liter; liter=cdr(liter)) {
        info->n1.name = caar(liter);
        info->n2.type = cdar(liter);
        info->last_in_list = (Nil == cdr(liter));
        _DUMP_SNIPPET_BEGIN_END(HDS_SYMBOL, dumper, info);
      }

      _DUMP_SNIPPET_END_INDENT(HDS_LIST_SYMBOLS, dumper, info);
    }
  }

  /* ---------------------------------------------------------------------- */
  { /* data (non-instance) defines */
    node_ptr lists[] = {
      HrcNode_get_defines(hrcNode),
      HrcNode_get_array_defines(hrcNode)
    };
    int idx;
    for (idx=0; idx < sizeof(lists)/sizeof(lists[0]); ++idx) {
      node_ptr liter;

      info->symb_cat = SYMBOL_DEFINE;
      info->list_is_empty = (Nil == lists[idx]);
      _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_SYMBOLS, dumper, info);

      for (liter=lists[idx]; Nil != liter; liter=cdr(liter)) {
        info->n1.name = caar(liter);
        info->n2.body = cdar(liter);
        info->last_in_list = (Nil == cdr(liter));
        _DUMP_SNIPPET_BEGIN_END(HDS_SYMBOL, dumper, info);
      }

      _DUMP_SNIPPET_END_INDENT(HDS_LIST_SYMBOLS, dumper, info);
    }
  }

  /* ---------------------------------------------------------------------- */
  { /* constants */
    node_ptr list = HrcNode_get_constants(hrcNode);
    node_ptr liter;

    info->symb_cat = SYMBOL_CONSTANT;
    info->list_is_empty = (Nil == list);
    _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_SYMBOLS, dumper, info);

    for (liter=list; Nil != liter; liter=cdr(liter)) {
      info->n1.name = car(liter);
      info->last_in_list = (Nil == cdr(liter));
      _DUMP_SNIPPET_BEGIN_END(HDS_SYMBOL, dumper, info);
    }

    _DUMP_SNIPPET_END_INDENT(HDS_LIST_SYMBOLS, dumper, info);
  }

  /* ---------------------------------------------------------------------- */
  { /* ASSIGN: invar, init, next */
    struct {
      HrcDumperSnippet id;
      node_ptr list;
    } lists[] = {
      { HDS_ASSIGN_INIT, HrcNode_get_init_assign_exprs(hrcNode) },
      { HDS_ASSIGN_INVAR, HrcNode_get_invar_assign_exprs(hrcNode) },
      { HDS_ASSIGN_NEXT, HrcNode_get_next_assign_exprs(hrcNode) },
    };

    boolean is_empty = true;
    int idx;
    for (idx=0; idx<sizeof(lists)/sizeof(lists[0]); ++idx) {
      if (Nil != lists[idx].list) {
        is_empty = false;
        break;
      }
    }

    info->list_is_empty = is_empty;
    _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_ASSIGNS, dumper, info);
    for (idx=0; idx<sizeof(lists)/sizeof(lists[0]); ++idx) {
      node_ptr liter;
      for (liter=lists[idx].list; Nil != liter; liter=cdr(liter)) {
        info->n1.name = caar(liter);
        info->n2.expr = cdar(liter);
        info->last_in_list = (Nil == cdr(liter));
        _DUMP_SNIPPET_BEGIN_END(lists[idx].id, dumper, info);
      }
    }

    _DUMP_SNIPPET_END_INDENT(HDS_LIST_ASSIGNS, dumper, info);
  }

  /* ---------------------------------------------------------------------- */
  { /* CONSTRAINS: INIT, INVAR, TRANS */
    struct {
      HrcDumperSnippet id;
      node_ptr list;
    } lists[] = {
      { HDS_CONSTRAINT_INIT, HrcNode_get_init_exprs(hrcNode) },
      { HDS_CONSTRAINT_INVAR, HrcNode_get_invar_exprs(hrcNode) },
      { HDS_CONSTRAINT_TRANS, HrcNode_get_trans_exprs(hrcNode) },
    };

    boolean is_empty = true;
    int idx;
    for (idx=0; idx<sizeof(lists)/sizeof(lists[0]); ++idx) {
      if (Nil != lists[idx].list) {
        is_empty = false;
        break;
      }
    }
    info->list_is_empty = is_empty;
    _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_CONSTRAINTS, dumper, info);

    for (idx=0; idx<sizeof(lists)/sizeof(lists[0]); ++idx) {
      node_ptr liter;
      for (liter=lists[idx].list; Nil != liter; liter=cdr(liter)) {
        info->n1.expr = car(liter);
        info->last_in_list = (Nil == cdr(liter));
        _DUMP_SNIPPET_BEGIN_END(lists[idx].id, dumper, info);
      }
    }
    _DUMP_SNIPPET_END_INDENT(HDS_LIST_CONSTRAINTS, dumper, info);
  }

  /* ---------------------------------------------------------------------- */
  { /* JUSTICE/FAIRNESS and COMPASSION */
    struct {
      HrcDumperSnippet id;
      node_ptr list;
    } lists[] = {
      { HDS_JUSTICE, HrcNode_get_justice_exprs(hrcNode) },
      { HDS_COMPASSION, HrcNode_get_compassion_exprs(hrcNode) },
    };

    boolean is_empty = true;
    int idx;
    for (idx=0; idx<sizeof(lists)/sizeof(lists[0]); ++idx) {
      if (Nil != lists[idx].list) {
        is_empty = false;
        break;
      }
    }
    info->list_is_empty = is_empty;
    _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_FAIRNESS, dumper, info);

    for (idx=0; idx<sizeof(lists)/sizeof(lists[0]); ++idx) {
      node_ptr liter;
      for (liter=lists[idx].list; Nil != liter; liter=cdr(liter)) {
        if (HDS_COMPASSION == lists[idx].id) {
        info->n1.expr = caar(liter);
        info->n2.expr = cdar(liter);
        }
        else {
          info->n1.expr = car(liter);
        }

        info->last_in_list = (Nil == cdr(liter));
        _DUMP_SNIPPET_BEGIN_END(lists[idx].id, dumper, info);
      }
    }
    _DUMP_SNIPPET_END_INDENT(HDS_LIST_FAIRNESS, dumper, info);
  }

  /* ---------------------------------------------------------------------- */
  { /* specifications (INVARSPEC CTLSPEC LTLSPEC PSLSPEC COMPUTE) */
    struct {
      Prop_Type spec_type;
      node_ptr list;
    } lists[] = {
      { Prop_Ctl, HrcNode_get_ctl_properties(hrcNode) },
      { Prop_Ltl, HrcNode_get_ltl_properties(hrcNode) },
      { Prop_Psl, HrcNode_get_psl_properties(hrcNode) },
      { Prop_Invar, HrcNode_get_invar_properties(hrcNode) },
      { Prop_Compute, HrcNode_get_compute_properties(hrcNode) },
    };

    boolean is_empty = true;
    int idx;
    for (idx=0; idx<sizeof(lists)/sizeof(lists[0]); ++idx) {
      if (Nil != lists[idx].list) {
        is_empty = false;
        break;
      }
    }
    info->list_is_empty = is_empty;
    _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_SPECS, dumper, info);

    for (idx=0; idx<sizeof(lists)/sizeof(lists[0]); ++idx) {
      node_ptr liter;
      for (liter=lists[idx].list; Nil != liter; liter=cdr(liter)) {

        info->spec_type = lists[idx].spec_type;
        info->n1.name = cdar(liter);
        info->n2.expr = caar(liter);
        info->last_in_list = (Nil == cdr(liter));
        _DUMP_SNIPPET_BEGIN_END(HDS_SPEC, dumper, info);
      }
    }

    _DUMP_SNIPPET_END_INDENT(HDS_LIST_SPECS, dumper, info);
  }


  /* ---------------------------------------------------------------------- */
  /* close module */
  info->n1.name = HrcNode_get_crude_name(hrcNode);
  info->n2.lineno = HrcNode_get_lineno(hrcNode);
  info->hrcNode = hrcNode;
  _DUMP_SNIPPET_END_INDENT(HDS_MOD, dumper, info);


  /* ---------------------------------------------------------------------- */
  { /* Recursive creation of child modules. Reversed children stack
       is used to preserve child definition order. */
    Siter iter;
    SLIST_FOREACH(rev_child_stack, iter) {
      HrcNode_ptr child;
      node_ptr assoc_key;
      node_ptr child_module_name;

      child = HRC_NODE(Siter_element(iter));
      child_module_name = HrcNode_get_name(child);

      /* Avoids to print the module multiple times */
      assoc_key = find_assoc(printed_module_map, child_module_name);
      if (Nil == assoc_key) {
        hrc_dump_module_instance(child, dumper, info, printed_module_map);
      }
    } /* end loop on children */
  }

  /* cleanup and exit */
  Slist_destroy(rev_child_stack);
}


/**Function********************************************************************

  Synopsis           [Dumps the compiler information]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_dump_compile_info(HrcNode_ptr hrcNode,
                                  HrcDumper_ptr dumper,
                                  HrcDumperInfo* info)
{
  node_ptr errors = Parser_get_syntax_errors_list();

  info->hrcNode = hrcNode;
  info->list_is_empty = (Nil == errors);
  _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_COMPILER_INFO, dumper, info);

  info->list_is_empty = (Nil == errors);
  _DUMP_SNIPPET_BEGIN_INDENT(HDS_LIST_SYNTAX_ERRORS, dumper, info);

  {
    node_ptr iter = errors;
    while (iter != Nil) {
      Parser_get_syntax_error(car(iter),
                              &(info->error.filename),
                              &(info->error.lineno),
                              &(info->error.token),
                              &(info->error.message));
      _DUMP_SNIPPET_BEGIN_END(HDS_ERROR, dumper, info);
      iter = cdr(iter);
    }
  }

  _DUMP_SNIPPET_END_INDENT(HDS_LIST_SYNTAX_ERRORS, dumper, info);
  _DUMP_SNIPPET_END_INDENT(HDS_LIST_COMPILER_INFO, dumper, info);
}
