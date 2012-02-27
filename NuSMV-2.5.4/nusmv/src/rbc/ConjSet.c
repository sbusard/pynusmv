/**CFile***********************************************************************

  FileName    [ConjSet.c]

  PackageName [rbc]

  Synopsis    [Implementaion of class 'ConjSet']

  Description [A ConjSet holds the associations between variables
	and the corresponding expression each variable can be
	substituted with.  A ConjSet is internally used by RBC
	inlining. In particular it is used by class InlineResult]

  SeeAlso     [ConjSet.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2.
  Copyright (C) 2007 by FBK-irst.

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

  Revision    [$Id: ConjSet.c,v 1.1.2.8 2010-02-08 09:12:51 nusmv Exp $]

******************************************************************************/

#include "ConjSet.h"
#include "rbcInt.h"

#include "utils/Slist.h"
#include "utils/assoc.h"
#include "utils/utils.h"

static char rcsid[] UTIL_UNUSED = "$Id: ConjSet.c,v 1.1.2.8 2010-02-08 09:12:51 nusmv Exp $";


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

  Synopsis    [ConjSet class definition]

  Description [A ConjSet holds the associations between variables
	and the corresponding expression each variable can be substituted with.
	A ConjSet is internally used by RBC inlining. In particular it is used
	by class InlineResult]

  SeeAlso     []

******************************************************************************/
typedef struct ConjSet_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  Rbc_Manager_t* mgr;
  hash_ptr set;
  node_ptr set_keys;
  Rbc_t** sarr; /* array for substitution */
  int nvars; /* number of allocated vars into sarr */
} ConjSet;


/**Struct**********************************************************************

  Synopsis    [ConjElem class definition]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct ConjElem_TAG
{
  Rbc_t* expr;  /* unflattened expression */
  Rbc_t* fexpr; /* flattened expression   */
  Slist_ptr deps;  /* unflattened expression dependencies */
} ConjElem;

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class ConjElem]

  Description []

******************************************************************************/
typedef struct ConjElem_TAG*  ConjElem_ptr;

/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class ConjElem]

  Description [These macros must be used respectively to cast and to check
  instances of class ConjElem]

******************************************************************************/
#define CONJ_ELEM(self) \
         ((ConjElem_ptr) self)

#define CONJ_ELEM_CHECK_INSTANCE(self) \
         (nusmv_assert(CONJ_ELEM(self) != CONJ_ELEM(NULL)))



/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE* nusmv_stderr;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void conj_set_init ARGS((ConjSet_ptr self, Rbc_Manager_t* mgr));
static void conj_set_deinit ARGS((ConjSet_ptr self));
static void conj_set_copy ARGS((const ConjSet_ptr self, ConjSet_ptr copy));

static void conj_set_flattenize ARGS((ConjSet_ptr self,
				      Rbc_t* var, hash_ptr pvars));

static boolean
conj_set_insert_element ARGS((ConjSet_ptr self, Rbc_t* var, ConjElem_ptr el));

static void
conj_elem_init ARGS((ConjElem_ptr self, Rbc_Manager_t* mgr, Rbc_t* expr));
static void conj_elem_deinit ARGS((ConjElem_ptr self));

static ConjElem_ptr ConjElem_create ARGS((Rbc_Manager_t* mgr, Rbc_t* expr));
static void ConjElem_destroy ARGS((ConjElem_ptr self));
static ConjElem_ptr ConjElem_copy ARGS((const ConjElem_ptr self));
static boolean ConjElem_is_smaller ARGS((const ConjElem_ptr self,
					 const ConjElem_ptr other));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The ConjSet class constructor]

  Description        [The ConjSet class constructor]

  SideEffects        []

  SeeAlso            [ConjSet_destroy]

******************************************************************************/
ConjSet_ptr ConjSet_create(Rbc_Manager_t* rbcm)
{
  ConjSet_ptr self = ALLOC(ConjSet, 1);
  CONJ_SET_CHECK_INSTANCE(self);

  conj_set_init(self, rbcm);
  return self;
}


/**Function********************************************************************

  Synopsis           [Copy constructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ConjSet_ptr ConjSet_copy(const ConjSet_ptr self)
{
  ConjSet_ptr copy;
  CONJ_SET_CHECK_INSTANCE(self);

  copy = ALLOC(ConjSet, 1);
  CONJ_SET_CHECK_INSTANCE(copy);

  conj_set_copy(self, copy);
  return copy;
}


/**Function********************************************************************

  Synopsis           [The ConjSet class destructor]

  Description        [The ConjSet class destructor]

  SideEffects        []

  SeeAlso            [ConjSet_create]

******************************************************************************/
void ConjSet_destroy(ConjSet_ptr self)
{
  CONJ_SET_CHECK_INSTANCE(self);

  conj_set_deinit(self);
  FREE(self);
}



/**Function********************************************************************

  Synopsis           [Adds a new variable assignment to set]

  Description [Will be kept onlt if 'better' then the possibly
  previous assigment]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ConjSet_add_var_assign(ConjSet_ptr self, Rbc_t* var, Rbc_t* expr)
{
  ConjElem_ptr _new;

  CONJ_SET_CHECK_INSTANCE(self);

  _new = ConjElem_create(self->mgr, expr);
  if (!conj_set_insert_element(self, var, _new)) {
    ConjElem_destroy(_new);
  }
}


/**Function********************************************************************

  Synopsis [Inherits as much as possible (provided that what it
  inherits is not worse than what it has collected so far) from the given
  ConjSet]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ConjSet_inherit_from(ConjSet_ptr self, const ConjSet_ptr other)
{
  node_ptr iter;
  CONJ_SET_CHECK_INSTANCE(self);

  for (iter=other->set_keys; iter != Nil; iter=cdr(iter)) {
    ConjElem_ptr el;
    Rbc_t* var;

    var = (Rbc_t*) car(iter);
    el = ConjElem_copy(CONJ_ELEM(find_assoc(other->set, (node_ptr) var)));

    if (!conj_set_insert_element(self, var, el)) {
      ConjElem_destroy(el);
    }
  }
}


/**Function********************************************************************

  Synopsis           [Makes the ConjSet flattened.]

  Description [Flattens the ConjSet, making minimal the graph of
  dependencies. A flatten ConjSet can then be used to substitute
  an expression]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ConjSet_flattenize(ConjSet_ptr self)
{
  hash_ptr pvars;
  int nvars, i;

  CONJ_SET_CHECK_INSTANCE(self);

  nusmv_assert(self->sarr == (Rbc_t**) NULL);

  nvars = Rbc_ManagerCapacity(self->mgr);
  self->sarr = ALLOC(Rbc_t*, nvars);
  nusmv_assert(self->sarr != (Rbc_t**) NULL);

  self->nvars = nvars;

  /* an hash to remember already processed vars */
  pvars = new_assoc();
  nusmv_assert(pvars != (hash_ptr) NULL);

  /* identity array */
  for (i=0; i<nvars; ++i) { self->sarr[i] = Rbc_GetIthVar(self->mgr, i); }

  {
    node_ptr iter;
    for (iter=self->set_keys; iter != Nil; iter=cdr(iter)) {
      Rbc_t* var = (Rbc_t*) car(iter);
      conj_set_flattenize(self, var, pvars);
    }
  }

  free_assoc(pvars);
}



/**Function********************************************************************

  Synopsis [Substitutes all variables occurring into f that belong
	to self with the corresponding expression.]

  Description [If self was previously flattened, the resulting RBC
	will be flattened as well, but only about those parts that has
	the same language of self]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Rbc_t* ConjSet_substitute(ConjSet_ptr self, Rbc_t* f)
{
  CONJ_SET_CHECK_INSTANCE(self);
  nusmv_assert(self->sarr != (Rbc_t**) NULL);

  return Rbc_SubstRbc(self->mgr, f, self->sarr);
}


/**Function********************************************************************

  Synopsis           [Returns the conjuction of self with the given formula]

  Description        [Returns a formula like:
	    for all v,exp belonging to self,
          (/\ (v <-> exp)) /\ f
	]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Rbc_t* ConjSet_conjoin(ConjSet_ptr self, Rbc_t* f)
{
  Rbc_t* res;
  node_ptr iter;

  CONJ_SET_CHECK_INSTANCE(self);

  res = f;
  for (iter=self->set_keys; iter != Nil; iter=cdr(iter)) {
    ConjElem_ptr el;
    Rbc_t *exp, *var, *iff;
    nusmv_ptrint sign;

    var = (Rbc_t*) car(iter);
    el = CONJ_ELEM(find_assoc(self->set, (node_ptr) var));

    /* use the flatttened version when available */
    exp = (el->fexpr != (Rbc_t*) NULL) ? el->fexpr : el->expr;
    sign = Dag_VertexIsSet(exp);
    Dag_VertexClear(exp);

    iff = Rbc_MakeIff(self->mgr, var, exp, sign);
    res = Rbc_MakeAnd(self->mgr, res, iff, RBC_TRUE);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Prints debugging information about self]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ConjSet_print(const ConjSet_ptr self, FILE* file)
{
  node_ptr iter;

  CONJ_SET_CHECK_INSTANCE(self);

  fprintf(file, "ConjSet:\n");

  for (iter=self->set_keys; iter != Nil; iter=cdr(iter)) {
    ConjElem_ptr el;
    Rbc_t* var;

    var = (Rbc_t*) car(iter);
    el = CONJ_ELEM(find_assoc(self->set, (node_ptr) var));

    Rbc_OutputSexpr(self->mgr, var, file);
    fprintf(file, " := ");
    Rbc_OutputSexpr(self->mgr, el->expr, file);
    fprintf(file, "\n");
    if (el->fexpr != (Rbc_t*) NULL) {
      fprintf(file, "(flat) := ");
      Rbc_OutputSexpr(self->mgr, el->fexpr, file);
      fprintf(file, "\n");
    }
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The ConjSet class private initializer]

  Description        [The ConjSet class private initializer]

  SideEffects        []

  SeeAlso            [ConjSet_create]

******************************************************************************/
static void conj_set_init(ConjSet_ptr self, Rbc_Manager_t* mgr)
{
  /* members initialization */
  self->mgr = mgr;
  self->set = new_assoc();
  self->set_keys = Nil;
  self->sarr = (Rbc_t**) NULL;
  self->nvars = 0;
}


/**Function********************************************************************

  Synopsis           [The ConjSet class private copy constructor]

  Description        [The ConjSet class private copy constructor]

  SideEffects        []

  SeeAlso            [ConjSet_copy]

******************************************************************************/
static void conj_set_copy(const ConjSet_ptr self, ConjSet_ptr copy)
{
  node_ptr iter;

  conj_set_init(copy, self->mgr);

  /* adjusts wrt init */
  for (iter=self->set_keys; iter != Nil; iter=cdr(iter)) {
    ConjElem_ptr el;
    Rbc_t* var;

    var = (Rbc_t*) car(iter);
    el = ConjElem_copy(CONJ_ELEM(find_assoc(self->set, (node_ptr) var)));

    insert_assoc(copy->set, (node_ptr) var, (node_ptr) el);
    copy->set_keys = cons((node_ptr) var, copy->set_keys);
  }

  copy->nvars = self->nvars;
  if (self->sarr != (Rbc_t**) NULL) {
    int i;

    copy->sarr = ALLOC(Rbc_t*, copy->nvars);
    nusmv_assert(copy->sarr != (Rbc_t**) NULL);
    for (i=0; i<copy->nvars; ++i) { copy->sarr[i] = self->sarr[i]; }
  }
}


/* a service for the deinititalizer */
static assoc_retval _destroy_conj_entry(char* key, char* _elem, char* arg)
{
  if (_elem != (char*) NULL) ConjElem_destroy(CONJ_ELEM(_elem));
  return ST_DELETE;
}


/**Function********************************************************************

  Synopsis           [The ConjSet class private deinitializer]

  Description        [The ConjSet class private deinitializer]

  SideEffects        []

  SeeAlso            [ConjSet_destroy]

******************************************************************************/
static void conj_set_deinit(ConjSet_ptr self)
{
  /* members deinitialization */
  clear_assoc_and_free_entries(self->set, _destroy_conj_entry);
  free_assoc(self->set);
  self->set = (hash_ptr) NULL;
  free_list(self->set_keys);
  if (self->sarr != (Rbc_t**) NULL) FREE(self->sarr);
}


/**Function********************************************************************

  Synopsis           [Private service for ConjSet_flattenize]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void conj_set_flattenize(ConjSet_ptr self, Rbc_t* var, hash_ptr pvars)
{
  ConjElem_ptr el;
  int idx;

  /* already processed? */
  if (find_assoc(pvars, (node_ptr) var) == (node_ptr) 1) return;
  insert_assoc(pvars, (node_ptr) var, (node_ptr) 1);

  el = CONJ_ELEM(find_assoc(self->set, (node_ptr) var));
  if (el == CONJ_ELEM(NULL)) {
    /* no association, keeps the identity */
    return;
  }

  idx = Rbc_GetVarIndex(var);
  nusmv_assert(idx >= 0);
  nusmv_assert(idx < Rbc_ManagerCapacity(self->mgr));

  /* base case, no dependencies */
  if (Slist_get_size(el->deps) == 0) {
    el->fexpr = el->expr;
  }
  else { /* resolves first all dependencies */
    Siter iter;
    void * data;

    SLIST_FOREACH(el->deps, iter) {
      data = Siter_element(iter);
      conj_set_flattenize(self, (Rbc_t*) data, pvars);
    }

    el->fexpr = Rbc_SubstRbc(self->mgr, el->expr, self->sarr);
  }

  self->sarr[idx] = el->fexpr;
  return;
}


/**Function********************************************************************

  Synopsis [Inserts 'el' only if it is better then what it has been
  collected so far]

  Description        [Returns true if el has been accepted, false otherwise.
  If false is returned, it is likely that the caller needs to destroy el]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean
conj_set_insert_element(ConjSet_ptr self, Rbc_t* var, ConjElem_ptr el)
{
  ConjElem_ptr _old;

  _old = CONJ_ELEM(find_assoc(self->set, (node_ptr) var));

  /* the same element */
  if (el == _old) return true;

  /* Previoulsy assigned: keeps the smaller */
  if (_old != CONJ_ELEM(NULL)) {
    if (ConjElem_is_smaller(_old, el)) return false;

    /* old must be substituted by el */
    ConjElem_destroy(_old);
  }
  else {
    /* adds the new key to the keys list */
    self->set_keys = cons((node_ptr) var, self->set_keys);
  }

  insert_assoc(self->set, (node_ptr) var, (node_ptr) el);
  return true;
}


/**Function********************************************************************

  Synopsis           [The ConjElem class constructor]

  Description        [The ConjElem class constructor]

  SideEffects        []

  SeeAlso            [ConjElem_destroy]

******************************************************************************/
static ConjElem_ptr ConjElem_create(Rbc_Manager_t* mgr, Rbc_t* expr)
{
  ConjElem_ptr self = ALLOC(ConjElem, 1);
  CONJ_ELEM_CHECK_INSTANCE(self);

  conj_elem_init(self, mgr, expr);

  return self;
}


/**Function********************************************************************

  Synopsis           [The ConjElem class destructor]

  Description        [The ConjElem class destructor]

  SideEffects        []

  SeeAlso            [ConjElem_create]

******************************************************************************/
static void ConjElem_destroy(ConjElem_ptr self)
{
  CONJ_ELEM_CHECK_INSTANCE(self);

  conj_elem_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Copy constructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static ConjElem_ptr ConjElem_copy(const ConjElem_ptr self)
{
  CONJ_ELEM_CHECK_INSTANCE(self);

  ConjElem_ptr copy = ALLOC(ConjElem, 1);
  CONJ_ELEM_CHECK_INSTANCE(copy);

  copy->expr = self->expr;
  copy->fexpr = self->fexpr;
  copy->deps = Slist_copy(self->deps);

  return copy;
}


/**Function********************************************************************

  Synopsis           [Predicate to decide whether a ConjElem_ptr is better
	than another]

  Description [The better ConjElem has less dependencies that the
  worse]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean ConjElem_is_smaller(const ConjElem_ptr self,
				   const ConjElem_ptr other)
{
  CONJ_ELEM_CHECK_INSTANCE(self);
  return Slist_get_size(self->deps) < Slist_get_size(other->deps);
}


/**Function********************************************************************

  Synopsis           [The ConjElem class private initializer]

  Description        [The ConjElem class private initializer]

  SideEffects        []

  SeeAlso            [ConjElem_create]

******************************************************************************/
static void conj_elem_init(ConjElem_ptr self, Rbc_Manager_t* mgr, Rbc_t* expr)
{
  /* members initialization */
  self->expr = expr;
  self->fexpr = (Rbc_t*) NULL;
  self->deps = RbcUtils_get_dependencies(mgr, expr, false);
}


/**Function********************************************************************

  Synopsis           [The ConjElem class private deinitializer]

  Description        [The ConjElem class private deinitializer]

  SideEffects        []

  SeeAlso            [ConjElem_destroy]

******************************************************************************/
static void conj_elem_deinit(ConjElem_ptr self)
{
  /* members deinitialization */
  Slist_destroy(self->deps);
}

/**AutomaticEnd***************************************************************/

