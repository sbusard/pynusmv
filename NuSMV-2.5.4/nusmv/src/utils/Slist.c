/**CFile***********************************************************************

  FileName    [Slist.c]

  PackageName [utils]

  Synopsis    [Implementation of Slist (Simple List) class]

  Description [Slist class is very simple one-directional list
  of pointers with modest functionality but as efficient implementation
  as possible.

  The main functionalities are:
  * it is possible to add and remove an element at the beginning
  of a list.
  * iterate over elements from beginning to the end.
  * reverse a list or create a reversed copy.

  To have more functionalities you can use Olist and have slightly less efficient
  implementation.]

  SeeAlso     []

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``addon.omcare'' package of NuSMV version 2.
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

#include "Slist.h"
#include "utils/list.h"

/*---------------------------------------------------------------------------*/
static char rcsid[] UTIL_UNUSED = "$Id: Slist.c,v 1.1.2.9 2010-02-24 15:45:05 nusmv Exp $";
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************

  Synopsis    [Implementation of Slist class]

  Description []

  SeeAlso     []

******************************************************************************/
struct Slist_TAG {
  unsigned int size;
  Snode_ptr first; /* first node of the list */
};

typedef struct Slist_TAG Slist;


/**Struct*********************************************************************

  Synopsis    [A node of the list]

  Description []

  SeeAlso     []

******************************************************************************/
struct Snode_TAG {
  void* element; /* an element stored in this node */
  Snode_ptr next; /* next node of a list */
};

typedef struct Snode_TAG Snode;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/* Macro DEBUG is used for debugging the function in this file */
#if 0
#define DEBUG(a)  a
#else
#define DEBUG(a)
#endif

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void slist_init ARGS((Slist_ptr self));
static void slist_deinit ARGS((Slist_ptr self));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates an instance of a Simple List ]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Slist_ptr Slist_create()
{
  Slist_ptr self = ALLOC(Slist, 1);
  slist_init(self);
  return self;
}

/**Function********************************************************************

  Synopsis    [Destroys a list instance]

  Description [The memory used by the list will be freed.
  Note: memory occupied by the elements is not freed! It is the user
  responsibility.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Slist_destroy (Slist_ptr self)
{
  slist_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Creates a copy of a given list]

  Description        [Note: input list does not change]

  SideEffects        []

  SeeAlso            [Slist_copy_reversed]

******************************************************************************/
Slist_ptr Slist_copy(Slist_ptr self)
{
  Slist_ptr new_list = Slist_create();
  Snode_ptr* new = &(new_list->first);

  Snode_ptr old;
  for (old = self->first; old != NULL; old = old->next) {
    *new = ALLOC(Snode, 1);
    (*new)->element = old->element;
    new = &((*new)->next);
   }
  (*new) = NULL;
  new_list->size = self->size;
  return new_list;
}


/**Function********************************************************************

  Synopsis           [Creates a copy of a given list with the order of elements
  reversed]

  Description        [Note: input list does not change]

  SideEffects        []

  SeeAlso            [Slist_copy]

******************************************************************************/
Slist_ptr Slist_copy_reversed(Slist_ptr self)
{
  Snode_ptr new = NULL;

  Snode_ptr old;
  for (old = self->first; old != NULL; old = old->next) {
    Snode_ptr tmp = ALLOC(Snode, 1);
    tmp->element = old->element;
    tmp->next = new;
    new = tmp;
  }
  Slist_ptr new_list = Slist_create();
  new_list->first = new;
  new_list->size = self->size;
  return new_list;
}


/**Function********************************************************************

  Synopsis           [Reverse the order of elements in the list]

  Description        [Note: all existing iterators pointing to the
  elements of the list may become invalid.
  Do not use them after this function call.]

  SideEffects        []

  SeeAlso            [Slist_copy_reversed]

******************************************************************************/
void Slist_reverse(Slist_ptr self)
{
  Snode_ptr old = self->first;
  Snode_ptr new = NULL;

  while (old) {
    Snode_ptr tmp = old;
    old = old->next;
    tmp->next = new;
    new = tmp;
  }
  self->first = new;
}


/**Function********************************************************************

  Synopsis           [Adds at the beginning of a list a new element]

  Description        []

  SideEffects        []

  SeeAlso            [Slist_append]

******************************************************************************/
void Slist_push(Slist_ptr self, void* element)
{
  /* create and initialize a new node */
  Snode_ptr node = ALLOC(Snode, 1);
  node->element = element;
  node->next = self->first;
  self->first = node;
  self->size += 1;
}


/**Function********************************************************************

  Synopsis           [Returns the size of a list]

  Description        []

  SideEffects        []

  SeeAlso            [Slist_append]

******************************************************************************/
unsigned int Slist_get_size(Slist_ptr self)
{
  nusmv_assert(self->size >= 0); /* the size cannot be negative */
  return self->size;
}


/**Function********************************************************************

  Synopsis           [Removes an element at the beginning of a list]

  Description [The removed element is returned.
  Existing iterators pointing to the first element become invalid
  after this function call and cannot be used any further.]

  SideEffects        []

  SeeAlso            [Slist_append]

******************************************************************************/
void* Slist_pop(Slist_ptr self)
{
  Snode_ptr node = self->first;
  self->first = node->next;

  void* element = node->element;
  FREE(node);
  self->size -= 1;
  return element;
}


/**Function********************************************************************

  Synopsis           [Returns the element at the beginning of a list]

  Description        []

  SideEffects        []

  SeeAlso            [Slist_append]

******************************************************************************/
void* Slist_top(Slist_ptr self)
{
  Snode_ptr node = self->first;
  void* element = node->element;
  return element;
}


/**Function********************************************************************

  Synopsis           [Returns true iff the list is empty]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Slist_is_empty(Slist_ptr self)
{
  return NULL == self->first;
}


/**Function********************************************************************

  Synopsis           [Returns true iff the two lists are equal
                      (contains the same elements in the same order)]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Slist_equals(const Slist_ptr self, const Slist_ptr other)
{
  Siter si, oi;

  if (self == other) {
    return true;
  }

  /* Different size => not equal */
  if (Slist_get_size(self) != Slist_get_size(other)) {
    return false;
  }

  /* Two empty lists are equal */
  if (Slist_get_size(self) == 0) {
    return true;
  }

  si = Slist_first(self);
  oi = Slist_first(other);

  while(!Siter_is_end(si)) {
    if(Siter_element(si) != Siter_element(oi)) {
      return false;
    }

    si = Siter_next(si);
    oi = Siter_next(oi);
  }

  return true;
}


/**Function********************************************************************

  Synopsis           [Returns an iterator pointing to a first element of a list]

  Description        [If the list is empty the iterator will point past
  the last element of a list (i.e. past the list). This means function
  Siter_is_end will return true in this case.
  NOTE: there is no need to free the iterator after using it.
  NOTE: it is allowed to assign one iterator to another one.
  ]

  SideEffects        []

  SeeAlso            [Siter_is_end, Siter_next, Siter_element]

******************************************************************************/
Siter Slist_first(Slist_ptr self)
{
  Siter iter;
  iter.node = self->first;
  return iter;
}


/**Function********************************************************************

  Synopsis           [Returns true iff an iterator points past the last element
  of a list.]

  Description        [The iterator must have been created with function
  Slist_first or Slist_next]

  SideEffects        []

  SeeAlso            [Slist_first, Siter_next, Siter_element]

******************************************************************************/
boolean Siter_is_end(Siter iter)
{
  return NULL == iter.node;
}


/**Function********************************************************************

  Synopsis [Sets the given iterator to the end, so that Siter_is_end
  returns true for it.]

  Description [This can useful in functions which need to produce
  an iterator.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Siter_make_end(Siter* iter)
{ iter->node = NULL; }


/**Function********************************************************************

  Synopsis           [Returns an iterator pointing to the next element
  of a list w.r.t. the element pointed by a provided iterator.]

  Description [ Precondition: this function can be applied only if
  Siter_is_end(iter) returns false.  ]

  SideEffects        []

  SeeAlso            [Slist_first, Siter_is_end, Siter_element]

******************************************************************************/
Siter Siter_next(Siter iter)
{
  Siter new;
  new.node = iter.node->next;
  return new;
}


/**Function********************************************************************

  Synopsis           [Returns a value of a list element pointed by
  a provided iterator]

  Description        [Precondition: this function can be applied only
  if Siter_is_end(iter) returns false]

  SideEffects        []

  SeeAlso            [Slist_first, Siter_is_end, Siter_next]

******************************************************************************/
void* Siter_element(Siter iter)
{
  nusmv_assert(iter.node != NULL); /* iterator is past the last element */
  return (iter.node)->element;
}


/**Function********************************************************************

  Synopsis           [Returns an iterator pointing to the first element
  equal to the given one]

  Description        [
  If there is no such element then on the returned iterator
  Siter_is_end(iter) will be true.]


  SideEffects        []

  SeeAlso            [Slist_first, Siter_is_end, Siter_next]

******************************************************************************/
Siter Slist_find(Slist_ptr self, const void* element)
{
  Siter iter;
  for (iter = Slist_first(self); !Siter_is_end(iter); iter = Siter_next(iter)) {
    if (Siter_element(iter) == element) break;
  }
  return iter;
}


/**Function********************************************************************

  Synopsis           [Checks whether the specified element is in the list or not]

  Description        []

  SideEffects        []

  SeeAlso            [Slist_first, Siter_is_end, Siter_next]

******************************************************************************/
boolean Slist_contains(Slist_ptr self, const void* element)
{
  return !Siter_is_end(Slist_find(self, element));
}


/**Function********************************************************************

  Synopsis           [Removes all the occurrencies of specified element if
                      present in the list. Returns true if the element was
                      removed, false otherwise]

  Description        []

  SideEffects        []

  SeeAlso            [Slist_first, Siter_is_end, Siter_next]

******************************************************************************/
boolean Slist_remove(Slist_ptr self, const void* element)
{
  SLIST_CHECK_INSTANCE(self);

  Snode_ptr current, previous;
  boolean res;

  res = false;
  previous = (Snode_ptr) NULL;
  current = self->first;
  while(current != (Snode_ptr)NULL) {
    if (current->element == element) {
      Snode_ptr next;

      next = current->next;

      if ((Snode_ptr)NULL != previous) {
        previous->next = next;
      }
      else {
        /* We are going to remove the first element */
        self->first = next;
      }
      FREE(current);
      res = true;
      current = next; /* Do not change previous */
    }
    else {
      previous = current;
      current = current->next;
    }
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Destroys the list and every element contained using the 
                      specified function]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Slist_destroy_and_free_elements(Slist_ptr self,
                                     Slist_free_function f)
{
  SLIST_CHECK_INSTANCE(self);

  Siter iter;
  for (iter = Slist_first(self); !Siter_is_end(iter); iter = Siter_next(iter)) {
    f(Siter_element(iter));
  }

  Slist_destroy(self);
}


/**Function********************************************************************

  Synopsis           [Pops all the elements of this list]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Slist_clear(Slist_ptr self)
{
  SLIST_CHECK_INSTANCE(self);

  Siter iter;
  for (iter = Slist_first(self); !Siter_is_end(iter); iter = Siter_next(iter)) {
    Slist_pop(self);
  }
}


/**Function********************************************************************

  Synopsis           [Appends two lists modifying self]

  Description        []

  SideEffects        [self is extended]

  SeeAlso            []

******************************************************************************/
void Slist_append(Slist_ptr self, const Slist_ptr other)
{
  SLIST_CHECK_INSTANCE(self);

  Siter iter;
  for (iter = Slist_first(other); !Siter_is_end(iter); iter = Siter_next(iter)) {
    Slist_push(self, Siter_element(iter));
  }
}


/**Function********************************************************************

  Synopsis           [Sorts the list in place]

  Description        [mergesort is used to sort the list. 
  worst case complexisty O(N log2(N)).

  cmp is comparison function returning value v, v < 0, v == 0 or v > 0]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Slist_sort(Slist_ptr self, int (*cmp)(void* el1, void* el2))
{  
  Snode_ptr list = self->first;
  Snode_ptr tail;
  int insize;

  if (NULL == list) return; /* no change */

  insize = 1;

  while (true) {
    Snode_ptr p = list;
    int merges = 0; /* count number of merges we do in this pass */

    list = tail = NULL;
    while (NULL != p) {
      Snode_ptr q = p;
      int psize, qsize, i;

      merges += 1;  /* there exists a merge to be done */

      psize = 0;
      /* step `insize' places along from p */
      for (i = 0; i < insize; ++i) {
        psize += 1;
        q = q->next;
        if (NULL == q) break;
      }

      /* if q hasn't fallen off end, we have two lists to merge */
      qsize = insize;

      /* now we have two lists; merge them */
      while (psize > 0 || (qsize > 0 && NULL != q)) {
        Snode_ptr e;

        /* decide whether next element of merge comes from p or q */
        if (psize == 0) {
          /* p is empty; e must come from q. */
          e = q; q = q->next; qsize--;
        } else if (qsize == 0 || NULL == q) {
          /* q is empty; e must come from p. */
          e = p; p = p->next; psize--;
        } else if (cmp(p->element,q->element) <= 0) {
          /* First element of p is lower (or same);
           * e must come from p. */
          e = p; p = p->next; psize--;
        } else {
          /* First element of q is lower; e must come from q. */
          e = q; q = q->next; qsize--;
        }

        /* add the next element to the merged list */
        if (NULL != tail) tail->next = e;
        else list = e;
        tail = e;
      }

      /* now p has stepped `insize' places along, and q has too */
      p = q;
    }
    tail->next = NULL;

    /* If we have done only one merge, we're finished. */    
    if (merges <= 1) break;

    /* keep going merging twice the size */
    insize <<= 1;
  }

  self->first = list;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the memory for a list instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void slist_init(Slist_ptr self)
{
  SLIST_CHECK_INSTANCE(self);
  self->size = 0;
  self->first = NULL;
}


/**Function********************************************************************

  Synopsis           [Deinitializes the memory from a list]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void slist_deinit(Slist_ptr self)
{
  SLIST_CHECK_INSTANCE(self);

  Snode_ptr node = self->first;
  while (node) {
    Snode_ptr tmp = node;
    node = tmp->next;
    FREE(tmp);
  };
  self->size = 0;
  self->first = NULL; /* for debugging */
}

