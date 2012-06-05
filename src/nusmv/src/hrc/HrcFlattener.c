/**CFile***********************************************************************

  FileName    [HrcFlattener.c]

  PackageName [hrc]

  Synopsis    [Implementation of class 'HrcFlattener']

  Description []

  SeeAlso     [HrcFlattener.h]

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK-irst.

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

  Revision    [$Id: HrcFlattener.c,v 1.1.2.9 2009-11-26 15:57:12 nusmv Exp $]

******************************************************************************/

#include "HrcFlattener.h"
#include "utils/utils.h"
#include "node/node.h"
#include "utils/Slist.h"
#include "compile/compile.h"
#include "compile/symb_table/ResolveSymbol.h"
#include "compile/compileInt.h"
#include "parser/symbols.h"
#include "utils/NodeList.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: HrcFlattener.c,v 1.1.2.9 2009-11-26 15:57:12 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [HrcFlattener class definition]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct HrcFlattener_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  FlatHierarchy_ptr hierarchy;
  SymbTable_ptr symb_table;
  SymbLayer_ptr layer;
  HrcNode_ptr root_node;

  boolean build_hierarchy;

} HrcFlattener;



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN FILE* nusmv_stderr;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define _PRINT(txt)                             \
  fprintf(nusmv_stderr, txt)

#define _PRINTLN(txt)                           \
  fprintf(nusmv_stderr, "%s\n", txt)

#define _NPRINT(node)                           \
  print_node(nusmv_stderr, node)

#define EXPR_AND(l,r)                           \
  find_node(AND, l, r)

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void
hrc_flattener_init ARGS((HrcFlattener_ptr self,
                         HrcNode_ptr node,
                         SymbTable_ptr symb_table,
                         SymbLayer_ptr layer));

static void
hrc_flattener_deinit ARGS((HrcFlattener_ptr self));

static node_ptr
hrc_flattener_concat_context ARGS((node_ptr ctx1,
                                   node_ptr ctx2));

static void
hrc_flattener_flatten_recur ARGS((HrcFlattener_ptr self,
                                  HrcNode_ptr node,
                                  node_ptr context));

static void
hrc_flattener_declare_variables ARGS((HrcFlattener_ptr self,
                                      node_ptr variables,
                                      node_ptr context,
                                      Instantiation_Variables_Mode_Type mode));

static node_ptr
hrc_flattener_contextualize_expr ARGS((node_ptr expr,
                                       node_ptr context));

static node_ptr
hrc_flattener_expression_and ARGS((node_ptr expr_list));

static void
hrc_flattener_populate_model ARGS((HrcFlattener_ptr self,
                                   HrcNode_ptr node,
                                   node_ptr context));


static void
hrc_flattener_populate_symb_table ARGS((HrcFlattener_ptr self,
                                        HrcNode_ptr node,
                                        node_ptr context));

static void
hrc_flattener_flatten_recur ARGS((HrcFlattener_ptr self,
                                  HrcNode_ptr node,
                                  node_ptr context));

static node_ptr
hrc_flattener_build_properties ARGS((HrcFlattener_ptr self,
                                     node_ptr prop_list,
                                     node_ptr ctx,
                                     short int type));

static void
hrc_flattener_instantiate_array_define ARGS((SymbTable_ptr st,
                                        SymbLayer_ptr layer,
                                        node_ptr name,
                                        node_ptr mod_name,
                                        node_ptr definition));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [HrcFlattener main routine]

  Description        [HrcFlattener top-level function. Given an Hrc hierarchy
                      and a symbol table (and eventually a symbol
                      layer), creates and returns a flat
                      hierarchy. Parameter layer can be NULL. Is so, a
                      new layer belonging to the given symbol table is
                      created. If not NULL, the layer must belong to
                      the given symbol table]

  SideEffects        []

  SeeAlso            [HrcFlattener_create]

******************************************************************************/
FlatHierarchy_ptr HrcToFlatHierarchy(HrcNode_ptr node,
                                     SymbTable_ptr symb_table,
                                     SymbLayer_ptr layer)
{
  HrcFlattener_ptr hf = HrcFlattener_create(node, symb_table, layer);
  FlatHierarchy_ptr hierarchy;

  HrcFlattener_flatten_hierarchy(hf);

  hierarchy = HrcFlattener_get_flat_hierarchy(hf);

  HrcFlattener_destroy(hf);

  return hierarchy;
}

/**Function********************************************************************

  Synopsis           [HrcFlattener main routine]

  Description        [HrcFlattener top-level function. Given an Hrc hierarchy
                      and a symbol table (and eventually a symbol
                      layer), creates and returns a sexpfsm. Parameter
                      layer can be NULL. Is so, a new layer belonging
                      to the given symbol table is created. If not
                      NULL, the layer must belong to the given symbol
                      table]

  SideEffects        [Adds new symbols to the given symbol table]

  SeeAlso            [HrcFlattener_create]

******************************************************************************/
SexpFsm_ptr HrcToSexpFsm(HrcNode_ptr node,
                         SymbTable_ptr symb_table,
                         SymbLayer_ptr layer)
{
  FlatHierarchy_ptr hierarchy;
  SexpFsm_ptr sexp;

  hierarchy = HrcToFlatHierarchy(node, symb_table, layer);
  sexp = SexpFsm_create(hierarchy,
                        FlatHierarchy_get_vars(hierarchy));

  /* SexpFsm creation duplicates the hierarchy */
  FlatHierarchy_destroy(hierarchy);

  return sexp;
}

/**Function********************************************************************

  Synopsis           [The HrcFlattener class constructor]

  Description        [The HrcFlattener class constructor. Parameter
                      layer can be NULL. Is so, a new layer belonging
                      to the given symbol table is created. If not
                      NULL, the layer must belong to the given symbol
                      table. The given hrc node must be the top-level
                      node.  Hrc Localize methods should be used first
                      if trying to flatten an instance which is not
                      the main one]

  SideEffects        []

  SeeAlso            [HrcFlattener_destroy]

******************************************************************************/
HrcFlattener_ptr HrcFlattener_create(HrcNode_ptr node,
                                     SymbTable_ptr symb_table,
                                     SymbLayer_ptr layer)
{
  HrcFlattener_ptr self = ALLOC(HrcFlattener, 1);
  HRC_FLATTENER_CHECK_INSTANCE(self);

  hrc_flattener_init(self, node, symb_table, layer);
  return self;
}

/**Function********************************************************************

  Synopsis           [Get the built flat hierarchy]

  Description        [Get the internally built flat hierarchy. The hierarchy
                      is populated only if
                      HrcFlattener_flatten_hierarchy was previously
                      called]

  SideEffects        []

  SeeAlso            [HrcFlattener_flatten_hierarchy]

******************************************************************************/
FlatHierarchy_ptr HrcFlattener_get_flat_hierarchy(HrcFlattener_ptr self)
{
  HRC_FLATTENER_CHECK_INSTANCE(self);
  return self->hierarchy;
}

/**Function********************************************************************

  Synopsis           [Get the symbol table]

  Description        [Gets the internally populated symbol table. The
                      st is populated only if
                      HrcFlattener_flatten_hierarchy was previously
                      called]

  SideEffects        []

  SeeAlso            [HrcFlattener_flatten_hierarchy]

******************************************************************************/
SymbTable_ptr HrcFlattener_get_symbol_table(HrcFlattener_ptr self)
{
  HRC_FLATTENER_CHECK_INSTANCE(self);
  return self->symb_table;
}

/**Function********************************************************************

  Synopsis           [Get the symbol layer]

  Description        [Gets the internally populated symbol layer. The
                      layer is populated only if
                      HrcFlattener_flatten_hierarchy was previously
                      called]

  SideEffects        []

  SeeAlso            [HrcFlattener_flatten_hierarchy]

******************************************************************************/
SymbLayer_ptr HrcFlattener_get_symbol_layer(HrcFlattener_ptr self)
{
  HRC_FLATTENER_CHECK_INSTANCE(self);
  return self->layer;
}

/**Function********************************************************************

  Synopsis           [Does the actual flattening.]

  Description        [This method does the actual flattening
                      job. Takes the input hrc node and processes it
                      in 2 steps: in the first step a first version of
                      the hierarchy is build, where expressions are
                      just contextualized but not flattened. After
                      this Compile_ProcessHierarchy is called and the
                      actual flat hierachy is built. The symbol table
                      is also filled]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcFlattener_flatten_hierarchy(HrcFlattener_ptr self)
{
  HRC_FLATTENER_CHECK_INSTANCE(self);

  self->build_hierarchy = true;

  /* First of all, build the empty hierarchy */
  self->hierarchy = FlatHierarchy_create(self->symb_table);
  /* Start from an empty assignment */
  FlatHierarchy_set_assign(self->hierarchy, cons(cons(Nil,Nil),Nil));

  /* Contextualize exressions and properties, fill the ST */
  hrc_flattener_flatten_recur(self, self->root_node, Nil);

  /* Add all variables to the hierarchy */
  {
    SymbLayerIter iter;
    SYMB_LAYER_FOREACH(self->layer, iter, STT_VAR) {
      node_ptr var = SymbLayer_iter_get_symbol(self->layer, &iter);
      FlatHierarchy_add_var(self->hierarchy, var);
    }
  }
  /* Do the actual flattening */
  Compile_ProcessHierarchy(self->symb_table, self->layer, self->hierarchy,
                           Nil, true, true);
}


/**Function********************************************************************

  Synopsis           [Fills the symbol table without building
                      any flat hierarchy]

  Description        [Fills the symbol table without building
                      any flat hierarchy]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcFlattener_populate_symbol_table(HrcFlattener_ptr self)
{
  self->build_hierarchy = false;
  hrc_flattener_flatten_recur(self, self->root_node, Nil);
}

/**Function********************************************************************

  Synopsis           [Dumps the flatten model on file "out"]

  Description        [Dumps the flatten model on file "out"]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcFlattener_write_flatten_model(HrcFlattener_ptr self,
                                      FILE* out)
{
  HRC_FLATTENER_CHECK_INSTANCE(self);

  nusmv_assert(FLAT_HIERARCHY(NULL) != self->hierarchy);

  array_t* names;
  names = SymbTable_get_class_layer_names(self->symb_table, (char*)NULL);

  /* We MUST force flattening because of the properties */
  Compile_WriteFlattenModel(out, self->symb_table, names,
                            "MODULE main", self->hierarchy, true);
}

/**Function********************************************************************

  Synopsis           [The HrcFlattener class destructor]

  Description        [The HrcFlattener class destructor]

  SideEffects        []

  SeeAlso            [HrcFlattener_create]

******************************************************************************/
void HrcFlattener_destroy(HrcFlattener_ptr self)
{
  HRC_FLATTENER_CHECK_INSTANCE(self);

  hrc_flattener_deinit(self);
  FREE(self);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Populates the symbol table for the given node in
                      the given context]

  Description        [All symbols of the HRC node are flattened and
                      declared into the symbol layer]

  SideEffects        []

  SeeAlso            [hrc_flattener_flatten_recur]

******************************************************************************/
static void hrc_flattener_populate_symb_table(HrcFlattener_ptr self,
                                              HrcNode_ptr node,
                                              node_ptr context)
{
  /* PARAMETERS */
  {
    node_ptr actuals = HrcNode_get_actual_parameters(node);
    node_ptr formals = HrcNode_get_formal_parameters(node);

    while (Nil != actuals) {
      /* Parameters are stored in HRC as cons nodes with car = name
         and cdr = type. We just need the name */
      node_ptr actual = caar(actuals);
      node_ptr formal = caar(formals);
      node_ptr form_flat = hrc_flattener_concat_context(context, formal);

      SymbLayer_declare_parameter(self->layer, form_flat, car(context), actual);

      formals = cdr(formals);
      actuals = cdr(actuals);
    }
  }

  /* State Variables */
  {
    node_ptr state_vars = HrcNode_get_state_variables(node);
    hrc_flattener_declare_variables(self, state_vars, context,
                                    State_Variables_Instantiation_Mode);
  }

  /* Frozen Variables */
  {
    node_ptr frozen_vars = HrcNode_get_frozen_variables(node);
    hrc_flattener_declare_variables(self, frozen_vars, context,
                                    Frozen_Variables_Instantiation_Mode);
  }

  /* Input Variables */
  {
    node_ptr input_vars = HrcNode_get_input_variables(node);
    hrc_flattener_declare_variables(self, input_vars, context,
                                    Input_Variables_Instantiation_Mode);
  }

  /* DEFINEs */
  {
    node_ptr defines = HrcNode_get_defines(node);
    while (Nil != defines) {
      node_ptr name = car(car(defines));
      node_ptr body = cdr(car(defines));
      ResolveSymbol_ptr rs;

      rs = SymbTable_resolve_symbol(self->symb_table, name, context);
      name = ResolveSymbol_get_resolved_name(rs);

      nusmv_assert(SymbLayer_can_declare_define(self->layer, name));
      SymbLayer_declare_define(self->layer, name, context, body);

      defines = cdr(defines);
    }
  }

  /*  ARRAY DEFINE */
  {
    node_ptr array_defines = HrcNode_get_array_defines(node);
    while (Nil != array_defines) {
      node_ptr name = car(car(array_defines));
      node_ptr body = cdr(car(array_defines));
      ResolveSymbol_ptr rs;

      rs = SymbTable_resolve_symbol(self->symb_table, name, context);
      name = ResolveSymbol_get_resolved_name(rs);

      nusmv_assert(SymbLayer_can_declare_array_define(self->layer, name));
      /* SymbLayer_declare_array_define(self->layer, name, context, body); */

      hrc_flattener_instantiate_array_define(self->symb_table, self->layer,
                                        name, context, body);

      array_defines = cdr(array_defines);
    }
  }

  /* CONSTANTS */
  /* Constants do not need any type of flattening */
  {
    node_ptr constants = HrcNode_get_constants(node);
    while (Nil != constants) {
      /* [SM] 25/08/2010: added normalization to constant.
         The constant inserted in the symbol table MUST be normalized,
         as it happens in the compile flatten process.
       */
      node_ptr constant = node_normalize(car(constants));

      if (SymbLayer_can_declare_constant(self->layer, constant)) {
        SymbLayer_declare_constant(self->layer, constant);
      }
      constants = cdr(constants);
    }
  }
}

/**Function********************************************************************

  Synopsis           [Adds to the hierarchy model informations of the
                      given hrc node into the given context]

  Description        [Does the actual flattening and contextualization
                      of expressions and properties.]

  SideEffects        []

  SeeAlso            [hrc_flattener_flatten_recur]

******************************************************************************/
static void hrc_flattener_populate_model(HrcFlattener_ptr self,
                                         HrcNode_ptr node,
                                         node_ptr context)
{

  /* INIT */
  {
    node_ptr inits = HrcNode_get_init_exprs(node);
    node_ptr new_init = hrc_flattener_expression_and(inits);
    new_init = hrc_flattener_contextualize_expr(new_init, context);
    node_ptr tmp = FlatHierarchy_get_init(self->hierarchy);
    FlatHierarchy_set_init(self->hierarchy,
                           EXPR_AND(new_init, tmp));

  }
  /* TRANS */
  {
    node_ptr transs = HrcNode_get_trans_exprs(node);
    node_ptr new_trans = hrc_flattener_expression_and(transs);
    new_trans = hrc_flattener_contextualize_expr(new_trans, context);
    node_ptr tmp = FlatHierarchy_get_trans(self->hierarchy);
    FlatHierarchy_set_trans(self->hierarchy,
                            EXPR_AND(new_trans, tmp));
  }
  /* INVAR */
  {
    node_ptr invars = HrcNode_get_invar_exprs(node);
    node_ptr new_invar = hrc_flattener_expression_and(invars);
    new_invar = hrc_flattener_contextualize_expr(new_invar, context);
    node_ptr tmp = FlatHierarchy_get_invar(self->hierarchy);
    FlatHierarchy_set_invar(self->hierarchy,
                            EXPR_AND(new_invar, tmp));
  }
  /* COMPASSION */
  {
    node_ptr compassions = HrcNode_get_compassion_exprs(node);
    node_ptr new_compassion = FlatHierarchy_get_compassion(self->hierarchy);
    while (Nil != compassions) {
      node_ptr comp = car(compassions);

      comp = hrc_flattener_contextualize_expr(comp, context);
      new_compassion = cons(comp, new_compassion);

      compassions = cdr(compassions);
    }
    FlatHierarchy_set_compassion(self->hierarchy, new_compassion);
  }

  /* JUSTICE */
  {
    node_ptr justices = HrcNode_get_justice_exprs(node);
    node_ptr new_justice = FlatHierarchy_get_justice(self->hierarchy);
    while (Nil != justices) {
      node_ptr just = car(justices);

      just = hrc_flattener_contextualize_expr(just, context);
      new_justice = cons(just, new_justice);

      justices = cdr(justices);
    }
    FlatHierarchy_set_justice(self->hierarchy, new_justice);
  }

  /* ASSIGN */
  {

    node_ptr assign_list = FlatHierarchy_get_assign(self->hierarchy);
    node_ptr instance_assign = Nil;

    /* -- Step 1 -> Create an unique big and of all assignments -- */
    {
      node_ptr init = HrcNode_get_init_assign_exprs(node);
      while (Nil != init) {
        node_ptr assign = car(init);
        assign = find_node(EQDEF, find_node(SMALLINIT, car(assign), Nil),
                           cdr(assign));

        instance_assign = EXPR_AND(assign, instance_assign);
        init = cdr(init);
      }
    }
    {
      node_ptr next = HrcNode_get_next_assign_exprs(node);
      while (Nil != next) {
        node_ptr assign = car(next);
        assign = find_node(EQDEF, find_node(NEXT, car(assign), Nil),
                           cdr(assign));
        instance_assign = EXPR_AND(assign, instance_assign);
        next = cdr(next);
      }
    }
    {
      node_ptr invar = HrcNode_get_invar_assign_exprs(node);
      while (Nil != invar) {
        node_ptr assign = car(invar);
        assign = find_node(EQDEF, car(assign), cdr(assign));
        instance_assign = EXPR_AND(assign, instance_assign);
        invar = cdr(invar);
      }
    }

    /* -- Step 2 -> Contextualize all assignments and add it to the
       hierarchy assignment field -- */
    if (Nil != instance_assign) {
      instance_assign = hrc_flattener_contextualize_expr(instance_assign,
                                                         context);

      instance_assign = EXPR_AND(instance_assign, cdr(car(assign_list)));


      setcdr(car(assign_list), instance_assign);
    }
  }

  /* LTL PROPERTIES */
  {
    node_ptr ltl = HrcNode_get_ltl_properties(node);
    node_ptr ltl_ctx = hrc_flattener_build_properties(self, ltl,
                                                      context, LTLSPEC);
    node_ptr old_ltl = FlatHierarchy_get_ltlspec(self->hierarchy);
    FlatHierarchy_set_ltlspec(self->hierarchy, append_ns(ltl_ctx, old_ltl));
  }

  /* INVAR PROPERTIES */
  {
    node_ptr invar = HrcNode_get_invar_properties(node);
    node_ptr invar_ctx = hrc_flattener_build_properties(self, invar,
                                                        context, INVARSPEC);
    node_ptr old_invar = FlatHierarchy_get_invarspec(self->hierarchy);
    FlatHierarchy_set_invarspec(self->hierarchy, append_ns(invar_ctx, old_invar));
  }

  /* CTL PROPERTIES */
  {
    node_ptr ctl = HrcNode_get_ctl_properties(node);
    node_ptr ctl_ctx = hrc_flattener_build_properties(self, ctl, context, SPEC);
    node_ptr old_ctl = FlatHierarchy_get_spec(self->hierarchy);
    FlatHierarchy_set_spec(self->hierarchy, append_ns(ctl_ctx, old_ctl));
  }

  /* PSL PROPERTIES */
  {
    node_ptr psl = HrcNode_get_psl_properties(node);
    node_ptr psl_ctx = hrc_flattener_build_properties(self, psl,
                                                      context, PSLSPEC);
    node_ptr old_psl = FlatHierarchy_get_pslspec(self->hierarchy);
    FlatHierarchy_set_pslspec(self->hierarchy, append_ns(psl_ctx, old_psl));
  }

  /* COMPUTE PROPERTIES */
  {
    node_ptr compute = HrcNode_get_compute_properties(node);
    node_ptr compute_ctx = hrc_flattener_build_properties(self, compute,
                                                          context, COMPUTE);
    node_ptr old_compute = FlatHierarchy_get_compute(self->hierarchy);
    FlatHierarchy_set_compute(self->hierarchy,
                              append_ns(compute_ctx, old_compute));
  }
}



/**Function********************************************************************

  Synopsis           [Does the actual flattening, recursively]

  Description        [Does the actual flattening and contextualization
                      of expressions and properties. Recursively
                      descends on module instances. All symbols are
                      flattened and declared into the symbol layer]

  SideEffects        []

  SeeAlso            [HrcFlattener_flatten_hierarchy]

******************************************************************************/
static void hrc_flattener_flatten_recur(HrcFlattener_ptr self,
                                        HrcNode_ptr node,
                                        node_ptr context)
{

  hrc_flattener_populate_symb_table(self, node, context);

  if (self->build_hierarchy) {
    hrc_flattener_populate_model(self, node, context);
  }

  /* Instances: do recursion */
  {
    Siter iter;
    SLIST_FOREACH(HrcNode_get_child_hrc_nodes(node), iter) {
      HrcNode_ptr child = Siter_element(iter);
      node_ptr name = HrcNode_get_instance_name(child);
      node_ptr new_context = hrc_flattener_concat_context(context, name);
      hrc_flattener_flatten_recur(self, child, new_context);
    }
  }
}


/**Function********************************************************************

  Synopsis           [Returns a list of flathierarchy-structure
                      compliant properties]

  Description        [Returns a list of properties decorated with
                      CONTEXT and property-type top-level node
                      (LTLSPEC, SPEC, COMPUTE, INVARSPEC, PSLSPEC)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr hrc_flattener_build_properties(HrcFlattener_ptr self,
                                               node_ptr prop_list,
                                               node_ptr ctx,
                                               short int type)
{
  node_ptr ctx_prop = Nil;
  while (Nil != prop_list) {
    node_ptr spec = car(prop_list);
    node_ptr prop_name = cdr(spec);

    if (Nil != prop_name) {
      ResolveSymbol_ptr rs;

      rs = SymbTable_resolve_symbol(self->symb_table, prop_name, ctx);
      prop_name = ResolveSymbol_get_resolved_name(rs);

      if (!FlatHierarchy_add_property_name(self->hierarchy, prop_name)) {
        internal_error("Property named %s already declared",
                       sprint_node(prop_name));
      }
    }

    spec = hrc_flattener_contextualize_expr(car(spec), ctx);
    spec = find_node(type, spec, prop_name);
    ctx_prop = cons(spec, ctx_prop);
    prop_list = cdr(prop_list);
  }
  return ctx_prop;
}


/**Function********************************************************************

  Synopsis           [Returns a big-and of the given expression list]

  Description        [Returns a big-and of the given expression list]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr hrc_flattener_expression_and(node_ptr expr_list)
{
  node_ptr new_expr = Nil;
  while (Nil != expr_list) {
    node_ptr expr = car(expr_list);
    new_expr = EXPR_AND(expr, new_expr);
    expr_list = cdr(expr_list);
  }
  return new_expr;
}


/**Function********************************************************************

  Synopsis           [Returns a contextualized version of the expression]

  Description        [Returns a contextualized version of the expression]

  SideEffects        []

  SeeAlso            [hrc_flattener_concat_context]

******************************************************************************/
static node_ptr hrc_flattener_contextualize_expr(node_ptr expr,
                                                 node_ptr context)
{
  node_ptr _expr;
  if (Nil == expr) return Nil;

  if (CONTEXT == node_get_type(expr)) {
    /* Concatenate contexts */
    context = hrc_flattener_concat_context(context, car(expr));
    _expr = cdr(expr);
  }
  else {
    _expr = expr;
  }

  return find_node(CONTEXT, context, _expr);
}

/**Function********************************************************************

  Synopsis           [Declares each variable of the given array within
                      the symbol table]

  Description        [Declares each variable of the given array within
                      the symbol table.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hrc_flattener_declare_variables(HrcFlattener_ptr self,
                                            node_ptr variables, node_ptr context,
                                            Instantiation_Variables_Mode_Type mode)
{
  while (Nil != variables) {
    node_ptr var = car(car(variables));
    node_ptr type = cdr(car(variables));

    nusmv_assert(CONS == node_get_type(car(variables)));

    /* Some identifiers may not have been created with find_node. Trap
       this here. */
    var = node_normalize(var);

    if (PROCESS == node_get_type(type)) {
      _PRINTLN("Processes are not yet supported by the HRC hierarchy");
      error_unreachable_code();
    }
    else {
      SymbType_ptr symbolicType
        = Compile_InstantiateType(self->symb_table,
                                  self->layer,
                                  var, type, context);
      boolean success
        = Compile_DeclareVariable(self->symb_table, self->layer,
                                  var, symbolicType, context, mode);
      nusmv_assert(success);
    }

    variables = cdr(variables);
  } /* while */
}

/**Function********************************************************************

  Synopsis           [Contatenates 2 contexts]

  Description        [Contatenates 2 contexts]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static node_ptr hrc_flattener_concat_context(node_ptr ctx1,
                                             node_ptr ctx2)
{
  /* return find_node(DOT,  */
  /*                  node_normalize(ctx1),  */
  /*                  node_normalize(ctx2)); */
  return CompileFlatten_concat_contexts(ctx1, ctx2);
}


/**Function********************************************************************

   Synopsis           [Instantiates the elements of a array define]

   Description [For every cell and every dimension create a correct
   binding in the symbol layer]


   SideEffects        [Elements are added to the layer an the symbol table]

   SeeAlso            []

******************************************************************************/
static void
hrc_flattener_instantiate_array_define(SymbTable_ptr st,
                                  SymbLayer_ptr layer,
                                  node_ptr name,
                                  node_ptr mod_name,
                                  node_ptr definition)
{
  if (!SymbLayer_can_declare_define(layer, name)) {
    error_redefining(name);
    error_unreachable_code();
  }

  switch (node_get_type(definition)) {
  case ARRAY_DEF:
    {
      node_ptr iter;
      int idx;
      nusmv_assert((cdr(definition) == Nil) &&
                   "Wrong node arity found: ARRAY_DEF must be unary!");

      /* Declare this symbol */
      SymbLayer_declare_array_define(layer, name, mod_name, definition);

      /* Instantiate every element of the array individually
         with first index = 0 */
      for (idx = 0, iter = car(definition);
           iter != Nil;
           idx += 1, iter = cdr(iter)) {
        /* definition has to be a list of values */
        nusmv_assert(CONS == node_get_type(iter));

        /* Instantiate name[idx] element */
        node_ptr index = find_node(NUMBER, NODE_FROM_INT(idx), Nil);
        hrc_flattener_instantiate_array_define(st, layer,
                                          find_node(ARRAY, name, index),
                                          mod_name, car(iter));
      }
      break;
    }

  default:
    {
      /* Declare this element */
      SymbLayer_declare_define(layer, name, mod_name, definition);
    }
  }
}


/**Function********************************************************************

  Synopsis           [The HrcFlattener class private initializer]

  Description        [The HrcFlattener class private initializer]

  SideEffects        []

  SeeAlso            [HrcFlattener_create]

******************************************************************************/
static void hrc_flattener_init(HrcFlattener_ptr self, HrcNode_ptr node,
                               SymbTable_ptr symb_table, SymbLayer_ptr layer)
{
  /* members initialization */
  self->root_node = node;

  /* The flattener can only deal with root nodes. If one wants to
     flatten a single instance, should first use the Hrc localizer
     (see hrc_localize_localize_hierarchy in hrcLocalize.c) */
  nusmv_assert(HrcNode_is_root(node));

  self->symb_table = symb_table;

  /* If no layer is given, create a new one */
  if (SYMB_LAYER(NULL) == layer) {
    layer = SymbTable_create_layer(symb_table, MODEL_LAYER_NAME,
                                   SYMB_LAYER_POS_DEFAULT);

    SymbTable_layer_add_to_class(self->symb_table, MODEL_LAYER_NAME,
                                 MODEL_LAYERS_CLASS);

    SymbTable_set_default_layers_class_name(self->symb_table, MODEL_LAYERS_CLASS);
  }

  self->layer = layer;


  self->hierarchy = FLAT_HIERARCHY(NULL);
}


/**Function********************************************************************

  Synopsis           [The HrcFlattener class private deinitializer]

  Description        [The HrcFlattener class private deinitializer]

  SideEffects        []

  SeeAlso            [HrcFlattener_destroy]

******************************************************************************/
static void hrc_flattener_deinit(HrcFlattener_ptr self)
{
  /* members deinitialization */
}


/**AutomaticEnd***************************************************************/

