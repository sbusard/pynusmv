/**CSourceFile*****************************************************************

  FileName    [NodeList.c]

  PackageName [utils]

  Synopsis    [This is a class exporting a node_ptr based list, but with
  a higher level and better performances]

  Description [It is supported by calling the
  constructor create_from_list.  Notice that at the moment a minimal
  bunch of functionalities is exported ]

  SeeAlso     [NodeList.h]

  Author      [Roberto Cavada, Andrea Micheli,
               complete internal reimplementation by Alessandro Mariotti]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2.
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

#include "NodeList.h"
#include "utils.h"
#include "assoc.h"
#include "Stack.h"

/*---------------------------------------------------------------------------*/
/* Types definition                                                          */
/*---------------------------------------------------------------------------*/

typedef struct Link_TAG {
  struct Link_TAG* prev;
  struct Link_TAG* next;

  node_ptr element;
} Link;

typedef struct NodeList_TAG {
  Link* head;
  Link* tail;

  unsigned long size;

  hash_ptr count_hash;
} NodeList;

/*---------------------------------------------------------------------------*/
/* Macros definition                                                         */
/*---------------------------------------------------------------------------*/

#define LINK_CHUNK_SIZE 1024

#define NULL_LINK                               \
  (Link*)NULL


/*---------------------------------------------------------------------------*/
/* Variables definition                                                      */
/*---------------------------------------------------------------------------*/

static Link* pool = NULL_LINK;
static Stack_ptr chunks = STACK(NULL);
static size_t reference_counter = 0;


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void node_list_init ARGS((NodeList_ptr self, node_ptr list));
static void node_list_deinit ARGS((NodeList_ptr self));

static inline Link*
node_list_alloc_link ARGS((const NodeList_ptr self));

static inline void
node_list_free_link ARGS((const NodeList_ptr self, Link* link));

static inline void
node_list_update_count ARGS((const NodeList_ptr self, const node_ptr elem,
                             const boolean deleting));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Creates a new list]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr NodeList_create()
{
  NodeList_ptr self = ALLOC(NodeList, 1);
  NODE_LIST_CHECK_INSTANCE(self);

  node_list_init(self, Nil);
  return self;
}


/**Function********************************************************************

  Synopsis [Constructor that creates a new NodeList that is a wrapper
  of the given list.]

  Description [self becomes a user of the given list, meaning that
  when self will be destroyed, it will not free the given list. It is a caller
  responsability of freeing the passed list when possible.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr NodeList_create_from_list(node_ptr list)
{
  NodeList_ptr self = ALLOC(NodeList, 1);
  NODE_LIST_CHECK_INSTANCE(self);

  node_list_init(self, list);
  return self;
}


/**Function********************************************************************

  Synopsis           [Class destroyer]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void NodeList_destroy(NodeList_ptr self)
{
  NODE_LIST_CHECK_INSTANCE(self);

  node_list_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Copies self and returns a new independent instance]

  Description        [Linear time]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr NodeList_copy(const NodeList_ptr self)
{
  NodeList_ptr copy;
  ListIter_ptr iter;

  NODE_LIST_CHECK_INSTANCE(self);

  copy = ALLOC(NodeList, 1);
  node_list_init(copy, Nil);

  NODE_LIST_FOREACH(self, iter) {
    node_ptr elem = NodeList_get_elem_at(self, iter);
    NodeList_append(copy, elem);
  }

  return copy;
}

/**Function********************************************************************

  Synopsis           [Appends a new node at the end of the list]

  Description        [Constant time]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void NodeList_append(NodeList_ptr self, node_ptr elem)
{
  Link* new_link;

  NODE_LIST_CHECK_INSTANCE(self);

  new_link = node_list_alloc_link(self);

  if (NULL_LINK == self->tail) {
    nusmv_assert(NULL_LINK == self->head);

    self->head = new_link;
  }
  else {
    new_link->prev = self->tail;
    self->tail->next = new_link;
  }

  self->tail = new_link;

  /* Finally set the element */
  new_link->element = elem;

  /* Remember the size.. */
  ++self->size;

  node_list_update_count(self, elem, false);
}


/**Function********************************************************************

  Synopsis           [Prepends a new node at the beginning of the list]

  Description        [Constant time]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void NodeList_prepend(NodeList_ptr self, node_ptr elem)
{
  Link* new_link;

  NODE_LIST_CHECK_INSTANCE(self);

  new_link = node_list_alloc_link(self);

  if (NULL_LINK == self->head) {
    nusmv_assert(NULL_LINK == self->tail);

    self->tail = new_link;
  }
  else {
    new_link->next = self->head;
    self->head->prev = new_link;
  }

  self->head = new_link;

  /* Finally set the element */
  new_link->element = elem;

  /* Remember the size.. */
  ++self->size;

  node_list_update_count(self, elem, false);
}



/**Function********************************************************************

  Synopsis [Inserts the given element before the node pointed by the
  given iterator]

  Description        [Constant time]

  SideEffects        []

  SeeAlso            [insert_after]

******************************************************************************/
void NodeList_insert_before(NodeList_ptr self, ListIter_ptr iter,
                            node_ptr elem)
{
  NODE_LIST_CHECK_INSTANCE(self);

  /* Insert tail */
  if (ListIter_is_end(iter)) {
    NodeList_append(self, elem);
  }
  else {
    Link* new_link = node_list_alloc_link(self);

    if (iter->prev == NULL_LINK) {
      nusmv_assert(iter == self->head);
      nusmv_assert(NULL_LINK != self->tail);

      self->head = new_link;
    }
    else {
      iter->prev->next = new_link;
    }

    new_link->prev = iter->prev;
    new_link->next = iter;
    iter->prev = new_link;

    /* Finally set the element */
    new_link->element = elem;

    ++self->size;
    node_list_update_count(self, elem, false);
  }
}


/**Function********************************************************************

  Synopsis [Inserts the given element after the node pointed by the
  given iterator]

  Description [Constant time. iter must be a valid iterator, and
  cannot point at the end of the list]

  SideEffects ]

  SeeAlso            [insert_before]

******************************************************************************/
void NodeList_insert_after(NodeList_ptr self, ListIter_ptr iter, node_ptr elem)
{
  Link* new_link;
  Link* next;

  NODE_LIST_CHECK_INSTANCE(self);

  /* Insert tail */
  nusmv_assert(!ListIter_is_end(iter));

  new_link = node_list_alloc_link(self);

  next = iter->next;

  iter->next = new_link;
  new_link->prev = iter;

  new_link->next = next;

  if (NULL_LINK == next) {
    nusmv_assert(self->tail == iter);
    self->tail = new_link;
  }
  else {
    next->prev = new_link;
  }

  /* Finally set the element */
  new_link->element = elem;

  ++self->size;
  node_list_update_count(self, elem, false);
}


/**Function********************************************************************

  Synopsis           [Removes the element pointed by the given iterator]

  Description        [The removed element is returned. The given iterator
                      won't be usable anymore. Constant time.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr NodeList_remove_elem_at(NodeList_ptr self, ListIter_ptr iter)
{
  node_ptr elem;

  NODE_LIST_CHECK_INSTANCE(self);
  nusmv_assert(NULL_LINK != iter);

  elem = iter->element;

  if (NULL_LINK == iter->prev) {
    nusmv_assert(iter == self->head);
    self->head = iter->next;
  }
  else {
    iter->prev->next = iter->next;
  }

  if (NULL_LINK == iter->next) {
    nusmv_assert(iter == self->tail);
    self->tail = iter->prev;
  }
  else {
    iter->next->prev = iter->prev;
  }

  node_list_free_link(self, iter);
  node_list_update_count(self, elem, true);

  --self->size;

  return elem;
}


/**Function********************************************************************

  Synopsis           [Removes the elements that are found in other list]

  Description        [Linear time on the size of self. No iteration is done
  if other is empty.  If not NULL, disposer is called on the removed
  element, passing disposer_arg. If the disposer returns true, the
  removal continues, otherwise it aborts and returns with the list as
  it is at that time. Returns the number of removed elements]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int NodeList_remove_elems(NodeList_ptr self, const NodeList_ptr other,
                          NodeListPred disposer, void* disposer_arg)
{
  ListIter_ptr iter;
  int count = 0;

  NODE_LIST_CHECK_INSTANCE(self);

  if (NodeList_get_length(other) == 0) {
    return 0;
  }

  iter = NodeList_get_first_iter(self);
  while(!ListIter_is_end(iter)) {
    node_ptr elem = NodeList_get_elem_at(self, iter);
    ListIter_ptr tmp = iter;
    iter = ListIter_get_next(iter);

    /* Chech whether elem has to be removed */
    if (NodeList_belongs_to(other, elem)) {
      NodeList_remove_elem_at(self, tmp);
      count ++;

      if (disposer != NULL) {
        boolean keep;

        keep = disposer(elem, disposer_arg);
        if (!keep) {
          return count;
        }
      }
    }
  }

  return count;
}


/**Function********************************************************************

  Synopsis           [Walks through the list, calling given funtion
  for each element]

  Description        [Returns the number of visited nodes, which can be less
  than the total number of elements since foo can decide to interrupt
  the walking]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int NodeList_foreach(NodeList_ptr self, NODE_LIST_FOREACH_FUN_P foo,
                     void* user_data)
{
  ListIter_ptr iter;
  boolean cont = true;
  int walks = 0;

  NODE_LIST_CHECK_INSTANCE(self);

  iter = NodeList_get_first_iter(self);
  while ( (! ListIter_is_end(iter)) && cont ) {
    cont = foo(self, iter, user_data);
    ++walks;
    iter = ListIter_get_next(iter);
  }

  return walks;
}


/**Function********************************************************************

  Synopsis           [Returns the number of elements in the list]

  Description        [Constant time]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int NodeList_get_length(const NodeList_ptr self)
{
  NODE_LIST_CHECK_INSTANCE(self);

  return self->size;
}


/**Function********************************************************************

  Synopsis           [Reverses the list]

  Description        [Linear time]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void NodeList_reverse(NodeList_ptr self)
{
  Link* curr = self->head;
  Link* tmp;

  self->head = self->tail;
  self->tail = curr;

  while (NULL_LINK != curr) {
    tmp = curr->next;
    curr->next = curr->prev;
    curr->prev = tmp;

    curr = tmp;
  }
}


/**Function********************************************************************

  Synopsis           [Append all the elements in src to self]

  Description        [Cost is linear in the size of src]

  SideEffects        [Content of self will change is src is not empty]

  SeeAlso            []

******************************************************************************/
void NodeList_concat(NodeList_ptr self, const NodeList_ptr src)
{
  Link* iter;

  NODE_LIST_CHECK_INSTANCE(self);
  NODE_LIST_CHECK_INSTANCE(src);

  iter = src->head;

  while (NULL_LINK != iter) {
    NodeList_append(self, iter->element);
    iter = iter->next;
  }
}

/**Function********************************************************************

  Synopsis           [Append all the elements in src to self, but only if
  each element does not occur in self already]

  Description        [Cost is linear in the size of src]

  SideEffects        [Content of self may change is src is not empty]

  SeeAlso            []

******************************************************************************/
void NodeList_concat_unique(NodeList_ptr self, const NodeList_ptr src)
{
  Link* iter;

  NODE_LIST_CHECK_INSTANCE(self);
  NODE_LIST_CHECK_INSTANCE(src);

  iter = src->head;

  while (NULL_LINK != iter) {

    if (!NodeList_belongs_to(self, iter->element)) {
      NodeList_append(self, iter->element);
    }

    iter = iter->next;
  }
}


/**Function********************************************************************

  Synopsis           [Returns true if given element belongs to self

  Description [Constant time (cost may depend on the internal hash
  status)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean NodeList_belongs_to(const NodeList_ptr self, node_ptr elem)
{
  return (Nil != find_assoc(self->count_hash, elem));
}


/**Function********************************************************************

  Synopsis           [Searches for an element in a list such that
  'pred'(element, 'arg') returns true.]

  Description        [Linear time search is used to
  find an element 'elem' such that function pred(elem, arg) returns
  true.
  An iterator pointing to the found element is returned.
  If the element is not found then ListIter_is_end will be true on the
  returned iterator.

  If pred is NULL then a search for an element equal to arg will be
  done (as if pred was a pointer-equality predicate). If pred is
  NULL and the searched element does not occur in the list, the
  function returns in constant time.]

  SideEffects        []

  SeeAlso            [ListIter_is_end, NodeList_belongs_to]

******************************************************************************/
ListIter_ptr
NodeList_search(const NodeList_ptr self, NodeListPred pred, void* arg)
{
  ListIter_ptr iter;

  if (pred == NULL) {
    if (!NodeList_belongs_to(self, (node_ptr) arg)) return NULL_LINK;

    NODE_LIST_FOREACH(self, iter) {
      if (NodeList_get_elem_at(self, iter) == (node_ptr) arg) return iter;
    }
  }
  else {
    NODE_LIST_FOREACH(self, iter) {
      if (pred(NodeList_get_elem_at(self, iter), arg)) return iter;
    }
  }

  return iter; /* end of list */
}


/**Function********************************************************************

  Synopsis           [Returns the number of occurrences of the given element]

  Description        [Constant time (cost may depend on the internal hash
  status)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int NodeList_count_elem(const NodeList_ptr self, node_ptr elem)
{
  return NODE_TO_INT(find_assoc(self->count_hash, elem));
}


/**Function********************************************************************

  Synopsis           [Returns the iterator pointing to the first element]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ListIter_ptr NodeList_get_first_iter(const NodeList_ptr self)
{
  return self->head;
}

/**Function********************************************************************

  Synopsis           [Returns the element at the position pointed by iter]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr NodeList_get_elem_at(const NodeList_ptr self, const ListIter_ptr iter)
{
  nusmv_assert(NULL_LINK != iter);
  return iter->element;
}

/**Function********************************************************************

  Synopsis           [Returns the following iterator]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ListIter_ptr ListIter_get_next(const ListIter_ptr self)
{
  nusmv_assert(NULL_LINK != self);

  return self->next;
}


/**Function********************************************************************

  Synopsis           [Returns true if the iteration is given up]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
boolean ListIter_is_end(const ListIter_ptr self)
{
  return (NULL_LINK == self);
}

/**Function********************************************************************

  Synopsis           [Returns the end iterator]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ListIter_ptr ListIter_get_end()
{
  return NULL_LINK;
}

/**Function********************************************************************

  Synopsis           [Returns a new list that contains all elements of
  self, after applying function foo to each element]

  Description [Elements are not copied. Returned list must be
  freed by the caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr NodeList_map(const NodeList_ptr self, NPFN foo)
{
  NodeList_ptr res;
  ListIter_ptr iter;

  NODE_LIST_CHECK_INSTANCE(self);
  res = NodeList_create();
  for (iter=NodeList_get_first_iter(self); !ListIter_is_end(iter);
       iter=ListIter_get_next(iter)) {
    NodeList_append(res, foo(NodeList_get_elem_at(self, iter)));
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns a new list that contains all elements of
  self for which function foo returned true. ]

  Description [Elements are not copied. Returned list must be
  freed by the caller]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr NodeList_filter(const NodeList_ptr self, BPFN foo)
{
  NodeList_ptr res;
  ListIter_ptr iter;

  NODE_LIST_CHECK_INSTANCE(self);
  res = NodeList_create();
  for (iter=NodeList_get_first_iter(self); !ListIter_is_end(iter);
       iter=ListIter_get_next(iter)) {
    node_ptr el = NodeList_get_elem_at(self, iter);
    if (foo(el)) NodeList_append(res, el);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Prints the nodes in the list, separated by spaces]

  Description        [The list must be a list of actual node_ptr]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void NodeList_print_nodes(const NodeList_ptr self, FILE* out)
{
  ListIter_ptr iter;
  for (iter = NodeList_get_first_iter(self);
       !ListIter_is_end(iter); iter = ListIter_get_next(iter)) {
    print_node(out, NodeList_get_elem_at(self, iter));
    fprintf(out, " ");
  }
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private initializer]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void node_list_init(NodeList_ptr self, node_ptr list)
{

  self->head = NULL_LINK;
  self->tail = NULL_LINK;

  if (0 == reference_counter) {
    nusmv_assert(NULL_LINK == pool);
    nusmv_assert(STACK(NULL) == chunks);

    pool = NULL_LINK;
    chunks = Stack_create();
  }

  ++reference_counter;

  self->count_hash = new_assoc();
  self->size = 0;

  while (Nil != list) {
    NodeList_append(self, car(list));
    list = cdr(list);
  }
}


/**Function********************************************************************

  Synopsis           [Private deinitializer]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void node_list_deinit(NodeList_ptr self)
{

  nusmv_assert(reference_counter > 0);
  --reference_counter;

  if (0 == reference_counter) {
    nusmv_assert(NULL_LINK != pool);
    nusmv_assert(STACK(NULL) != chunks);

    while (!Stack_is_empty(chunks)) {
      Link* chunk = Stack_pop(chunks);
      FREE(chunk);
    }

    Stack_destroy(chunks);

    chunks = STACK(NULL);
    pool = NULL_LINK;
  }
  else {
    Link* l = self->head;
    Link* t;
    int i = 0;

    while (NULL_LINK != l) {
      t = l->next;
      node_list_free_link(self, l);
      l = t; ++i;
    }

    nusmv_assert(i == self->size);
  }

  free_assoc(self->count_hash);
}

/**Function********************************************************************

  Synopsis           [Retrieves a Link instance in the pool]

  Description        [Retrieves a Link instance in the pool.
                      If the pool is empty, allocates LINK_CHUNK_SIZE
                      new Link instances (by using only 1 ALLOC)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static inline Link* node_list_alloc_link(const NodeList_ptr self)
{
  Link* res = NULL_LINK;

  nusmv_assert(STACK(NULL) != chunks);

  if (NULL_LINK == pool) {
    Link* chunk = ALLOC(Link, LINK_CHUNK_SIZE);
    int i;

    Stack_push(chunks, chunk);
    pool = chunk;

    for (i = 0; i < LINK_CHUNK_SIZE - 1; ++i) {
      (chunk + i)->next = (chunk + i + 1);
    }

    (chunk + i)->next = NULL_LINK;
  }

  res = pool;
  pool = res->next;

  nusmv_assert(NULL_LINK != res);

  res->prev = NULL_LINK;
  res->next = NULL_LINK;
  res->element = Nil;

  return res;
}

/**Function********************************************************************

  Synopsis           [Puts the given Link instance in the pool]

  Description        [Puts the given Link instance in the pool]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static inline void node_list_free_link(const NodeList_ptr self, Link* link)
{
  Link* tmp = pool;
  pool = link;
  link->next = tmp;
}

/**Function********************************************************************

  Synopsis           [Keeps count of the number of duplicate elements
                      in the list]

  Description        [Keeps count of the number of duplicate elements
                      in the list]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static inline void
node_list_update_count(const NodeList_ptr self, const node_ptr elem,
                       const boolean deleting)
{
  int val = NODE_TO_INT(find_assoc(self->count_hash, elem));
  insert_assoc(self->count_hash, elem,
               (deleting ? NODE_FROM_INT(val - 1) : NODE_FROM_INT(val + 1)));
}
