/**CFile***********************************************************************

  FileName    [InlineResult.c]

  PackageName [rbc]

  Synopsis    [Implementation of class 'InlineResult']

  Description []

  SeeAlso     [InlineResult.h]

  Author      [Roberto Cavada, Michele Dorigatti]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2.
  Copyright (C) 2007 by FBK-irst.
  Copyright (C) 2011 by FBK.

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

  Revision    [$Id: InlineResult.c,v 1.1.2.7 2007-04-04 12:00:14 nusmv Exp $]

******************************************************************************/
#include "InlineResult.h"
#include "ConjSet.h"
#include "rbcInt.h"
#include "utils/utils.h"
#include "utils/error.h"
/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
#define RBC_MAX_OUTDEGREE 2
extern FILE* nusmv_stderr;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/**Struct**********************************************************************

  Synopsis    [InlineResult class definition]

  Description [The class represents the result that is returned by
  rbc inlining. The result is lazy, meaning that a minimal result
        is initially calculated during inlining, and all other results
        are calculated and cached when they are requested.

        After an InlienResult is created on a given formula f, it is
  possible to query for:
        - the original formula f
  - inlined fomula fin
  - the conjuction set c
  - the inlined fin conjucted with c
        ]

  SeeAlso     []

******************************************************************************/
typedef struct InlineResult_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  Rbc_Manager_t* mgr;
  ConjSet_ptr conj;
  Rbc_t* f;      /* original f */
  Rbc_t* fns;    /* f after C construction, not substituted by C */
  Rbc_t* fin;    /* fns substituted by C */
  Rbc_t* finc;   /* fin & C */
  Rbc_t* c;      /* Big AND of conj */
} InlineResult;


/**Struct**********************************************************************
  Synopsis      [Data passing in inlining-DFS ]
  Description   [Data passing in inlining-DFS ]
  SeeAlso       []
******************************************************************************/
typedef struct InlineDfsData_TAG {
  Rbc_Manager_t* mgr;
  InlineResult_ptr res;
  Rbc_t* tmp_res;
} InlineDfsData;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void
inline_result_init ARGS((InlineResult_ptr self,
                         Rbc_Manager_t* rbcm, Rbc_t* f));

static void inline_result_deinit ARGS((InlineResult_ptr self));

static void
inline_result_copy ARGS((const InlineResult_ptr self, InlineResult_ptr copy));

static void inline_result_calc_cset ARGS((InlineResult_ptr self));


static int inline_set ARGS((Rbc_t* f, char* _data, nusmv_ptrint sign));
static void inline_first ARGS((Rbc_t* f, char* _data, nusmv_ptrint sign));
static void inline_back ARGS((Rbc_t* f, char* _data, nusmv_ptrint sign));
static void inline_last ARGS((Rbc_t* f, char* _data, nusmv_ptrint sign));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The InlineResult class constructor]

  Description        [The InlineResult class constructor]

  SideEffects        []

  SeeAlso            [InlineResult_destroy]

******************************************************************************/
InlineResult_ptr InlineResult_create(Rbc_Manager_t* mgr, Rbc_t* f)
{
  InlineResult_ptr self = ALLOC(InlineResult, 1);
  INLINE_RESULT_CHECK_INSTANCE(self);

  inline_result_init(self, mgr, f);
  return self;
}


/**Function********************************************************************

  Synopsis           [The InlineResult class copy constructor]

  Description        [The InlineResult class copy constructor]

  SideEffects        []

  SeeAlso            [InlineResult_destroy]

******************************************************************************/
InlineResult_ptr InlineResult_copy(const InlineResult_ptr self)
{
  InlineResult_ptr copy = ALLOC(InlineResult, 1);
  INLINE_RESULT_CHECK_INSTANCE(self);

  inline_result_copy(self, copy);
  return copy;
}

/**Function********************************************************************

  Synopsis           [The InlineResult class destructor]

  Description        [The InlineResult class destructor]

  SideEffects        []

  SeeAlso            [InlineResult_create]

******************************************************************************/
void InlineResult_destroy(InlineResult_ptr self)
{
  INLINE_RESULT_CHECK_INSTANCE(self);

  inline_result_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Returns the original formula f that was submitted to
                      the constructor]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Rbc_t* InlineResult_get_original_f(InlineResult_ptr self)
{
  INLINE_RESULT_CHECK_INSTANCE(self);
  return self->f;
}


/**Function********************************************************************

  Synopsis [Returns the inlined formula, _without_ the conjuction
  set]

  Description [A lazy approach to SAT can exploit the fact that if
  a model satisfies inlined f, then it satisfies f as well. If a
  counterexample must be shown though, inlined f must be conjuct
  with the conjuction set.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Rbc_t* InlineResult_get_inlined_f(InlineResult_ptr self)
{
  INLINE_RESULT_CHECK_INSTANCE(self);
  return self->fin;
}


/**Function********************************************************************

  Synopsis           [Returns the inlined f conjucted with the conjuction set]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Rbc_t* InlineResult_get_inlined_f_and_c(InlineResult_ptr self)
{
  INLINE_RESULT_CHECK_INSTANCE(self);
  if (self->finc == (Rbc_t*) NULL) {
    self->finc = Rbc_MakeAnd(self->mgr,
                             InlineResult_get_c(self),
                             InlineResult_get_inlined_f(self),
                             RBC_TRUE);
  }

  return self->finc;
}


/**Function********************************************************************

  Synopsis           [Returns a formula representing the conjuction set]

  Description        [The conjuction set is made into a formula like:

        foreach (v,exp) belonging to the conjuction set,

  (\/ v <-> exp) \/ inlined_f
        ]

  SideEffects        []

  SeeAlso            [ConjSet]

******************************************************************************/
Rbc_t* InlineResult_get_c(InlineResult_ptr self)
{
  INLINE_RESULT_CHECK_INSTANCE(self);
  if (self->c == (Rbc_t*) NULL) {
    self->c = ConjSet_conjoin(self->conj, Rbc_GetOne(self->mgr));
  }

  return self->c;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The InlineResult class private initializer]

  Description        [The InlineResult class private initializer]

  SideEffects        []

  SeeAlso            [InlineResult_create]

******************************************************************************/
static void
inline_result_init(InlineResult_ptr self, Rbc_Manager_t* rbcm, Rbc_t* f)
{
  /* members initialization */
  self->mgr = rbcm;
  self->conj = ConjSet_create(rbcm);
  self->f = f;
  self->fns = (Rbc_t*) NULL;;
  self->fin = (Rbc_t*) NULL;
  self->finc = (Rbc_t*) NULL;
  self->c = (Rbc_t*) NULL;

  /* performs the C set construction and inlining */
  inline_result_calc_cset(self);

  self->fin = ConjSet_substitute(self->conj, self->fns);
}


/**Function********************************************************************

  Synopsis           [The InlineResult class private copy constructor]

  Description        [The InlineResult class private copy constructor]

  SideEffects        []

  SeeAlso            [InlineResult_copy]

******************************************************************************/
static void
inline_result_copy(const InlineResult_ptr self, InlineResult_ptr copy)
{
  /* members initialization */
  copy->mgr = self->mgr;
  copy->conj = ConjSet_copy(self->conj);
  copy->f = self->f;
  copy->fns = self->fns;
  copy->fin = self->fin;
  copy->finc = self->finc;
  copy->c = self->c;
}


/**Function********************************************************************

  Synopsis           [The InlineResult class private deinitializer]

  Description        [The InlineResult class private deinitializer]

  SideEffects        []

  SeeAlso            [InlineResult_destroy]

******************************************************************************/
static void inline_result_calc_cset(InlineResult_ptr self)
{
  Dag_DfsFunctions_t funcs;
  InlineDfsData data;

  /* clears the user fields. */
  Dag_Dfs(self->f, &dag_DfsClean, (char*) NULL);

  /* sets up the DFS functions */
  funcs.Set        = inline_set;
  funcs.FirstVisit = inline_first;
  funcs.BackVisit  = inline_back;
  funcs.LastVisit  = inline_last;

  /* sets up data */
  data.mgr = self->mgr;
  data.res = self;

  /* Calling DFS on f. */
  Dag_Dfs(self->f, &funcs, (char*)(&data));

  /* flattenizes the produced ConjSet */
  ConjSet_flattenize(self->conj);

  self->fns = data.tmp_res;
}


/**Function********************************************************************

  Synopsis           [The InlineResult class private deinitializer]

  Description        [The InlineResult class private deinitializer]

  SideEffects        []

  SeeAlso            [InlineResult_destroy]

******************************************************************************/
static void inline_result_deinit(InlineResult_ptr self)
{
  /* members deinitialization */
  ConjSet_destroy(self->conj);
}



/**Function********************************************************************

  Synopsis    [Checks for a var assignment and adds it to the conjuction set]

  Description [First, looks for the rbc f in cache and eventually gets the
               previous result.
               Else, checks the symbol of f for an IFF or for its two
               level AND equivalent. In these cases if finds a var assignment
               (one vertex being a VAR and the other not), adds it to the
               conjunction set.
               Returns 1 for backtracking, -1 if it does not need.]

  SideEffects [data->tmp_res is always set, except when the symbol of f is
               RBCVAR or a not negated RBCAND.
               if a var assignment is found, _data->res->conj is updated.]

  SeeAlso     []

******************************************************************************/
static int inline_set(Rbc_t* f, char* _data, nusmv_ptrint sign)
{
  InlineDfsData* data = (InlineDfsData*) _data;

  { /* reuses previous result searching in cache */
    InlineResult_ptr cir = rbc_inlining_cache_lookup_result(RbcId(f, sign));
    if (cir != INLINE_RESULT(NULL)) {
      ConjSet_inherit_from(data->res->conj, cir->conj);
      data->tmp_res = cir->fns;
      return 1; /* backtrack */
    }
  }

  switch (f->symbol) {
  case RBCTOP:
  case RBCVAR:
    return -1; /* continue (will be handled by other handler) */

  case RBCAND:
    /* possible inlining, check for an IFF equivalence */
    /* (~s)AND(~AND(x,y),~AND(~x,~y)) = (s)IFF(x,y) */
    {

#if !RBC_ENABLE_ITE_CONNECTIVE
      Rbc_t* left = RBC_GET_LEFTMOST_CHILD(f);
      Rbc_t* right = RBC_GET_SECOND_CHILD(f);
      if ((RbcGetRef(left)->symbol == RBCAND) &&
          (RbcGetRef(right)->symbol == RBCAND) &&
          (RbcIsSet(left)) &&
          (RbcIsSet(right)))
        {
          Rbc_t* l1 = RBC_GET_LEFTMOST_CHILD(left);
          Rbc_t* l2 = RBC_GET_SECOND_CHILD(left);
          Rbc_t* r1 = RBC_GET_LEFTMOST_CHILD(right);
          Rbc_t* r2 = RBC_GET_SECOND_CHILD(right);
          if (((l1 == RbcId(r1, RBC_FALSE)) || (l1 == RbcId(r2, RBC_FALSE))) &&
              ((l2 == RbcId(r1, RBC_FALSE)) || (l2 == RbcId(r2, RBC_FALSE))))
            {
              /* makes v <-> phi */
              if ((RbcGetRef(l1)->symbol != RBCVAR) &&
                  (RbcGetRef(l2)->symbol == RBCVAR))
                {
                  Rbc_t* tmp = l1;
                  l1 = l2; l2 = tmp;
                }
              /* is it a var assignment? */
              if (RbcGetRef(l1)->symbol == RBCVAR)
                {
                  /* no signed variables in cache */
                  if (RbcIsSet(l1))
                    {
                      /* move the "IFF's" sign to the right */
                      ConjSet_add_var_assign(data->res->conj,
                                             RbcId(l1, RBC_FALSE),
                                             RbcId(l2, sign));
                    }
                  else
                    {
                      /* move the "IFF's" sign to the right */
                      ConjSet_add_var_assign(data->res->conj,
                                             l1,
                                             RbcId(l2, sign ^ RBC_FALSE));
                    }
                  /* keeps it, will be substituted later */
                  data->tmp_res = RbcId(f, sign);
                  break;
                }
            }
        }
#endif
      /* AND, continue */
      if (sign == RBC_TRUE) return -1;
      /* OR, keep it, and backtrack */
      data->tmp_res = RbcId(f, sign);
      break;
    }

  case RBCIFF: {
    /* inlining, collects the IFF and removes it */
    Rbc_t* left = RBC_GET_LEFTMOST_CHILD(f);
    Rbc_t* right = RBC_GET_SECOND_CHILD(f);

    if (RbcGetRef(left)->symbol != RBCVAR &&
        RbcGetRef(right)->symbol == RBCVAR) { /* makes v <-> phi */
      Rbc_t* tmp = left;
      left = right; right = tmp;
    }

    /* is it a var assignment? */
    if (RbcGetRef(left)->symbol == RBCVAR) {
      /* move the IFF's sign to the right */
      ConjSet_add_var_assign(data->res->conj, left, RbcId(right, sign));
    }

    /* keeps it, will be subtituted later */
    data->tmp_res = RbcId(f, sign);
    break;
  }

  case RBCITE:
    data->tmp_res = RbcId(f, sign);
    break;

  default:
    error_unreachable_code(); /* no other cases */
  }

  return 1; /* backtrack */
}


/**Function********************************************************************

  Synopsis    [DFS private function]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void inline_first(Rbc_t* f, char* _data, nusmv_ptrint sign)
{
  nusmv_assert(f->symbol == RBCAND || f->symbol == RBCVAR ||
               f->symbol == RBCTOP);

  /* Reset the counter for the sons list */
  if (f->symbol == RBCAND) {
    f->gRef = (char*) ALLOC(Rbc_t*, RBC_MAX_OUTDEGREE);
    f->iRef = 0;
  }
}


/**Function********************************************************************

  Synopsis    [DFS private function]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void inline_back(Rbc_t* f, char* _data, nusmv_ptrint sign)
{
  InlineDfsData* data = (InlineDfsData*) _data;

  nusmv_assert(f->symbol == RBCAND || f->symbol == RBCTOP);

  /* Get the current result and add it to the temp list of sons */
  if (f->symbol == RBCAND) {
    nusmv_assert(f->iRef < RBC_MAX_OUTDEGREE && f->iRef >= 0);
    nusmv_assert(data->tmp_res != (Rbc_t*) NULL);
    ((Rbc_t**)(f->gRef))[(f->iRef)++] = data->tmp_res;
  }
}


/**Function********************************************************************

  Synopsis    [DFS private function]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void inline_last(Rbc_t* f, char* _data, nusmv_ptrint sign)
{
  InlineDfsData* data = (InlineDfsData*) _data;
  switch (f->symbol) {
  case RBCVAR:
    ConjSet_add_var_assign(data->res->conj, f, RbcId(data->mgr->one, sign));
    data->tmp_res = RbcId(f, sign);
    break;

  case RBCAND: {
    Rbc_t** sons = (Rbc_t**)(f->gRef);
    nusmv_assert(f->iRef == 2);

    data->tmp_res = Rbc_MakeAnd(data->mgr, sons[0], sons[1], sign);

    FREE(sons);
    break;
  }

  default:
    error_unreachable_code(); /* no other possible cases */
  }

}

/**AutomaticEnd***************************************************************/

