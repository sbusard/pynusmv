/**CFile***********************************************************************

  FileName    [Sset.c]

  PackageName [addons.omcare]

  Synopsis    [Implementation of Sset (Sorted Set) class]

  Description [Sset class implements a set (container) of element
  ordered according to their keys (max 64 bit signed integer
  values) and having some data (void* values). Note that keys have
  to be unique, i.e. this class does not support multi-sets.

  The implementation of this classes is based on AVL Tree -- a
  balanced binary search tree kept balanced using the AVL algorithm
  invented by G.M. Adelson-Velsky and E.M. Landis. The description of
  the algorithm is taken from "The art of computer programming" vol 3 by
  Knuth. Some inspiration to implementation was taken from Goletas Library
  by Maksim Goleta.

  The main functionality is:
  * the insertion, search for and deletion of an element is performed
    in time O(log2 N) where N is the number of elements in the set.
  ]

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

#include <stddef.h>
#include "Sset.h"
#include "utils/error.h" /* for rpterr */
#include "utils/defs.h" 

/*---------------------------------------------------------------------------*/
static char rcsid[] UTIL_UNUSED = "$Id: Sset.c,v 1.1.2.4 2009-09-04 11:44:30 nusmv Exp $";
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/
/**Enum**********************************************************************

  Synopsis    [Define a range of possible balances of a tree node]

  Description [Note for developers: See the description of struct Ssnode_TAG
  and for more info]

  SeeAlso     []

******************************************************************************/
enum SSET_BALANCE {
  SSET_BALANCED = 0, /* both children have the same height */
  SSET_R_BALANCED = 1, /* right child has height 1 greater than the left one */
  SSET_L_BALANCED = 3, /* left child has height 1 greater than the right one */
};

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro**********************************************************************

  Synopsis    [This macro returns the balance field of a tree node]

  Description [See the description of struct Ssnode_TAG for more info.]

  SeeAlso     []

******************************************************************************/
#define SSET_GET_BALANCE(node)    (PTR_TO_INT((node)->parent) & 0x3)


/**Macro**********************************************************************

  Synopsis    [This macro sets up the balance field of a tree node]

  Description [See the description of struct Ssnode_TAG for more info.
  NOTE: the integer number "balance" can be only one of SSET_BALANCE]

  SeeAlso     []

******************************************************************************/
#define SSET_SET_BALANCE(node, balance) \
  { DEBUG(nusmv_assert((balance & ~0x3) == 0));                         \
    ((node)->parent =                                                    \
        PTR_FROM_INT(void*, (PTR_TO_INT((node)->parent) & ~0x3) | (balance))); \
  }

/**Macro**********************************************************************

  Synopsis    [This macro increments balance]

  Description [If balance was SSET_L_BALANCED it becomes SSET_BALANCED,
  if it was SSET_BALANCED it becomes SSET_R_BALANCED, otherwise
  the behavior is undefined.
  See the description of struct Ssnode_TAG for more info.]

  SeeAlso     []

******************************************************************************/
#define SSET_INC_BALANCE(node) \
  ((node)->parent =                                                     \
      PTR_FROM_INT(void*, (((PTR_TO_INT((node)->parent) & 0x3) + 1) & 0x3) \
                          | (PTR_TO_INT((node)->parent) & ~0x3))          \
  )


/**Macro**********************************************************************

  Synopsis    [This macro decrement balance]

  Description [If balance was SSET_R_BALANCED it becomes SSET_BALANCED,
  if it was SSET_BALANCED it becomes SSET_L_BALANCED, otherwise
  the behavior is undefined.
  See the description of struct Ssnode_TAG for more info.]

  SeeAlso     []

******************************************************************************/
#define SSET_DEC_BALANCE(node) \
  ((node)->parent =                                                     \
      PTR_FROM_INT(void*, (((PTR_TO_INT((node)->parent) & 0x3) - 1) & 0x3) \
                          | (PTR_TO_INT((node)->parent) & ~0x3))          \
  )


/**Macro**********************************************************************

  Synopsis    [Returns a pointer to the parent of a give node]

  Description [See the description of struct Ssnode_TAG for more info.]

  SeeAlso     []

******************************************************************************/
#define SSET_GET_PARENT(node) PTR_FROM_INT(void*, PTR_TO_INT((node)->parent) & ~0x3)


/**Macro**********************************************************************

  Synopsis    [Sets a pointer to the parent of a give node]

  Description [See the description of struct Ssnode_TAG for more info.]

  SeeAlso     []

******************************************************************************/
#define SSET_SET_PARENT(node, _parent) \
  { DEBUG(nusmv_assert((PTR_TO_INT(_parent) & 0x3) == 0));               \
    (node)->parent = PTR_FROM_INT(void*, (PTR_TO_INT((node)->parent) & 0x3) \
                                         | PTR_TO_INT(_parent));             \
  }

/**Macro**********************************************************************

  Synopsis    [Sets the parent pointer and the balance of a given node]

  Description [See the description of struct Ssnode_TAG for more info.
  "balance" has to be one of SSET_BALANCE]

  SeeAlso     []

******************************************************************************/
#define SSET_SET_PARENT_BALANCE(node, _parent, balance) \
  { DEBUG(nusmv_assert((PTR_TO_INT(_parent) & 0x3) == 0 && ((balance) & ~0x3) == 0)); \
    ((node)->parent = PTR_FROM_INT(void*, PTR_TO_INT(_parent) | (balance))) ; \
  }


/**Macro**********************************************************************

  Synopsis    [Sets the parent pointer and the balance of a given node
  taking the values from another node]

  Description [See the description of struct Ssnode_TAG for more info.
  "balance" has to be one of SSET_BALANCE]

  SeeAlso     []

******************************************************************************/
#define SSET_COPY_PARENT_BALANCE_TO(node, another) \
  ((node)->parent = (another)->parent)

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct*********************************************************************

  Synopsis    [Implementation of Sset class]

  Description []

  SeeAlso     []

******************************************************************************/
struct Sset_TAG {
  Ssnode_ptr root; /* the root of the binary tree */
  size_t size; /* the number of elements in the tree */
};

typedef struct Sset_TAG Sset;


/**Struct*********************************************************************

  Synopsis    [A node of the tree]

  Description [ NOTE for developers: the structure is the standard AVL
  tree node. The only difference is that the "balance" field is kept in the 2
  lowest bits of the "parent" field. Note that implementation of 
  SSET_BALANCE and a few macros depends on this agreement, 
  and there is an assertion that pointer should always have two lowest bits
  set to 0.]

  SeeAlso     []

******************************************************************************/
struct Ssnode_TAG {
  signed long long int key;  /* the key of a node */
  Ssnode_ptr left; /* the left child of the node, i.e. a set of smaller elements.
                      This pointer may be NULL */
  Ssnode_ptr right; /* the right child of the node, i.e. a set of greater elements.
                       This pointer may be NULL */
  Ssnode_ptr parent; /* the parent of the node. It can be NULL iff 
                        the given node is the root of the tree */
  void* element; /* an element stored in this node */
};

typedef struct Ssnode_TAG Ssnode;


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
static void s_set_init ARGS((Sset_ptr self));
static void s_set_deinit ARGS((Sset_ptr self));

static Ssnode_ptr s_set_copy ARGS((Ssnode_ptr node, void* (*func)(void*)));

static Ssnode_ptr s_set_new_node ARGS((signed long long int key, 
                                       Ssnode_ptr parent));
static size_t s_set_free_nodes ARGS((Ssnode_ptr node));

static Ssnode_ptr s_set_find ARGS((Sset_ptr self, signed long long int key));
static Ssnode_ptr s_set_find_closest_le ARGS((Sset_ptr self, 
                                              signed long long int key));
static Ssnode_ptr s_set_find_closest_ge ARGS((Sset_ptr self, 
                                              signed long long int key));
static Ssnode_ptr s_set_find_insert ARGS((Sset_ptr self, 
                                          signed long long int key,
                                          boolean* is_found));

static void s_set_delete_node ARGS((Sset_ptr self, Ssnode_ptr node));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Creates an instance of a Sorted Set ]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Sset_ptr Sset_create()
{
  Sset_ptr self = ALLOC(Sset, 1);
  s_set_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis    [Destroys a set instance]

  Description [The memory used by the set will be freed. 
  Note: memory occupied by the elements is not freed! It is the user
  responsibility.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Sset_destroy (Sset_ptr self)
{
  s_set_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Creates a copy of the given set instance]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Sset_ptr Sset_copy (const Sset_ptr self)
{
  Sset_ptr new_set = Sset_create();
  new_set->size = self->size;
  new_set->root = self->root == NULL ? NULL : s_set_copy(self->root, NULL);
  return new_set;
}


/**Function********************************************************************

  Synopsis    [Creates a copy of the given set instance, copying each
               element by calling given function.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
Sset_ptr Sset_copy_func (const Sset_ptr self, void* (*func)(void*))
{
  Sset_ptr new_set = Sset_create();
  new_set->size = self->size;
  new_set->root = self->root == NULL ? NULL : s_set_copy(self->root, func);
  return new_set;
}


/**Function********************************************************************

  Synopsis    [Insert an element "element" under 
  the key "key" into the set]

  Description [
  Returns true if a new node was created and false if a node with the given
  key has already existed (in which case nothing is changed).
  Note: all the existing iterators remain valid.
  ]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Sset_insert(Sset_ptr self, signed long long int key, void* element)
{

  boolean is_found;
  Ssnode_ptr new_node = s_set_find_insert(self, key, &is_found);
  
  if (is_found) return false;

  new_node->element = element;
  return true;
}


/**Function********************************************************************

  Synopsis    [Looks up for an element with a given key]

  Description [Returns an iterator pointing to the found element.
  If there is no such element Ssiter_is_valid() returns false
  on the returned iterator.

  The operation takes O(log2 N) time (N is the size of the set).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Ssiter Sset_find(Sset_ptr self, signed long long int key)
{
  Ssiter iter = {.node = s_set_find(self, key)};
  return iter;
}


/**Function********************************************************************

  Synopsis         [Looks up for the closest element whose key is less than
                    or equal a given key.]

  Description      [Returns an iterator pointing to the found element.
                    If there is no such element Ssiter_is_valid()
                    returns false on the returned iterator.

                    The operation takes O(log2 N) time (N is the
                    size of the set).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Ssiter Sset_find_le(Sset_ptr self, signed long long int key)
{
  Ssiter iter = {.node = s_set_find_closest_le(self, key)};
  return iter;
}


/**Function********************************************************************

  Synopsis         [Looks up for the closest element whose key is greater than
                    or equal a given key.]

  Description      [Returns an iterator pointing to the found element.
                    If there is no such element Ssiter_is_valid()
                    returns false on the returned iterator.

                    The operation takes O(log2 N) time (N is the
                    size of the set).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Ssiter Sset_find_ge(Sset_ptr self, signed long long int key)
{
  Ssiter iter = {.node = s_set_find_closest_ge(self, key)};
  return iter;
}


/**Function********************************************************************

  Synopsis    [Looks up for an element with a given key and if does not 
  exist it is created]

  Description [Returns an iterator pointing to the found (created) element.
  If is_found != NULL, *is_found is set to true if the element 
  was found and false if it was created.

  The operation takes O(log2 N) time (N is the size of the set).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
Ssiter Sset_find_insert(Sset_ptr self, signed long long int key, 
                        boolean* is_found)
{
  boolean _is_found;
  Ssiter iter = {.node = s_set_find_insert(self, key, &_is_found)};
  if (NULL != is_found) *is_found = _is_found;
  return iter;
}


/**Function********************************************************************

  Synopsis    [Removes an element with key "key" from the set.]

  Description [The returned value is the element stored in the deleted node.
  If parameter "is_found" is no NULL, "*is_found" is set to
  true if such an element with the provided key was found, and false otherwise.
  Note: if an element with the key does no exist in the set the 
  returned value is NULL.

  The operation takes O(log2 N) time (N is the size of the set).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void* Sset_delete(Sset_ptr self, signed long long int key, boolean* is_found)
{
  Ssnode_ptr to_be_deleted = s_set_find(self, key); /* a node to be deleted */
  void* element;

  if (NULL == to_be_deleted) { /* no element with key "key" exists */
    if (NULL != is_found) *is_found = false;
    return NULL;
  }
 
  if (NULL != is_found) *is_found = true;
  element = to_be_deleted->element;
  s_set_delete_node(self, to_be_deleted);

  return element;

}


/**Function********************************************************************

  Synopsis    [Removes an element pointed by the iterator.]

  Description [
  Precondition: the iterator should be returned one by
  Ssiter_first, Ssiter_last, Ssiter_next, Ssiter_prev. 
  Precondition: an element pointed by iterator has to belong to this set.
  Precondition: Ssiter_is_valid(iter) has to be return true.

  WARNING: After this function call the iterator will have undefined value
  and no operation is allowed with it except assignment of a new value.
  
  The operation takes O(log2 N) time (N is the size of the set).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Sset_delete_iter(Sset_ptr self, Ssiter iter)
{
  s_set_delete_node(self, iter.node);
  return ;
}


/**Function********************************************************************

  Synopsis    [Returns the number of elements in a set]

  Description [Constant time operation]

  SideEffects []

  SeeAlso     []

******************************************************************************/
size_t Sset_get_size(Sset_ptr self)
{
  return self->size;
}


/**Function********************************************************************

  Synopsis    [Returns true iff the set is empty]

  Description [Constant time operation]

  SideEffects []

  SeeAlso     []

******************************************************************************/
boolean Sset_is_empty(Sset_ptr self)
{
  return NULL == self->root;
}


/**Function********************************************************************

  Synopsis    [Returns an iterator pointing to a first element of a set,
  i.e. element with the smallest key.]

  Description [If the set is empty Ssiter_is_valid() will be false
  on the returned iterator.
  NOTE: there is no need to free the iterator after using it.
  NOTE: it is allowed to assign one iterator to another one.
  NOTE: The operation may take up to O(log2 N) time (N is the size of the set). 
  ]

  SideEffects []

  SeeAlso     [Ssiter_is_end, Ssiter_next, Ssiter_element]

******************************************************************************/
Ssiter Sset_first(Sset_ptr self)
{
  Ssiter iter = {.node = self->root };
  if (NULL != iter.node) {
    while (iter.node->left != NULL) iter.node = iter.node->left;
  }

  return iter;
}


/**Function********************************************************************

  Synopsis    [Returns an iterator pointing to the last element of a set,
  i.e. element with the greatest key.]

  Description [If the set is empty Ssiter_is_valid() will be false
  on the returned iterator.
  NOTE: there is no need to free the iterator after using it.
  NOTE: it is allowed to assign one iterator to another one.
  NOTE: The operation may take up to O(log2 N) time (N is the size of the set). 
  ]

  SideEffects []

  SeeAlso     [Ssiter_is_end, Ssiter_next, Ssiter_element]

******************************************************************************/
Ssiter Sset_last(Sset_ptr self)
{
  Ssiter iter = {.node = self->root};
  if (NULL != iter.node) {
    while (iter.node->right != NULL) iter.node = iter.node->right;
  }

  return iter;
}


/**Function********************************************************************

  Synopsis    [Returns an iterator pointing to the next element 
  of a set w.r.t. the element pointed by a provided iterator, i.e.
  element with a greater key.]

  Description [ Precondition: this function can be applied only if
  Ssiter_is_valid(iter) returns true.  ]

  SideEffects []

  SeeAlso     [Sset_first, Ssiter_is_end, Ssiter_element]

******************************************************************************/
Ssiter Ssiter_next(Ssiter iter)
{
  Ssnode_ptr p = iter.node;

  /* if there is a right child then the next element is
     its lowest left child */
  if (p->right != NULL) { 
    p = p->right;
    while (p->left != NULL) p = p->left;
  }
  else {
    /* if there is no right child => find the first parent 
       which we reach through a left child */
    Ssnode_ptr parent;
    while ((parent = SSET_GET_PARENT(p)) != NULL && p == parent->right) {
      p = parent;
    }
    p = parent;
  }

  iter.node = p;
  return iter;
}


/**Function********************************************************************

  Synopsis    [Returns an iterator pointing to the previous element 
  of a set w.r.t. the element pointed by a provided iterator, i.e.
  element with a smaller key.]

  Description [ Precondition: this function can be applied only if
  Ssiter_is_valid(iter) returns true.]

  SideEffects []

  SeeAlso     [Sset_first, Ssiter_is_valid, Ssiter_element]

******************************************************************************/
Ssiter Ssiter_prev(Ssiter iter)
{
  Ssnode_ptr p = iter.node;

  /* if there is a left child then the previous element is
     its lowest right child */
  if (p->left != NULL) { /* return the left child if any */
    p = p->left;
    while (p->right != NULL) p = p->right;
  }
  else {
    /* if there is no left child => find the first parent 
       which we reach through a right child */
    Ssnode_ptr parent;
    while ((parent = SSET_GET_PARENT(p)) != NULL && p == parent->left) {
      p = parent;
    }
    p = parent;
  }

  iter.node = p;
  return iter;
}


/**Function********************************************************************

  Synopsis    [Returns true iff an iterator points a valid node
  of a set, i.e. not past the last element or before the first element
  of a set.]

  Description [The iterator must have been created with function
  Sset_first, Sset_last, Sset_next or Sset_prev.
  NOTE: the function is constant time.
  WARNING: if the function returns false no other function
  should be invoked on the given iterator!]

  SideEffects []

  SeeAlso     [Sset_first, Ssiter_next, Ssiter_element]

******************************************************************************/
boolean Ssiter_is_valid(Ssiter iter)
{
  return NULL != iter.node;
}


/**Function********************************************************************

  Synopsis    [Returns a value stored in an element pointed by 
  a provided iterator]

  Description [Precondition: this function can be applied only
  if Ssiter_is_valid(iter) returns true]

  SideEffects []

  SeeAlso     [Sset_first, Sset_next, Ssiter_is_valid, Ssiter_next]

******************************************************************************/
void* Ssiter_element(Ssiter iter)
{
  nusmv_assert(iter.node != NULL); /* iterator is past the last element */
  return iter.node->element;
}


/**Function********************************************************************

  Synopsis    [Returns a key stored in an element pointed by 
  a provided iterator (and which was used to order the elements)]

  Description [Precondition: this function can be applied only
  if Ssiter_is_valid(iter) returns true]

  SideEffects []

  SeeAlso     [Sset_first, Sset_next, Ssiter_is_valid, Ssiter_next]

******************************************************************************/
signed long long int Ssiter_key(Ssiter iter)
{
  nusmv_assert(iter.node != NULL); /* iterator is past the last element */
  return iter.node->key;
}


/**Function********************************************************************

  Synopsis    [Sets up a value stored in an element pointed by 
  a provided iterator]

  Description [Precondition: this function can be applied only
  if Ssiter_is_valid(iter) returns true]

  SideEffects []

  SeeAlso     [Sset_first, Sset_next, Ssiter_is_valid, Ssiter_next]

******************************************************************************/
void Ssiter_set_element(Ssiter iter, void* element)
{
  nusmv_assert(iter.node != NULL); /* iterator is past the last element */
  iter.node->element = element;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/* used in Ssiter_test */
static long time_to_free;
static size_t num_of_free;
/**Function********************************************************************

  Synopsis    [The function tests the class implementation]

  Description [The function is no in the interface but can be invoked 
  by Developers in order to test the implementation of the class.

  This function should be used only by developers to test the changed in the class.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void Ssiter_test()
{
  if (DEBUG(0 &&) 1) rpterr("To test the class Sset macro DEBUG should be enabled"
                            " in file Sset.c \n");
  Sset_ptr set1, set2;
  Ssiter iter1, iter2;
  int i, n1, n2;
  long t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11;
  const size_t N = 1000000; /* the number of created elements */
  signed long long int buf[N];
  boolean b;

  set1 = Sset_create();   nusmv_assert(Sset_is_empty(set1));
  Sset_destroy(set1);

  set1 = Sset_create();   nusmv_assert(Sset_get_size(set1) == 0);
  set2 = Sset_copy(set1); nusmv_assert(Sset_get_size(set2) == 0);
  Sset_destroy(set1);     nusmv_assert(Sset_get_size(set2) == 0);
  Sset_destroy(set2);

  for (i = 0; i < N; ++i) buf[i] = -1;

  /* find-insertion */
  t1 = util_cpu_time();
  set1 = Sset_create();
  for (i = 0, n1 = 0; i < N; ++i) {
    int r = utils_random(); if (r == -1) r = 0; /* -1 is a special value */
    iter1 = Sset_find_insert(set1, r, &b);
    
    if (!b) { 
      n1 += 1; 
      Ssiter_set_element(iter1, PTR_FROM_INT(void*, i));
      buf[i] = r; 
    }
  }
  t1 = util_cpu_time() - t1;
  nusmv_assert(Sset_get_size(set1) == n1);

  printf("TEST of Sorted Set Class (Sset) is finished successfully.\n"
         "The following statistics has been collected (for a table with %d elements):\n"
         "\t time to create a set (with %lu repeated tries): %.3f\n",
         n1, N - n1, (float)t1 / 1000);
         
  /* forward walking + accessing */
  t2 = util_cpu_time();
  n2 = 0;
  for (iter1 = Sset_first(set1); Ssiter_is_valid(iter1) ; iter1 = Ssiter_next(iter1)) {
    n2 ++;
    nusmv_assert(buf[(size_t)Ssiter_element(iter1)] == Ssiter_key(iter1));
  }
  t2 = util_cpu_time() - t2;
  nusmv_assert(n1 == n2);
    
  printf("\t time of forward walk with access: %.3f\n", (float)t2 / 1000);

  /* backward walking + accessing */
  t3 = util_cpu_time();
  n2 = 0;
  for (iter1 = Sset_last(set1); Ssiter_is_valid(iter1) ; iter1 = Ssiter_prev(iter1)) {
    n2 ++;
    nusmv_assert(buf[(size_t)Ssiter_element(iter1)] == Ssiter_key(iter1));
  }
  t3 = util_cpu_time() - t3;
  nusmv_assert(n1 == n2);

  printf("\t time of backward walk with access: %.3f\n",(float)t3 / 1000);

  /* finding */
  t4 = util_cpu_time();
  for (i = 0; i < N; ++i) {
    iter1 = Sset_find(set1, buf[i]);
    nusmv_assert((buf[i] == -1) == !Ssiter_is_valid(iter1));
    nusmv_assert(!Ssiter_is_valid(iter1) || (size_t)Ssiter_element(iter1) == i);
  }
  t4 = util_cpu_time() - t4;
  
  printf("\t time of searching for all elements (with %lu failed tries): %.3f\n",
         N - n1, (float)t4 / 1000);

  /* copying */
  t5 = util_cpu_time();
  set2 = Sset_copy(set1);
  t5 = util_cpu_time() - t5;
  
  printf("\t time of copying: %.3f\n", (float)t5 / 1000);

  /* destroying*/
  t6 = util_cpu_time();
  time_to_free = 0;
  num_of_free = 0;
  Sset_destroy(set1);
  t6 = util_cpu_time() - t6;

  printf("\t time of destroying the set of %" PRIuPTR \
         " elements: %.3f (freeing took %.3f)\n",
         num_of_free, (float)t6 / 1000, (float)time_to_free / 1000);

  /* removing elements randomly */
  t7 = util_cpu_time();
  time_to_free = 0;
  num_of_free = 0;
  for (i = 0; i < N; ++i) {
    size_t x = (size_t)Sset_delete(set2, buf[i], &b);
    nusmv_assert((b && i == x) || (!b && buf[i] == -1 && x == 0));
  }
  t7 = util_cpu_time() - t7;
  nusmv_assert(Sset_is_empty(set2));
  Sset_destroy(set2);
 
  printf("\t time of deleting %" PRIuPTR \
         " elements randomly (with %" PRIuPTR " failed tries) : " \
         "%.3f(freeing took %.3f)\n",
         num_of_free, N - n1, (float)t7 / 1000, (float)time_to_free / 1000);

  /* inserting elements in direct order */
  t8 = util_cpu_time();
  set1 = Sset_create();
  for (i = 0; i < n1; ++i) {
    b = Sset_insert(set1, i, PTR_FROM_INT(void*, i));
    nusmv_assert(b);
  }
  t8 = util_cpu_time() - t8;

  printf("\t time of inserting element in direct order: %.3f\n", (float)t8 / 1000);

  /* removing elements in direct order */
  t9 = util_cpu_time();
  time_to_free = 0;
  num_of_free = 0;
  iter1 = Sset_first(set1); 
  i = 0;
  while (Ssiter_is_valid(iter1)) {
    iter2 = Ssiter_next(iter1);
    nusmv_assert(Ssiter_key(iter1) == i && (size_t)Ssiter_element(iter1) == i);
    Sset_delete_iter(set1, iter1);
    iter1 = iter2;
    ++i;
  }
  t9 = util_cpu_time() - t9;
  nusmv_assert(i == n1);
  nusmv_assert(Sset_is_empty(set1));
  Sset_destroy(set1);
  
  printf("\t time of removing %" PRIuPTR \
         " elements in direct order: %.3f (freeing took %.3f)\n",
         num_of_free, (float)t9 / 1000, (float)time_to_free / 1000);

  /* inserting elements in opposite order */
  t10 = util_cpu_time();
  set1 = Sset_create();
  for (i = n1-1; i >=0; --i) {
    b = Sset_insert(set1, i, PTR_FROM_INT(void*, i));
    nusmv_assert(b);
  }
  t10 = util_cpu_time() - t10;

  printf("\t time of inserting elements in opposite order: %.3f\n", (float)t10/ 1000);

  /* removing elements in opposite order */
  t11 = util_cpu_time();
  time_to_free = 0;
  num_of_free = 0;
  iter1 = Sset_last(set1); 
  i = n1 - 1;
  while (Ssiter_is_valid(iter1)) {
    iter2 = Ssiter_prev(iter1);
    nusmv_assert(Ssiter_key(iter1) == i && (size_t)Ssiter_element(iter1) == i);
    Sset_delete_iter(set1, iter1);
    iter1 = iter2;
    --i;
  }
  t11 = util_cpu_time() - t11;
  nusmv_assert(i == -1);
  nusmv_assert(Sset_is_empty(set1));
  Sset_destroy(set1);
  
  printf("\t time of removing %" PRIuPTR \
         " elements in opposite order: %.3f (freeing took %.3f)\n", 
         num_of_free, (float)t11 / 1000, (float)time_to_free / 1000);

  printf("\t total time: %.3f\n",
         (float)(t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11)/1000);

  return;
}



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Initializes the memory for a set instance]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void s_set_init(Sset_ptr self)
{
  SSET_CHECK_INSTANCE(self);
  
  self->root = NULL;
  self->size = 0;
}


/**Function********************************************************************

  Synopsis    [Deinitializes the memory from a set]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void s_set_deinit(Sset_ptr self)
{
  SSET_CHECK_INSTANCE(self);

  if (NULL != self->root) {
    DEBUG(size_t i = ) s_set_free_nodes(self->root);
    DEBUG(nusmv_assert(i == self->size));
  }
  else {
    nusmv_assert(0 == self->size);
  }
  self->root = NULL; /* for debugging */
  self->size = -1; /* for debugging */
}


/**Function********************************************************************

  Synopsis    [Creates a copy of a given tree]

  Description [Precondition: the tree should no be empty.
  Warning: the parent of returned node equals the parent of the input node.

  Optional func parameter is a function takinf voi* and returning void*, for 
  copying the elements]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static Ssnode_ptr s_set_copy(Ssnode_ptr node, void* (*func)(void*))
{
  Ssnode_ptr left  = node->left != NULL ? s_set_copy(node->left, func) : NULL;
  Ssnode_ptr new_node = ALLOC(Ssnode, 1);
  Ssnode_ptr right = node->right != NULL ? s_set_copy(node->right, func) : NULL;

  new_node->key = node->key;
  new_node->left = left;
  new_node->right = right;
  /* note: the parent is set of another tree but it will be reset 
     by previously invoked instance of this function */
  SSET_COPY_PARENT_BALANCE_TO(new_node, node); 
  
  new_node->element = (NULL == func) ? node->element : func(node->element);

  if (left != NULL)  SSET_SET_PARENT(left, new_node);
  if (right != NULL) SSET_SET_PARENT(right, new_node);

  return new_node;
}


/**Function********************************************************************

  Synopsis    [Function allocates a new Ssnode and sets its 
  fields to the provided values.]

  Description ["left", "right", "element" and "balance" is set to 0.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static Ssnode_ptr s_set_new_node(signed long long int key, Ssnode_ptr parent)
{
  Ssnode_ptr p = ALLOC(Ssnode, 1);
  p->key = key;
  p->left = NULL;
  p->right = NULL;
  SSET_SET_PARENT_BALANCE(p, parent, SSET_BALANCED);
  p->element = NULL;

  return p;
}


/**Function********************************************************************

  Synopsis    [Function de-allocates a Ssnode and all its children]

  Description [The elements themselves are not freed.  The returned
  value is the number of freed elements (returned only if DEBUG is
  enabled).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static size_t s_set_free_nodes(Ssnode_ptr node)
{
  DEBUG(size_t i = 0);
  if (NULL != node->left) {
    DEBUG(nusmv_assert(node->left->key < node->key));
    DEBUG(i +=) s_set_free_nodes(node->left);
  }
  if (NULL != node->right) {
    DEBUG(nusmv_assert(node->right->key > node->key));
    DEBUG(i +=) s_set_free_nodes(node->right);
  }

  DEBUG(time_to_free -= util_cpu_time());

  FREE(node);

  DEBUG(time_to_free += util_cpu_time(); num_of_free += 1;);

  return DEBUG(i + ) 1;
}

/**Function********************************************************************

  Synopsis    [Looks for an element with a given key]

  Description [The found element is returned. If no such element
  exists NULL is returned]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static Ssnode_ptr s_set_find(Sset_ptr self, signed long long int key)
{
  Ssnode_ptr p = self->root;

  while (p != NULL) {
    if (key < p->key) p = p->left;
    else if (key > p->key)  p = p->right;
    else return p;
  }
  return NULL;
}


/**Function********************************************************************

  Synopsis [Looks for an element closest (less or equal than) a given key]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static Ssnode_ptr s_set_find_closest_le(Sset_ptr self, signed long long int key)
{
  Ssnode_ptr p = self->root;
  Ssnode_ptr best = (Ssnode_ptr) NULL;

  while (p != NULL) {
    if (key < p->key) p = p->left;
    else if (key > p->key)  {
      best = p;
      p = p->right; /* see if can find some better value */
    }
    else return p; /* found it! */
  }

  return best;
}


/**Function********************************************************************

  Synopsis [Looks for an element closest (greater or equal than) a given key]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
static Ssnode_ptr s_set_find_closest_ge(Sset_ptr self, signed long long int key)
{
  Ssnode_ptr p = self->root;
  Ssnode_ptr best = (Ssnode_ptr) NULL;

  while (p != NULL) {
    if (key < p->key) {
      best = p;
      p = p->left; /* see if can find some better value */
    }
    else if (key > p->key)  p = p->right;
    else return p; /* found it! */
  }

  return best;
}


/**Function********************************************************************

  Synopsis    [Looks for an element with a given key. If such element
  does not exists it is created.]

  Description [
  is_found is set to true if the element exists in the tree and false otherwise.
  Precondition: is_found should not be NULL;
  Note: all the existing iterators remain valid.
  Returns a node with the given key.

  SideEffects []

  SeeAlso     []

******************************************************************************/
static Ssnode_ptr s_set_find_insert(Sset_ptr self, signed long long int key, 
                                    boolean* is_found)
{
  Ssnode_ptr p = self->root;
  Ssnode_ptr new_node;

  if (p == NULL) { /* special case: empty tree */
    p = s_set_new_node(key, NULL);

    self->root = p;
    self->size = 1;
    *is_found = false;
    return p;
  }

  while (true) {
    if (key < p->key) {
      if (p->left != NULL) p = p->left;
      else {
        new_node = s_set_new_node(key, p);
        p->left = new_node;
        SSET_DEC_BALANCE(p);
        break;
      }
    }
    else if (key > p->key) {
      if (p->right != NULL) p = p->right;
      else {
        new_node = s_set_new_node(key, p);
        p->right = new_node;
        SSET_INC_BALANCE(p);
        break;
      }
    }
    else { /* the key is found */
      *is_found = true;
      return p;
    }
  } /* while */
  
  /* such key has not been found and new element is created.
     Now it is time to balance the tree.
  */
    
  self->size += 1;

  Ssnode_ptr parent = SSET_GET_PARENT(p);

  
  while (SSET_GET_BALANCE(p) != SSET_BALANCED && parent != NULL) {

    if (parent->left == p) {

      p = parent;
      parent = SSET_GET_PARENT(p);

      if (SSET_GET_BALANCE(p) != SSET_L_BALANCED) SSET_DEC_BALANCE(p);
      else {
        Ssnode_ptr b = p->left;
          
        if (SSET_GET_BALANCE(b) == SSET_L_BALANCED) { /* left reflection of case 1 
                                                        in Knuth's book */
          SSET_SET_PARENT(b, parent);
            
          if (parent == NULL) self->root = b;
          else {
            if (parent->left == p) parent->left = b;
            else parent->right = b;
          }
            
          p->left = b->right;
            
          if (p->left != NULL) SSET_SET_PARENT(p->left, p);

          b->right = p;
          SSET_SET_PARENT(p, b);
            
          SSET_SET_BALANCE(b, SSET_BALANCED);
          SSET_SET_BALANCE(p, SSET_BALANCED);
        }
        else { /* left reflection of case 2 in Knuth's book */
          Ssnode_ptr x = b->right;

          SSET_SET_PARENT(x, parent);
            
          if (parent == NULL) self->root = x;
          else {
            if (parent->left == p) parent->left = x;
            else parent->right = x;
          }

          b->right = x->left;

          if (b->right != NULL) SSET_SET_PARENT(b->right, b);

          p->left = x->right;

          if (p->left != NULL) SSET_SET_PARENT(p->left, p);


          x->left = b;
          x->right = p;

          SSET_SET_PARENT(b, x);
          SSET_SET_PARENT(p, x);

          if (SSET_GET_BALANCE(x) == SSET_L_BALANCED) {
            SSET_SET_BALANCE(b, SSET_BALANCED);
            SSET_SET_BALANCE(p, SSET_R_BALANCED);
          }
          else if (SSET_GET_BALANCE(x) == SSET_BALANCED) {
            SSET_SET_BALANCE(b, SSET_BALANCED);
            SSET_SET_BALANCE(p, SSET_BALANCED);
          }
          else { /* SSET_GET_BALANCE(x) == SSET_R_BALANCED */
            SSET_SET_BALANCE(b, SSET_L_BALANCED);
            SSET_SET_BALANCE(p, SSET_BALANCED);
          }
            
          SSET_SET_BALANCE(x, SSET_BALANCED);
        }
          
        break;
      } /* else */
    } /* end of if (parent->left == p) */
    else { /* parent->right == p */

      p = parent;
      parent = SSET_GET_PARENT(p);

      if (SSET_GET_BALANCE(p) != SSET_R_BALANCED) SSET_INC_BALANCE(p);
      else {
        Ssnode_ptr b = p->right;
          
        if (SSET_GET_BALANCE(b) == SSET_R_BALANCED) { /* case 1 in Knuth's book */
          SSET_SET_PARENT(b, parent);
              
          if (parent == NULL) self->root = b;
          else {
            if (parent->left == p) parent->left = b;
            else parent->right = b;
          }

          p->right = b->left;
          if (p->right != NULL) SSET_SET_PARENT(p->right, p);


          b->left = p;
          SSET_SET_PARENT(p, b);
            
          SSET_SET_BALANCE(b, SSET_BALANCED);
          SSET_SET_BALANCE(p, SSET_BALANCED);
        }
        else { /* case 2 in Knuth's book */
          Ssnode_ptr x = b->left;

          SSET_SET_PARENT(x, parent);

          if (parent == NULL) self->root = x;
          else {
            if (parent->left == p) parent->left = x;
            else parent->right = x;
          }

          b->left = x->right;
          if (b->left != NULL) SSET_SET_PARENT(b->left, b);

          p->right = x->left;
          if (p->right != NULL) SSET_SET_PARENT(p->right, p);

          x->right = b;
          x->left = p;

          SSET_SET_PARENT(b, x);
          SSET_SET_PARENT(p, x);
            
          if (SSET_GET_BALANCE(x) == SSET_R_BALANCED) {
            SSET_SET_BALANCE(b, SSET_BALANCED);
            SSET_SET_BALANCE(p, SSET_L_BALANCED);
          }
          else if (SSET_GET_BALANCE(x) == SSET_BALANCED) {
            SSET_SET_BALANCE(b, SSET_BALANCED);
            SSET_SET_BALANCE(p, SSET_BALANCED);
          }
          else { /* SSET_GET_BALANCE(x) == SSET_L_BALANCED */
            SSET_SET_BALANCE(b, SSET_R_BALANCED);
            SSET_SET_BALANCE(p, SSET_BALANCED);
          }
            
          SSET_SET_BALANCE(x, SSET_BALANCED);
        } /* else */

        break;
      }
    }
  } /* while */

  *is_found = false;
  return new_node;
}


/**Function********************************************************************

  Synopsis    [Removes a given node from the set.]

  Description [
  Precondition: the node has to belong to the provided set.
  The operation takes O(log2 N) time (N is the size of the set).]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void s_set_delete_node(Sset_ptr self, Ssnode_ptr to_be_deleted)
{
  boolean leftDecrease; /* a flag of which child trees is decreased, left or right */
  Ssnode_ptr to_be_balanced; /* a node which requires balancing */
  
  Ssnode_ptr parent = SSET_GET_PARENT(to_be_deleted);

  /* --- remove the node from the tree and reform the adjusting sub-trees --- */

  /* -- Case 1 : node to_be_deleted does not have a right child */
  if (to_be_deleted->right == NULL) { 
    if (to_be_deleted->left != NULL) SSET_SET_PARENT(to_be_deleted->left, parent);
    
    if (parent == NULL) {
      self->root = to_be_deleted->left;
      to_be_balanced = NULL; /* no balance required */
      leftDecrease = false; /* this is to avoid warning */
    }
    else {
      if (to_be_deleted == parent->left) {
        parent->left = to_be_deleted->left;
        leftDecrease = true;
      }
      else {
        parent->right = to_be_deleted->left;
        leftDecrease = false;
      }
      to_be_balanced = parent;
    }
  }
  /* -- Case 2 : the right child of to_be_deleted does no have a left child */
  else if (to_be_deleted->right->left == NULL) {
    if (to_be_deleted->left != NULL) {
      SSET_SET_PARENT(to_be_deleted->left, to_be_deleted->right);
      to_be_deleted->right->left = to_be_deleted->left;
    }

    SSET_COPY_PARENT_BALANCE_TO(to_be_deleted->right, to_be_deleted);
    
    if (parent == NULL) self->root = to_be_deleted->right;
    else {
      if (to_be_deleted == parent->left) parent->left = to_be_deleted->right;
      else parent->right = to_be_deleted->right;
    }
    
    to_be_balanced = to_be_deleted->right;
    leftDecrease = false;
  }
  /* -- Case 3: the right child of to_be_deleted has a left child */
  else  {
    /* find a node which is just greater than the removed one. */
    Ssnode_ptr s = to_be_deleted->right->left;
    while (s->left != NULL) s = s->left;
        
    Ssnode_ptr s_parent = SSET_GET_PARENT(s);

    /* detach this greater node from the tree */
    s_parent->left = s->right;
    
    if (s->right != NULL) {
      SSET_SET_PARENT(s->right, s_parent);
    }

    /* move this greater node to the place of the removed one */
    if (to_be_deleted->left != NULL) {
      SSET_SET_PARENT(to_be_deleted->left, s);
      s->left = to_be_deleted->left;
    }
        
    SSET_SET_PARENT(to_be_deleted->right, s);
    s->right = to_be_deleted->right;
    
    
    SSET_COPY_PARENT_BALANCE_TO(s, to_be_deleted);
        
    if (parent == NULL) self->root = s;
    else {
      if (to_be_deleted == parent->left) parent->left = s;
      else parent->right = s;
    }
    
    to_be_balanced = s_parent;
    leftDecrease = true;
  }

  
  /* --- re-balance the tree if it is required --- */
  while (NULL != to_be_balanced) {
    
    parent = SSET_GET_PARENT(to_be_balanced);

    /* -- the height of the left child has been decreased */
    if (leftDecrease) {
      
      if (SSET_GET_BALANCE(to_be_balanced) == SSET_BALANCED) {
        SSET_SET_BALANCE(to_be_balanced, SSET_R_BALANCED);
        to_be_balanced = NULL; /* end of balancing */
      }
      else if (SSET_GET_BALANCE(to_be_balanced) == SSET_L_BALANCED) {
        SSET_SET_BALANCE(to_be_balanced, SSET_BALANCED);
        /* continue balancing up */
      }
      else { /* re-balance is required since sub-tree was SSET_R_BALANCED (i.e. 1)
                and now the difference in heights becomes 2. */
        Ssnode_ptr x = to_be_balanced->right;
        
        /* case 1 : the balance of the node and its right child is opposite */
        if (SSET_GET_BALANCE(x) == SSET_L_BALANCED) {
          Ssnode_ptr w = x->left;
          
          SSET_SET_PARENT(w, parent);
          if (parent == NULL) self->root = w;
          else {
            if (parent->left == to_be_balanced) parent->left = w;
            else parent->right = w;
          }

          x->left = w->right;
          if (x->left != NULL) SSET_SET_PARENT(x->left, x);

          to_be_balanced->right = w->left;
          if (to_be_balanced->right != NULL) {
            SSET_SET_PARENT(to_be_balanced->right, to_be_balanced);
          }

          w->right = x;
          SSET_SET_PARENT(x, w);

          w->left = to_be_balanced;
          SSET_SET_PARENT(to_be_balanced, w);

          if (SSET_GET_BALANCE(w) == SSET_R_BALANCED) {
            SSET_SET_BALANCE(x, SSET_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_L_BALANCED);
          }
          else if (SSET_GET_BALANCE(w) == SSET_BALANCED) {
            SSET_SET_BALANCE(x, SSET_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_BALANCED);
          }
          else { /* SSET_GET_BALANCE(w) == SSET_L_BALANCED */
            SSET_SET_BALANCE(x, SSET_R_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_BALANCED);
          }

          SSET_SET_BALANCE(w, SSET_BALANCED);

          to_be_balanced = w; /* continue balancing up */
        }
        else {            
          /* case 2 : the balance of the node and its right child is NOT opposing */

          SSET_SET_PARENT(x, parent);
          if (parent != NULL) {
            if (parent->left == to_be_balanced) parent->left = x;
            else parent->right = x;
          }
          else self->root = x;

          to_be_balanced->right = x->left;
          if (to_be_balanced->right != NULL) {
            SSET_SET_PARENT(to_be_balanced->right, to_be_balanced);
          }

          x->left = to_be_balanced;
          SSET_SET_PARENT(to_be_balanced, x);

          if (SSET_GET_BALANCE(x) == SSET_BALANCED) {
            SSET_SET_BALANCE(x, SSET_L_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_R_BALANCED);

            to_be_balanced = NULL; /* end the balancing */
          }
          else { /* balance of x is R-balance (i.e. 1) */
            SSET_SET_BALANCE(x, SSET_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_BALANCED);
            
            to_be_balanced = x; /* continue balancing up */
          }
        }
      }
    } /* end of if (leftDecrease) */
    
    /* -- the height of the right child has been decreased */
    else { /* leftDecrease == 0 */

      if (SSET_GET_BALANCE(to_be_balanced) == SSET_BALANCED) {
        SSET_SET_BALANCE(to_be_balanced, SSET_L_BALANCED);
        to_be_balanced = NULL; /* end of balancing */
      }
      else if (SSET_GET_BALANCE(to_be_balanced) == SSET_R_BALANCED) {
        SSET_SET_BALANCE(to_be_balanced, SSET_BALANCED);
        /* continue balancing up */
      }
      else { /* re-balance is required since sub-tree was SSET_L_BALANCED (i.e. -1)
                and now the difference in heights becomes 2. */
        Ssnode_ptr x = to_be_balanced->left;

        /* case 1 : the balance of the node and its left child is opposite */
        if (SSET_GET_BALANCE(x) == SSET_R_BALANCED) {
          Ssnode_ptr w = x->right;

          SSET_SET_PARENT(w, parent);
          if (parent == NULL) self->root = w;
          else {
            if (parent->left == to_be_balanced) parent->left = w;
            else parent->right = w;
          }

          x->right = w->left;
          if (x->right != NULL) SSET_SET_PARENT(x->right, x);

          to_be_balanced->left = w->right;
          if (to_be_balanced->left != NULL) {
            SSET_SET_PARENT(to_be_balanced->left, to_be_balanced);
          }

          w->left = x;
          SSET_SET_PARENT(x, w);

          w->right = to_be_balanced;
          SSET_SET_PARENT(to_be_balanced, w);

          if (SSET_GET_BALANCE(w) == SSET_L_BALANCED) {
            SSET_SET_BALANCE(x, SSET_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_R_BALANCED);
          }
          else if (SSET_GET_BALANCE(w) == SSET_BALANCED) {
            SSET_SET_BALANCE(x, SSET_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_BALANCED);
          }
          else {/* SSET_GET_BALANCE(w) == SSET_R_BALANCED */
            SSET_SET_BALANCE(x, SSET_L_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_BALANCED);
          }

          SSET_SET_BALANCE(w, SSET_BALANCED);

          to_be_balanced = w; /* continue balancing up */
        }
        else {            
          /* case 2 : the balance of the node and its right child is NOT opposing */
          SSET_SET_PARENT(x, parent);
          if (parent != NULL) {
            if (parent->left == to_be_balanced) parent->left = x;
            else parent->right = x;
          }
          else self->root = x;

          to_be_balanced->left = x->right;
          if (to_be_balanced->left != NULL) {
            SSET_SET_PARENT(to_be_balanced->left, to_be_balanced);
          }

          x->right = to_be_balanced;
          SSET_SET_PARENT(to_be_balanced, x);

          if (SSET_GET_BALANCE(x) == SSET_BALANCED) {
            SSET_SET_BALANCE(x, SSET_R_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_L_BALANCED);
            
            to_be_balanced = NULL; /* end the balancing */
          }
          else { /* balance of x is L-balance (i.e. -1) */
            SSET_SET_BALANCE(x, SSET_BALANCED);
            SSET_SET_BALANCE(to_be_balanced, SSET_BALANCED);

            to_be_balanced = x; /* continue balancing up */
          }
        }
      }
    }
    
    /* -- if balancing is still required go to the parent */
    if (to_be_balanced != NULL) {
      if (parent == NULL) to_be_balanced = NULL;  /* end of balancing */
      else {
        leftDecrease = (to_be_balanced == parent->left);
        to_be_balanced = parent;
      }
    }
  } /* while */

  
  /* --- free the removed node and update the size of the tree --- */
  DEBUG(time_to_free -= util_cpu_time());

  FREE(to_be_deleted);

  DEBUG(time_to_free += util_cpu_time(); num_of_free += 1;);

  self->size -= 1;
  DEBUG(nusmv_assert(self->size >= 0));

  return ;
}
