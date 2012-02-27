/**CFile***********************************************************************

  FileName    [FairnessList.c]

  PackageName [fsm.bdd]

  Synopsis    [Declares the interface for the classes that contains fairness
  conditions]

  Description [Exported classes are:
  - FairnessList (pure, base class)
  - JusticeList
  - CompassionList 
  ]
  
  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``fsm.bdd'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst. 

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

#include "FairnessList.h"

#include "bddInt.h"
#include "utils/object_private.h"
#include "utils/error.h"
#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: FairnessList.c,v 1.1.2.5.4.1.6.1 2007-04-30 11:52:55 nusmv Exp $";


#define FAIRNESS_LIST_ITERATOR(x) \
       ((FairnessListIterator_ptr) x)

#define END_ITERATOR \
       FAIRNESS_LIST_ITERATOR(Nil)


typedef struct FairnessList_TAG 
{
  INHERITS_FROM(Object);

  FairnessListIterator_ptr first;
  DdManager* dd;

  /* ---------------------------------------------------------------------- */ 
  /*     Virtuals                                                           */
  /* ---------------------------------------------------------------------- */   

} FairnessList;


typedef struct JusticeList_TAG
{
  INHERITS_FROM(FairnessList);

} JusticeList; 


typedef struct CompassionList_TAG
{
  INHERITS_FROM(FairnessList);

} CompassionList; 

/* ---------------------------------------------------------------------- */


static void fairness_list_finalize ARGS((Object_ptr object, void* dummy));

static void fairness_list_init ARGS((FairnessList_ptr self, 
                                     DdManager* dd_manager));

static void fairness_list_deinit ARGS((FairnessList_ptr self));

static void fairness_list_deinit_aux ARGS((FairnessList_ptr self, 
                                           FairnessListIterator_ptr iter));

static void fairness_list_finalize ARGS((Object_ptr object, void* dummy));

static Object_ptr fairness_list_copy ARGS((const Object_ptr self));

static node_ptr 
fairness_list_copy_aux ARGS((const FairnessList_ptr self, 
                             const FairnessListIterator_ptr iter));


/**Function********************************************************************

  Synopsis     [Base class constructor]

  Description  []
  
  SideEffects  []

******************************************************************************/
FairnessList_ptr FairnessList_create(DdManager* dd_manager)
{
  FairnessList_ptr self = ALLOC(FairnessList, 1);
  FAIRNESS_LIST_CHECK_INSTANCE(self);

  fairness_list_init(self, dd_manager);
  
  return self;
}


/**Function********************************************************************

  Synopsis     [Use to start iteration]

  Description  []
  
  SideEffects  []

******************************************************************************/
FairnessListIterator_ptr FairnessList_begin(const FairnessList_ptr self)
{
  FAIRNESS_LIST_CHECK_INSTANCE(self);  
  return self->first;
}


boolean FairnessList_is_empty(const FairnessList_ptr self)
{
  FAIRNESS_LIST_CHECK_INSTANCE(self);  
  return self->first == END_ITERATOR;
}


/**Function********************************************************************

  Synopsis     [Use to check end of iteration]

  Description  []
  
  SideEffects  []

******************************************************************************/
boolean FairnessListIterator_is_end(const FairnessListIterator_ptr self)
{
  return (self == END_ITERATOR);
}


/**Function********************************************************************

  Synopsis     [use to iterate on an list iterator]

  Description  []
  
  SideEffects  []

******************************************************************************/
FairnessListIterator_ptr 
FairnessListIterator_next(const FairnessListIterator_ptr self) 
{
  FairnessListIterator_ptr res = END_ITERATOR;
  
  if (self != END_ITERATOR) {
    res = FAIRNESS_LIST_ITERATOR( cdr(NODE_PTR(self)) );
  }

  return res;
}


/**Function********************************************************************

  Synopsis     [Constructor for justice fairness constraints list]

  Description  [Call FairnessList_destroy to destruct self]
  
  SideEffects  []

******************************************************************************/
JusticeList_ptr JusticeList_create(DdManager* dd_manager)
{
  JusticeList_ptr self = ALLOC(JusticeList, 1);
  JUSTICE_LIST_CHECK_INSTANCE(self);

  fairness_list_init(FAIRNESS_LIST(self), dd_manager);

  return self;  
}


/**Function********************************************************************

  Synopsis     [Getter for BddStates pointed by given iterator]

  Description  [Returned bdd is referenced]
  
  SideEffects  []

******************************************************************************/
BddStates JusticeList_get_p(const JusticeList_ptr self, 
                            const FairnessListIterator_ptr iter)
{
  BddStates res; 
  node_ptr bdd;

  JUSTICE_LIST_CHECK_INSTANCE(self);
  nusmv_assert(iter != END_ITERATOR);

  bdd = car((node_ptr) iter);
  nusmv_assert(node_get_type(bdd) == BDD);
  res = BDD_STATES(car(bdd));
  bdd_ref((bdd_ptr) res);

  return res;
} 


/**Function********************************************************************

  Synopsis     [Appends the given bdd to the list]

  Description [Given bdd is referenced, so the caller should free it
  when it is no longer needed]
  
  SideEffects  []

******************************************************************************/
void JusticeList_append_p(JusticeList_ptr self, BddStates p)
{
  node_ptr new;

  JUSTICE_LIST_CHECK_INSTANCE(self);
  
  bdd_ref((bdd_ptr) p);

  new = new_node(BDD, (node_ptr) p, Nil);
  FAIRNESS_LIST(self)->first = cons((node_ptr) new, 
                                    FAIRNESS_LIST(self)->first);
}



/**Function********************************************************************

  Synopsis     [Creates the union of the two given fairness lists. Result
  goes into self]

  Description  []
  
  SideEffects  [self changes]

******************************************************************************/
void JusticeList_apply_synchronous_product(JusticeList_ptr self, 
                                           const JusticeList_ptr other)
{
  FairnessListIterator_ptr iter;

  JUSTICE_LIST_CHECK_INSTANCE(self);
  
  iter = FairnessList_begin(FAIRNESS_LIST(other));
  while (! FairnessListIterator_is_end(iter)) {
    BddStates p = JusticeList_get_p(other, iter);
    
    JusticeList_append_p(self, p);
    bdd_free( FAIRNESS_LIST(self)->dd, p );

    iter = FairnessListIterator_next(iter);
  } 
}



/**Function********************************************************************

  Synopsis     [Constructor for compassion fairness constraints list]

  Description  [Call FairnessList_destroy to destruct self]
  
  SideEffects  []

******************************************************************************/
CompassionList_ptr CompassionList_create(DdManager* dd_manager)
{
  CompassionList_ptr self = ALLOC(CompassionList, 1);
  COMPASSION_LIST_CHECK_INSTANCE(self);

  fairness_list_init(FAIRNESS_LIST(self), dd_manager);

  return self;  
}


/**Function********************************************************************

  Synopsis     [Getter of left-side bdd pointed by given iterator]

  Description  [Returned bdd is referenced]
  
  SideEffects  []

******************************************************************************/
BddStates CompassionList_get_p(const CompassionList_ptr self, 
                               const FairnessListIterator_ptr iter)
{
  BddStates res;
  node_ptr couple;
  node_ptr bdd;

  COMPASSION_LIST_CHECK_INSTANCE(self);
  nusmv_assert(iter != END_ITERATOR);
  
  couple = car((node_ptr) iter);
  nusmv_assert(node_get_type(couple) == CONS);

  bdd = car(couple);
  nusmv_assert(node_get_type(bdd) == BDD);

  res = BDD_STATES(car(bdd));
  bdd_ref((bdd_ptr) res);

  return res;
} 


/**Function********************************************************************

  Synopsis     [Getter of right-side bdd pointed by given iterator]

  Description  [Returned bdd is referenced]
  
  SideEffects  []

******************************************************************************/
BddStates CompassionList_get_q(const CompassionList_ptr self, 
                               const FairnessListIterator_ptr iter)
{
  BddStates res;
  node_ptr couple;
  node_ptr bdd;

  COMPASSION_LIST_CHECK_INSTANCE(self);
  nusmv_assert(iter != END_ITERATOR);
  
  couple = car((node_ptr) iter);
  nusmv_assert(node_get_type(couple) == CONS);

  bdd = cdr(couple);
  nusmv_assert(node_get_type(bdd) == BDD);

  res = BDD_STATES(car(bdd));
  bdd_ref((bdd_ptr) res);

  return res;
} 


/**Function********************************************************************

  Synopsis     [Appends the given BDDs to the list]

  Description [Given bdds are referenced, so the caller should free it
  when it is no longer needed]
  
  SideEffects  []

******************************************************************************/
void CompassionList_append_p_q(CompassionList_ptr self, 
                               BddStates p, BddStates q)
{
  node_ptr bdd_l, bdd_r;
  node_ptr couple;
  
  COMPASSION_LIST_CHECK_INSTANCE(self);
  
  bdd_l = new_node(BDD, (node_ptr) p, Nil);
  bdd_r = new_node(BDD, (node_ptr) q, Nil);

  couple = cons(bdd_l, bdd_r);
  FAIRNESS_LIST(self)->first = cons(couple, FAIRNESS_LIST(self)->first);

  bdd_ref((bdd_ptr) p);
  bdd_ref((bdd_ptr) q);
}


/**Function********************************************************************

  Synopsis     [Creates the union of the two given fairness lists. Result
  goes into self]

  Description  []
  
  SideEffects  [self changes]

******************************************************************************/
void CompassionList_apply_synchronous_product(CompassionList_ptr self, 
                                              const CompassionList_ptr other)
{
  FairnessListIterator_ptr iter;

  COMPASSION_LIST_CHECK_INSTANCE(self);

  iter = FairnessList_begin(FAIRNESS_LIST(other));
  while (! FairnessListIterator_is_end(iter)) {
    BddStates p = CompassionList_get_p(other, iter);
    BddStates q = CompassionList_get_q(other, iter);
    
    
    CompassionList_append_p_q(self, p, q);
    bdd_free( FAIRNESS_LIST(self)->dd, p );
    bdd_free( FAIRNESS_LIST(self)->dd, q );

    iter = FairnessListIterator_next(iter);
  } 
  
}



/* ---------------------------------------------------------------------- */
/* Static members                                                         */
/* ---------------------------------------------------------------------- */

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

******************************************************************************/
static void fairness_list_init(FairnessList_ptr self, 
                               DdManager* dd_manager)
{
  /* initializes the base class */
  object_init(OBJECT(self));

  self->first = END_ITERATOR;
  self->dd   = dd_manager;

  OVERRIDE(Object, finalize) = fairness_list_finalize;
  OVERRIDE(Object, copy) = fairness_list_copy;
}

static void fairness_list_deinit(FairnessList_ptr self)
{
  object_deinit(OBJECT(self));
  
  /* deletes the list and any contained BDDs */
  fairness_list_deinit_aux(self, self->first);
}


static void fairness_list_deinit_aux(FairnessList_ptr self, 
                                     FairnessListIterator_ptr iter)
{
  if( ! FairnessListIterator_is_end(iter) ) {

    switch(node_get_type(NODE_PTR(iter))) {
    case CONS:
      fairness_list_deinit_aux( self, 
                                FAIRNESS_LIST_ITERATOR(car(NODE_PTR(iter))) );
      fairness_list_deinit_aux( self, 
                                FAIRNESS_LIST_ITERATOR(cdr(NODE_PTR(iter))) );
      break;

    case BDD:
      bdd_free(self->dd, (bdd_ptr) car(NODE_PTR(iter)));
      break;

    default:
      internal_error("fairness_list_deinit_aux: unexpected  %d-type node.", 
                     node_get_type(NODE_PTR(iter)));  
    }
    
    free_node(NODE_PTR(iter));
  }  
}

static void fairness_list_finalize(Object_ptr object, void* dummy)
{
  FairnessList_ptr self = FAIRNESS_LIST(object);
  
  fairness_list_deinit(self);
  FREE(self);
}


static Object_ptr fairness_list_copy(const Object_ptr object)
{
  FairnessList_ptr self = FAIRNESS_LIST(object);

  FairnessList_ptr copy = ALLOC(FairnessList, 1);
  FAIRNESS_LIST_CHECK_INSTANCE(copy);

  /* copies base class instances: */
  object_copy_aux(OBJECT(self), OBJECT(copy));

  /* copies meembers: */
  copy->dd = self->dd;

  /* copies the list of bdds */
  copy->first = fairness_list_copy_aux(self, self->first);

  return OBJECT(copy);  
}


static node_ptr fairness_list_copy_aux(const FairnessList_ptr self, 
                                       const FairnessListIterator_ptr iter)
{
  node_ptr res;

  if( ! FairnessListIterator_is_end(iter)) {

    node_ptr iter2 = NODE_PTR(iter);
    FairnessListIterator_ptr left; 
    FairnessListIterator_ptr right;
    
    switch (node_get_type(iter2)) {

    case CONS:
      left  = FAIRNESS_LIST_ITERATOR(car(iter2));
      right = FAIRNESS_LIST_ITERATOR(cdr(iter2));
      res = cons( fairness_list_copy_aux(self, left), 
                  fairness_list_copy_aux(self, right) );
      break;

    case BDD:
      res = new_node(BDD, 
                     NODE_PTR(bdd_dup((bdd_ptr) car(iter2)) ), 
                     Nil);
      break;

    default:
      internal_error("fairness_list_copy_aux: unexpected  %d-type node.", 
                     node_get_type(iter2));  
    }
    
  }
  else {
    res = Nil;
  }

  return res;
}

