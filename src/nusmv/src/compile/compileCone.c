/**CFile***********************************************************************

  FileName    [compileCone.c]

  PackageName [compile]

  Synopsis    [Computes the cone of influence of the model variables.]

  Description [This file contains the functions needed for computing
  the cone of influence (COI) of a given formula. The COI of all the
  variables in the model is pre-computed ancd cached the first time
  a cone of influence is required (function <code>initCoi</code>.
  Functions are also provided that compute the dependency variables
  for a formula, namely those variables that appear in the formula
  or in one of the definitions the formula depends on.]

  SeeAlso     []

  Author      [Marco Roveri and Marco Pistore]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst.

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

#include "compileInt.h"

#include "compile/symb_table/SymbTable.h"

#include "set/set.h"
#include "utils/ustring.h"
#include "utils/assoc.h"
#include "parser/symbols.h"
#include "utils/error.h"
#include "enc/enc.h"


#include "parser/psl/psl_symbols.h"
#include "parser/psl/pslNode.h"

#include "compile/symb_table/ResolveSymbol.h"

static char rcsid[] UTIL_UNUSED = "$Id: compileCone.c,v 1.11.4.12.4.15.4.38 2010-01-29 15:22:32 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

const int _UNTIMED_CURRENT = -2;
const int _UNTIMED_NEXT = -1;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis     [Indicates that the dependency computation is ongoing.]

  Description  [The value used during the building of dependencies of
  defined symbols to keep track that compuation is ongoing to discover
  circular definitions.]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define BUILDING_DEP_SET (Set_t)-10


/**Macro***********************************************************************

  Synopsis     [Indicates that the dependency is empty]

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define EMPTY_DEP_SET (Set_t)-11


/**Macro***********************************************************************

  Synopsis     [Indicates that no dependency has been yet computed.]

  Description  []

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define NO_DEP_SET (Set_t)-12


/**Macro***********************************************************************

  Synopsis     [Indicates that the COI computation should be verbose.]

  Description  [Indicates that the COI computation should be verbose.]

  SideEffects  []

  SeeAlso      []

******************************************************************************/
#define COI_VERBOSE (opt_verbose_level_gt(OptsHandler_get_instance(), 2))


/**Macro***********************************************************************

  Synopsis     [Clears and frees (with entries) given hash]

  Description  []

  SeeAlso      []

******************************************************************************/
#define FREE_HASH_ENTRIES(hash, func) \
  if (hash != (hash_ptr) NULL) {      \
    clear_assoc_and_free_entries(hash, func); \
    free_assoc(hash);                         \
    hash = (hash_ptr) NULL;                   \
  }


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Variable********************************************************************

  Synopsis    [The hash of cone for a variable]

  Description [This hash associates to a variable the corresponding
  cone of influence, once it has been computes.]

  SeeAlso     []

******************************************************************************/
static hash_ptr coi_hash = (hash_ptr) NULL;
void init_coi_hash() {
  if (coi_hash == (hash_ptr) NULL) {
    coi_hash = new_assoc();
    nusmv_assert(coi_hash != (hash_ptr) NULL);
  }
}
void insert_coi_hash(node_ptr key, Set_t value) {
  nusmv_assert(coi_hash != (hash_ptr)NULL);
  if (value != BUILDING_DEP_SET && value != NO_DEP_SET && value != NULL) {
    value = Set_Freeze(value);
    value = Set_Copy(value);
  }
  insert_assoc(coi_hash, key, (node_ptr)value);
}
Set_t lookup_coi_hash(node_ptr key) {
  nusmv_assert(coi_hash != (hash_ptr)NULL);
  return (Set_t) find_assoc(coi_hash, key);
}

static assoc_retval coi_hash_free(char *key, char *data, char * arg)
{
  Set_t element = (Set_t) data;

  if (((Set_t)NULL != element) && (NO_DEP_SET != element)) {
    Set_ReleaseSet(element);
  }
  return ASSOC_DELETE;
}

void clear_coi_hash() {
  FREE_HASH_ENTRIES(coi_hash, coi_hash_free);
}


/**Variable********************************************************************

  Synopsis    [The hash of dependencies for defined symbols]

  Description [This hash associates to a defined atom the
  corresponding set]

  SeeAlso     []

******************************************************************************/
static hash_ptr define_dep_hash = (hash_ptr)NULL;
void init_define_dep_hash() {
  define_dep_hash = new_assoc();
  nusmv_assert(define_dep_hash != (hash_ptr)NULL);
}
void insert_define_dep_hash(node_ptr key, Set_t value) {
  nusmv_assert(define_dep_hash != (hash_ptr) NULL);

  if (value != BUILDING_DEP_SET && value != NULL && value != EMPTY_DEP_SET) {
    value = Set_Freeze(value);
    value = Set_Copy(value);
  }
  insert_assoc(define_dep_hash, key, (node_ptr) value);
}
Set_t lookup_define_dep_hash(node_ptr key) {
  nusmv_assert(define_dep_hash != (hash_ptr)NULL);
  return (Set_t) find_assoc(define_dep_hash, key);
}

static assoc_retval define_dep_hash_free(char *key, char *data, char * arg)
{
  Set_t element = (Set_t) data;

  if ((element != (Set_t) NULL) && (element != BUILDING_DEP_SET) &&
      (element != EMPTY_DEP_SET)) {
    Set_ReleaseSet(element);
  }
  return ASSOC_DELETE;
}

void clear_define_dep_hash()
{
  FREE_HASH_ENTRIES(define_dep_hash, define_dep_hash_free);
}


/**Variable********************************************************************

  Synopsis    [The hash of dependencies for a given formula.]

  Description [This hash associates to each formula the corresponding
  set of dependencies.]

  SeeAlso     []

******************************************************************************/
static  hash_ptr dependencies_hash = (hash_ptr)NULL;
void init_dependencies_hash() {
  dependencies_hash = new_assoc();
  nusmv_assert(dependencies_hash != (hash_ptr)NULL);
}
void insert_dependencies_hash(node_ptr key, Set_t value) {
  nusmv_assert(dependencies_hash != (hash_ptr)NULL);
  if (value != BUILDING_DEP_SET && value != NULL && value != EMPTY_DEP_SET) {
    value = Set_Freeze(value);
    value = Set_Copy(value);
  }
  insert_assoc(dependencies_hash, key, NODE_PTR(value));
}
Set_t lookup_dependencies_hash(node_ptr key) {
  nusmv_assert(dependencies_hash != (hash_ptr)NULL);
  return (Set_t) find_assoc(dependencies_hash, key);
}

static assoc_retval dependencies_hash_free(char *key, char *data, char * arg)
{
  Set_t element = (Set_t) data;

  if (element != (Set_t)NULL && element != BUILDING_DEP_SET &&
      element != EMPTY_DEP_SET) {
    Set_ReleaseSet(element);
  }
  return ASSOC_DELETE;
}

void clear_dependencies_hash() {
  FREE_HASH_ENTRIES(dependencies_hash, dependencies_hash_free);
}

static node_ptr mk_hash_key(node_ptr e, node_ptr c, SymbFilterType filter,
                            boolean preserve_time, int time)
{
  return find_node(CONS,
                   NODE_FROM_INT(time),
                   find_node(CONS,
                             find_node(CONS,
                                       find_node(CONTEXT, c, e),
                                       NODE_FROM_INT(filter)),
                             NODE_FROM_INT(preserve_time)));
}



/**Variable********************************************************************

  Synopsis    [The hash of constants of a given formula.]

  Description [This hash associates to each formula the corresponding
  set of constants]

  SeeAlso     []

******************************************************************************/
static  hash_ptr consts_hash = (hash_ptr)NULL;
void init_consts_hash()
{
  consts_hash = new_assoc();
  nusmv_assert(consts_hash != (hash_ptr)NULL);
}
void insert_consts_hash(node_ptr key, Set_t value)
{
  nusmv_assert(consts_hash != (hash_ptr)NULL);
  if (value != NULL && value != EMPTY_DEP_SET) {
    value = Set_Copy(Set_Freeze(value));
  }
  insert_assoc(consts_hash, key, (node_ptr) value);
}
Set_t lookup_consts_hash(node_ptr key)
{
  nusmv_assert(consts_hash != (hash_ptr)NULL);
  return (Set_t) find_assoc(consts_hash, key);
}
static assoc_retval consts_hash_free(char *key, char *data, char * arg)
{
  Set_t element = (Set_t) data;

  if (element != (Set_t)NULL && element != EMPTY_DEP_SET) {
    Set_ReleaseSet(element);
  }
  return ASSOC_DELETE;
}
void clear_consts_hash()
{
  FREE_HASH_ENTRIES(consts_hash, consts_hash_free);
}


/**Variable********************************************************************

  Synopsis    [The hash of cone at depth 0 for a variable]

  Description [This hash associates to a variable the corresponding
  cone of influence at depth 0, once it has been computes.]

  SeeAlso     []

******************************************************************************/
static hash_ptr coi0_hash = (hash_ptr) NULL;
void init_coi0_hash() {
  if (coi0_hash == (hash_ptr) NULL) {
    coi0_hash = new_assoc();
    nusmv_assert(coi0_hash != (hash_ptr) NULL);
  }
}
void insert_coi0_hash(node_ptr key, Set_t value) {
  nusmv_assert(coi0_hash != (hash_ptr)NULL);
  if (value != BUILDING_DEP_SET && value != NO_DEP_SET && value != NULL) {
    value = Set_Freeze(value);
    value = Set_Copy(value);
  }
  insert_assoc(coi0_hash, key, (node_ptr)value);
}
Set_t lookup_coi0_hash(node_ptr key) {
  nusmv_assert(coi0_hash != (hash_ptr)NULL);
  return (Set_t) find_assoc(coi0_hash, key);
}

static assoc_retval coi0_hash_free(char *key, char *data, char * arg)
{
  Set_t element = (Set_t) data;

  if (((Set_t)NULL != element) && (NO_DEP_SET != element)) {
    Set_ReleaseSet(element);
  }
  return ASSOC_DELETE;
}

void clear_coi0_hash() {
  FREE_HASH_ENTRIES(coi0_hash, coi0_hash_free);
}

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static Set_t
formulaGetDependenciesRecur ARGS((const SymbTable_ptr,
                                  node_ptr, node_ptr,
                                  SymbFilterType, boolean, int));


static Set_t
formulaGetDefinitionDependencies ARGS((const SymbTable_ptr, node_ptr,
                                       SymbFilterType,
                                       boolean preserve_time, int time));

static void
coiInit ARGS((const SymbTable_ptr symb_table, FlatHierarchy_ptr hierarchy));


static Set_t
formulaGetConstantsRecur ARGS((const SymbTable_ptr symb_table,
                               node_ptr formula, node_ptr context));

#if 0
/* Disabled because no longer used */
static Set_t
get_array_cells ARGS((const SymbTable_ptr symb_table, node_ptr arr));

static void resolve_range ARGS((SymbTable_ptr st,
                          node_ptr range, node_ptr context,
                          int* low, int* high));
#endif

static Set_t
_coi_get_var_coi0 ARGS((SymbTable_ptr st,
                        FlatHierarchy_ptr hierarchy,
                        node_ptr var,
                        boolean * nonassign,
                        boolean use_cache));


static Set_t
computeCoiVar ARGS((SymbTable_ptr st, FlatHierarchy_ptr fh, node_ptr var));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis           [Computes dependencies of a given SMV expression]

  Description        [The set of dependencies of a given formula are
  computed. A traversal of the formula is performed. Each time a
  variable is encountered, it is added to the so far computed
  set. When a formula depends on a next variable, then the
  corresponding current variable is added to the set. When an atom is
  found a call to <tt>formulaGetDefinitionDependencies</tt> is
  performed to compute the dependencies. Returned set must be disposed
  by the caller]

  SideEffects        []

  SeeAlso            [formulaGetDefinitionDependencies]

******************************************************************************/
Set_t Formula_GetDependencies(const SymbTable_ptr symb_table,
                              node_ptr formula, node_ptr context)
{
  return Formula_GetDependenciesByType(symb_table, formula,
                                       context, VFT_ALL, false);
}

/**Function********************************************************************

  Synopsis           [Computes the dependencies of an SMV expression by type]

  Description        [The set of dependencies of a given formula are
  computed, as in Formula_GetDependencies, but the variable type filters the
  dependency collection.

  If flag preserve_time is true, then entries in the returned set
  will preserve the time they occur within the formula. For
  example, formula 'a & next(b) = 2 & attime(c, 2) < 4' returns
  {a,b,c} if preserve_time is false, and {a, next(b), attime(c, 2)}
  if preserve_time is true.

  Returned set must be disposed by the caller]

  SideEffects        []

  SeeAlso            [formulaGetDependenciesByTypeAux
  formulaGetDefinitionDependencies]

******************************************************************************/
Set_t
Formula_GetDependenciesByType(const SymbTable_ptr symb_table,
                              node_ptr formula, node_ptr context,
                              SymbFilterType filter,
                              boolean preserve_time)
{
  Set_t result;
  int temp;

  if (formula == Nil) return Set_MakeEmpty();

  temp = yylineno;

  yylineno = node_get_lineno(formula);
  result = formulaGetDependenciesRecur(symb_table, formula, context, filter,
                                       preserve_time, _UNTIMED_CURRENT);
  yylineno = temp;

  return result;
}


/**Function********************************************************************

  Synopsis           [Compute the dependencies of two set of formulae]

  Description [Given a formula and a list of fairness constraints, the
  set of variables occurring in them is computed. Returned Set must be
  disposed by the caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t Formulae_GetDependencies(const SymbTable_ptr symb_table,
                               node_ptr formula,
                               node_ptr justice, node_ptr compassion)
{
  Set_t result1, result2, result3;

  result1 = Formula_GetDependencies(symb_table, formula, Nil);
  result2 = Formula_GetDependencies(symb_table, justice, Nil);
  result3 = Formula_GetDependencies(symb_table, compassion, Nil);

  result1 = Set_Union(result1, result2);
  result1 = Set_Union(result1, result3);
  Set_ReleaseSet(result3);
  Set_ReleaseSet(result2);
  return result1;
}


/**Function********************************************************************

  Synopsis           [Calculates the set of constants occurring into
  the given formula]

  Description [Given a formula the set of constants occurring in
  them is computed and returned. Returned set must be disposed by the caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t Formula_GetConstants(const SymbTable_ptr symb_table,
                           node_ptr formula, node_ptr context)
{
  return formulaGetConstantsRecur(symb_table, formula, context);
}

/**Function********************************************************************

  Synopsis           [Computes the COI of a given expression]

  Description        [Computes the COI of a given expression,
                      up to step "steps" (or fixpoint if steps = -1).
                      If not NULL, if the fixpoint has been reached
                      (ie: there are no more dependencies), reached_fixpoint
                      is set to true.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t ComputeCOIFixpoint(const SymbTable_ptr symb_table,
                         const FlatHierarchy_ptr hierarchy,
                         const Expr_ptr expression,
                         const int steps,
                         boolean* reached_fixpoint)
{
  Set_t deps;
  Set_t symbols_left;
  int i = 0;

  deps = Formula_GetDependencies(symb_table, expression, Nil);

  if (steps == -1) {
    return ComputeCOI(symb_table, deps);
  }

  symbols_left = Set_Copy(deps);

  while ((i < steps) && (!Set_IsEmpty(symbols_left))) {
    Set_Iterator_t iter;
    Set_t tmp = Set_MakeEmpty();

    SET_FOREACH(symbols_left, iter) {
      Set_t base;
      node_ptr var = Set_GetMember(symbols_left, iter);
      boolean nonassign = false;

      /* Here we do no enable memoizing for cone at depth 0. The
         reason being that we are not guaranteed the hierarchy is the
         mainFlatHierarchy. If it is not, than possible momoized
         values may be wrong for this hierarchy */
      base = _coi_get_var_coi0(symb_table, hierarchy, var, &nonassign, false);

      tmp = Set_Union(tmp, base);
      Set_ReleaseSet(base);
    }

    /* Remove from the freshly found variables the variables we
       already know  */
    Set_ReleaseSet(symbols_left);
    symbols_left = Set_Copy(tmp);
    symbols_left = Set_Difference(symbols_left, deps);
    deps = Set_Union(deps, tmp);
    Set_ReleaseSet(tmp);

    ++i;
  }

  if ((boolean*)NULL != reached_fixpoint) {
    if (Set_IsEmpty(symbols_left)) *reached_fixpoint = true;
    else *reached_fixpoint = false;
  }

  Set_ReleaseSet(symbols_left);

  return deps;
}

/**Function********************************************************************

  Synopsis           [Computes the COI of a given set of variables, defined
  within the given symb_table]

  Description        [Computes the COI of a given set of variables, defined
  within the given symb_table. Returned Set must be disposed by the caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t ComputeCOI(const SymbTable_ptr symb_table, Set_t base)
{
  Set_t coi = Set_Copy(base);
  Set_Iterator_t iter;

  if (! cmp_struct_get_coi(cmps)) {
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "Initializing Cone Of Influence...\n");
    }


    /*   [MR2??]: by making it explicit on which hierarchy it operates.*/

    coiInit(symb_table, mainFlatHierarchy);
    cmp_struct_set_coi(cmps);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 1)) {
      fprintf(nusmv_stderr, "COI initialized.\n");
    }
  }

  SET_FOREACH(base, iter) {
    node_ptr var = Set_GetMember(base, iter);


    /*   [MR2??]: by making it explicit on which hierarchy it operates. */
    Set_t varcoi = computeCoiVar(symb_table, mainFlatHierarchy, var);
    coi = Set_Union(coi, varcoi);
  }

  return coi;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Compute the dependencies of an atom]

  Description [This function computes the dependencies of an atom. If
  the atom corresponds to a variable then the singleton with the
  variable is returned. If the atom corresponds to a "running"
  condition the singleton with variable PROCESS_SELECTOR_VAR_NAME is
  returned. Otherwise if the atom corresponds to a defined symbol the
  dependency set corresponding to the body of the definition is
  computed and returned. filter specifies what variables we are
  interested to, as in Formula_GetDependenciesByType, and
  is_inside_next is supposed to be true if the atom is inside a Next,
  false otherwise. Returned set must be disposed by the caller]

  SideEffects        [The <tt>define_dep_hash</tt> is modified in
  order to memoize previously computed dependencies of defined symbols.]

  SeeAlso            [Formula_GetDependencies]

******************************************************************************/

static Set_t
formulaGetDefinitionDependencies(const SymbTable_ptr symb_table,
                                 node_ptr formula, SymbFilterType filter,
                                 boolean preserve_time, int time)
{
  Set_t result;
  if (SymbTable_is_symbol_var(symb_table, formula)) {
    if (((filter & VFT_INPUT) && SymbTable_is_symbol_input_var(symb_table,
                                                               formula)) ||
         ((filter & VFT_CURRENT) && SymbTable_is_symbol_state_var(symb_table,
                                                                  formula)) ||
         ((filter & VFT_FROZEN) && SymbTable_is_symbol_frozen_var(symb_table,
                                                                  formula))) {
      if (preserve_time) {
        if ( _UNTIMED_NEXT == time &&
             !SymbTable_is_symbol_input_var(symb_table, formula) ) {
          formula = Expr_next(formula, symb_table);
        }
        else if (time >= 0) {
          formula = Expr_attime(formula, time, symb_table);
        }
      }


      return Set_MakeSingleton((Set_Element_t) formula);
    }
    /* a variable filtered out */
    return Set_MakeEmpty();
  } /* end of outer if branch */

  if (SymbTable_is_symbol_define(symb_table, formula)) {
    node_ptr key = mk_hash_key(formula, Nil, filter, preserve_time, time);
    result = lookup_define_dep_hash(key);
    if (result == BUILDING_DEP_SET) { error_circular(formula); }

    if (result == EMPTY_DEP_SET) {
      if (filter & VFT_DEFINE) {
        result = Set_MakeSingleton(formula);
        /* We mark the formula as closed, storing the computed
           dependencies for further use. */
        insert_define_dep_hash(key, result);
        return result;
      }
      else {
        return Set_MakeEmpty();
      }
    }

    if (result == (Set_t) NULL) {
      /* We mark the formula as open and we start looking for the body
         dependencies. */
      node_ptr nformula;
      insert_define_dep_hash(key, BUILDING_DEP_SET);
      io_atom_push(formula);
      nformula = SymbTable_get_define_body(symb_table, formula);
      result = formulaGetDependenciesRecur(symb_table, nformula,
                            SymbTable_get_define_context(symb_table, formula),
                            filter, preserve_time, time);
      io_atom_pop();

      /* We add define to the result set if defines are include in filter */
      if (filter & VFT_DEFINE) {
        result = Set_AddMember(result, formula);
      }

      /* We mark the formula as closed, storing the computed
         dependencies for further use. */
      if (Set_IsEmpty(result)) insert_define_dep_hash(key, EMPTY_DEP_SET);
      else insert_define_dep_hash(key, result);
    }
    else {
      result = Set_Copy(result);
    }
  }
  else if (SymbTable_is_symbol_array_define(symb_table, formula)) {
    /* Recursively compute the dependencies for this define array */
    node_ptr nformula = SymbTable_get_array_define_body(symb_table, formula);
    result = formulaGetDependenciesRecur(symb_table, nformula,
                    SymbTable_get_array_define_context(symb_table, formula),
                    filter, preserve_time, time);

    /* We add the define array to the result set if defines are include in
       filter */
    if (filter & VFT_DEFINE) {
      result = Set_AddMember(result, formula);
    }
  }
  else if (SymbTable_is_symbol_variable_array(symb_table, formula)) {
    /* Array dependencies are all it's elements */

    SymbType_ptr type;
    int low, high;
    int i;

    type = SymbTable_get_variable_array_type(symb_table, formula);
    low = SymbType_get_array_lower_bound(type);
    high = SymbType_get_array_upper_bound(type);

    result = Set_MakeEmpty();

    for (i = low; i <= high; ++i) {
      node_ptr index = find_node(NUMBER, NODE_FROM_INT(i), Nil);
      node_ptr arr_var = find_node(ARRAY, formula, index);
      Set_t ret;

      ret = formulaGetDefinitionDependencies(symb_table, arr_var, filter,
                                             preserve_time, time);

      result = Set_Union(result, ret);
    }

  }
  else {
    fprintf(nusmv_stderr, "Undefined symbol \"");
    print_node(nusmv_stderr, formula);
    fprintf(nusmv_stderr, "\"\n");
    nusmv_exit(1);
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Recursive call to Formula_GetDependenciesByType]

  Description        [Recursive call to Formula_GetDependenciesByType.
  Returned set must be released by the caller.]

  SideEffects        []

  SeeAlso            [formulaGetDefinitionDependencies
Formula_GetDependenciesByType]

******************************************************************************/
static Set_t
formulaGetDependenciesRecur(const SymbTable_ptr symb_table,
                            node_ptr formula, node_ptr context,
                            SymbFilterType filter,
                            boolean preserve_time, int time)
{
  Set_t result;
  node_ptr hash_key;

  /* 0 for filter means no variables are looked for. Do not create
     a special constant in SymbFilterType, otherwise it will be
     visible outside (which may be not good). */
  if (formula == Nil || filter == 0) return Set_MakeEmpty();

  hash_key = mk_hash_key(formula, context, filter, preserve_time, time);
  result = lookup_dependencies_hash(hash_key);

  if (result == EMPTY_DEP_SET) return Set_MakeEmpty();
  if (result != (Set_t) NULL) return Set_Copy(result);

  switch (node_get_type(formula)) {
  case CONTEXT:
    result = formulaGetDependenciesRecur(symb_table, cdr(formula),
                                         car(formula), filter,
                                         preserve_time, time);
    break;

  case BIT:
    /* ignore bits, consider only scalar vars */
    result = formulaGetDependenciesRecur(symb_table, car(formula), context,
                                         filter, preserve_time, time);
    break;

  case ATOM:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, formula, context);

      name = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_error(rs)) {
        ResolveSymbol_throw_error(rs);
      }

      if (ResolveSymbol_is_parameter(rs)) {
        /* a formal parameter */
        node_ptr param;
        param = SymbTable_get_flatten_actual_parameter(symb_table, name);
        result = formulaGetDependenciesRecur(symb_table, param, context,
                                             filter, preserve_time, time);
      }
      else if (ResolveSymbol_is_constant(rs)) result = Set_MakeEmpty();
      else { /* it should be a defined symbol, running, or a variable */
        result = formulaGetDefinitionDependencies(symb_table,
                                                  name, filter,
                                                  preserve_time, time);
      }

      break;
    }

    /* a variable of word type */
  case SIGNED_WORD:
  case UNSIGNED_WORD:
    /* ignore width (a constant), consider only bits */
    result = formulaGetDependenciesRecur(symb_table, car(formula),
                                         context, filter, preserve_time, time);
    break;

    /* no dependencies */
  case FAILURE:
  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case UWCONST:
  case SWCONST:
  case TWODOTS: /* Sets */
  case WSIZEOF: /* It is not important the expression inside, it is
                   like a constant */
  case CAST_TOINT: /* It is not important the expression inside, it is like
                  a constant */
    result = Set_MakeEmpty();
    break;

    /* unary operation */
  case NEXT:   /* next(alpha), with alpha possibly complex, thus ... */
    /* nested next are checked by type checker */
    if(filter & VFT_NEXT) {
      /* the next variables become the current variables from this frame on */
      filter = (filter & (~VFT_NEXT)) | VFT_CURRENT;
    }
    else if (filter & VFT_CURRENT) {
      filter = filter & (~VFT_CURRENT);
    }

    /* input and frozen variables are searched for independently of
       next-operator */
    result = formulaGetDependenciesRecur(symb_table, car(formula), context,
                                         filter, preserve_time, _UNTIMED_NEXT);
    break;

  case ATTIME: /* the variable without time information are returned */
    {
      int time2 = Expr_attime_get_time(formula);
      result = formulaGetDependenciesRecur(symb_table, car(formula),
                                           context, filter,
                                           preserve_time, time2);
      break;
    }

  case NOT:    /* Unary boolean connectives */
  case UMINUS:
  case EX:    /* CTL unary Temporal Operators */
  case SMALLINIT:  /* used for init(expr) */
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case EBF:    /* CTL unary bounded Temporal Operators */
  case ABF:
  case EBG:
  case ABG:
  case EBU:    /* CTL binary bounded Temporal Operators */
  case ABU:
  case OP_NEXT:    /* LTL unary Temporal Operators */
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
  case CAST_BOOL: /* Casts */
  case CAST_WORD1:
  case CAST_SIGNED:
  case CAST_UNSIGNED:
    result = formulaGetDependenciesRecur(symb_table, car(formula), context,
                                         filter, preserve_time, time);
    break;

  case WRESIZE: /* For this it is important only the expression on the lhs */
    result = formulaGetDependenciesRecur(symb_table, car(formula), context,
                                         filter, preserve_time, time);
    break;

    /* binary operation */
  case EXTEND:
  case EQDEF: /* assignment */
  case CONS:
  case UNION:
  case SETIN:
  case COLON:
  case PLUS:    /* Numerical Operations */
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
  case LSHIFT:  /* Binary shifts and rotates */
  case RSHIFT:
  case LROTATE:
  case RROTATE:
  case CONCATENATION: /* concatenation */
  case EQUAL:   /* Comparison Operations */
  case NOTEQUAL:
  case LT:
  case GT:
  case LE:
  case GE:
  case AND:    /* Binary boolean connectives */
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
  case EU:     /* CTL binary  Temporal Operators */
  case AU:
  case UNTIL:    /* LTL binary Temporal Operators */
  case RELEASES:
  case SINCE:
  case TRIGGERED:
  case MAXU:    /* MIN MAX operators */
  case MINU:
    {
      Set_t right = formulaGetDependenciesRecur(symb_table,
                                                cdr(formula), context,
                                                filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                           car(formula), context,
                                           filter, preserve_time, time);
      result = Set_Union(result, right);
      Set_ReleaseSet(right);
      break;
    }

    /* 3-arity operations */
  case CASE:
  case IFTHENELSE:
    {
      Set_t then_arg  = formulaGetDependenciesRecur(symb_table,
                                                    cdr(car(formula)), context,
                                                    filter, preserve_time, time);
      Set_t else_arg  = formulaGetDependenciesRecur(symb_table,
                                                    cdr(formula), context,
                                                    filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                           car(car(formula)),context,
                                           filter, preserve_time, time);

      result = Set_Union(Set_Union(result, then_arg), else_arg);
      Set_ReleaseSet(else_arg);
      Set_ReleaseSet(then_arg);
      break;
    }

  case NFUNCTION:
    result = formulaGetDependenciesRecur(symb_table,
                                         cdr(formula), context,
                                         filter, preserve_time, time);
  break;

  case BIT_SELECTION:
    {
      Set_t high_bit = formulaGetDependenciesRecur(symb_table,
                                                   car(cdr(formula)), context,
                                                   filter, preserve_time, time);
      Set_t low_bit = formulaGetDependenciesRecur(symb_table,
                                                  cdr(cdr(formula)), context,
                                                  filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                           car(formula), context,
                                           filter, preserve_time, time);

      result = Set_Union(Set_Union(result, high_bit), low_bit);
      Set_ReleaseSet(low_bit);
      Set_ReleaseSet(high_bit);
      break;
    }

  case COUNT:
    {
      node_ptr list = car(formula);

      result = Set_MakeEmpty();

      while (Nil != list) {
        Set_t tmp = formulaGetDependenciesRecur(symb_table, car(list), context,
                                                filter, preserve_time, time);
        result = Set_Union(result, tmp);
        Set_ReleaseSet(tmp);

        list = cdr(list);
      }

      break;
    }
    /* Operations on WORDARRAYs */
  case WAWRITE:
    {
      Set_t location = formulaGetDependenciesRecur(symb_table,
                                                   car(cdr(formula)), context,
                                                   filter, preserve_time, time);
      Set_t address = formulaGetDependenciesRecur(symb_table,
                                                  cdr(cdr(formula)), context,
                                                  filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                           car(formula), context,
                                           filter, preserve_time, time);
      result = Set_Union(Set_Union(result, location), address);
      Set_ReleaseSet(address);
      Set_ReleaseSet(location);
      break;
    }

  case WAREAD:
    {
      Set_t location = formulaGetDependenciesRecur(symb_table,
                                                   cdr(formula), context,
                                                   filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                           car(formula), context,
                                           filter, preserve_time, time);
      result = Set_Union(result, location);
      Set_ReleaseSet(location);
      break;
    }


    /* name cases */
  case DOT:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, formula, context);
      name = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_constant(rs)) {
        result = Set_MakeEmpty();
      }
      else {
        result = formulaGetDefinitionDependencies(symb_table, name, filter,
                                                  preserve_time, time);
      }
      break;
    }

  case ARRAY:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, formula, context);
      name = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_defined(rs)) {
        /* this array is identifier-with-brackets */
        result = formulaGetDefinitionDependencies(symb_table, name, filter,
                                                  preserve_time, time);
      }
      else {
        /* this array is an expression => process the children */
        result = formulaGetDependenciesRecur(symb_table,
                                             car(formula), context,
                                             filter, preserve_time, time);
        Set_t tmp = formulaGetDependenciesRecur(symb_table,
                                                cdr(formula), context,
                                                filter, preserve_time, time);
        result = Set_Union(result, tmp);
        Set_ReleaseSet(tmp);
      }
      break;
    }


  case ARRAY_DEF:
    {
      node_ptr iter;

      result = Set_MakeEmpty();

      for (iter = car(formula); iter != Nil; iter = cdr(iter)) {
        nusmv_assert(CONS == node_get_type(iter));
        Set_t tmp = formulaGetDependenciesRecur(symb_table,
                                                car(iter), context,
                                                filter, preserve_time, time);
        result = Set_Union(result, tmp);
        Set_ReleaseSet(tmp);
      }
      break;
    }



  case PSL_INF:
  case PSL_RANGE:
    result = Set_MakeEmpty();
    break;

  case PSL_WSELECT:
    {
      PslNode_ptr right = psl_node_get_right(formula);

      Set_t object   = formulaGetDependenciesRecur(symb_table,
                                                   psl_node_get_left(formula),
                                                   context, filter,
                                                   preserve_time, time);
      Set_t high_bit = formulaGetDependenciesRecur(symb_table,
                                                   psl_node_get_left(right),
                                                   context, filter,
                                                   preserve_time, time);
      Set_t low_bit  = formulaGetDependenciesRecur(symb_table,
                                                   psl_node_get_right(right),
                                                   context, filter,
                                                   preserve_time, time);

      result = Set_Union(Set_Union(object, high_bit), low_bit);
      Set_ReleaseSet(low_bit);
      Set_ReleaseSet(high_bit);
      break;
    }

  case PSL_SERE:
  case PSL_SERECOMPOUND:
    result = formulaGetDependenciesRecur(symb_table,
                                         psl_node_get_left(formula), context,
                                         filter, preserve_time, time);
    break;

  case PSL_CONCATENATION:
    {

      Set_t right = formulaGetDependenciesRecur(symb_table,
                                                psl_node_get_right(formula),
                                                context,
                                                filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                           psl_node_get_left(formula),
                                           context,
                                           filter, preserve_time, time);
      result = Set_Union(result, right);
      Set_ReleaseSet(right);
      break;
    }

  case PSL_SERECONCAT:
  case PSL_SEREFUSION:
    {
      Set_t right = formulaGetDependenciesRecur(symb_table,
                                                psl_node_get_right(formula),
                                                context,
                                                filter, preserve_time, time);
      result = formulaGetDependenciesRecur(symb_table,
                                           psl_node_get_left(formula),
                                           context,
                                           filter, preserve_time, time);
      result = Set_Union(result, right);
      Set_ReleaseSet(right);
      break;
    }

  case PSL_SEREREPEATED:
    result = formulaGetDependenciesRecur(symb_table,
                           psl_node_sere_repeated_get_expr(formula),
                           context, filter, preserve_time, time);
    break;

  case PSL_REPLPROP:

    result = formulaGetDependenciesRecur(symb_table,
                           psl_node_repl_prop_get_property(formula),
                           context, filter, preserve_time, time);
    break;

  case PSL_PIPEMINUSGT:
  case PSL_PIPEEQGT:
  case PSL_DIAMONDMINUSGT:
    {
      Set_t con = formulaGetDependenciesRecur(symb_table,
                         psl_node_suffix_implication_get_consequence(formula),
                                              context,
                                              filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                           psl_node_suffix_implication_get_premise(formula),
                                           context,
                                           filter, preserve_time, time);
      result = Set_Union(result, con);
      Set_ReleaseSet(con);
      break;
    }

  case PSL_ALWAYS:
  case PSL_NEVER:
  case PSL_EVENTUALLYBANG:
    result = formulaGetDependenciesRecur(symb_table,
                                         psl_node_get_left(formula),
                                         context,
                                         filter, preserve_time, time);
    break;

  case PSL_WITHINBANG:
  case PSL_WITHIN:
  case PSL_WITHINBANG_:
  case PSL_WITHIN_:
    {
      Set_t n2 = formulaGetDependenciesRecur(symb_table,
                               psl_node_get_right(psl_node_get_left(formula)),
                                             context,
                                             filter, preserve_time, time);
      Set_t n3 = formulaGetDependenciesRecur(symb_table,
                                             psl_node_get_right(formula),
                                             context,
                                             filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                psl_node_get_left(psl_node_get_left(formula)),
                                           context,
                                           filter, preserve_time, time);

      result = Set_Union(Set_Union(result, n2), n3);
      Set_ReleaseSet(n3);
      Set_ReleaseSet(n2);
      break;
    }

  case PSL_NEXT_EVENT_ABANG:
  case PSL_NEXT_EVENT_A:
  case PSL_NEXT_EVENT_EBANG:
  case PSL_NEXT_EVENT_E:
  case PSL_NEXT_EVENTBANG:
  case PSL_NEXT_EVENT:
  case PSL_NEXT_ABANG:
  case PSL_NEXT_EBANG:
  case PSL_NEXT_A:
  case PSL_NEXT_E:
  case PSL_NEXTBANG:
  case PSL_NEXT:
  case PSL_X:
  case PSL_XBANG:
    {
      Set_t n2 = formulaGetDependenciesRecur(symb_table,
                                      psl_node_extended_next_get_when(formula),
                                             context,
                                             filter, preserve_time, time);
      Set_t n3 = formulaGetDependenciesRecur(symb_table,
                                 psl_node_extended_next_get_condition(formula),
                                             context,
                                             filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                      psl_node_extended_next_get_expr(formula),
                                             context,
                                             filter, preserve_time, time);

      result = Set_Union(Set_Union(result, n2), n3);
      Set_ReleaseSet(n3);
      Set_ReleaseSet(n2);
      break;
    }

  case PSL_BEFOREBANG:
  case PSL_BEFORE:
  case PSL_BEFOREBANG_:
  case PSL_BEFORE_:
  case PSL_UNTILBANG:
  case PSL_UNTIL:
  case PSL_UNTILBANG_:
  case PSL_UNTIL_:
  case PSL_ABORT:
  case PSL_W:
  case PSL_OR:
  case PSL_CARET:
  case PSL_TILDE:
  case PSL_EQEQ:
  case PSL_PIPEPIPE:
  case PSL_AMPERSANDAMPERSAND:
  case PSL_WHILENOTBANG:
  case PSL_WHILENOT:
  case PSL_WHILENOTBANG_:
  case PSL_WHILENOT_:
    {
      Set_t right = formulaGetDependenciesRecur(symb_table,
                                                psl_node_get_right(formula),
                                                context,
                                                filter, preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                           psl_node_get_left(formula),
                                           context,
                                           filter, preserve_time, time);
      result = Set_Union(result, right);
      Set_ReleaseSet(right);
      break;
    }

  case PSL_ITE:
    {
      Set_t then_arg  = formulaGetDependenciesRecur(symb_table,
                                             psl_node_get_ite_then(formula),
                                                    context, filter,
                                                    preserve_time, time);
      Set_t else_arg  = formulaGetDependenciesRecur(symb_table,
                                             psl_node_get_ite_else(formula),
                                                    context, filter,
                                                    preserve_time, time);

      result = formulaGetDependenciesRecur(symb_table,
                                           psl_node_get_ite_cond(formula),
                                           context, filter,
                                           preserve_time, time);
      result = Set_Union(Set_Union(result, then_arg), else_arg);
      Set_ReleaseSet(else_arg);
      Set_ReleaseSet(then_arg);
      break;
    }

  default:
      fprintf(nusmv_stderr,
              "\nFormula_GetDependencies: Reached undefined connective (%d)\n",
              node_get_type(formula));
      nusmv_exit(1);
  }

  if (Set_IsEmpty(result)) insert_dependencies_hash(hash_key, EMPTY_DEP_SET);
  else insert_dependencies_hash(hash_key, result);

  return result;
}


/**Function********************************************************************

  Synopsis           [Pre-compute the COI of the variables]

  Description        [Computes the COI of all the variables occurring within
  the symbol table]

  SideEffects        []

  SeeAlso            [ComputeCOI]

******************************************************************************/
static void coiInit(const SymbTable_ptr symb_table, FlatHierarchy_ptr hierarchy)
{
  SymbTableIter iter;

  init_coi_hash(); /* no action is taken here if already initialized */

  if (COI_VERBOSE) { fprintf(nusmv_stdout,"*** INIT COI ***\n"); }

  SYMB_TABLE_FOREACH(symb_table, iter, STT_VAR) {
    node_ptr var = SymbTable_iter_get_symbol(symb_table, &iter);
    boolean nonassign = false;
    Set_t base;

    /* We guarantee the set is initialized with something different
       from NULL */
    insert_coi0_hash(var, NO_DEP_SET);

    base = _coi_get_var_coi0(symb_table, hierarchy, var, &nonassign, true);

    /* We associate to the var no set */
    insert_coi_hash(var, NO_DEP_SET);

    if (COI_VERBOSE) {
      fprintf(nusmv_stdout,"Variable  ");
      print_node(nusmv_stdout, var);
      fprintf(nusmv_stdout,"\n");

      if (nonassign) {
        fprintf(nusmv_stdout,"  Has non-assign constraints\n");
      }

      fprintf(nusmv_stdout,"  Initial coi: ");
      Set_PrintSet(nusmv_stdout, base, NULL, NULL);
      fprintf(nusmv_stdout,"\n");
    }

    Set_ReleaseSet(base);
  } /* vars iteration */
}



/* Returns the set of constants occurring into the given formula */
static Set_t
formulaGetConstantsRecur(const SymbTable_ptr symb_table,
                         node_ptr formula, node_ptr context)
{
  node_ptr key = find_node(CONTEXT, context, formula);
  Set_t result;
  if (formula == Nil) return Set_MakeEmpty();

  result = lookup_consts_hash(key);
  if (result == EMPTY_DEP_SET) return Set_MakeEmpty();
  if (result != (Set_t) NULL) return Set_Copy(result);

  switch (node_get_type(formula)) {
  case CONTEXT:
    result = formulaGetConstantsRecur(symb_table, cdr(formula), car(formula));
    break;

  case BIT:
    /* ignore bits, consider only scalar vars */
    return Set_MakeEmpty();

    /* name cases */
  case DOT:
  case ARRAY:
  case ATOM:
    {
      ResolveSymbol_ptr rs;
      node_ptr name;

      rs = SymbTable_resolve_symbol(symb_table, formula, context);

      name = ResolveSymbol_get_resolved_name(rs);

      if (ResolveSymbol_is_error(rs)) ResolveSymbol_throw_error(rs);

      if (ResolveSymbol_is_constant(rs)) {
        result = Set_MakeSingleton((Set_Element_t) formula);
      }
      else if (ResolveSymbol_is_parameter(rs)) {
        node_ptr param;
        param = SymbTable_get_flatten_actual_parameter(symb_table, name);
        result = formulaGetConstantsRecur(symb_table, param, context);
      }
      /* it should be a defined symbol, running, or a variable */
      else if (ResolveSymbol_is_var(rs)) {
        result = Set_MakeEmpty();
      }
      else if (ResolveSymbol_is_define(rs)) {
        result = formulaGetConstantsRecur(symb_table,
                     SymbTable_get_define_body(symb_table, formula),
                     SymbTable_get_define_context(symb_table, formula));
      }
      else {

        /* only vars are remaining and they can be ignored */
        nusmv_assert(SymbTable_is_symbol_var(symb_table, name) ||
                     SymbTable_is_symbol_variable_array(symb_table, name));

        result = Set_MakeEmpty();
      }
    }

    /* a word var */
  case SIGNED_WORD:
  case UNSIGNED_WORD:
  case FAILURE:
  case TWODOTS: /* Sets */
    return Set_MakeEmpty();

  case TRUEEXP:
  case FALSEEXP:
  case NUMBER:
  case NUMBER_FRAC:
  case NUMBER_REAL:
  case NUMBER_EXP:
  case NUMBER_UNSIGNED_WORD:
  case NUMBER_SIGNED_WORD:
  case UWCONST:
  case SWCONST:
    result = Set_MakeSingleton((Set_Element_t) formula);
    break;


  case ATTIME:   /* unary operation */
  case NEXT:   /* unary operation */
  case NOT:    /* Unary boolean connectives */
  case UMINUS:
  case EX:    /* CTL unary Temporal Operators */
  case SMALLINIT:  /* used for init(expr) */
  case AX:
  case EF:
  case AF:
  case EG:
  case AG:
  case EBF:    /* CTL unary bounded Temporal Operators */
  case ABF:
  case EBG:
  case ABG:
  case EBU:    /* CTL binary bounded Temporal Operators */
  case ABU:
  case OP_NEXT:    /* LTL unary Temporal Operators */
  case OP_PREC:
  case OP_NOTPRECNOT:
  case OP_FUTURE:
  case OP_ONCE:
  case OP_GLOBAL:
  case OP_HISTORICAL:
  case CAST_BOOL: /* Casts */
  case CAST_WORD1:
  case CAST_SIGNED:
  case CAST_UNSIGNED:
  case WSIZEOF: /* We extract possible constant in the body */
  case CAST_TOINT:  /* We extract possible constant in the body */
    result = formulaGetConstantsRecur(symb_table, car(formula), context);
    break;

    /* binary operation */
  case EXTEND:
  case WRESIZE:
  case EQDEF: /* assignment */
  case CONS:
  case UNION:
  case SETIN:
  case COLON:
  case PLUS:    /* Numerical Operations */
  case MINUS:
  case TIMES:
  case DIVIDE:
  case MOD:
  case LSHIFT:  /* Binary shifts and rotates */
  case RSHIFT:
  case LROTATE:
  case RROTATE:
  case CONCATENATION: /* concatenation */
  case EQUAL:   /* Comparison Operations */
  case NOTEQUAL:
  case LT:
  case GT:
  case LE:
  case GE:
  case AND:    /* Binary boolean connectives */
  case OR:
  case XOR:
  case XNOR:
  case IMPLIES:
  case IFF:
  case EU:     /* CTL binary  Temporal Operators */
  case AU:
  case UNTIL:    /* LTL binary Temporal Operators */
  case RELEASES:
  case SINCE:
  case TRIGGERED:
  case MAXU:    /* MIN MAX operators */
  case MINU:
    {
      Set_t right = formulaGetConstantsRecur(symb_table, cdr(formula), context);
      result = formulaGetConstantsRecur(symb_table, car(formula), context);
      result = Set_Union(result, right);
      Set_ReleaseSet(right);
      break;
    }

  case NFUNCTION:
    result = formulaGetConstantsRecur(symb_table, cdr(formula), context);
    break;

    /* 3-arity operations */
  case CASE:
  case IFTHENELSE:
    {
      Set_t condition = formulaGetConstantsRecur(symb_table,
                                                 car(car(formula)), context);
      Set_t then_arg  = formulaGetConstantsRecur(symb_table,
                                                 cdr(car(formula)), context);
      Set_t else_arg  = formulaGetConstantsRecur(symb_table,
                                                 cdr(formula), context);

      result = Set_Union(Set_Union(condition, then_arg), else_arg);
      Set_ReleaseSet(else_arg);
      Set_ReleaseSet(then_arg);
      break;
    }

  case BIT_SELECTION:
    {
      Set_t object   = formulaGetConstantsRecur(symb_table,
                                                car(formula), context);
      Set_t high_bit = formulaGetConstantsRecur(symb_table,
                                                car(cdr(formula)), context);
      Set_t low_bit  = formulaGetConstantsRecur(symb_table,
                                                cdr(cdr(formula)), context);
      result = Set_Union(Set_Union(object, high_bit), low_bit);
      Set_ReleaseSet(low_bit);
      Set_ReleaseSet(high_bit);
      break;
    }

  case COUNT:
    {
      node_ptr list = car(formula);

      result = Set_MakeEmpty();

      while (Nil != list) {
        Set_t tmp = formulaGetConstantsRecur(symb_table, car(list), context);

        result = Set_Union(result, tmp);
        Set_ReleaseSet(tmp);

        list = cdr(list);
      }

      break;
    }

    /* Operations on WORDARRAYs */
  case WAWRITE:
    {
      Set_t memory   = formulaGetConstantsRecur(symb_table,
                                                car(formula), context);
      Set_t location = formulaGetConstantsRecur(symb_table,
                                                car(cdr(formula)), context);
      Set_t address  = formulaGetConstantsRecur(symb_table,
                                                cdr(cdr(formula)), context);
      result = Set_Union(Set_Union(memory, location), address);
      Set_ReleaseSet(address);
      Set_ReleaseSet(location);
      break;
    }

  case WAREAD:
    {
      Set_t memory = formulaGetConstantsRecur(symb_table,
                                              car(formula), context);
      Set_t location = formulaGetConstantsRecur(symb_table,
                                                cdr(formula), context);
      result = Set_Union(memory, location);
      Set_ReleaseSet(location);
      break;
    }



  case PSL_INF:
  case PSL_RANGE:
    result = Set_MakeEmpty();
    break;

  case PSL_WSELECT:
    {
      PslNode_ptr right = psl_node_get_right(formula);

      Set_t object   = formulaGetConstantsRecur(symb_table,
                                                psl_node_get_left(formula),
                                                context);
      Set_t high_bit = formulaGetConstantsRecur(symb_table,
                                                psl_node_get_left(right),
                                                context);
      Set_t low_bit  = formulaGetConstantsRecur(symb_table,
                                                psl_node_get_right(right),
                                                context);

      result = Set_Union(Set_Union(object, high_bit), low_bit);
      Set_ReleaseSet(low_bit);
      Set_ReleaseSet(high_bit);
      break;
    }

  case PSL_SERE:
  case PSL_SERECOMPOUND:
    result = formulaGetConstantsRecur(symb_table, psl_node_get_left(formula),
                                      context);
    break;

  case PSL_SERECONCAT:
  case PSL_SEREFUSION:
  case PSL_CONCATENATION:
    {
      Set_t left = formulaGetConstantsRecur(symb_table,
                                            psl_node_get_left(formula), context);
      Set_t right = formulaGetConstantsRecur(symb_table,
                                             psl_node_get_right(formula), context);
      result = Set_Union(left, right);
      Set_ReleaseSet(right);
      break;
    }

  case PSL_SEREREPEATED:
    result = formulaGetConstantsRecur(symb_table,
                                      psl_node_sere_repeated_get_expr(formula),
                                      context);
    break;

  case PSL_REPLPROP:
    result = formulaGetConstantsRecur(symb_table,
                                      psl_node_repl_prop_get_property(formula),
                                      context);
    break;

  case PSL_PIPEMINUSGT:
  case PSL_PIPEEQGT:
  case PSL_DIAMONDMINUSGT:
    {
      Set_t pre = formulaGetConstantsRecur(symb_table,
                    psl_node_suffix_implication_get_premise(formula),
                                           context);
      Set_t con = formulaGetConstantsRecur(symb_table,
                    psl_node_suffix_implication_get_consequence(formula),
                                           context);
      result = Set_Union(pre, con);
      Set_ReleaseSet(con);
      break;
    }

  case PSL_ALWAYS:
  case PSL_NEVER:
  case PSL_EVENTUALLYBANG:
    result = formulaGetConstantsRecur(symb_table,
                                      psl_node_get_left(formula), context);
    break;

  case PSL_WITHINBANG:
  case PSL_WITHIN:
  case PSL_WITHINBANG_:
  case PSL_WITHIN_:
    {
      Set_t n1 = formulaGetConstantsRecur(symb_table,
                     psl_node_get_left(psl_node_get_left(formula)), context);
      Set_t n2 = formulaGetConstantsRecur(symb_table,
                     psl_node_get_right(psl_node_get_left(formula)), context);
      Set_t n3 = formulaGetConstantsRecur(symb_table,
                     psl_node_get_right(formula), context);

      result = Set_Union(Set_Union(n1, n2), n3);
      Set_ReleaseSet(n3);
      Set_ReleaseSet(n2);
      break;
    }

  case PSL_NEXT_EVENT_ABANG:
  case PSL_NEXT_EVENT_A:
  case PSL_NEXT_EVENT_EBANG:
  case PSL_NEXT_EVENT_E:
  case PSL_NEXT_EVENTBANG:
  case PSL_NEXT_EVENT:
  case PSL_NEXT_ABANG:
  case PSL_NEXT_EBANG:
  case PSL_NEXT_A:
  case PSL_NEXT_E:
  case PSL_NEXTBANG:
  case PSL_NEXT:
  case PSL_X:
  case PSL_XBANG:
    {
      Set_t n1 = formulaGetConstantsRecur(symb_table,
                     psl_node_extended_next_get_expr(formula), context);
      Set_t n2 = formulaGetConstantsRecur(symb_table,
                     psl_node_extended_next_get_when(formula), context);
      Set_t n3 = formulaGetConstantsRecur(symb_table,
                     psl_node_extended_next_get_condition(formula), context);

      result = Set_Union(Set_Union(n1, n2), n3);
      Set_ReleaseSet(n3);
      Set_ReleaseSet(n2);
      break;
    }

  case PSL_BEFOREBANG:
  case PSL_BEFORE:
  case PSL_BEFOREBANG_:
  case PSL_BEFORE_:
  case PSL_UNTILBANG:
  case PSL_UNTIL:
  case PSL_UNTILBANG_:
  case PSL_UNTIL_:
  case PSL_ABORT:
  case PSL_W:
  case PSL_OR:
  case PSL_CARET:
  case PSL_TILDE:
  case PSL_EQEQ:
  case PSL_PIPEPIPE:
  case PSL_AMPERSANDAMPERSAND:
  case PSL_WHILENOTBANG:
  case PSL_WHILENOT:
  case PSL_WHILENOTBANG_:
  case PSL_WHILENOT_:
    {
      Set_t left = formulaGetConstantsRecur(symb_table,
                                            psl_node_get_left(formula),
                                            context);
      Set_t right = formulaGetConstantsRecur(symb_table,
                                             psl_node_get_right(formula),
                                             context);
      result = Set_Union(left, right);
      Set_ReleaseSet(right);
      break;
    }

  case PSL_ITE:
    {
      Set_t condition = formulaGetConstantsRecur(symb_table,
                                                 psl_node_get_ite_cond(formula),
                                                 context);
      Set_t then_arg  = formulaGetConstantsRecur(symb_table,
                                                 psl_node_get_ite_then(formula),
                                                 context);
      Set_t else_arg  = formulaGetConstantsRecur(symb_table,
                                                 psl_node_get_ite_else(formula),
                                                 context);

      result = Set_Union(Set_Union(condition, then_arg), else_arg);
      Set_ReleaseSet(else_arg);
      Set_ReleaseSet(then_arg);
      break;
    }


  default:
    {
      fprintf(nusmv_stderr,
              "\nformulaGetConstantsRecur: Reached undefined connective (%d)\n",
              node_get_type(formula));
      nusmv_exit(1);
    }
  }

  if (!Set_IsEmpty(result)) {
    insert_consts_hash(key, result);
  }

  return result;
}


/**Function********************************************************************

  Synopsis           [Computes the complete cone for a given variable.]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static Set_t
computeCoiVar(SymbTable_ptr st, FlatHierarchy_ptr fh, node_ptr var)
{
  Set_t result;

  result = lookup_coi_hash(var);

  if (NO_DEP_SET != result) return result;

  result = Set_MakeSingleton(var);

  {
    Set_t vars_left = Set_Copy(result);
    Set_Iterator_t iter;
    Set_t new_vars, vdeps;
    boolean nonassign = false;
    node_ptr v;

    while (!Set_IsEmpty(vars_left)) {

      new_vars = Set_MakeEmpty();

      SET_FOREACH(vars_left, iter) {
        v = NODE_PTR(Set_GetMember(vars_left, iter));
        vdeps = _coi_get_var_coi0(st, fh, v, &nonassign, true);
        new_vars = Set_Union(new_vars, vdeps);
        Set_ReleaseSet(vdeps);
      }
      /* Put in vars_left the new fresh variables */
      Set_ReleaseSet(vars_left);
      vars_left = Set_Copy(new_vars);
      vars_left = Set_Difference(vars_left, result);
      result = Set_Union(result, new_vars);
      Set_ReleaseSet(new_vars);
    }

    Set_ReleaseSet(vars_left);
  }

  insert_coi_hash(var, result);

  return result;
}


/**Function********************************************************************

  Synopsis           [Given a variable it returns the cone at depth 0.]

  Description        [Given a variable it returns the cone at depth
  0. If use_cache is true, then the result is memoized on the
  cache. When use_cache is true, it is assumed the hierarchy to be the
  mainFlatHierarchy. An assertion enforces this condition.]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/

static Set_t _coi_get_var_coi0(SymbTable_ptr st,
                               FlatHierarchy_ptr hierarchy,
                               node_ptr var,
                               boolean * nonassign,
                               boolean use_cache)
{
  Set_t result = NO_DEP_SET;

  if (use_cache) {
    nusmv_assert((hierarchy == mainFlatHierarchy));
    result = lookup_coi0_hash(var);
  }

  if (NO_DEP_SET != result) return Set_Copy(result);

  {
    node_ptr invar_var = var;
    node_ptr init_var = find_node(SMALLINIT, var, Nil);
    node_ptr next_var = find_node(NEXT, var, Nil);

    node_ptr invar_a= FlatHierarchy_lookup_assign(hierarchy, invar_var);
    node_ptr invar_c= FlatHierarchy_lookup_constrains(hierarchy, invar_var);
    node_ptr init_a = FlatHierarchy_lookup_assign(hierarchy, init_var);
    node_ptr init_c = FlatHierarchy_lookup_constrains(hierarchy, init_var);
    node_ptr next_a = FlatHierarchy_lookup_assign(hierarchy, next_var);
    node_ptr next_c = FlatHierarchy_lookup_constrains(hierarchy, next_var);

    result = Set_MakeEmpty();

    /* Process normal assignments and INIT constraints */
    if (Nil != invar_a) {
      Set_t deps = Formula_GetDependencies(st, invar_a, Nil);

      result = Set_Union(result, deps);
      Set_ReleaseSet(deps);
    }
    if (Nil != invar_c) {
      Set_t deps = Formula_GetDependencies(st, invar_c, Nil);

      *nonassign = true;
      result = Set_Union(result, deps);
      Set_ReleaseSet(deps);
    }

    /* Process init assignments and INIT constraints */
    if (Nil != init_a) {
      Set_t deps = Formula_GetDependencies(st, init_a, Nil);
      result = Set_Union(result, deps);
      Set_ReleaseSet(deps);
    }
    if (Nil != init_c) {
      Set_t deps = Formula_GetDependencies(st, init_c, Nil);

      *nonassign = true;
      result = Set_Union(result, deps);
      Set_ReleaseSet(deps);
    }

    /* Process next assignments and TRANS constraints */
    if (Nil != next_a) {
      Set_t deps = Formula_GetDependencies(st, next_a, Nil);
      result = Set_Union(result, deps);
      Set_ReleaseSet(deps);
    }
    if (Nil != next_c) {
      Set_t deps = Formula_GetDependencies(st, next_c, Nil);

      *nonassign = true;
      result = Set_Union(result, deps);
      Set_ReleaseSet(deps);
    }
  }

  if (use_cache) {
    insert_coi0_hash(var, result);
  }

  return result;
}
