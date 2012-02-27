/**CFile***********************************************************************

  FileName    [dd.c]

  PackageName [dd]

  Synopsis [NuSMV interface to the Decision Diagram Package of the
  University of Colorado.]

  Description [This file implements the interface between the NuSMV
  system and the California University Decision Diagram (henceforth
  referred as CUDD). The CUDD package is a generic implementation of a
  decision diagram data structure. For the time being, only Boole
  expansion is implemented and the leaves in the in the nodes can be
  the constants zero, one or any arbitrary value. A coding standard
  has been defined. I.e all the functions acting on BDD and ADD have
  \"bdd\" and \"add\" respectively as prefix.
  <p><br>
  The BDD or ADD returned as a result of an operation are always
  referenced (see the CUDD User Manual for more details about this),
  and need to be dereferenced when the result is no more necessary to
  computation, in order to release the memory associated to it when
  garbage collection occurs.
  All the functions takes as first argument the decision diagram
  manager (henceforth referred as DdManager).]

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

******************************************************************************/

#include "ddInt.h"
#include "enc/operators.h"
#include "parser/symbols.h" /* for FAILURE value */

static char rcsid[] UTIL_UNUSED = "$Id: dd.c,v 1.7.6.12.2.1.2.6.4.20 2010-02-08 12:25:27 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
node_ptr one_number = Nil;
node_ptr zero_number = Nil;
node_ptr true_const = Nil;
node_ptr false_const = Nil;
node_ptr boolean_range = Nil;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define common_error(variable, message)         \
  if ((variable) == NULL) {                     \
    rpterr("%s", message);                    \
    nusmv_exit(1);                              \
  }

#define common_error2(dd, variable, variable2, message) \
  if ((variable) == NULL) {                             \
    rpterr("%s", message);                            \
    Cudd_RecursiveDeref((dd),(variable2));              \
    nusmv_exit(1);                                      \
  }


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void InvalidType(FILE *file, char *field, char *expected);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Creates a new DD manager.]

  Description [Creates a new DD manager, initializes the table, the
  basic constants and the projection functions.<br>
  \"maxMemory\" (the last parameter of the function \"Cudd_Init\")
  is set to 0. In such a way \"Cudd_Init\" decides suitables values
  for the maximum size of the cache and for the limit for fast unique
  table growth based on the available memory. Returns a pointer to the
  manager if successful; else abort depending the mode (interactive or
  batch) the system is used.]

  SideEffects        []

  SeeAlso            [quit_dd_package]

******************************************************************************/
DdManager* init_dd_package()
{
  DdManager* dd;

  one_number = find_node(NUMBER, NODE_FROM_INT(1), Nil);
  zero_number = find_node(NUMBER, NODE_FROM_INT(0), Nil);

  true_const = Expr_true();
  false_const = Expr_false();

  boolean_range = cons(false_const, cons(true_const, Nil));
  dd = Cudd_Init(0, 0, UNIQUE_SLOTS, CACHE_SLOTS, 0,
                 zero_number, one_number, false_const, true_const);
  common_error(dd, "init_dd_package: Unable to initialize the manager.");
  return(dd);
}

/**Function********************************************************************

  Synopsis           [Deletes resources associated with a DD manager.]

  Description        [Deletes resources associated with a DD manager and
  resets the global statistical counters. (Otherwise, another manager
  subsequently created would inherit the stats of this one.)]

  SideEffects        []

  SeeAlso            [init_dd_package]

******************************************************************************/
void quit_dd_package(DdManager * dd)
{
  Cudd_Quit(dd);
}

/**Function********************************************************************

  Synopsis [Checks the unique table for nodes with non-zero reference
  counts.]

  Description [Checks the unique table for nodes with non-zero
  reference counts. It is normally called before dd_quit to make sure
  that there are no memory leaks due to missing add/bdd_free's.
  Takes into account that reference counts may saturate and that the
  basic constants and the projection functions are referenced by the
  manager.  Returns the number of nodes with non-zero reference count.
  (Except for the cases mentioned above.)]

  SideEffects []

******************************************************************************/
int dd_checkzeroref(DdManager * dd)
{
  return Cudd_CheckZeroRef(dd);
} /* end of dd_checkzeroref */

/**Function********************************************************************

  Synopsis           [Returns the number of nodes in the unique table.]

  Description        [Returns the total number of nodes currently in the unique
  table, including the dead nodes.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int get_dd_nodes_allocated(DdManager * dd){
  return(Cudd_ReadKeys(dd));
}

/**Function********************************************************************

  Synopsis [Applies function <code>f</code> to the list of BDD/ADD <code>l</code>.]

  Description [This function acts like the Lisp <tt>mapcar</tt>. It returns
  the list of the result of the application of function \code>f</code> to each
  element of list <code>l</code>.]

  SideEffects        []

  SeeAlso            [map walk walk_dd]

******************************************************************************/
node_ptr map_dd(DdManager * dd, NPFDD f, node_ptr l)
{
  node_ptr t;

  if (l == Nil) return(Nil);
  t = (*f)(dd, (DdNode *)car(l));
  return(cons(t,map_dd(dd, f, cdr(l))));
}

/**Function********************************************************************

  Synopsis [Applies function <code>f</code> to the list of BDD/ADD <code>l</code>.]

  Description [This function acts like the <tt>map_dd</dd>. This functions
  applies the function <code>f</code> to each element of list
  <code>l</code>. Nothing is returned, performs side-effects on the elements.]

  SideEffects        []

  SeeAlso            [map walk map_dd]

******************************************************************************/
void walk_dd(DdManager * dd, VPFDD f, node_ptr l)
{
  if (l == Nil) return;
  (*f)(dd, (DdNode *)car(l));
  walk_dd(dd, f, cdr(l));
}

/**Function********************************************************************

  Synopsis    [Prints out statistic and setting of the DD manager.]

  Description [Prints out statistics and settings for a CUDD manager.]

  SideEffects []

******************************************************************************/
void dd_print_stats(DdManager *mgr, FILE *file)
{
  Cudd_PrintInfo(mgr, file);

  /* Print some guidance to the parameters */
  fprintf(file, "\nMore detailed information about the semantics ");
  fprintf(file, "and values of these parameters\n");
  fprintf(file, "can be found in the documentation about the CU ");
  fprintf(file, "Decision Diagram Package.\n");

  return;
} /* end of dd_print_stats */

/**Function********************************************************************

  Synopsis    [Builds a group of variables that should stay adjacent
  during reordering.]

  Description [Builds a group of variables that should stay adjacent
  during reordering. The group is made up of n variables. The first
  variable in the group is f. The other variables are the n-1
  variables following f in the order at the time of invocation of this
  function. Returns a handle to the variable group if successful else fail.]

  SideEffects [Modifies the variable tree.]

******************************************************************************/
dd_block * dd_new_var_block(DdManager * dd, int start_index, int offset)
{
  MtrNode *group;

  if (start_index == MAX_VAR_INDEX) return(NULL);
  /*
    We use MTR_FIXED because we want the internal order of variablesa
    in the groups to be preserved (MTR_FIXED does not preserve them.).
  */
  group = Cudd_MakeTreeNode(dd, start_index, offset, MTR_FIXED);
  common_error(group, "dd_new_var_block: group = NULL");

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
    fprintf(nusmv_stderr, "dd_new_var_block: low=%d, idx=%d, size=%d\n",
            group->low, group->index, group->size);
  }

  return((dd_block *) group);
} /* end of dd_new_var_block */


/**Function********************************************************************

  Synopsis    [Dissolves a group previously created by dd_new_var_block]

  Description [Dissolves a group previously created by
  dd_new_var_block.  Returns 0 if the group was actually removed, 1
  otherwise (that may be not due to an error)]

  SideEffects [Modifies the variable tree.]

******************************************************************************/
int dd_free_var_block(DdManager* dd, dd_block* group)
{
  int res;

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 5)) {
    fprintf(nusmv_stderr, "dd_free_var_block: low=%d, idx=%d, size=%d\n",
            group->low, group->index, group->size);
  }

  if (Mtr_DissolveGroup((MtrNode*) group) == (MtrNode*) NULL) res = 1;
  else res = 0;

  return res;
}


/**Function********************************************************************

  Synopsis    [Returns the index of the variable currently in the i-th
  position of the order.]

  Description [Returns the index of the variable currently in the i-th
  position of the order. If the index is MAX_VAR_INDEX, returns
  MAX_VAR_INDEX; otherwise, if the index is out of bounds fails.]

  SideEffects []

******************************************************************************/
int dd_get_index_at_level(DdManager *dd, int level)
{
  int result;

  result = Cudd_ReadInvPerm(dd, level);
  if (result < 0) {
    rpterr("dd_get_index_at_level: level %d out of bound.", level);
    nusmv_exit(1);
  }
  return(result);
} /* end of dd_get_index_at_level */


/**Function********************************************************************

  Synopsis    [Returns the current position of the i-th variable in the
  order.]

  Description [Returns the current position of the i-th variable in the
  order. If the index is CUDD_MAXINDEX, returns CUDD_MAXINDEX; otherwise,
  if the index is out of bounds returns -1.]

  SideEffects [None]

  SeeAlso     [Cudd_ReadInvPerm Cudd_ReadPermZdd]

******************************************************************************/
int dd_get_level_at_index(DdManager *dd, int index)
{
  int result;

  result = Cudd_ReadPerm(dd, index);
  return result;
}


/**Function********************************************************************

  Synopsis    [Returns the number of BDD variables in existance.]

  Description [Returns the number of BDD variables in existance.]

  SideEffects []

******************************************************************************/
int dd_get_size(DdManager *dd)
{
  return Cudd_ReadSize(dd);
} /* end of dd_get_size */

/**Function********************************************************************

  Synopsis    [Reorders variables according to given permutation.]

  Description [Reorders variables according to given permutation.
  The i-th entry of the permutation array contains the index of the variable
  that should be brought to the i-th level.  The size of the array should be
  equal or greater to the number of variables currently in use.
  Returns 1 in case of success; 0 otherwise.]

  SideEffects [Changes the variable order for all diagrams and clears
  the cache.]

  SeeAlso []

******************************************************************************/
int dd_set_order(DdManager* dd, int* permutation)
{
  return Cudd_ShuffleHeap(dd, permutation);
} /* end of dd_set_order */

/**Function********************************************************************

  Synopsis    [Enables automatic dynamic reordering of BDDs and ADDs.]

  Description [Enables automatic dynamic reordering of BDDs and
  ADDs. Parameter method is used to determine the method used for
  reordering. If REORDER_SAME is passed, the method is
  unchanged.]

  SideEffects []

  SeeAlso     [dd_autodyn_disable dd_reordering_status]

******************************************************************************/
void dd_autodyn_enable(DdManager * dd, dd_reorderingtype method)
{
  Cudd_AutodynEnable(dd, method);
} /* end of dd_autodyn_enable */

/**Function********************************************************************

  Synopsis    [Disables automatic dynamic reordering of BDD and ADD.]

  Description [Disables automatic dynamic reordering of BDD and ADD.]

  SideEffects []

  SeeAlso     [dd_autodyn_enable dd_reordering_status]

******************************************************************************/
void dd_autodyn_disable(DdManager *dd)
{
  Cudd_AutodynDisable(dd);
} /* end of dd_autodyn_disable */

/**Function********************************************************************

  Synopsis    [Reports the status of automatic dynamic reordering of BDDs
  and ADDs.]

  Description [Reports the status of automatic dynamic reordering of
  BDDs and ADDs. Parameter method is set to the reordering method
  currently selected. Returns 1 if automatic reordering is enabled; 0
  otherwise.]

  SideEffects [Parameter method is set to the reordering method currently
  selected.]

  SeeAlso     [dd_autodyn_disable dd_autodyn_enable]

******************************************************************************/
int dd_reordering_status(DdManager *dd, dd_reorderingtype * method)
{
  return(Cudd_ReorderingStatus(dd, method));
} /* end of dd_reordering_status */

/**Function********************************************************************

  Synopsis    [Main dynamic reordering routine.]

  Description [Main dynamic reordering routine.
  Calls one of the possible reordering procedures:
  <ul>
  <li>Swapping
  <li>Sifting
  <li>Symmetric Sifting
  <li>Group Sifting
  <li>Window Permutation
  <li>Simulated Annealing
  <li>Genetic Algorithm
  <li>Dynamic Programming (exact)
  </ul>

  For sifting, symmetric sifting, group sifting, and window
  permutation it is possible to request reordering to convergence.<p>

  Returns 1 in case of success; 0 otherwise. In the case of symmetric
  sifting (with and without convergence) returns 1 plus the number of
  symmetric variables, in case of success.<p>

  This functions takes as arguments:
  <ul>
  <li> <tt>dd</tt> the DD manager;
  <li> <tt>heuristics</tt> method used for reordering;
  <li> <tt>minsize</tt> bound below which no reordering occurs;
  </ul>
  ]

  SeeAlso     [Cudd_ReduceHeap]

  SideEffects [Changes the variable order for all diagrams and clears
  the cache.]

******************************************************************************/
int dd_reorder(DdManager *dd, int method, int minsize)
{
  int result;

  result = Cudd_ReduceHeap(dd, method, minsize);
  if (result == 0) {
    rpterr("dd_reorder: reordering of ADD/BDD fails.");
    nusmv_exit(1);
  }
  return(result);
} /* end of dd_reorder */


/**Function********************************************************************

  Synopsis    [Returns the number of times reordering has occurred.]

  Description [Returns the number of times reordering has occurred in
  the manager. The number includes both the calls to Cudd_ReduceHeap
  from the application program and those automatically performed by
  the package. However, calls that do not even initiate reordering are
  not counted. A call may not initiate reordering if there are fewer
  than minsize live nodes in the manager, or if CUDD_REORDER_NONE is
  specified as reordering method. The calls to Cudd_ShuffleHeap are
  not counted.]

  SeeAlso     []

  SideEffects []

******************************************************************************/
int dd_get_reorderings(DdManager* dd)
{
  return Cudd_ReadReorderings(dd);
}


/**Function********************************************************************

  Synopsis [Gets the internal reordering method used.]

  Description [Returns the internal reordering method used.]

  SideEffects []

******************************************************************************/
dd_reorderingtype dd_get_ordering_method (DdManager * dd)
{
  dd_reorderingtype method;

  (void) Cudd_ReorderingStatus(dd, &method);
  return(method);
}

/**Function********************************************************************

  Synopsis    [Converts a string to a dynamic ordering method type.]

  Description [Converts a string to a dynamic ordering method type. If string
  is not "sift" or "window", then returns REORDER_.]

  SideEffects []

******************************************************************************/
int StringConvertToDynOrderType(char *string)
{

  if (strcmp("random", string) == 0) {
    return REORDER_RANDOM;
  }
  else if (strcmp("random_pivot", string) == 0) {
    return  REORDER_RANDOM_PIVOT;
  }
  else if (strcmp("sift", string) == 0) {
    return REORDER_SIFT;
  }
  else if (strcmp("sift_converge", string) == 0) {
    return  REORDER_SIFT_CONV;
  }
  else if (strcmp("symmetry_sift", string) == 0) {
    return  REORDER_SYMM_SIFT;
  }
  else if (strcmp("symmetry_sift_converge", string) == 0) {
    return  REORDER_SYMM_SIFT_CONV;
  }
  else if (strcmp("window2", string) == 0) {
    return REORDER_WINDOW2;
  }
  else if (strcmp("window3", string) == 0) {
    return  REORDER_WINDOW3;
  }
  else if (strcmp("window4", string) == 0) {
    return  REORDER_WINDOW4;
  }
  else if (strcmp("window2_converge", string) == 0) {
    return  REORDER_WINDOW2_CONV;
  }
  else if (strcmp("window3_converge", string) == 0) {
    return  REORDER_WINDOW3_CONV;
  }
  else if (strcmp("window4_converge", string) == 0) {
    return  REORDER_WINDOW4_CONV;
  }
  else if (strcmp("group_sift", string) == 0) {
    return  REORDER_GROUP_SIFT;
  }
  else if (strcmp("group_sift_converge", string) == 0) {
    return  REORDER_GROUP_SIFT_CONV;
  }
  else if (strcmp("annealing", string) == 0) {
    return  REORDER_ANNEALING;
  }
  else if (strcmp("genetic", string) == 0) {
    return  REORDER_GENETIC;
  }
  else if (strcmp("exact", string) == 0) {
    return  REORDER_EXACT;
  }
  else if (strcmp("linear", string) == 0) {
    return  REORDER_LINEAR;
  }
  else if (strcmp("linear_converge", string) == 0) {
    return  REORDER_LINEAR_CONV;
  }
  else if (strcmp("same", string) == 0) {
    return REORDER_SAME;
  }
  else {
    return REORDER_NONE;
  }
}

/**Function********************************************************************

  Synopsis    [Converts a dynamic ordering method type to a string.]

  Description [Converts a dynamic ordering method type to a string.  This
  string must NOT be freed by the caller.]

  SideEffects []

******************************************************************************/
char * DynOrderTypeConvertToString(int method)
{
  if (method == REORDER_NONE) {
    return "";
  }
  else if (method == REORDER_RANDOM) {
    return "random";
  }
  else if (method == REORDER_RANDOM_PIVOT) {
    return "random_pivot";
  }
  else if ((method == REORDER_SIFT)) {
    return "sift";
  }
  else if (method == REORDER_SIFT_CONV) {
    return "sift_converge";
  }
  else if (method == REORDER_SYMM_SIFT) {
    return "symmetry_sift";
  }
  else if (method == REORDER_SYMM_SIFT_CONV) {
    return "symmetry_sift_converge";
  }
  else if (method == REORDER_WINDOW2) {
    return "window2";
  }
  else if (method == REORDER_WINDOW3) {
    return "window3";
  }
  else if (method == REORDER_WINDOW4) {
    return "window4";
  }
  else if (method == REORDER_WINDOW2_CONV) {
    return "window2_converge";
  }
  else if (method == REORDER_WINDOW3_CONV) {
    return "window3_converge";
  }
  else if (method == REORDER_WINDOW4_CONV) {
    return "window4_converge";
  }
  else if (method == REORDER_GROUP_SIFT) {
    return "group_sift";
  }
  else if (method == REORDER_GROUP_SIFT_CONV) {
    return "group_sift_converge";
  }
  else if (method == REORDER_ANNEALING) {
    return "annealing";
  }
  else if (method == REORDER_GENETIC) {
    return "genetic";
  }
  else if (method == REORDER_EXACT) {
    return "exact";
  }
  else if (method == REORDER_LINEAR) {
    return "linear";
  }
  else if (method == REORDER_LINEAR_CONV) {
    return "linear_converge";
  }
  else if (method == REORDER_SAME) {
    return "same";
  }
  else {
    fail("unrecognized method");
  }
}


/**Function********************************************************************

  Synopsis [Sets the internal parameters of the package to the given values.]

  Description [The CUDD package has a set of parameters that can be assigned
  different values. This function receives a table which maps strings to
  values and sets the parameters represented by the strings to the pertinent
  values. Some basic type checking is done. It returns 1 if everything is
  correct and 0 otherwise.]

  SideEffects []

******************************************************************************/
int dd_set_parameters(DdManager *mgr, OptsHandler_ptr opt, FILE *file)
{
  int reorderMethod;
  dd_reorderingtype zddReorderMethod;
  st_table *newValueTable;
  st_generator *stgen;
  char *paramName;
  char *paramValue;

  /* Initial value of the variables. */
  reorderMethod = REORDER_SAME;
  zddReorderMethod = REORDER_SAME;

  /* Build a new table with the parameter names but with
  ** the prefix removed. */
  newValueTable = st_init_table(st_ptrcmp, st_ptrhash);
  OPTS_FOREACH_OPTION(opt, (char **)&paramName, (char **)&paramValue) {
    if (strncmp(paramName, "BDD.", 4) == 0) {
      st_insert(newValueTable, (char *)&paramName[4],
                (char *)paramValue);
    }
  }

  st_foreach_item(newValueTable, stgen, (char **)&paramName, (char **)&paramValue) {
    unsigned int uvalue;
    char *invalidChar;

    invalidChar = (char *)NULL;
    if (strcmp(paramName, "Hard limit for cache size") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Hard limit for cache size", "unsigned integer");
      }
      else {
        Cudd_SetMaxCacheHard(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "Cache hit threshold for resizing") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
                InvalidType(file, "Cache hit threshold for resizing",
                            "unsigned integer");
      }
      else {
        Cudd_SetMinHit(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "Garbage collection enabled") == 0) {
      if (strcmp(paramValue, "yes") == 0) {
        Cudd_EnableGarbageCollection(mgr);
      }
      else if (strcmp(paramValue, "no") == 0) {
        Cudd_DisableGarbageCollection(mgr);
      }
      else {
        InvalidType(file, "Garbage collection enabled", "(yes,no)");
      }
    }
    else if (strcmp(paramName, "Limit for fast unique table growth") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Limit for fast unique table growth", "unsigned integer");
      }
      else {
        Cudd_SetLooseUpTo(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "Maximum number of variables sifted per reordering")
             == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Maximum number of variables sifted per reordering",
                    "unsigned integer");
      }
      else {
        Cudd_SetSiftMaxVar(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "Maximum number of variable swaps per reordering")
             == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Maximum number of variable swaps per reordering",
                    "unsigned integer");
      }
      else {
        Cudd_SetSiftMaxSwap(mgr, uvalue);
      }
    }
    else if (strcmp(paramName,
                    "Maximum growth while sifting a variable") == 0) {
      double value;

      value = strtod(paramValue, &invalidChar);
      if (*invalidChar) {
        InvalidType(file, "Maximum growth while sifting a variable", "real");
      }
      else {
        Cudd_SetMaxGrowth(mgr, value);
      }
    }
    else if (strcmp(paramName, "Dynamic reordering of BDDs enabled") == 0) {
      if (strcmp(paramValue, "yes") == 0) {
        Cudd_AutodynEnable(mgr, reorderMethod);
      }
      else if (strcmp(paramValue, "no") == 0) {
        Cudd_AutodynDisable(mgr);
      }
      else {
        InvalidType(file, "Dynamic reordering of BDDs enabled", "(yes,no)");
      }
    }
    else if (strcmp(paramName, "Default BDD reordering method") == 0) {
      dd_reorderingtype reorderInt;

      reorderMethod = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || reorderMethod < 0) {
        InvalidType(file, "Default BDD reordering method", "integer");
      }
      else {
        if (Cudd_ReorderingStatus(mgr, &reorderInt)) {
          Cudd_AutodynEnable(mgr, reorderMethod);
        }
      }
    }
    else if (strcmp(paramName, "Dynamic reordering of ZDDs enabled") == 0) {
      if (strcmp(paramValue, "yes") == 0) {
        Cudd_AutodynEnableZdd(mgr, zddReorderMethod);
      }
      else if (strcmp(paramValue, "no") == 0) {
        Cudd_AutodynDisableZdd(mgr);
      }
      else {
        InvalidType(file, "Dynamic reordering of ZDDs enabled", "(yes,no)");
      }
    }
    else if (strcmp(paramName, "Default ZDD reordering method") == 0) {
      dd_reorderingtype reorderInt;

      zddReorderMethod = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || zddReorderMethod < 0) {
        InvalidType(file, "Default ZDD reordering method", "integer");
      }
      else {
        if (Cudd_ReorderingStatusZdd(mgr, &reorderInt)) {
          Cudd_AutodynEnableZdd(mgr, zddReorderMethod);
        }
      }
    }
    else if (strcmp(paramName, "Realignment of ZDDs to BDDs enabled") == 0) {
      if (strcmp(paramValue, "yes") == 0) {
        Cudd_zddRealignEnable(mgr);
      }
      else if (strcmp(paramValue, "no") == 0) {
        Cudd_zddRealignDisable(mgr);
      }
      else {
        InvalidType(file, "Realignment of ZDDs to BDDs enabled", "(yes,no)");
      }
    }
    else if (strcmp(paramName, "Realignment of BDDs to ZDDs enabled") == 0) {
      if (strcmp(paramValue, "yes") == 0) {
        Cudd_bddRealignEnable(mgr);
      }
      else if (strcmp(paramValue, "no") == 0) {
        Cudd_bddRealignDisable(mgr);
      }
      else {
        InvalidType(file, "Realignment of BDDs to ZDDs enabled", "(yes,no)");
      }
    }
    else if (strcmp(paramName, "Dead nodes counted in triggering reordering") == 0) {
      if (strcmp(paramValue, "yes") == 0) {
        Cudd_TurnOnCountDead(mgr);
      }
      else if (strcmp(paramValue, "no") == 0) {
        Cudd_TurnOffCountDead(mgr);
      }
      else {
        InvalidType(file, "Dead nodes counted in triggering reordering", "(yes,no)");
      }
    }
    else if (strcmp(paramName, "Group checking criterion") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Group checking criterion", "integer");
      }
      else {
        Cudd_SetGroupcheck(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "Recombination threshold") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Recombination threshold", "integer");
      }
      else {
        Cudd_SetRecomb(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "Symmetry violation threshold") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Symmetry violation threshold", "integer");
      }
      else {
        Cudd_SetSymmviolation(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "Arc violation threshold") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Arc violation threshold", "integer");
      }
      else {
        Cudd_SetArcviolation(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "GA population size") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar  || uvalue < 0) {
        InvalidType(file, "GA population size", "integer");
      }
      else {
        Cudd_SetPopulationSize(mgr, uvalue);
      }
    }
    else if (strcmp(paramName, "Number of crossovers for GA") == 0) {
      uvalue = (unsigned int) strtol(paramValue, &invalidChar, 10);
      if (*invalidChar || uvalue < 0) {
        InvalidType(file, "Number of crossovers for GA", "integer");
      }
      else {
        Cudd_SetNumberXovers(mgr, uvalue);
      }
    }
    else {
      fprintf(file, "Warning: Parameter %s not recognized.", paramName);
      fprintf(file, " Ignored.\n");
    }
  } /* end of st_foreach_item */

  /* Clean up. */
  st_free_table(newValueTable);

  return(1);

} /* end of dd_set_parameters */



/**Function********************************************************************

  Synopsis    [Prints a disjoint sum of products.]

  Description [Prints a disjoint sum of product cover for the function
  rooted at node. Each product corresponds to a path from node a leaf
  node different from the logical zero, and different from the
  background value. Uses the standard output.  Returns 1 if successful;
  0 otherwise.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int dd_printminterm(
  DdManager * manager,
  dd_ptr node)
{
  return(Cudd_PrintMinterm(manager, node));
}

/**Function********************************************************************

  Synopsis           [Writes a dot file representing the argument DDs.]

  Description        [Writes a file representing the argument DDs in a format
  suitable for the graph drawing program dot.

  It returns 1 in case of success; 0 otherwise (e.g., out-of-memory,
  file system full).

  Cudd_DumpDot does not close the file: This is the caller
  responsibility. Cudd_DumpDot uses a minimal unique subset of the
  hexadecimal address of a node as name for it.

  If the argument inames is non-null, it is assumed to hold the pointers
  to the names of the inputs. Similarly for onames.
  Cudd_DumpDot uses the following convention to draw arcs:
    <ul>
    <li> solid line: THEN arcs;
    <li> dotted line: complement arcs;
    <li> dashed line: regular ELSE arcs.
    </ul>

  The dot options are chosen so that the drawing fits on a letter-size
  sheet.
  ]

  SideEffects        []

  SeeAlso            [dd_dump_davinci]
******************************************************************************/
int dd_dump_dot(
  DdManager * dd /* manager */,
  int  n /* number of output nodes to be dumped */,
  dd_ptr * f /* array of output nodes to be dumped */,
  const char ** inames /* array of input names (or NULL) */,
  const char ** onames /* array of output names (or NULL) */,
  FILE * fp /* pointer to the dump file */)
{
  return(Cudd_DumpDot(dd, n, (DdNode **)f, 
		      (char**) inames, (char**) onames, fp));
}

/**Function********************************************************************

  Synopsis           [Writes a daVnci file representing the argument DDs.]

  Description        [Writes a daVnci file representing the argument
  DDs. For a better description see the \"Cudd_DumpDaVinci\" documentation
  in the CUDD package.]

  SideEffects        []

  SeeAlso            [dd_dump_davinci]
******************************************************************************/
int dd_dump_davinci(
  DdManager * dd /* manager */,
  int  n /* number of output nodes to be dumped */,
  dd_ptr * f /* array of output nodes to be dumped */,
  const char ** inames /* array of input names (or NULL) */,
  const char ** onames /* array of output names (or NULL) */,
  FILE * fp /* pointer to the dump file */)
{
  return(Cudd_DumpDaVinci(dd, n, (DdNode **)f, 
			  (char**) inames, (char**) onames, fp));
}

/**Function********************************************************************

  Synopsis           [Reads the constant TRUE ADD of the manager.]

  Description        [Reads the constant TRUE ADD of the manager.]

  SideEffects        []

  SeeAlso            [add_false]
******************************************************************************/
add_ptr add_true(DdManager * dd)
{
  DdNode * result = Cudd_ReadTrue(dd);

  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis     [Returns the then child of an internal node.]

  Description  [Returns the then child of an internal node. If
  <code>f</code> is a constant node, the result is
  unpredictable. Notice that the reference count of the returned node
  is not incremented.]

  SideEffects  [none]

  SeeAlso      [add_else]

******************************************************************************/
add_ptr add_then(DdManager *dd, add_ptr f)
{
  return((add_ptr)Cudd_T(f));
}

/**Function********************************************************************

  Synopsis    [Returns the index of the node.]

  Description [Returns the index of the node.]

  SideEffects [None]

  SeeAlso []

******************************************************************************/
int add_index(DdManager *dd, add_ptr f)
{
  return(Cudd_NodeReadIndex((DdNode *)f));
}


/**Function********************************************************************

  Synopsis     [Returns the else child of an internal node.]

  Description  [Returns the else child of an internal node. If
  <code>f</code> is a constant node, the result is
  unpredictable. Notice that the reference count of the returned node
  is not incremented.]

  SideEffects  [none]

  SeeAlso      [add_else]

******************************************************************************/
add_ptr add_else(DdManager *dd, add_ptr f)
{
  return((add_ptr)Cudd_E(f));
}

/**Function********************************************************************

  Synopsis           [Reads the constant FALSE ADD of the manager.]

  Description        [Reads the constant FALSE ADD of the manager.]

  SideEffects        []

  SeeAlso            [add_true]
******************************************************************************/
add_ptr add_false(DdManager * dd)
{
  DdNode * result = Cudd_ReadFalse(dd);

  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis           [Check if the ADD is true.]

  Description        [Check if the ADD is true.]

  SideEffects        []

  SeeAlso            [add_true]

******************************************************************************/
int add_is_true(DdManager * dd, add_ptr f)
{
  return((DdNode *)f == Cudd_ReadTrue(dd));
}

/**Function********************************************************************

  Synopsis           [Check if the ADD is false.]

  Description        [Check if the ADD is false.]

  SideEffects        []

  SeeAlso            [add_false]

******************************************************************************/
int add_is_false(DdManager * dd, add_ptr f)
{
  return((DdNode *)f == Cudd_ReadFalse(dd));
}

/**Function********************************************************************

  Synopsis           [Reads the constant one ADD of the manager.]

  Description        [Reads the constant one ADD of the manager.]

  SideEffects        []

  SeeAlso            [add_false]
******************************************************************************/
add_ptr add_one(DdManager * dd)
{
  DdNode * result = Cudd_ReadOne(dd);

  Cudd_Ref(result);
  return((add_ptr)result);
}


/**Function********************************************************************

  Synopsis           [Reads the constant zero ADD of the manager.]

  Description        [Reads the constant zero ADD of the manager.]

  SideEffects        []

  SeeAlso            [add_true]
******************************************************************************/
add_ptr add_zero(DdManager * dd)
{
  DdNode * result = Cudd_ReadZero(dd);

  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis           [Check if the ADD is one.]

  Description        [Check if the ADD is one.]

  SideEffects        []

  SeeAlso            [add_true]

******************************************************************************/
int add_is_one(DdManager * dd, add_ptr f)
{
  return((DdNode *)f == Cudd_ReadOne(dd));
}

/**Function********************************************************************

  Synopsis           [Check if the ADD is zero.]

  Description        [Check if the ADD is zero.]

  SideEffects        []

  SeeAlso            [add_false]

******************************************************************************/
int add_is_zero(DdManager * dd, add_ptr f)
{
  return((DdNode *)f == Cudd_ReadZero(dd));
}

/**Function********************************************************************

  Synopsis           [Reference an ADD node.]

  Description        [Reference an ADD node.]

  SideEffects        [The reference count of the node is incremented by one.]

  SeeAlso            [add_deref add_free]
******************************************************************************/
void add_ref(add_ptr fn)
{
  Cudd_Ref(fn);
}

/**Function********************************************************************

  Synopsis           [Dereference an ADD node.]

  Description        [Dereference an ADD node.]

  SideEffects        [The reference count of the node is decremented by one.]

  SeeAlso            [add_ref add_free]
******************************************************************************/
void add_deref(add_ptr fn)
{
  Cudd_Deref(fn);
}

/**Function********************************************************************

  Synopsis           [Dereference an ADD node. If it dies, recursively decreases
  the reference count of its children.]

  Description        [Decreases the reference count of node. If the node dies,
  recursively decreases the reference counts of its children. It is used to
  dispose off an ADD that is no longer needed.]

  SideEffects        [The reference count of the node is decremented by one,
  and if the node dies a recursive dereferencing is applied to its children.]

  SeeAlso            []
******************************************************************************/
void add_free(DdManager * dd, add_ptr dd_node)
{
  common_error(dd_node, "add_free: dd_node = NULL");
  Cudd_RecursiveDeref(dd, (DdNode *)dd_node);
}

/**Function********************************************************************

  Synopsis           [Creates a copy of an ADD node.]

  Description        [Creates a copy of an ADD node.]

  SideEffects        [The reference count is increased by one unit.]

  SeeAlso            [add_ref add_free add_deref]

******************************************************************************/
add_ptr add_dup(add_ptr dd_node)
{
  Cudd_Ref(dd_node);
  return(dd_node);
}

/**Function********************************************************************

  Synopsis           [Creates an returns an ADD for constant leaf_node.]

  Description        [Retrieves the ADD for constant leaf_node if it already
  exists, or creates a new ADD.  Returns a pointer to the
  ADD if successful; fails otherwise.]

  SideEffects        [The reference count of the node is incremented by one unit.]

******************************************************************************/
add_ptr add_leaf(DdManager * dd, node_ptr leaf_node)
{
  DdNode * result;

  result = Cudd_addConst(dd,leaf_node);
  common_error(result, "add_leaf: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis           [Returns 1 if the ADD node is a constant node.]

  Description        [Returns 1 if the ADD node is a constant node (rather than an
  internal node). All constant nodes have the same index (MAX_VAR_INDEX).]

  SideEffects        []

******************************************************************************/
int add_isleaf(add_ptr dd_node)
{
  return(Cudd_IsConstant(dd_node));
}

/**Function********************************************************************

  Synopsis           [Returns 1 if the BDD node is a constant node.]

  Description        [Returns 1 if the BDD node is a constant node (rather than an
  internal node). All constant nodes have the same index (MAX_VAR_INDEX).]

  SideEffects        []

******************************************************************************/
int bdd_isleaf(bdd_ptr dd_node)
{
  return(Cudd_IsConstant(dd_node));
}

/**Function********************************************************************

  Synopsis           [Returns the value of a constant node.]

  Description        [Returns the value of a constant node. If <code>Leaf</code>
  is an internal node, a failure occurs.]

  SideEffects        []

******************************************************************************/
node_ptr add_get_leaf(DdManager * dd, add_ptr Leaf)
{
  if (!Cudd_IsConstant((DdNode *)Leaf)) {
    rpterr("add_get_leaf: not a leaf!");
    nusmv_exit(1);
  }
  return((node_ptr)Cudd_V(Leaf));
}

/**Function********************************************************************

  Synopsis           [Checks the unique table of the DdManager for the
  existence of an internal node.]

  Description        [Checks the unique table for the existence of an internal
  node. If it does not exist, it creates a new one. The reference
  count of whatever is returned is increased by one unit. For a newly
  created node, increments the reference counts of what T and E point
  to.  Returns a pointer to the new node if successful; a failure
  occurs if memory is exhausted or if reordering took place.]

  SideEffects        []

******************************************************************************/
add_ptr add_build(DdManager * dd, int level, add_ptr t, add_ptr e)
{
  DdNode * result = NULL;

  if (t == e) return add_dup(t);

  /* A reorder might take place. In this case, keey retrying */
  while (result == NULL) {
    result = (DdNode *)cuddUniqueInter(dd, level, (DdNode *)t, (DdNode *)e);

    /* If result is null and no reorderind took place,
       then something went wrong */
    nusmv_assert(result != NULL || dd->reordered);
  }

  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis           [Returns the ADD variable with index <code>index</code>.]

  Description        [Retrieves the ADD variable with index
  <code>index</code> if it already exists, or creates a new ADD
  variable. Returns a pointer to the variable if successful; a failure
  is generated otherwise.  An ADD variable differs from a BDD variable
  because it points to the arithmetic zero, instead of having a
  complement pointer to 1. The returned value is referenced.]

  SideEffects        []

  SeeAlso            [add_new_var_at_level]

******************************************************************************/
add_ptr add_new_var_with_index(DdManager * dd, int index)
{
  add_ptr result;

  if ((DdHalfWord)index >= (MAX_VAR_INDEX - 1)) {
    rpterr("Unable to allocate a new BDD variable, max. number exceeded (%d)", index);
    nusmv_exit(1);
  }

  result = Cudd_addIthVar(dd, index);
  common_error(result, "add_new_var_with_index: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis           [Returns a new ADD variable at a specified level.]

  Description        [Creates a new ADD variable. The new variable has an
  index equal to the largest previous index plus 1 and is positioned at
  the specified level in the order.  Returns a pointer to the new
  variable if successful; a failure is generated otherwise. The
  returned value is referenced.]

  SideEffects        []

  SeeAlso            [add_new_var_with_index]

******************************************************************************/
add_ptr add_new_var_at_level(DdManager * dd, int level)
{
  DdNode * result;

  result = Cudd_addNewVarAtLevel(dd,level);
  common_error(result, "add_new_var_at_level: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Converts an ADD to a BDD.]

  Description [Converts an ADD to a BDD. Only TRUE and FALSE leaves
  are admitted. Returns a pointer to the resulting BDD if successful;
  NULL otherwise.]

  SideEffects []

  SeeAlso     [bdd_to_add bdd_to_01_add]

******************************************************************************/
bdd_ptr add_to_bdd(DdManager * dd, add_ptr fn)
{
  DdNode * result;
  extern node_ptr zero_number;

  result = Cudd_addBddBooleanMap(dd,fn);
  common_error(result, "add_to_bdd: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Converts an ADD to a BDD according to a strict threshold]

  Description [Converts an ADD to a BDD by replacing all discriminants
  greater than value k with TRUE, and all other discriminants with
  FALSE. Returns a pointer to the resulting BDD if successful; a
  failure is generated otherwise.]

  SideEffects []

  SeeAlso     [add_to_bdd_threshold add_to_bdd bdd_to_01_add]

******************************************************************************/
bdd_ptr add_to_bdd_strict_threshold(DdManager * dd, add_ptr fn, int k)
{
  /* Note for developers: 
     Why not use cudd version?
     [AMa] Because the Cudd version cannot handle NuSMV numbers.
     [AMa] So we use the boolean version preparing the add using NuSMV node_gt
  */
  add_ptr add_k = add_leaf(dd, find_node(NUMBER, NODE_FROM_INT(k), Nil));
  add_ptr tmp = add_apply(dd, node_gt, fn, add_k);
  bdd_ptr result = add_to_bdd(dd, tmp);
  add_free(dd, tmp);
  add_free(dd, add_k);
  return result;
}

/**Function********************************************************************

  Synopsis    [Converts a BDD to a FALSE-TRUE ADD.]

  Description [Converts a BDD to a FALSE-TRUE ADD. Returns a pointer to the
  resulting ADD if successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [add_to_bdd bdd_to_01_add]

******************************************************************************/
add_ptr bdd_to_add(DdManager * dd, bdd_ptr fn)
{
  DdNode * result;

  result = Cudd_BddToAdd(dd, (DdNode *)fn);
  common_error(result, "bdd_to_add: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Converts a BDD to a 0-1 ADD.]

  Description [Converts a BDD to a 0-1 ADD. Returns a pointer to the
  resulting ADD if successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_to_add]

******************************************************************************/
add_ptr bdd_to_01_add(DdManager * dd, bdd_ptr fn)
{
  DdNode * result;

  result = Cudd_BddTo01Add(dd, (DdNode *)fn);
  common_error(result, "bdd_to_01_add: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies AND to the corresponding discriminants of f and g.]

  Description [Applies logical AND to the corresponding discriminants
  of f and g. f and g must have only FALSE or TRUE as terminal
  nodes. Returns a pointer to the result if successful; a failure is
  generated otherwise.]

  SideEffects []

  SeeAlso     [add_or add_xor add_not]

******************************************************************************/
add_ptr add_and(DdManager * dd, add_ptr a, add_ptr b)
{
  DdNode * result;

  result = Cudd_addAnd(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "add_and: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies OR to the corresponding discriminants of f and g.]

  Description [Applies logical OR to the corresponding discriminants
  of f and g. f and g must have only FALSE or TRUE as terminal
  nodes. Returns a pointer to the result if successful; a failure is
  generated otherwise.]

  SideEffects []

  SeeAlso     [add_and add_xor add_not add_imply]

******************************************************************************/
add_ptr add_or(DdManager * dd, add_ptr a, add_ptr b)
{
  DdNode * result;

  result = Cudd_addOr(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "add_or: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies XOR to the corresponding discriminants of f and g.]

  Description [Applies logical XOR to the corresponding discriminants
  of f and g. f and g must have only FALSE or TRUE as terminal nodes. Returns
  a pointer to the result if successful; a failure is generated
  otherwise.]

  SideEffects []

  SeeAlso     [add_or add_and add_not add_imply]

******************************************************************************/
add_ptr add_xor(DdManager * dd, add_ptr a, add_ptr b)
{
  DdNode * result;

  result = Cudd_addXor(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "add_xor: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}


/**Function********************************************************************

  Synopsis    [Applies XNOR to the corresponding discriminants of f and g.]

  Description [Applies logical XNOR to the corresponding discriminants
  of f and g. f and g must have only FALSE or TRUE as terminal
  nodes. Returns a pointer to the result if successful; a failure is
  generated otherwise.]

  SideEffects []

  SeeAlso     [add_xor add_or add_and add_not add_imply]

******************************************************************************/
add_ptr add_xnor(DdManager * dd, add_ptr a, add_ptr b)
{
  DdNode * tmp;
  DdNode * result;

  tmp = Cudd_addNot(dd,(DdNode *)b);
  common_error(tmp, "add_xnor: not(b) = NULL");
  Cudd_Ref(tmp);

  result = Cudd_addXor(dd, (DdNode *)a, (DdNode *)tmp);
  common_error(result, "add_xor: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies NOT to the corresponding discriminant of f.]

  Description [Applies logical NOT to the corresponding discriminant
  of f.  f must have only FALSE or TRUE as terminal nodes. Returns a
  pointer to the result if successful; a failure is generated
  otherwise.]

  SideEffects []

  SeeAlso     [add_and add_xor add_or add_imply]

******************************************************************************/
add_ptr add_not(DdManager * dd, add_ptr a)
{
  DdNode * result;

  result = Cudd_addNot(dd, (DdNode *)a);
  common_error(result, "add_not: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies IMPLY to the corresponding discriminants of f and g.]

  Description [Applies logical IMPLY to the corresponding
  discriminants of f and g.  f and g must have only FALSE or TRUE as
  terminal nodes. Returns a pointer to the result if successful; a
  failure is generated otherwise.]

  SideEffects []

  SeeAlso     [add_and add_xor add_or add_not]

******************************************************************************/
add_ptr add_implies(DdManager * dd, add_ptr a, add_ptr b)
{
  DdNode * tmp;
  DdNode * result;

  tmp = Cudd_addNot(dd,(DdNode *)a);
  common_error(tmp, "add_implies: not(a) = NULL");
  Cudd_Ref(tmp);
  result = Cudd_addOr(dd, tmp, (DdNode *)b);
  common_error2(dd, result, tmp, "add_implies: result = NULL");
  Cudd_Ref(result);
  Cudd_RecursiveDeref(dd, tmp);
  return((add_ptr)result);
}


/**Function********************************************************************

  Synopsis    [Applies IFF to the corresponding discriminants of f and g.]

  Description [Applies logical IFF to the corresponding discriminants
  of f and g.  f and g must have only FALSE or TRUE as terminal
  nodes. Returns a pointer to the result if successful; a failure is
  generated otherwise.]

  SideEffects []

  SeeAlso     [add_and add_xor add_or add_not]

******************************************************************************/
add_ptr add_iff(DdManager * dd, add_ptr a, add_ptr b)
{
  DdNode * tmp;
  DdNode * result;

  tmp = Cudd_addXor(dd, (DdNode *)a, (DdNode *)b);
  common_error(tmp, "add_iff: xor(a,b) = NULL");
  Cudd_Ref(tmp);

  result = Cudd_addNot(dd, tmp);
  common_error2(dd, result, tmp, "add_iff: result = NULL");
  Cudd_Ref(result);

  Cudd_RecursiveDeref(dd, tmp);
  return((add_ptr) result);
}


/**Function********************************************************************

  Synopsis    [Applies AND to the corresponding discriminants of f and g.]

  Description [Applies logical AND to the corresponding discriminants
  of f and g and stores the result in f. f and g must have only FALSE
  or TRUE as terminal nodes.]

  SideEffects [The result is stored in the first operand.]

  SeeAlso     [add_and]

******************************************************************************/
void add_and_accumulate(DdManager * dd, add_ptr *a, add_ptr b)
{
  DdNode * result;

  result = Cudd_addAnd(dd,(DdNode *) *a, (DdNode *)b);
  common_error(result, "add_and_accumulate: result = NULL");
  Cudd_Ref(result);
  Cudd_RecursiveDeref(dd, (DdNode *) *a);
  *a = (add_ptr)result;
}

/**Function********************************************************************

  Synopsis    [Applies OR to the corresponding discriminants of f and g.]

  Description [Applies logical OR to the corresponding discriminants
  of f and g and stores the result in f. f and g must have only FALSE
  or TRUE as terminal nodes.]

  SideEffects [The result is stored in the first operand.]

  SeeAlso     [add_and]

******************************************************************************/
void add_or_accumulate(DdManager * dd, add_ptr *a, add_ptr b)
{
  DdNode * result;

  result = Cudd_addOr(dd,(DdNode *) *a, (DdNode *)b);
  common_error(result, "add_or_accumulate: result = NULL");
  Cudd_Ref(result);
  Cudd_RecursiveDeref(dd, (DdNode *) *a);
  *a = (add_ptr)result;
}

/**Function********************************************************************

  Synopsis    [Applies binary op to the corresponding discriminants of f and g.]

  Description [Returns a pointer to the result if successful; a failure is
  generated otherwise.]

  SideEffects []

******************************************************************************/
add_ptr add_apply(DdManager * dd, NPFNN op, add_ptr f, add_ptr g)
{
  DdNode * result;

  result = Cudd_addApply(dd, op, (DdNode *)f, (DdNode *)g);
  common_error(result, "add_apply: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies unary op to the corresponding discriminant of f]

  Description [Returns a pointer to the result if successful; a failure is
  generated otherwise.

  NOTE: At the moment CUDD does not have unary 'apply', so you have
  to provide a binary op, which is actually unary and applies to
  the first operand only.]

  SideEffects []

******************************************************************************/
add_ptr add_monadic_apply(DdManager * dd, NPFNN/*NPFCVT*/ op, add_ptr f)
{
  /* Function Cudd_addMonadicApply appears in later version of CUDD.
     Here we try to mimic its behaviour */
  add_ptr _true = add_true(dd);
  DdNode * result;

  result = Cudd_addApply(dd, op, (DdNode *)f, (DdNode *) _true);
  common_error(result, "add_monadic_apply: result = NULL");
  Cudd_Ref(result);

  add_free(dd, _true);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis           [Abstracts away variables from an ADD.]

  Description        [Abstracts away variables from an ADD, summing up the values
                      of the merged branches.]

  SideEffects        []

******************************************************************************/
add_ptr add_exist_abstract(DdManager* dd, add_ptr a, bdd_ptr b)
{
  DdNode * cube;
  DdNode * result = (DdNode*)NULL;

  cube = Cudd_BddToAdd(dd, (DdNode *)b);
  common_error(cube, "add_exist_abstract: cube = NULL");

  result = Cudd_addAbstract(dd, node_plus, (DdNode *)a, (DdNode *)cube);
  common_error(result, "add_exist_abstract: result = NULL");

  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Implements ITE(f,g,h).]

  Description [Implements ITE(f,g,h). This procedure assumes that f is
  a FALSE-TRUE ADD.  Returns a pointer to the resulting ADD if
  successful; a failure is generated otherwise.]

  SideEffects []

******************************************************************************/
add_ptr add_ifthenelse(DdManager * dd, add_ptr If, add_ptr Then, add_ptr Else)
{
  DdNode * result;

  result = Cudd_addIte(dd, (DdNode *)If, (DdNode *)Then, (DdNode *)Else);
  common_error(result, "add_ifthenelse: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Computes the difference between two ADD cubes.]

  Description [Computes the difference between two ADD cubes, i.e. the
  cube of ADD variables belonging to cube a and not belonging to cube
  b. Returns a pointer to the resulting cube; a failure is generated
  otherwise.]

  SideEffects []

  SeeAlso     [bdd_cube_diff]

******************************************************************************/
add_ptr add_cube_diff(DdManager * dd, add_ptr a, add_ptr b)
{
  DdNode * result;

  result = Cudd_addCubeDiff(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "add_cube_diff: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Permutes the variables of an ADD.]

  Description [Given a permutation in array permut, creates a new ADD
  with permuted variables. There should be an entry in array permut
  for each variable in the manager. The i-th entry of permut holds the
  index of the variable that is to substitute the i-th variable.
  Returns a pointer to the resulting ADD if successful; a failure is
  generated otherwise. The reuslt is referenced.]

  SideEffects []

  SeeAlso     [bdd_permute]

******************************************************************************/
add_ptr add_permute(DdManager * dd, add_ptr fn, int * permut)
{
  DdNode *result;

  result = Cudd_addPermute(dd, (DdNode *)fn, permut);
  common_error(result, "add_permute: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Finds the variables on which an ADD depends on.]

  Description [Finds the variables on which an ADD depends on.
  Returns an ADD consisting of the product of the variables if
  successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_support]

*****************************************************************************/
add_ptr add_support(DdManager * dd, add_ptr fn)
{
  DdNode * tmp_1, * result;

  tmp_1 = Cudd_Support(dd, (DdNode *)fn);
  common_error(tmp_1, "add_support: tmp_1 = NULL");
  Cudd_Ref(tmp_1);
  result = Cudd_BddToAdd(dd, tmp_1);
  common_error2(dd, result, tmp_1, "add_support: result = NULL");
  Cudd_RecursiveDeref(dd, tmp_1);
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis [ADD restrict according to Coudert and Madre's algorithm (ICCAD90).]

  Description [ADD restrict according to Coudert and Madre's algorithm
  (ICCAD90). Returns the restricted ADD if successful; a failure is
  generated otherwise.
  If application of restrict results in an ADD larger than the input
  ADD, the input ADD is returned.]

  SideEffects []

******************************************************************************/
add_ptr add_simplify_assuming(DdManager * dd, add_ptr a, add_ptr b)
{
  DdNode * result;

  result = Cudd_addRestrict(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "add_simplify_assuming: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Counts the number of ADD nodes in an ADD.]

  Description [Counts the number of ADD nodes in an ADD. Returns the number
  of nodes in the graph rooted at node.]

  SideEffects []

  SeeAlso     [add_count_minterm]

******************************************************************************/
int add_size(DdManager * dd, add_ptr fn)
{
  return(Cudd_DagSize((DdNode *)fn));
}

/**Function********************************************************************

  Synopsis    [Counts the number of ADD minterms of an ADD.]

  Description [Counts the number of minterms of an ADD. The function is
  assumed to depend on nvars variables. The minterm count is
  represented as a double, to allow for a larger number of variables.
  Returns the number of minterms of the function rooted at node. The
  result is parameterized by the number of \"nvars\" passed as argument.]

  SideEffects []

  SeeAlso     [bdd_size bdd_count_minterm]

******************************************************************************/
double add_count_minterm(DdManager * dd, add_ptr fn, int nvars)
{
  return(Cudd_CountMinterm(dd, (DdNode *)fn, nvars));
}

/**Function********************************************************************

  Synopsis    [Given the result of add_if_then it returns the leaf corresponding.]

  Description [Given the result of add_if_then it returns the leaf
  corresponding. The ADD is traversed according to the rules given as
  a result of add_if_then. If it is costant, then the corresponding
  value is returned. The Else branch is recursively traversed, if the
  result of this travesring is an ELSE_CNST, then the result of the
  traversing of the Then branch is returned.]

  SideEffects []

  SeeAlso     [add_if_then]

******************************************************************************/
node_ptr add_value(DdManager * dd, add_ptr fn)
{
  node_ptr result;

  result = Cudd_add_value((DdNode *)fn);
  return(result);
}

/**Function********************************************************************

  Synopsis    [Given a minterm, it returns an ADD indicating the rules
  to traverse the ADD.]

  Description [Given a minterm, it returns an ADD indicating the rules
  to traverse the ADD.]

  SideEffects []

  SeeAlso     [add_value]

******************************************************************************/
add_ptr add_if_then(DdManager * dd, add_ptr I, add_ptr T)
{
  DdNode * result;

  result = Cudd_addIfThen(dd, (DdNode *)I, (DdNode *)T);
  common_error(result, "add_if_then: result = NULL");
  Cudd_Ref(result);
  return((add_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies a generic function to constant nodes.]

  Description [Applies a generic function <tt>VPFCVT op</tt> to the
  constants nodes of <tt>f</tt>.]

  SideEffects []

******************************************************************************/
void add_walkleaves(VPFCVT op, add_ptr f)
{
 Cudd_addWalkLeaves(op, (DdNode *)f);
 return;
}

/**Function********************************************************************

  Synopsis           [Reads the constant TRUE BDD of the manager.]

  Description        [Reads the constant TRUE BDD of the manager.]

  SideEffects        []

  SeeAlso            [bdd_false]
******************************************************************************/
bdd_ptr bdd_true(DdManager * dd)
{
  DdNode * result = Cudd_ReadTrue(dd);

  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis           [Reads the constant FALSE BDD of the manager.]

  Description        [Reads the constant FALSE BDD of the manager.]

  SideEffects        []

  SeeAlso            [bdd_true]
******************************************************************************/
bdd_ptr bdd_false(DdManager * dd)
{
  DdNode * result = Cudd_ReadLogicFalse(dd);

  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis           [Check if the BDD is TRUE.]

  Description        [Check if the BDD is TRUE.]

  SideEffects        []

  SeeAlso            [bdd_true]
******************************************************************************/
int bdd_is_true(DdManager * dd, bdd_ptr f)
{
  return((DdNode *)f == Cudd_ReadTrue(dd));
}


/**Function********************************************************************

  Synopsis           [Check if the BDD is not true.]

  Description        [Check if the BDD is not true.]

  SideEffects        []

  SeeAlso            [bdd_true]
******************************************************************************/
int bdd_isnot_true(DdManager * dd, bdd_ptr f)
{
  return((DdNode *)f != Cudd_ReadTrue(dd));
}

/**Function********************************************************************

  Synopsis           [Check if the BDD is false.]

  Description        [Check if the BDD is false.]

  SideEffects        []

  SeeAlso            [bdd_false]
******************************************************************************/
int bdd_is_false(DdManager * dd, bdd_ptr f)
{
  return((DdNode *)f == Cudd_ReadLogicFalse(dd));
}

/**Function********************************************************************

  Synopsis           [Check if the BDD is not false.]

  Description        [Check if the BDD is not false.]

  SideEffects        []

  SeeAlso            [bdd_false]
******************************************************************************/
int bdd_isnot_false(DdManager * dd, bdd_ptr f)
{
  return((DdNode *)f != Cudd_ReadLogicFalse(dd));
}


/**Function********************************************************************

  Synopsis           [Reference an BDD node.]

  Description        [Reference an BDD node.]

  SideEffects        [The reference count of the node is incremented by one.]

  SeeAlso            [bdd_deref bdd_free]
******************************************************************************/
void bdd_ref(bdd_ptr dd_node)
{
  Cudd_Ref(dd_node);
}

/**Function********************************************************************

  Synopsis           [Dereference an BDD node.]

  Description        [Dereference an BDD node.]

  SideEffects        [The reference count of the node is decremented by one.]

  SeeAlso            [bdd_ref bdd_free]
******************************************************************************/
void bdd_deref(bdd_ptr dd_node)
{

  Cudd_Deref(dd_node);
}

/**Function********************************************************************

  Synopsis           [Dereference an BDD node. If it dies, recursively decreases
  the reference count of its children.]

  Description        [Decreases the reference count of node. If the node dies,
  recursively decreases the reference counts of its children. It is used to
  dispose off a BDD that is no longer needed.]

  SideEffects        [The reference count of the node is decremented by one,
  and if the node dies a recursive dereferencing is applied to its children.]

  SeeAlso            []
******************************************************************************/
void bdd_free(DdManager * dd, bdd_ptr dd_node)
{
  common_error(dd_node, "bdd_free: dd_node = NULL");

  Cudd_RecursiveDeref(dd, (DdNode *)dd_node);
}

/**Function********************************************************************

  Synopsis           [Creates a copy of an BDD node.]

  Description        [Creates a copy of an BDD node.]

  SideEffects        [The reference count is increased by one unit.]

  SeeAlso            [bdd_ref bdd_free bdd_deref]

******************************************************************************/
bdd_ptr bdd_dup(bdd_ptr dd_node)
{
  Cudd_Ref(dd_node);
  return(dd_node);
}

/**Function********************************************************************

  Synopsis    [Applies NOT to the corresponding discriminant of f.]

  Description [Applies logical NOT to the corresponding discriminant of f.
  f must be a BDD. Returns a pointer to the result if successful; a
  failure is generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_and bdd_xor bdd_or bdd_imply]

******************************************************************************/
bdd_ptr bdd_not(DdManager * dd, bdd_ptr fn)
{
  DdNode * result;

  result = Cudd_Not(fn);
  common_error(result, "bdd_not: result == NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies AND to the corresponding discriminants of f and g.]

  Description [Applies logical AND to the corresponding discriminants
  of f and g. f and g must be BDDs. Returns a pointer to the result if
  successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_or bdd_xor bdd_not]

******************************************************************************/
bdd_ptr bdd_and(DdManager * dd, bdd_ptr a, bdd_ptr b)
{
  DdNode * result;

  result = Cudd_bddAnd(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "bdd_and: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies OR to the corresponding discriminants of f and g.]

  Description [Applies logical OR to the corresponding discriminants
  of f and g. f and g must be BDDs. Returns a pointer to the result if
  successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_and bdd_xor bdd_not]

******************************************************************************/
bdd_ptr bdd_or(DdManager * dd, bdd_ptr a, bdd_ptr b)
{
  DdNode * result;

  result = Cudd_bddOr(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "bdd_or: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies XOR to the corresponding discriminants of f and g.]

  Description [Applies logical XOR to the corresponding discriminants
  of f and g. f and g must be BDDs. Returns a pointer to the result if
  successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_or bdd_imply bdd_not]

******************************************************************************/
bdd_ptr bdd_xor(DdManager * dd, bdd_ptr a, bdd_ptr b)
{
  DdNode * result;

  result = Cudd_bddXor(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "bdd_xor: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies IFF to the corresponding discriminants of f and g.]

  Description [Applies logical IFF to the corresponding discriminants
  of f and g. f and g must be BDDs. Returns a pointer to the result if
  successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_or bdd_xor bdd_not]

******************************************************************************/
bdd_ptr bdd_iff(DdManager * dd, bdd_ptr a, bdd_ptr b)
{
  DdNode * tmp_1;
  DdNode * result;

  tmp_1 = Cudd_bddXor(dd, (DdNode *)a, (DdNode *)b);
  common_error(tmp_1, "bdd_iff: bdd_xor(a,b) = NULL");
  Cudd_Ref(tmp_1);
  result = Cudd_Not(tmp_1);
  common_error2(dd, result, tmp_1, "bdd_iff: result = NULL");
  Cudd_Ref(result);
  Cudd_RecursiveDeref(dd, tmp_1);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies IMPLY to the corresponding discriminants of f and g.]

  Description [Applies logical IMPLY to the corresponding discriminants
  of f and g. f and g must be BDDs. Returns a pointer to the result if
  successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_or bdd_xor bdd_not]

******************************************************************************/
bdd_ptr bdd_imply(DdManager * dd, bdd_ptr a, bdd_ptr b)
{
  DdNode * tmp_1;
  DdNode * result;

  tmp_1 = Cudd_Not((DdNode *)a);
  common_error(tmp_1, "bdd_imply: not(a) = NULL");
  Cudd_Ref(tmp_1);
  result = Cudd_bddOr(dd, tmp_1, (DdNode *)b);
  common_error2(dd, result, tmp_1, "bdd_imply: result = NULL");
  Cudd_Ref(result);
  Cudd_RecursiveDeref(dd, tmp_1);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Applies AND to the corresponding discriminants of f and g.]

  Description [Applies logical AND to the corresponding discriminants
  of f and g and stores the result in f. f and g must be two BDDs. The
  result is referenced.]

  SideEffects [The result is stored in the first operand and referenced.]

  SeeAlso     [bdd_and]

******************************************************************************/
void bdd_and_accumulate(DdManager * dd, bdd_ptr * a, bdd_ptr b)
{
  DdNode * result;

  result = Cudd_bddAnd(dd, (DdNode *)*a, (DdNode *)b);
  common_error(result, "bdd_and_accumulate: result = NULL");
  Cudd_Ref(result);
  Cudd_RecursiveDeref(dd, (DdNode *)*a);
  *a = result;
  return;
}

/**Function********************************************************************

  Synopsis    [Applies OR to the corresponding discriminants of f and g.]

  Description [Applies logical OR to the corresponding discriminants
  of f and g and stores the result in f. f and g must be two BDDs. The
  result is referenced.]

  SideEffects [The result is stored in the first operand and referenced.]

  SeeAlso     [bdd_and]

******************************************************************************/
void bdd_or_accumulate(DdManager * dd, bdd_ptr * a, bdd_ptr b)
{
  DdNode * result;

  result = Cudd_bddOr(dd, (DdNode *)*a, (DdNode *)b);
  common_error(result, "bdd_or_accumulate: result = NULL");
  Cudd_Ref(result);
  Cudd_RecursiveDeref(dd, (DdNode *) *a);
  *a = result;
  return;
}

/**Function********************************************************************

  Synopsis [Existentially abstracts all the variables in cube from fn.]

  Description [Existentially abstracts all the variables in cube from fn.
  Returns the abstracted BDD if successful; a failure is generated
  otherwise.]

  SideEffects []

  SeeAlso     [bdd_forall]

******************************************************************************/
bdd_ptr bdd_forsome(DdManager * dd, bdd_ptr fn, bdd_ptr cube)
{
  DdNode * result;

  result = Cudd_bddExistAbstract(dd, (DdNode *)fn, (DdNode *)cube);
  common_error(result, "bdd_forsome: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Universally abstracts all the variables in cube from f.]

  Description [Universally abstracts all the variables in cube from f.
  Returns the abstracted BDD if successful; a failure is generated
  otherwise.]

  SideEffects []

  SeeAlso     [bdd_forsome]

******************************************************************************/
bdd_ptr bdd_forall(DdManager * dd, bdd_ptr fn, bdd_ptr cube)
{
  DdNode * result;

  result = Cudd_bddUnivAbstract(dd, (DdNode *)fn, (DdNode *)cube);
  common_error(result, "bdd_forall: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Permutes the variables of a BDD.]

  Description [Given a permutation in array permut, creates a new BDD
  with permuted variables. There should be an entry in array permut
  for each variable in the manager. The i-th entry of permut holds the
  index of the variable that is to substitute the i-th variable.
  Returns a pointer to the resulting BDD if successful; a failure is
  generated otherwise. The result is referenced.]

  SideEffects []

  SeeAlso     [bdd_permute]

******************************************************************************/
bdd_ptr bdd_permute(DdManager * dd, bdd_ptr fn, int * permut)
{
  DdNode * result;

  result = Cudd_bddPermute(dd, (DdNode *)fn, permut);
  common_error(result, "bdd_permute: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis [Takes the AND of two BDDs and simultaneously abstracts the
  variables in cube.]

  Description [Takes the AND of two BDDs and simultaneously abstracts
  the variables in cube. The variables are existentially abstracted.
  Returns a pointer to the result if successful; a failure is
  generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_and bdd_forsome]

******************************************************************************/
bdd_ptr bdd_and_abstract(DdManager *dd, bdd_ptr T, bdd_ptr S, bdd_ptr V)
{
  DdNode * result;

  result = Cudd_bddAndAbstract(dd, (DdNode *)T, (DdNode *)S, (DdNode *)V);
  common_error(result, "bdd_and_abstract: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis [BDD restrict according to Coudert and Madre's algorithm
  (ICCAD90).]

  Description [BDD restrict according to Coudert and Madre's algorithm
  (ICCAD90). Returns the restricted BDD if successful; a failure is
  generated otherwise.
  If application of restrict results in an BDD larger than the input
  BDD, the input BDD is returned.]

  SideEffects []

******************************************************************************/
bdd_ptr bdd_simplify_assuming(DdManager *dd, bdd_ptr fn, bdd_ptr c)
{
  DdNode * result;

  result = Cudd_bddRestrict(dd, (DdNode *)fn, (DdNode *)c);
  common_error(result, "bdd_simplify_assuming: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Restrict operator as described in Coudert et al. ICCAD90.]

  Description [Restrict operator as described in Coudert et
  al. ICCAD90.  Always returns a BDD not larger than the input
  <code>f</code> if successful; a failure is generated otherwise. The
  result is referenced.]

  SideEffects []

  SeeAlso     [bdd_simplify_assuming]

******************************************************************************/
bdd_ptr bdd_minimize(DdManager *dd, bdd_ptr fn, bdd_ptr c)
{
  DdNode * result;

  result = Cudd_bddRestrict(dd, (DdNode *)fn, (DdNode *)c);
  common_error(result, "bdd_minimize: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
} /* end of bdd_minimize */

/**Function********************************************************************

  Synopsis    [Computes f constrain c.]

  Description [Computes f constrain c (f @ c).
  Uses a canonical form: (f' @ c) = ( f @ c)'.  (Note: this is not true
  for c.)  List of special cases:
    <ul>
    <li> F @ 0 = 0
    <li> F @ 1 = F
    <li> 0 @ c = 0
    <li> 1 @ c = 1
    <li> F @ F = 1
    <li> F @ F'= 0
    </ul>
  Returns a pointer to the result if successful; a failure is
  generated otherwise.]

  SideEffects []

  SeeAlso     [bdd_minimize bdd_simplify_assuming]

******************************************************************************/
bdd_ptr bdd_cofactor(DdManager * dd, bdd_ptr f, bdd_ptr g)
{
  DdNode *result;

  /* We use Cudd_bddConstrain instead of Cudd_Cofactor for generality. */
  result = Cudd_bddConstrain(dd, (DdNode *)f, (DdNode *)g);
  common_error(result, "bdd_cofactor: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
} /* end of bdd_cofactor */


/**Function********************************************************************

  Synopsis    [Return a minimum size BDD between bounds.]

  SideEffects []

  SeeAlso     [bdd_minimize bdd_simplify_assuming bdd_cofactor]

******************************************************************************/
bdd_ptr bdd_between(DdManager *dd, bdd_ptr f_min, bdd_ptr f_max)
{
  bdd_ptr care_set, ret;

  care_set = bdd_imply(dd, f_min, f_max);
  ret = bdd_minimize(dd, f_min, care_set);
  bdd_free(dd, care_set);
  /*
    The size of ret is never larger than the size of f_min. We need
    only to check ret against f_max.
  */
  if (bdd_size(dd, f_max) <= bdd_size(dd, ret)) {
    bdd_free(dd, ret);
    return(bdd_dup(f_max));
  } else {
    return(ret);
  }
} /* end of bdd_between */

/**Function********************************************************************

  Synopsis           [Determines whether f is less than or equal to g.]

  Description        [Returns 1 if f is less than or equal to g; 0 otherwise.
  No new nodes are created.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int bdd_entailed(DdManager * dd, bdd_ptr f, bdd_ptr g)
{
  int result;

  result = Cudd_bddLeq(dd, (DdNode *)f, (DdNode *)g);
  return(result);
}


/**Function********************************************************************

  Synopsis           [Determines whether an intersection between
  f and g is not empty]

  Description        [Returns 1 if an intersection between
  f and g is not empty; 0 otherwise.
  No new nodes are created.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int bdd_intersected(DdManager * dd, bdd_ptr f, bdd_ptr g)
{
  if (bdd_is_false(dd, f) || bdd_is_false(dd, g)) return 0;

  DdNode* not_g = Cudd_Not((DdNode *)g);

  int result = Cudd_bddLeq(dd, (DdNode *)f, (DdNode *)not_g);

  return !result;
}


/**Function********************************************************************

  Synopsis           [Returns the then child of a bdd node.]

  Description        [Returns the then child of a bdd node. The node
  must not be a leaf node. Notice that this funxction does not save
  the bdd. Is the responsibility of the user to save it if it is the case.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
bdd_ptr bdd_then(DdManager * dd, bdd_ptr f)
{
  bdd_ptr result;

  if (Cudd_IsConstant((DdNode *)f)) {
    rpterr("bdd_then: called on a constant bdd.");
    result = (bdd_ptr)NULL;
    nusmv_exit(1);
  }
  else {
    result = (bdd_ptr)Cudd_T(f);
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Returns the else child of a bdd node.]

  Description        [Returns the else child of a bdd node. The node
  must not be a leaf node. Notice that this funxction does not save
  the bdd. Is the responsibility of the user to save it if it is the case.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
bdd_ptr bdd_else(DdManager * dd, bdd_ptr f)
{
  bdd_ptr result;

  if (Cudd_IsConstant((DdNode *)f)) {
    rpterr("bdd_else: called on a constant bdd.");
    result = (bdd_ptr)NULL;
    nusmv_exit(1);
  }
  else {
    result = (bdd_ptr)Cudd_E(f);
  }
  return(result);
}

/**Function********************************************************************

  Synopsis           [Returns 1 if the BDD pointer is complemented.]

  Description        [Returns 1 if the BDD pointer is complemented.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int bdd_iscomplement(DdManager * dd, bdd_ptr f)
{
  return(Cudd_IsComplement((DdNode *)f));
}

/**Function********************************************************************

  Synopsis           [Finds the current position of variable index in the
  order.]

  Description        [Finds the current position of variable index in the
  order.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int bdd_readperm(DdManager * dd, bdd_ptr f)
{
  int result;

  result = Cudd_ReadPerm(dd, Cudd_NodeReadIndex((DdNode *)f));
  return(result);
}

/**Function********************************************************************

  Synopsis           [Returns the index of the node.]

  Description        [Returns the index of the node.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int bdd_index(DdManager * dd, bdd_ptr f)
{
  int result;

  result = Cudd_NodeReadIndex((DdNode *)f);
  return(result);
}

/**Function********************************************************************

  Synopsis           [Implements ITE(i,t,e).]

  Description        [Implements ITE(i,t,e). Returns a pointer to the
  resulting BDD if successful;  a failure is
  generated otherwise.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
bdd_ptr bdd_ite(DdManager * dd, bdd_ptr i, bdd_ptr t, bdd_ptr e)
{
  DdNode * result;

  result = Cudd_bddIte(dd, (DdNode *)i, (DdNode *)t, (DdNode *)e);
  common_error(result, "bdd_ite: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Counts the number of BDD nodes in an BDD.]

  Description [Counts the number of BDD nodes in an BDD. Returns the number
  of nodes in the graph rooted at node.]

  SideEffects []

  SeeAlso     [bdd_count_minterm]

******************************************************************************/
int bdd_size(DdManager * dd, bdd_ptr fn)
{
  return(Cudd_DagSize((DdNode *)fn));
}

/**Function********************************************************************

  Synopsis    [Counts the number of BDD minterms of an BDD.]

  Description [Counts the number of minterms of an BDD. The function is
  assumed to depend on nvars variables. The minterm count is
  represented as a double, to allow for a larger number of variables.
  Returns the number of minterms of the function rooted at node. The
  result is parameterized by the number of \"nvars\" passed as argument.]

  SideEffects []

  SeeAlso     [bdd_size bdd_count_minterm]

******************************************************************************/
double bdd_count_minterm(DdManager * dd, bdd_ptr fn, int nvars)
{
  return(floor(Cudd_CountMinterm(dd, (DdNode *)fn, nvars)));
}

/**Function********************************************************************

  Synopsis    [Finds the variables on which an BDD depends on.]

  Description [Finds the variables on which an BDD depends on.
  Returns an BDD consisting of the product of the variables if
  successful; a failure is generated otherwise.]

  SideEffects []

  SeeAlso     [add_support]

*****************************************************************************/
bdd_ptr bdd_support(DdManager *dd, bdd_ptr fn)
{
  DdNode * result;

  result = Cudd_Support(dd, (DdNode *)fn);
  common_error(result, "bdd_support: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis [Picks one on-set minterm deterministically from the given BDD.]

  Description [Picks one on-set minterm deterministically from the
  given DD. The minterm is in terms of vars. Builds a BDD for the
  minterm and returns a pointer to it if successful; a failure is
  generated otherwise. There are two reasons why the procedure may fail: It may
  run out of memory; or the function fn may be the constant 0. The
  result is referenced.]

  SideEffects []

******************************************************************************/
bdd_ptr bdd_pick_one_minterm(DdManager * dd, bdd_ptr fn, bdd_ptr * vars, int n)
{
  DdNode * result;

  if (bdd_is_false(dd, fn)) {
    Cudd_Ref(fn);
    return(fn);
  }
  else {
    result = Cudd_bddPickOneMintermNR(dd, (DdNode *)fn, (DdNode **)vars, n);
    common_error(result, "bdd_pick_one_minterm: result = NULL");
    Cudd_Ref(result);
    return((bdd_ptr)result);
  }
}

/**Function********************************************************************

  Synopsis    [Picks one on-set minterm randomly from the given DD.]

  Description [Picks one on-set minterm randomly from the given DD. The
  minterm is in terms of vars. Builds a BDD for the minterm and returns a
  pointer to it if successful; a failure is generated otherwise. There
  are two reasons why the procedure may fail: It may run out of
  memory; or the function f may be the constant 0.]

  SideEffects []

******************************************************************************/
bdd_ptr bdd_pick_one_minterm_rand(DdManager * dd, bdd_ptr fn, bdd_ptr * vars, int n)
{
  DdNode * result;

  if (bdd_is_false(dd, fn)) {
    Cudd_Ref(fn);
    return(fn);
  }
  else {
    result = Cudd_bddPickOneMinterm(dd, (DdNode *)fn, (DdNode **)vars, n);
    common_error(result, "bdd_pick_one_minterm_rand: result = NULL");
    Cudd_Ref(result);
    return((bdd_ptr)result);
  }
}

/**Function********************************************************************

  Synopsis           [Returns the array of All Possible Minterms]

  Description        [Takes a minterm and returns an array of all its terms,
  according to variables specified in the array vars[].  Notice that the array
  of the result has to be previously allocated, and its size must be greater
  or equal the number of the minterms of the "minterm" function. The array
  contains referenced BDD so it is necessary to dereference them after their
  use. Calls Cudd_PickAllTerms avoiding to pass it a true picking-from set of
        states.]

  SideEffects        []

  SeeAlso            [bdd_pick_one_minterm_rand bdd_pick_one_minterm]

******************************************************************************/
int bdd_pick_all_terms(
  DdManager * dd           /* dd manager */,
  bdd_ptr   pick_from_set  /* minterm from which to pick  all term */,
  bdd_ptr   * vars         /* The array of vars to be put in the returned array */,
  int       vars_dim       /* The size of the above array */,
  bdd_ptr   * result       /* The array used as return value */,
  int       result_dim     /* The size of the above array */)
{
  if (bdd_is_true(dd, pick_from_set)) {
    bdd_ptr not_var0 = bdd_not(dd, vars[0]);

    if (Cudd_PickAllTerms(dd, vars[0], vars, vars_dim, result) == 1) {
      fprintf(nusmv_stderr, "Error from Cudd_PickAllTerms.\n");
      bdd_free(dd, not_var0);
      return 1;
    }
    nusmv_assert((result_dim % 2) == 0);
    if ( Cudd_PickAllTerms(dd, not_var0, vars, vars_dim,
                          result + result_dim/2) == 1 ) {
      fprintf(nusmv_stderr, "Error from Cudd_PickAllTerms.\n");
      bdd_free(dd, not_var0);
      return 1;
    }
    bdd_free(dd, not_var0);
  }
  else
    if (Cudd_PickAllTerms(dd, pick_from_set, vars, vars_dim, result) == 1) {
      fprintf(nusmv_stderr, "Error from Cudd_PickAllTerms.\n");
      return 1;
    }
  return 0;
}

/**Function********************************************************************

  Synopsis           [Returns the BDD variable with index <code>index</code>.]

  Description        [Retrieves the BDD variable with index <code>index</code>
  if it already exists, or creates a new BDD variable. Returns a
  pointer to the variable if successful; a failure is generated
  otherwise. The returned value is referenced.]

  SideEffects        []

  SeeAlso            [bdd_new_var_at_level add_new_var_at_level]

******************************************************************************/
bdd_ptr bdd_new_var_with_index(DdManager * dd, int index)
{
  DdNode * result;

  result = Cudd_bddIthVar(dd, index);
  common_error(result, "bdd_new_var_with_index: result = NULL");
  /* bdd var does not require to be referenced when created */
  return bdd_dup((bdd_ptr) result);
}


/**Function********************************************************************

  Synopsis    [Computes the difference between two BDD cubes.]

  Description [Computes the difference between two BDD cubes, i.e. the
  cube of BDD variables belonging to cube a and not belonging to cube
  b. Returns a pointer to the resulting cube; a failure is generated
  otherwise.]

  SideEffects []

  SeeAlso     [add_cube_diff]

******************************************************************************/
bdd_ptr bdd_cube_diff(DdManager * dd, bdd_ptr a, bdd_ptr b)
{
  DdNode * result;

  result = Cudd_bddCubeDiff(dd, (DdNode *)a, (DdNode *)b);
  common_error(result, "bdd_cube_diff: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Computes the union between two BDD cubes.]

  Description [Computes the union between two BDD cubes, i.e. the
  cube of BDD variables belonging to cube a OR to cube b.
  Returns a pointer to the resulting cube; a failure is generated
  otherwise.]

  SideEffects []

  SeeAlso     [bdd_cube_intersection,bdd_and]

******************************************************************************/
bdd_ptr bdd_cube_union(DdManager * dd, bdd_ptr a, bdd_ptr b)
{
  bdd_ptr result;
  result = bdd_and(dd,a,b);
  common_error(result, "bdd_cube_union: result = NULL");
  return(result);
}

/**Function********************************************************************

  Synopsis    [Computes the intersection between two BDD cubes.]

  Description [Computes the difference between two BDD cubes, i.e. the
  cube of BDD variables belonging to cube a AND belonging to cube
  b. Returns a pointer to the resulting cube; a failure is generated
  otherwise.]

  SideEffects []

  SeeAlso     [bdd_cube_union,bdd_cube_diff]

******************************************************************************/
bdd_ptr bdd_cube_intersection(DdManager * dd, bdd_ptr a, bdd_ptr b)
{
  bdd_ptr result,tmp;
  tmp = bdd_cube_diff(dd , a , b);
  result= bdd_cube_diff(dd , a , tmp);
  bdd_free(dd,tmp);
  common_error(result, "bdd_cube_intersection: result = NULL");
  return(result);
}

/**Function********************************************************************

  Synopsis           [Returns the index of the lowest variable in the BDD a.]

  Description        [Returns the index of the lowest variable in the
  BDD, i.e. the variable in BDD a with the highest position in the
  ordering. ]

  SideEffects        []

******************************************************************************/
int bdd_get_lowest_index(DdManager * dd, bdd_ptr a)
{
  int result;

  result = Cudd_BddGetLowestVar(dd, (DdNode *)a);
  return(result);
}


/**Function********************************************************************

  Synopsis           [Finds a satisfying path in the BDD d.]

  Description        [Finds a satisfying path in the BDD d. This path should
  not include all variabales. It only need ot include the levels needed to
  satify the BDD.]

  SideEffects        []

******************************************************************************/
bdd_ptr bdd_get_one_sparse_sat(DdManager * dd, bdd_ptr d) {
  bdd_ptr result, old, zero;
  bdd_ptr T, E, node, vi, nvi;

  int reord_status;
  dd_reorderingtype rt;

  /* temporary disables reordering */
  reord_status = dd_reordering_status(dd, &rt);
  if (reord_status == 1) { dd_autodyn_disable(dd); }

  /* if it is a constant BDD (true or false), then return it */
  if (bdd_is_true(dd, d)) return bdd_true(dd);
  if (bdd_is_false(dd, d)) return bdd_false(dd);

  node = d;
  zero = bdd_false(dd);
  old = bdd_true(dd);

  while (true) {
    /* no need to free them, reference is not incremented */
    T = bdd_then(dd, node);
    E = bdd_else(dd, node);

    /* Take care of the fact that the node can be complemented */
    if (bdd_iscomplement(dd, node)) {
      T = bdd_not(dd, T);
      bdd_deref(T);
      E = bdd_not(dd, E);
      bdd_deref(E);
    }

    /* Get the bdd variable at the given level */
    vi = bdd_new_var_with_index(dd, bdd_index(dd, node));
    /* reference it since bdd_new_var_with_index does not reference */
    bdd_ref(vi);

    /* If then is true, build assignment and stop */
    if (bdd_is_true(dd, T)) {
      result = bdd_ite(dd, old, vi, zero);
      bdd_free(dd, vi);
      bdd_free(dd, old);
      break;
    }

    /* If else is true, build assignment and stop */
    if (bdd_is_true(dd, E)) {
      nvi = bdd_not(dd, vi);
      result = bdd_ite(dd, old, nvi, zero);
      bdd_free(dd, nvi);
      bdd_free(dd, vi);
      bdd_free(dd, old);
      break;
    }

    /* If then is false, build assignment and recurse on the else */
    if (bdd_is_false(dd, T)) {
      nvi = bdd_not(dd, vi);
      result = bdd_ite(dd, old, nvi, zero);
      bdd_free(dd, nvi);
      bdd_free(dd, old);
      old = result;
      node = E;
    }
    /* If else is false, build assignment and recurse on the then */
    else if (bdd_is_false(dd, E)) {
      result = bdd_ite(dd, old, vi, zero);
      bdd_free(dd, old);
      old = result;
      node = T;
    }
    /* If neither then nor else are true/false, build assignment
       considering the current variable positive, and recurse on the
       then */
    else {
      result = bdd_ite(dd, old, vi, zero);
      bdd_free(dd, old);
      old = result;
      node = T;
    }
    bdd_free(dd, vi);

  } /* loop */

  bdd_free(dd, zero);

  /* re-enable reordering if it is required */
  if (reord_status == 1) { dd_autodyn_enable(dd,rt); }

  return result;
}


/**Function********************************************************************

  Synopsis    [Expands cube to a prime implicant of f.]

  Description [Expands cube to a prime implicant of f. Returns the prime
  if successful; NULL otherwise.  In particular, NULL is returned if cube
  is not a real cube or is not an implicant of f.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
bdd_ptr bdd_make_prime(DdManager *dd, bdd_ptr cube, bdd_ptr b) {
  DdNode * result;

  result = Cudd_bddMakePrime(dd, (DdNode *)cube, (DdNode *)b);
  common_error(result, "bdd_make_prime: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}


/**Function********************************************************************

  Synopsis    [Finds a largest cube in a BDD.]

  Description [Finds a largest cube in a BDD b, i.e. an implicant of BDD b.
  Notice that, it is not guaranteed to be the largest implicant of b.]

  SideEffects [The number of literals of the cube is returned in length.]

  SeeAlso     []

******************************************************************************/
bdd_ptr bdd_largest_cube(DdManager *dd, bdd_ptr b, int *length) {
  DdNode * result;

  result = Cudd_LargestCube(dd, (DdNode *)b, length);
  common_error(result, "bdd_largest_cube: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Finds a prime implicant for a BDD.]

  Description [Finds the prime implicant of a BDD b based on the largest cube
  in low where low implies b.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
bdd_ptr bdd_compute_prime_low(DdManager * dd, bdd_ptr b, bdd_ptr low) {
  int length;
  bdd_ptr result, implicant;

  /* a minterm is an implicant */
  implicant = bdd_largest_cube(dd, low, &length);
  /* Expand the minterm to be a prime implicant */
  result = bdd_make_prime(dd, implicant, b);
  bdd_free(dd, implicant);

  return result;
}

/**Function********************************************************************

  Synopsis    [Finds a set of prime implicants for a BDD.]

  Description [Finds the set of prime implicants of a BDD b that are
  implied by low where low implies b.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
array_t * bdd_compute_primes_low(DdManager * dd, bdd_ptr b, bdd_ptr low) {
  array_t * result = array_alloc(bdd_ptr, 0);
  bdd_ptr curr_low = bdd_dup(low);

  while(bdd_isnot_false(dd, curr_low)) {
    bdd_ptr prime, notprime;

    prime = bdd_compute_prime_low(dd, b, curr_low);
    array_insert_last(bdd_ptr, result, prime);
    /* */
    notprime = bdd_not(dd, prime);
    bdd_and_accumulate(dd, &curr_low, notprime);
    bdd_free(dd, notprime);
  }
  bdd_free(dd, curr_low);
  return result;
}

/**Function********************************************************************

  Synopsis    [Finds a set of prime implicants for a BDD.]

  Description [Finds the set of prime implicants of a BDD b.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
array_t * bdd_compute_primes(DdManager * dd, bdd_ptr b) {
  array_t * result;
  bdd_ptr low = bdd_dup(b);

  result = bdd_compute_primes_low(dd, b, low);
  bdd_free(dd, low);

  return result;
}


/**Function********************************************************************

  Synopsis    [Finds the essential variables of a DD.]

  Description [Returns the cube of the essential variables. A positive
  literal means that the variable must be set to 1 for the function to be
  1. A negative literal means that the variable must be set to 0 for the
  function to be 1. Returns a pointer to the cube BDD if successful;
  NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
bdd_ptr bdd_compute_essentials(DdManager *dd, bdd_ptr b) {
  DdNode * result;

  result = Cudd_FindEssential(dd, b);
  common_error(result, "bdd_compute_essentials: result = NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Determines whether f is less than or equal to g.]

  Description [Returns 1 if f is less than or equal to g; 0 otherwise.
  No new nodes are created.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int bdd_leq(DdManager *dd, bdd_ptr f, bdd_ptr g) {
  return(Cudd_bddLeq(dd, f, g));
}

/**Function********************************************************************

  Synopsis [Swaps two sets of variables of the same size (x and y) in
  the BDD f.]

  Description [Swaps two sets of variables of the same size (x and y)
  in the BDD f. The size is given by n. The two sets of variables are
  assumed to be disjoint.  Returns a pointer to the resulting BDD if
  successful; an error (which either results in a jump to the last CATCH-FAIL
  block, or in a call to exit()) is triggered otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
bdd_ptr bdd_swap_variables(DdManager *dd, bdd_ptr f,
                           bdd_ptr *x_varlist,
                           bdd_ptr *y_varlist,
                           int n) {

  DdNode *result;

  result = Cudd_bddSwapVariables(dd, f, x_varlist, y_varlist, n);
  common_error(result, "bdd_swap_variables: result == NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Writes a blif file representing the argument BDDs.]

  Description [Writes a blif file representing the argument BDDs as a
  network of multiplexers. One multiplexer is written for each BDD
  node. It returns 1 in case of success; 0 otherwise (e.g.,
  out-of-memory, file system full, or an ADD with constants different
  from 0 and 1).  bdd_DumpBlif does not close the file: This is the
  caller responsibility. bdd_DumpBlif uses a minimal unique subset of
  the hexadecimal address of a node as name for it.  If the argument
  inames is non-null, it is assumed to hold the pointers to the names
  of the inputs. Similarly for onames.]

  SideEffects [None]

  SeeAlso     [bdd_DumpBlifBody dd_dump_dot]

******************************************************************************/
int bdd_DumpBlif(
  DdManager* dd     /* manager */,
  int n             /* number of output nodes to be dumped */,
  bdd_ptr* f        /* array of output nodes to be dumped */,
  char** inames     /* array of input names (or NULL) */,
  char** onames     /* array of output names (or NULL) */,
  char* mname       /* model name (or NULL) */,
  FILE* fp          /* pointer to the dump file */)
{
  return(Cudd_DumpBlif(dd, n, f, inames, onames, mname, fp));
}

/**Function********************************************************************

  Synopsis    [Writes a blif body representing the argument BDDs.]

  Description [Writes a blif body representing the argument BDDs as a
  network of multiplexers.  No header (.model, .inputs, and .outputs) and
  footer (.end) are produced by this function.  One multiplexer is written
  for each BDD node. It returns 1 in case of success; 0 otherwise (e.g.,
  out-of-memory, file system full, or an ADD with constants different
  from 0 and 1).  bdd_DumpBlifBody does not close the file: This is the
  caller responsibility. bdd_DumpBlifBody uses a minimal unique subset of
  the hexadecimal address of a node as name for it.  If the argument
  inames is non-null, it is assumed to hold the pointers to the names
  of the inputs. Similarly for onames. This function prints out only
  .names part.]

  SideEffects [None]

  SeeAlso     [bdd_DumpBlif dd_dump_dot]

******************************************************************************/
int bdd_DumpBlifBody(
  DdManager* dd     /* manager */,
  int  n            /* number of output nodes to be dumped */,
  bdd_ptr* f        /* array of output nodes to be dumped */,
  char** inames     /* array of input names (or NULL) */,
  char** onames     /* array of output names (or NULL) */,
  FILE* fp          /* pointer to the dump file */)
{
  return(Cudd_DumpBlifBody(dd, n, f, inames, onames, fp));
}

/**Function********************************************************************

  Synopsis    [Substitutes g for x_v in the BDD for f.]

  Description [Substitutes g for x_v in the BDD for f. v is the index of the
  variable to be substituted. bdd_compose passes the corresponding
  projection function to the recursive procedure, so that the cache may
  be used.  Returns the composed BDD if successful; an error (which either
  results in a jump to the last CATCH-FAIL  block, or in a call to exit())
  is triggered otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
bdd_ptr bdd_compose(DdManager *dd, bdd_ptr f, bdd_ptr g, int v) {

  DdNode *result;

  result = Cudd_bddCompose(dd, f, g, v);
  common_error(result, "bdd_compose: result == NULL");
  Cudd_Ref(result);
  return((bdd_ptr)result);
}

/**Function********************************************************************

  Synopsis    [Returns the reference count of a node.]

  Description [Returns the reference count of a node. The node pointer can be
  either regular or complemented.]

  SideEffects [None]

  SeeAlso []

******************************************************************************/
int bdd_ref_count(bdd_ptr n)
{
  int result;
  DdNode *N;

  N = Cudd_Regular((DdNode*)n);
  common_error(N, "Could not make node regular, while getting ref-count.");
  result = N->ref;
  return(result);
}

/**Function********************************************************************

  Synopsis    [Computes the value of a function with given variable values.]

  Description [Computes the value (0 or 1) of the given function with the given
  values for variables. The parameter "values" must be an array, at least as
  long as the number of indices in the BDD.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int calculate_bdd_value(DdManager* mgr, bdd_ptr f, int* values)
{
  unsigned char invert = Cudd_IsComplement(f) ? 1u : 0u;
  while (!Cudd_IsConstant(f)) {
    if (values[(Cudd_Regular(f))->index]) f = Cudd_T(f);
    else f = Cudd_E(f);
    invert ^= (Cudd_IsComplement(f) ? 1u : 0u);
  }
  return(invert ? 0 : 1);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis [Function to print a warning that an illegal value was read.]

  SideEffects        []

  SeeAlso            [bdd_set_parameters]

******************************************************************************/
static void InvalidType(FILE *file, char *field, char *expected)
{
    fprintf(file, "Warning: In parameter \"%s\"\n", field);
    fprintf(file, "Illegal type detected. %s expected\n", expected);

} /* end of InvalidType */
