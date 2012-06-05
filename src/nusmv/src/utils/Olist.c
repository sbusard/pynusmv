/**CFile***********************************************************************

  FileName    [Olist.c]

  PackageName [addons.omcare]

  Synopsis    [Implementation of Olist class]

  Description [Olist class is One-directional List of pointers.
  ]

  SeeAlso     [Olist is One-directional List of pointers.
  This class is less efficient that Slist but provided more functionalities.
  The main difference from Slist is that with Olist:
  * all operations on iterators requires additional dereference.
  * it is possible to add and removed elements at arbitrary places in the list ]

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

#include "Olist.h"
#include "utils/list.h"
#include "node/node.h" /* for print_node */
#include <stddef.h> /* for offsetof */

/*---------------------------------------------------------------------------*/
static char rcsid[] UTIL_UNUSED = "$Id: Olist.c,v 1.1.2.5 2010-02-18 09:52:38 nusmv Exp $";
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

  Synopsis    [Implementation of Olist class]

  Description []

  SeeAlso     []

******************************************************************************/
struct Olist_TAG {
  Onode_ptr first; /* first node of the list */
  Onode_ptr last; /* last node of the list */
  int size; /* the size of the list */
};

typedef struct Olist_TAG Olist;


/**Struct*********************************************************************

  Synopsis    [A node of the list]

  Description []

  SeeAlso     []

******************************************************************************/
struct Onode_TAG {
  void* element; /* an element stored in this node */
  Onode_ptr next; /* next node of a list */
};

typedef struct Onode_TAG Onode;


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/* Macro DEBUG is used for debugging the functions in this file */
#if 0
#define DEBUG(a)  a
#else 
#define DEBUG(a) 
#endif

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void olist_init ARGS((Olist_ptr self));
static void olist_deinit ARGS((Olist_ptr self));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates an instance of a One-directional List ]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Olist_ptr Olist_create()
{
  Olist_ptr self = ALLOC(Olist, 1);
  olist_init(self);
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
void Olist_destroy (Olist_ptr self)
{
  olist_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Creates a copy of a given list]

  Description        [Note: input list does not change]

  SideEffects        []

  SeeAlso            [Olist_copy_reversed]

******************************************************************************/
Olist_ptr Olist_copy(Olist_ptr self)
{
  Olist_ptr new_list = Olist_create();

  Onode_ptr* new = &(new_list->first);
  Onode_ptr tmp = NULL;
  Onode_ptr old;

  for (old = self->first; old != NULL; old = old->next) {
    tmp = ALLOC(Onode, 1);
    tmp->element = old->element;
    *new = tmp;
    new = &(tmp->next);
   }
  (*new) = NULL;
  new_list->last = tmp;
  new_list->size = self->size;

  return new_list;
}


/**Function********************************************************************

  Synopsis           [Creates a copy of a given list with the order of elements 
  reversed]

  Description        [Note: input list does not change]

  SideEffects        []

  SeeAlso            [Olist_copy]

******************************************************************************/
Olist_ptr Olist_copy_reversed(Olist_ptr self)
{
  Olist_ptr new_list = Olist_create();
  if (Olist_is_empty(self)) return new_list;

  Onode_ptr new;
  Onode_ptr old = self->first;;

  /* process the first element separately */
  new = ALLOC(Onode, 1);
  new->element = old->element;
  new->next = NULL;

  new_list->last = new;
  
  for (old = old->next; old != NULL; old = old->next) {
    Onode_ptr tmp = ALLOC(Onode, 1);
    tmp->element = old->element;
    tmp->next = new;
    new = tmp;
  }
  new_list->first = new;
  new_list->size = self->size;
  return new_list;
}

/**Function********************************************************************

  Synopsis           [Creates a copy of a given list 
  with all its elements except the provided one.]

  Description        [Note: input list does not change]

  SideEffects        []

  SeeAlso            [Olist_copy]

******************************************************************************/
Olist_ptr Olist_copy_without_element(Olist_ptr self, void* element)
{
  Olist_ptr new_list = Olist_create();

  Onode_ptr* new = &(new_list->first);
  Onode_ptr tmp = NULL;
  Onode_ptr old;
  int num = 0;

  for (old = self->first; old != NULL; old = old->next) {
    if (old->element != element) {
      tmp = ALLOC(Onode, 1);
      tmp->element = old->element;
      *new = tmp;
      new = &(tmp->next);
      num += 1;
    }
   }
  (*new) = NULL;
  new_list->last = tmp;
  new_list->size = num;

  return new_list;
}

/**Function********************************************************************

  Synopsis           [Reverse the order of elements in the list]

  Description        [Note: all existing iterators pointing to the 
  elements of the list may become invalid.]

  SideEffects        []

  SeeAlso            [Olist_copy_reversed]

******************************************************************************/
void Olist_reverse(Olist_ptr self)
{
  Onode_ptr old = self->first;
  self->first = self->last;
  self->last = old;

  Onode_ptr new = NULL;

  while (old) {
    Onode_ptr tmp = old;
    old = old->next;
    tmp->next = new;
    new = tmp;
  }
}


/**Function********************************************************************

  Synopsis [Moves the content from one list to another]

  Description [The content is moved from "self" to "to_list" before
  given iterator. If "to_list" is not empty the moved content is
  added at the end (appended).

  Note: all existing iterators pointing to the elements of the list
  may become invalid.]

  SideEffects        []

  SeeAlso            [Olist_copy_reversed]

******************************************************************************/
void Olist_move(Olist_ptr self, Olist_ptr to_list, Oiter iter_to)
{
  if (self->first == NULL) return; /* nothing to move */
  
  if (to_list->first == NULL) { /* to_list is empty */
    *to_list = *self; /* simply copy all the fields */
  }
  else {    
    if (Oiter_is_end(iter_to)) {
      /* to the tail */
      to_list->last->next = self->first;
    }
    else {
      self->last->next = (*iter_to.node);
      *iter_to.node = self->first;
    }
    to_list->size += self->size;
  }

  self->first = NULL;
  self->last = NULL;
  self->size = 0;
}

/**Function********************************************************************

  Synopsis [Moves the content from one list to another]

  Description [This function is similar to Olist_move
  with iter_to set up to point past the last element.
  Note: all existing iterators pointing to the elements of the list
  may become invalid.]

  SideEffects        []

  SeeAlso            [Olist_move]

******************************************************************************/
void Olist_move_all(Olist_ptr self, Olist_ptr to_list)
{
  Oiter it;
  Oiter_make_end(&it);
  Olist_move(self, to_list, it);
}

/**Function********************************************************************

  Synopsis [Removes all the elements of the list, i.e.  makes the
  list empty]

  Description [ After this function call, Olist_is_empty(self)
  always returns true.

  Note: all existing iterators pointing to the elements of the list
  becomes invalid.]

  SideEffects        []

  SeeAlso            [Olist_copy_reversed]

******************************************************************************/
void Olist_clean(Olist_ptr self)
{
  olist_deinit(self);
  self->first = NULL;
  self->last = NULL;
  self->size = 0;
}


/**Function********************************************************************

  Synopsis           [Adds at the beginning of a list a new element]

  Description        []

  SideEffects        []

  SeeAlso            [Olist_append]

******************************************************************************/
void Olist_prepend(Olist_ptr self, void* element)
{
  /* create and initialize a new node */
  Onode_ptr node = ALLOC(Onode, 1);
  node->element = element;
  node->next = self->first;

  /* update the list's fields  */
  if (NULL == self->last) {
    /* there is no last element => the list is empty */
    nusmv_assert(NULL == self->first);
    nusmv_assert(0 == self->size);
    self->last = node;
  }
  self->first = node;
  self->size += 1;
}


/**Function********************************************************************

  Synopsis    [Adds at the end of a list a new element]

  Description []

  SideEffects []

  SeeAlso     [Olist_prepend]

******************************************************************************/
void Olist_append(Olist_ptr self, void* element)
{
  /* create and initialize a new node */
  Onode_ptr node = ALLOC(Onode, 1);
  node->element = element;
  node->next = NULL;

  /* update the list's fields  */
  if (self->first == NULL) {
    /* there is no first element => the list is empty */
    nusmv_assert(NULL == self->last);
    nusmv_assert(0 == self->size);
    self->first = node;
  }
  else {
    self->last->next = node;
  }
  
  self->last = node;
  self->size += 1;
}


/**Function********************************************************************

  Synopsis    [Removes a first element of a list]

  Description [The removed element is returned.
  Precondition: the list must not be empty.

  NOTE: all iterators already pointing to the element next to the
  first one will become invalid.  Any operations on them are
  prohibited.  
  ADVICE: do not use several iterators over the same list
  if deletion operation is possible.]

  SideEffects []

  SeeAlso     [Olist_append, Olist_prepend]

******************************************************************************/
void* Olist_delete_first(Olist_ptr self)
{
  nusmv_assert(self->first != NULL); /* list must not be empty */

  /* create and initialize a new node */
  Onode_ptr node = self->first;
  void* element = node->element;
  self->first = node->next;

  /* update the last element if first == last */
  if (self->last == node) {
    nusmv_assert(self->first == NULL); /* list must be empty now */
    self->last = NULL;
  }
  FREE(node);
  self->size -= 1;

  return element;
}


/**Function********************************************************************

  Synopsis           [Returns the size of a list]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Olist_get_size(const Olist_ptr self)
{
  nusmv_assert(self->size >= 0); /* the size cannot be negative */
  return self->size;
}


/**Function********************************************************************

  Synopsis           [Returns true iff the list is empty]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Olist_is_empty(Olist_ptr self)
{
  return NULL == self->first;
}


/**Function********************************************************************

  Synopsis           [Returns true iff the list contains the given element]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Olist_contains(const Olist_ptr self, const void* element)
{
  Oiter iter;

  OLIST_FOREACH(self, iter) {
    if (Oiter_element(iter) == element) {
      return true;
    }
  }

  return false;
}


/**Function********************************************************************

  Synopsis           [Tries to remove all the occurrencies of the specified
                      element from the list, returns true if an element was
                      removed, false otherwise]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean Olist_remove(Olist_ptr self, const void* element)
{
  Oiter iter;
  boolean result;

  result = false;
  iter = Olist_first(self);
  while(!Oiter_is_end(iter)) {
    if (Oiter_element(iter) == element) {
      iter = Olist_delete(self, iter, (void**)NULL);
      result = true;
    }
    else {
      iter = Oiter_next(iter);
    }
  }

  return result;
}


/**Function********************************************************************

  Synopsis [Returns an iterator pointing to a first element of a
  list]

  Description [If the list is empty the iterator will point past
  the last element of a list (i.e. past the list). This means
  function Oiter_is_end will return true in this case.  NOTE: there
  is no need to free the iterator after using it.  NOTE: it is
  allowed to assign one iterator to another one.  NOTE: deletion
  the elements of the list may make the iterator invalid (see
  corresponding delete functions).  ]

  SideEffects        []

  SeeAlso            [Oiter_last, Oiter_is_end, Oiter_next, Oiter_element]

******************************************************************************/
Oiter Olist_first(Olist_ptr self)
{
  Oiter iter;
  /* actually the iterator points to a "next" pointer of previous
     element or to self->first. Such a way we have access to the part
     of the list which points to a given node, i.e. we can modify the
     structure of the list (e.g. remove a given node).
  */
  iter.node = &(self->first);
  return iter;
}


/**Function********************************************************************

  Synopsis [Returns an iterator pointing to a last element of a
  list]

  Description [If the list is empty the iterator will point past
  the last element of a list (i.e. past the list). This means
  function Oiter_is_end will return true in this case.  NOTE: there
  is no need to free the iterator after using it.  NOTE: it is
  allowed to assign one iterator to another one.  NOTE: deletion
  the elements of the list may make the iterator invalid (see
  corresponding delete functions).  ]

  SideEffects        []

  SeeAlso            [Oiter_first, Oiter_is_end, Oiter_next, Oiter_element]

******************************************************************************/
Oiter Olist_last(Olist_ptr self)
{
  Oiter iter;
  /* actually the iterator points to a "next" pointer of previous
     element or to self->first. Such a way we have access to the part
     of the list which points to a given node, i.e. we can modify the
     structure of the list (e.g. remove a given node).
  */
  iter.node = &(self->last);
  return iter;
}


/**Function********************************************************************

  Synopsis           [Returns true if iter corresponds to the first iter.]

  Description        [Returns true if iter corresponds to the first iter.]

  SideEffects        []

  SeeAlso            [Oiter_is_end, Oiter_next, Oiter_element]

******************************************************************************/
boolean Olist_iter_is_first(Olist_ptr self, Oiter iter) 
{
  return ( *(iter.node) == self->first);
}


/**Function********************************************************************

  Synopsis           [Returns true if iter corresponds to the first iter.]

  Description        [Returns true if iter corresponds to the first iter.]

  SideEffects        []

  SeeAlso            [Oiter_is_end, Oiter_next, Oiter_element]

******************************************************************************/
boolean Olist_iter_is_last(Olist_ptr self, Oiter iter) 
{
  return ( *(iter.node) == self->last);
}


/**Function********************************************************************

  Synopsis           [Returns true iff an iterator points past the last element
  of a list.]

  Description        [The iterator must have been created with function
  Olist_first or Olist_next]

  SideEffects        []

  SeeAlso            [Olist_first, Oiter_next, Oiter_element]

******************************************************************************/
boolean Oiter_is_end(Oiter iter)
{
  /* iter.node ==NULL if iter has not been initialized */
  return NULL == *(iter.node);
}


/**Function********************************************************************

  Synopsis [Sets the given iterator to the end, so that Oiter_is_end
  returns true for it.]

  Description [This can useful in functions which need to produce
  an iterator.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Oiter_make_end(Oiter* iter)
{ iter->node = NULL; }


/**Function********************************************************************

  Synopsis           [Returns an iterator pointing to the next element 
  of a list w.r.t. the element pointed by a provided iterator.]

  Description        [Precondition: this function can be applied only
  if Oiter_is_end(iter) returns false]

  SideEffects        []

  SeeAlso            [Olist_first, Oiter_is_end, Oiter_element]

******************************************************************************/
Oiter Oiter_next(Oiter iter)
{
  nusmv_assert(*iter.node != NULL); /* iterator is past the last element */

  Oiter new;
  new.node = &((*iter.node)->next);
  return new;
}


/**Function********************************************************************

  Synopsis           [Returns a value of a list element pointed by 
  a provided iterator]

  Description        [Precondition: this function can be applied only
  if Oiter_is_end(iter) returns false]

  SideEffects        []

  SeeAlso            [Olist_first, Oiter_is_end, Oiter_next]

******************************************************************************/
void* Oiter_element(Oiter iter)
{
  nusmv_assert(*iter.node != NULL); /* iterator is past the last element */
  return (*iter.node)->element;
}

/**Function********************************************************************

  Synopsis           [Sets a new value to the list element pointed by 
  a provided iterator]

  Description        [Precondition: this function can be applied only
  if Oiter_is_end(iter) returns false]

  SideEffects        []

  SeeAlso            [Olist_first, Oiter_is_end, Oiter_next, Oiter_element]

******************************************************************************/
void Oiter_set_element(Oiter iter, void* element)
{
  nusmv_assert(*iter.node != NULL); /* iterator is past the last element */
  (*iter.node)->element = element;
}

/**Function********************************************************************

  Synopsis    [Insert a new element into the list "self" directly after
  an element pointed by "iter"]

  Description [Precondition: iter must point to elements of list "self"
  and NOT past the last element of the list.
  If iter is not an iterator of list self there will be 
  problems with memory which are usually very difficult to debug.

  NOTE: after the function call iterators pointing to the element
  after iter will now point to the newly inserted element. All other
  existing iterators (including iter) will point to the same element
  as before. 

  Returns an iterator pointing to the newly inserted element.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Oiter Olist_insert_after(Olist_ptr self, Oiter iter, void* element)
{
  nusmv_assert(NULL != *iter.node); /* points past the last element */


  Onode_ptr node = ALLOC(Onode, 1);
  node->element = element;
  node->next = (*iter.node)->next;
  (*iter.node)->next = node;
  
  if (NULL == node->next) { /* the last element of a list changed */
    nusmv_assert(self->last == *iter.node);
    self->last = node;
  }

  self->size += 1;

  iter.node = &(*iter.node)->next;
  return iter;
}


/**Function********************************************************************

  Synopsis    [Insert a new element into the list "self" directly before
  an element pointed by "iter"]

  Description [Precondition: iter must point to elements of list "self"
  or past the last element of the list.

  If the iterator points past the last element of a list then
  this function is equivalent to calling Olist_append(self, element).

  NOTE: All existing iterators equal to "iter" (and iter itself) 
  after insertion will point to the newly created element.
  All other iterators remain intact.

  Returns an iterator pointing to the newly inserted element.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Oiter Olist_insert_before(Olist_ptr self, Oiter iter, void* element)
{
  Onode_ptr node = ALLOC(Onode, 1);
  node->element = element;
  node->next = *iter.node;
  *iter.node = node;

  if (NULL == node->next) { /* update the last element */
    self->last = node;
  }

  self->size += 1;
  return iter;
}


/**Function********************************************************************

  Synopsis    [Removes an element pointed by an iterator from a list]

  Description [
  
  Precondition: iter must point to elements of list "self" and
  NOT the past the last element of the list.

  The element being removed is returned in argument *element (only if
  element != NULL).
  
  Returns an iterator pointing to the element after removed one.
  
  NOTE: all iterators already pointing to the next element will become invalid. 
  Any operations on them are prohibited.
  ADVICE: do not use several iterators over the same list if deletion 
  operation is possible.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Oiter Olist_delete(Olist_ptr self, Oiter iter, void** element)
{
  nusmv_assert(*iter.node != NULL); /* should not point past the last element */

  Onode_ptr node = *iter.node;
  if (element != NULL) *element = node->element;

  *iter.node = node->next;

  if (self->last == node) { /* the last element was removed => care required */
    nusmv_assert(NULL == node->next);
    if (NULL == self->first) { /* this means the list is empty */
      self->last = NULL;
    }
    else {
      /* below expression find the address of the last element */
      self->last = (Onode_ptr)((char*)(iter.node) - offsetof(Onode, next));
    }
  }

  node->element = NULL; /* for debugging */
  node->next = NULL; /* for debugging */
  FREE(node);

  self->size -= 1;
  nusmv_assert(self->size >= 0);

  return iter;
}


/**Function********************************************************************

  Synopsis           [Sorts the list in place]

  Description        [mergesort is used to sort the list. 
  worst case complexisty O(N log2(N)).

  cmp is comparison function returning value v, v < 0, v == 0 or v > 0]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Olist_sort(Olist_ptr self, int (*cmp)(void* el1, void* el2))
{  
  Onode_ptr list = self->first;
  Onode_ptr tail;
  int insize;

  if (NULL == list) return; /* no change */

  insize = 1;

  while (true) {
    Onode_ptr p = list;
    int merges = 0; /* count number of merges we do in this pass */

    list = tail = NULL;
    while (NULL != p) {
      Onode_ptr q = p;
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
        Onode_ptr e;

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
  self->last = tail;
}


/**Function********************************************************************

  Synopsis           [Prints the elements of the list using 
  print_node and putting string ", " between elements]

  Description        [
  Precondition: all elements of the list have to be node_ptr.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Olist_print_node(Olist_ptr self, FILE* output)
{
  Oiter it;
  OLIST_FOREACH(self, it) {
    print_node(output, Oiter_element(it));
    if (!Oiter_is_end(it)) fprintf(output, ", ");
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [This is a test function]

  Description [Prototype of this function is not defined anywhere.
  Thus to use it define the prototype where you want and then invoke
  this function.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void olist_testing_function()
{
  printf("TESTING Olist class : in process ....\n");

  Olist_ptr list1, list2, keeper;
  Oiter iter1;
  void* v1 = (void*)1;
  void* v2 = (void*)2;
  void* v3 = (void*)3;
  void* v4 = (void*)4;
  void* elem;

  list1 = Olist_create();                  keeper = list1;
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_append(list1, v1);
  nusmv_assert(Oiter_element(Olist_first(list1)) == v1);
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_prepend(list1, v1);
  nusmv_assert(Oiter_element(Olist_first(list1)) == v1);
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_append(list1, v1);
  Olist_prepend(list1, v2);
  nusmv_assert(Oiter_element(Olist_first(list1)) == v2);
  Olist_destroy(list1);

  list1 = Olist_create();
  nusmv_assert(Olist_is_empty(list1));
  nusmv_assert(0 == Olist_get_size(list1));
  Olist_prepend(list1, v1);
  Olist_append(list1, v2);
  nusmv_assert(Oiter_element(Olist_first(list1)) == v1);
  nusmv_assert(!Olist_is_empty(list1));
  nusmv_assert(2 == Olist_get_size(list1));
  elem = Olist_delete_first(list1);
  nusmv_assert(elem == v1);
  nusmv_assert(Oiter_element(Olist_first(list1)) == v2);
  nusmv_assert(!Olist_is_empty(list1));
  nusmv_assert(1 == Olist_get_size(list1));
  elem = Olist_delete_first(list1);
  nusmv_assert(elem == v2);
  nusmv_assert(Olist_is_empty(list1));
  nusmv_assert(0 == Olist_get_size(list1));
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_prepend(list1, v1);
  Olist_prepend(list1, v2);
  Olist_prepend(list1, v3);
  iter1 = Olist_first(list1);
  nusmv_assert(!Oiter_is_end(iter1));
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Oiter_next(iter1);
  nusmv_assert(!Oiter_is_end(iter1));
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  nusmv_assert(!Oiter_is_end(iter1));
  nusmv_assert(Oiter_element(iter1) == v1);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_is_end(iter1));
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_append(list1, v1);
  Olist_append(list1, v2);
  Olist_append(list1, v3);
  iter1 = Olist_first(list1);
  nusmv_assert(!Oiter_is_end(iter1));
  nusmv_assert(Oiter_element(iter1) == v1);
  iter1 = Oiter_next(iter1);
  nusmv_assert(!Oiter_is_end(iter1));
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  nusmv_assert(!Oiter_is_end(iter1));
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_is_end(iter1));
  Olist_destroy(list1);

  list2 = Olist_create();
  Olist_append(list2, v1);
  Olist_append(list2, v2);
  Olist_append(list2, v3);
  list1 = Olist_copy(list2);
  Olist_destroy(list2);
  iter1 = Olist_first(list1);
  nusmv_assert(Oiter_element(iter1) == v1);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_is_end(iter1));
  Olist_destroy(list1);

  list2 = Olist_create();
  Olist_append(list2, v1);
  Olist_append(list2, v2);
  Olist_append(list2, v3);
  list1 = Olist_copy_reversed(list2);
  Olist_destroy(list2);
  iter1 = Olist_first(list1);
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v1);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_is_end(iter1));
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_append(list1, v1);
  Olist_append(list1, v2);
  Olist_append(list1, v3);
  Olist_reverse(list1);
  iter1 = Olist_first(list1);
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v1);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_is_end(iter1));
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_append(list1, v2);
  Olist_append(list1, v4);
  iter1 = Olist_first(list1);
  Olist_insert_after(list1, iter1, v3);
  Olist_insert_before(list1, iter1, v1);
  nusmv_assert(Oiter_element(iter1) == v1);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v4);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_is_end(iter1));
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_append(list1, v1);
  Olist_append(list1, v3);
  iter1 = Oiter_next(Olist_first(list1));
  iter1 = Olist_insert_after(list1, iter1, v4);
  nusmv_assert(Oiter_element(iter1) == v4);
  iter1 = Oiter_next(Olist_first(list1));
  iter1 = Olist_insert_before(list1, iter1, v2);
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Olist_first(list1);
  nusmv_assert(Oiter_element(iter1) == v1);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v4);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_is_end(iter1));
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_append(list1, v1);
  Olist_append(list1, v2);
  iter1 = Oiter_next(Oiter_next(Olist_first(list1)));
  iter1 = Olist_insert_before(list1, iter1, v3);
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Olist_first(list1);
  nusmv_assert(Oiter_element(iter1) == v1);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_element(iter1) == v3);
  iter1 = Oiter_next(iter1);
  nusmv_assert(Oiter_is_end(iter1));
  Olist_destroy(list1);

  list1 = Olist_create();
  Olist_append(list1, v1);
  Olist_append(list1, v2);
  Olist_append(list1, v3);
  Olist_append(list1, v4);
  iter1 = Olist_first(list1);
  iter1 = Olist_delete(list1, iter1, &elem);
  nusmv_assert(elem == v1);
  nusmv_assert(Oiter_element(iter1) == v2);
  iter1 = Oiter_next(iter1);
  iter1 = Olist_delete(list1, iter1, &elem);
  nusmv_assert(elem == v3);
  nusmv_assert(Oiter_element(iter1) == v4);
  iter1 = Olist_delete(list1, iter1, &elem);
  nusmv_assert(elem == v4);
  nusmv_assert(Oiter_is_end(iter1));
  nusmv_assert(Olist_get_size(list1) == 1);
  Olist_destroy(list1);

  /* This assert is based on a guess that the memory is allocated
     always the same way.  Since all the memory allocated in this
     function is freed this allocation should be exactly the same as
     the first allocation of a list.
  */
  list1 = Olist_create();                  //nusmv_assert(keeper == list1);
  Olist_destroy(list1);

  printf("TESTING Olist class : DONE\n");
  return;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the memory for a list instance]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void olist_init(Olist_ptr self)
{
  OLIST_CHECK_INSTANCE(self);
  
  self->first = NULL;
  self->last = NULL;
  self->size = 0;
}


/**Function********************************************************************

  Synopsis           [Deinitializes the memory from a list]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void olist_deinit(Olist_ptr self)
{
  OLIST_CHECK_INSTANCE(self);

  Onode_ptr node = self->first;
  Onode_ptr tmp = NULL;

  while (node) {
    /* decrease the size of a list */
    DEBUG(self->size -= 1);
    /* there is a problem with memory. It is likely iter 
     of one list was used with another list in insert or delete functions */
    DEBUG(nusmv_assert(self->size >= 0));
    /* The possible problem as above */
    DEBUG(nusmv_assert(node->next != NULL || node == self->last));

    tmp = node;
    node = tmp->next;
    FREE(tmp);
  };
    
  DEBUG(nusmv_assert(0 == self->size));/* there is a problem with size */
  self->first = NULL; /* for debugging */
  self->last = NULL; /* for debugging */
  self->size = -1; /* for debugging */
}

