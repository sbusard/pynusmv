/**CFile***********************************************************************

  FileName    [setSet.c]

  PackageName [set]

  Synopsis    [Generic Set Data Structure]

  Description [This package provides an implementation of sets. Sets
  are ordered, meaning that elements can be traversed in the same
  chronological order they have been added. Uniqueness is not assured,
  meaning that you might have to idential sets that are stored into
  two different structures. This means that you cannot compare two
  sets by comparing their sets. For further details see the
  description about the Set_t structure]

  SeeAlso     []

  Author      [Marco Roveri, Roberto Cavada]

  Copyright   [
  This file is part of the ``set'' package of NuSMV version 2. 
  Copyright (C) 2000-2001 by FBK-irst and 2008 by FBK-irst.

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

#include "setInt.h" 
#include "utils/NodeList.h"

static char UTIL_UNUSED rcsid[] = "$Id: setSet.c,v 1.9.14.3.4.8 2010-02-05 20:23:03 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis           [Structure for ordered sets]

  Description [Sets are containers for elements that cannot occur into
  them more then once. Sets can be travered through iterators, and
  chronological ordering which elements are inserted into them is
  preserved when travered. A set can be frozen at time t, meaning that
  that the set cannot be later changed until the set is destroyed.
  Freezing a set make that set unchangeble in time, and allows for
  more efficient operations like set copy. Copying a frozen set has
  the only effect of incrementing a reference counting for that set,
  without any need for actually copying the set content. Freezing a
  set whose content does not need to be changed later on is therefore
  always a good idea to make set operations and memory usage more
  efficient.

  It is also important to mark that when storing a set in memory
  (e.g. in memoized operations, like in dependencies hashes) it is
  needed to freeze the set, otherwise external code might change the
  set content with side-effect with weird results, as explained below..

  Operations like AddMember, RemoveMember, Union, Difference, etc. do
  not create a new set, instead they modify the set they are applied
  to. For example given two sets S1 and S2, S1 U S2 (set union) can be
  obtained by calling
  
  Set_Union(S1, S2)

  If S1 is not a frozen set, the result goes to S1 (with side-effect),
  and no copy is performed. If S1 is frozen, S1 is copied to a new set
  S1' and then side effect is performed on S1' to add members in
  S2. All this operation is carried out automatically in a transparent
  manner, but it is required that operations that modify sets all
  returns a set that can be different from the set they are applied
  to. The returned value has to be assigned to a variable. The right
  set protocol then requires an explicit assignment:

  S1 = Set_Union(S1, S2)

  To save memory the empty set is represented with a NULL pointer,
  that is another reason why an explicit assigment is required, and
  that justify the fact that in general S1 and S1' may be different. 

  When a set is no longer used, it has to be freed with method
  ReleaseSet. This either frees the set and the memory it uses, or
  decreases the set's reference counting. 

  Reference counting is applied only for frozen sets. When a frozen
  set is copied its reference counting is incremented. When a frozen
  set is released, the reference counting is decremented and the set
  is freed only if its reference counting reaches the value of 0,
  meaning that there are no longer users of that set.

  Notice that in previous operation:

  S1 = Set_Union(S1, S2)

  If S1 is a frozen set, this is the sequence of actions that are
  involved:

  1. S1 is copied into a temporary set S1'
  2. S1' is unioned with S2 (with side-effect on S1') 
  3. S1 is released (and possibly freed if needed)
  4. S1' is returned as a new non-frozen set and assigned to S1. 

  Pass 3 is remarkable here. Suppose that a set is stored into a
  permanent memory area (like a cache, a hash, etc.). When storing the
  set, it has to be frozen and a carefully reference counting has to
  be takein into account. When looking up previously stored set and
  returning that set (e.g. in memoizing) is is important to return a
  copy of the (frozen) set, and explicitly ask the user to release the
  returned set when no longer used. This prevents previous step 3 to
  release sets that are still in usage for example inside the
  cache. For example:

  Set_t s1 = some_memoizing_function();
  Set_t s2 = Set_AddMember(s1, element);
  ...
  Set_ReleaseSet(s2);

  Here s2 is different from s1 (as s1 is frozen and AddMember would
  change it otherwise). Even if function some_memoizing_function
  requires the user to release returned set, there is no need to
  release s1 (and in fact you do not have to, or you have a bug). This
  allows to write second line Set_t s2 = ... as:

  s1 = Set_AddMember(s1, element);

  At the end you will have only to release s1 (as prescribed by
  function some_memoizing_function)

  ]

  SideEffects        []

******************************************************************************/
typedef struct Set_TAG {
  NodeList_ptr list;
  int* fam;

} Set;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
static NodeList_ptr set_empty_list = NODE_LIST(NULL);


/* #include "utils/assoc.h" */
/* static hash_ptr set_op_hash = (hash_ptr) NULL; */
/* static void insert_op_hash(node_ptr key, Set_t value)  */
/* { */
/*   nusmv_assert(set_op_hash != (hash_ptr) NULL); */
/*   if (value != NULL) {  */
/*     Set_Freeze(value);  */
/*     value = Set_Copy(value); */
/*   } */
/*   insert_assoc(set_op_hash, key, (node_ptr) value); */
/* } */
/* static Set_t lookup_op_hash(node_ptr key)  */
/* { */
/*   nusmv_assert(set_op_hash != (hash_ptr) NULL); */
/*   return (Set_t) find_assoc(set_op_hash, key); */
/* } */


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void set_union_to_set_aux ARGS((node_ptr a, Set_t* set));


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static Set_t set_create ARGS((void));
static Set_t set_copy ARGS((const Set_t self));
static Set_t set_copy_actual ARGS((const Set_t self));
static void set_destroy ARGS((Set_t self));
static void set_check_list ARGS((Set_t self));
static Set_t set_check_frozen ARGS((Set_t self));


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the set package]

  Description [Initializes the set package. See also Set_Quit() to
  deinitialize it]

  SideEffects        []

******************************************************************************/
void set_pkg_init()
{
  if (set_empty_list == NODE_LIST(NULL)) {
    set_empty_list = NodeList_create();
  }

/*
  if (set_op_hash == (hash_ptr) NULL) {
    set_op_hash = new_assoc();
  }
*/
}

/**Function********************************************************************

  Synopsis           [De-Initializes the set package]

  Description [De-Initializes the set package. Use after Set_init()]

  SideEffects        []

******************************************************************************/
void set_pkg_quit()
{
  if (set_empty_list != NODE_LIST(NULL)) {
    NodeList_destroy(set_empty_list);
    set_empty_list = NODE_LIST(NULL);
  }

/*  if (set_op_hash != (hash_ptr) NULL) {
    free_assoc(set_op_hash);
    set_op_hash = (hash_ptr) NULL;
  }
*/
}



/**Function********************************************************************

  Synopsis           [Create a generic empty set]

  Description        [This function creates an empty set.]

  SideEffects        []

******************************************************************************/
Set_t Set_MakeEmpty() 
{
  return (Set_t) NULL;
}

/**Function********************************************************************

  Synopsis           [Given a list, builds a corresponding set]

  Description        [Given a list, builds a corresponding set]

  SideEffects        []

  SeeAlso            [Set_MakeSingleton]

******************************************************************************/
Set_t Set_Make(node_ptr l)
{
  Set_t result;

  if (l == Nil) return Set_MakeEmpty();
  result = set_create();
  for (; l != Nil; l = cdr(l)) { 
    result = Set_AddMember(result, (Set_Element_t) car(l)); 
  }
  return result;
}


/**Function********************************************************************

  Synopsis           [Creates a Singleton]

  Description        [Creates a set with a unique element.]

  SideEffects        []

******************************************************************************/
Set_t Set_MakeSingleton(Set_Element_t el)
{
  Set_t set = set_create();
  set = Set_AddMember(set, el); 
  return set;
}


/**Function********************************************************************

  Synopsis           [Given an union node, builds a corresponding set]

  Description        [Given an union node, builds a corresponding set]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
Set_t Set_MakeFromUnion(node_ptr _union)
{
  Set_t result = Set_MakeEmpty();
  set_union_to_set_aux(_union, &result);
  return result;
}


/**Function********************************************************************

  Synopsis           [Returns the independent copy of a set]

  Description [If the set was frozen, returned set is equal to the set
  given as input, and its reference counting is incremented. See
  description about the structure Set_t for further information]

  SideEffects        []

  SeeAlso            [Set_MakeSingleton]

******************************************************************************/
Set_t Set_Copy(const Set_t set)
{
  return set_copy(set);
}

/**Function********************************************************************

  Synopsis           [Frees a set]

  Description [Releases the memory associated to the given set. If the
  set was frozen, reference counting is taken into account. See
  description about the structure Set_t for further information]

  SideEffects        []

******************************************************************************/
void Set_ReleaseSet(Set_t set) 
{    
  set_destroy(set);
}

/**Function********************************************************************

  Synopsis    [Frees a set of sets ]

  Description [Assuming that an input set consists of elements each of
  which is also a set this function applies Set_ReleaseSet to the input
  set and every set in it.]

  SideEffects [Set_ReleaseSet]

******************************************************************************/
void Set_ReleaseSetOfSet(Set_t set)
{
  Set_Iterator_t iter;
  SET_FOREACH(set, iter) {
    Set_t sub_set = (Set_t)Set_GetMember(set, iter);
    Set_ReleaseSet(sub_set);
  }
  Set_ReleaseSet(set);
}

/**Function********************************************************************

  Synopsis           [Set Emptiness]

  Description        [Checks for Set Emptiness. Constant time]

  SideEffects        []

******************************************************************************/
boolean Set_IsEmpty (const Set_t set) 
{
  return (set == NULL || set->list == NULL || 
          NodeList_get_length(set->list) == 0);
}

/**Function********************************************************************

  Synopsis           [Set memberships]

  Description        [Checks if the given element is a member of the
  set. It returns <tt>True</tt> if it is a member, <tt>False</tt>
  otherwise. Constant time]

  SideEffects        []

******************************************************************************/
boolean Set_IsMember(const Set_t set, Set_Element_t el) 
{
  return (set != NULL && set->list != NULL && 
          NodeList_belongs_to(set->list, (node_ptr) el));
}

/**Function********************************************************************

  Synopsis           [Set Cardinality]

  Description        [Computes the cardinality of the given set. Constant time]

  SideEffects        []

******************************************************************************/
int Set_GiveCardinality(const Set_t set) 
{
  if (set == NULL || set->list == NULL) return 0;
  return NodeList_get_length(set->list);
}

/**Function********************************************************************

  Synopsis           [Adds a new element to the set]

  Description [Add in order (at the end) a new element. Constant time
  if not frozen, linear time if frozen. See description about the
  structure Set_t for further information]

  SideEffects        [If set is not frozen, set is changed internally]

******************************************************************************/
Set_t Set_AddMember(Set_t set, Set_Element_t el) 
{
  if (set == NULL) set = Set_MakeSingleton(el);
  else {
    if (Set_IsMember(set, el)) return set;
/*
    node_ptr key = find_node(COLON, (node_ptr) __func__, find_node(COLON, (node_ptr) set, (node_ptr) el));
    Set_t res = lookup_op_hash(key);
    if (res != NULL) return res;
*/
    set = set_check_frozen(set);
    set_check_list(set);
    if (!NodeList_belongs_to(set->list, (node_ptr) el)) {
      NodeList_append(set->list, (node_ptr) el);
    }
    
/*    insert_op_hash(key, res); */
  }

  return set;
}

/**Function********************************************************************

  Synopsis           [Removes the given element from the set, if found]

  Description        [The new set is returned. Linear time. See
  description about the structure Set_t for further information]

  SideEffects [If set is not frozen, set is changed internally. If
  after removal set is empty, it is also released.]

******************************************************************************/
Set_t Set_RemoveMember(Set_t set, Set_Element_t el) 
{  
  if (!Set_IsEmpty(set) && Set_IsMember(set, el)) {
    Set_t sel;

    set = set_check_frozen(set);
    sel = Set_MakeSingleton(el);
    set = Set_Difference(set, sel);    
    Set_ReleaseSet(sel);
  }
  return set;
}
  


/**Function********************************************************************

  Synopsis           [Adds all new elements found in list]

  Description [Add in order (at the end) all new elements. Linear
  time in the size of list if not frozen, linear time in size of
  set + size of list if frozen. See description about the structure
  Set_t for further information]

  SideEffects        [If set is not frozen, set is changed internally]

******************************************************************************/
Set_t Set_AddMembersFromList(Set_t set, const NodeList_ptr list)
{
  ListIter_ptr iter;
  NODE_LIST_FOREACH(list, iter) {
    set = Set_AddMember(set, (Set_Element_t) NodeList_get_elem_at(list, iter));
  }
  return set;
}


/**Function********************************************************************

  Synopsis           [Checks if set1 contains set2]

  Description        [Returns true iff set2 is a subset of set1. Linear in 
  the size of set2]

  SideEffects        []

******************************************************************************/
boolean Set_Contains(const Set_t set1, const Set_t set2) 
{ 
  Set_Iterator_t iter;

  if (Set_IsEmpty(set2) || (set1 == set2)) return true;
  if (Set_GiveCardinality(set1) < Set_GiveCardinality(set2)) return false;
  SET_FOREACH(set2, iter) {
    if (!Set_IsMember(set1, Set_GetMember(set2, iter))) return false;
  }

  return true;
}

/**Function********************************************************************

  Synopsis           [Checks if set1 = set2]

  Description [Returns true iff set1 contains the same elements of
  set2. Linear in the size of set2]

  SideEffects        []

******************************************************************************/
boolean Set_Equals(const Set_t set1, const Set_t set2) 
{ 
  if (set1 == set2) return true;
  if (Set_GiveCardinality(set1) != Set_GiveCardinality(set2)) return false;
  return Set_Contains(set1, set2);
}


/**Function********************************************************************

  Synopsis           [Checks set1 and set2 has at least one common element]

  Description [Returns true iff set1 contains at least one element from
  set2. Linear in the size of set1]

  SideEffects        []

******************************************************************************/
boolean Set_Intersects(const Set_t set1, const Set_t set2) 
{ 
  Set_Iterator_t iter;

  if (Set_IsEmpty(set1) || Set_IsEmpty(set2)) return false;
  if (set1 == set2) return true;
  SET_FOREACH(set1, iter) {
    if (Set_IsMember(set2, Set_GetMember(set1, iter))) return true;
  }

  return false;
}


/**Function********************************************************************

  Synopsis           [Set Union]

  Description [Computes the Union of two sets. If set1 is not frozen,
  set1 is changed by adding members of set2. If set1 is frozen, it is
  before copied into a new set. 

  If set1 is not frozen, complexity is linear in the cardinality od
  set2, otherwise it is linear in the cardinality(set1) +
  cardinality(set2)

  See description about the structure Set_t for further information]

  SideEffects        [If set is not frozen, set is changed internally.]

******************************************************************************/
Set_t Set_Union(Set_t set1, const Set_t set2) 
{
  if (Set_IsEmpty(set1)) return Set_Copy(set2);
  if (Set_IsEmpty(set2)) return set1;
  if (Set_Contains(set1, set2)) return set1;

  set1 = set_check_frozen(set1);
  NodeList_concat_unique(set1->list, set2->list); 
  return set1;
}


/**Function********************************************************************

  Synopsis           [Set intersection]

  Description [Computes the Set intersection. Linear time on the
  cardinality of set1+set2. See description about the structure Set_t for
  further information]

  SideEffects [If set1 is not frozen, set1 is changed internally. If
  after intersection set1 is empty, it is also released and the empty
  set is returned.]

******************************************************************************/
Set_t Set_Intersection(Set_t set1, const Set_t set2) 
{
  Set_t set11;

  if (Set_IsEmpty(set1) || Set_IsEmpty(set2)) {
    Set_ReleaseSet(set1);
    return Set_MakeEmpty();
  }

  set11 = Set_Copy(set1);
  set11 = Set_Difference(set11, set2);
  
  set1 = set_check_frozen(set1);
  set1 = Set_Difference(set1, set11);
  Set_ReleaseSet(set11);

  return set1;
}


/**Function********************************************************************

  Synopsis    [Set Difference]

  Description [Computes the Set Difference. Linear time on the
  cardinality of set1. See description about the structure Set_t for
  further information]

  SideEffects [If set1 is not frozen, set1 is changed internally. If
  after difference set1 is empty, it is also released and the empty
  set is returned.]

******************************************************************************/
Set_t Set_Difference(Set_t set1, const Set_t set2) 
{
  if (Set_IsEmpty(set1) || Set_IsEmpty(set2)) return set1;

  set1 = set_check_frozen(set1);
  NodeList_remove_elems(set1->list, set2->list, NULL /* no disposal */, NULL);
  
  if (Set_IsEmpty(set1)) {
    Set_ReleaseSet(set1);
    set1 = Set_MakeEmpty();
  }

  return set1;
}


/**Function********************************************************************

  Synopsis           [Returns the rest of a set from a starting point]

  Description [Given a set and an iterator within that set, returns a
  new set containing all the elements that are found in to the input
  set from the iterator to then end of the set. Returned set must be
  disposed by the caller. 

  WARNING!! Deprecated method. This method is provided only for
  backward compatibility and should be no longer used in new code.]

  SideEffects        []

******************************************************************************/
Set_t Set_GetRest(const Set_t set, Set_Iterator_t from) 
{
  Set_t res = Set_MakeEmpty();
  Set_Iterator_t iter;
  for (iter=from; !Set_IsEndIter(iter); iter=Set_GetNextIter(iter)) {
    res = Set_AddMember(res, Set_GetMember(set, iter));
  }
  return res;
}


/**Function********************************************************************

  Synopsis           [Freezes a set]

  Description [Use when a set has to be memoized
  or stored in memory permanently. When frozen, a set content is
  frozen, meaning that no change can occur later on this set. If the
  set is tried to be changed later, a new copy will be created and
  changes will be applied to that copy. When a frozen set is copied, a
  reference counter is incremented and the same instance is returned
  in constant time. When a frozen set is destroyed, it is destroyed
  only when its ref counter reaches 0 (meaning it is the very last
  instance of that set). Set is also returned for a functional
  flavour. See description about the structure Set_t for
  further information]

  SideEffects        [set is changed internally if not already frozen]

  SeeAlso            []

******************************************************************************/
Set_t Set_Freeze(Set_t set)
{
  if (set != NULL && set->fam == (int*) NULL) {
    set->fam = ALLOC(int, 1);
    nusmv_assert(set->fam != (int*) NULL);
    *(set->fam) = 1;
  }
  return set;
}

/**Function********************************************************************

  Synopsis           [Provides an iterator to the "first" element of the set]

  Description [Returns an iterator to the "first" element of the set.
  Since sets are ordered, iterating through a set means to traverse
  the elements into the set in the same chronological ordering they
  have been previoulsy added to the set. If a set is changed, any
  previous stored iterator on that set might become invalid.]

  SideEffects        []

******************************************************************************/
Set_Iterator_t Set_GetFirstIter(Set_t set1)
{
  if (Set_IsEmpty(set1)) return (Set_Iterator_t) ListIter_get_end();
  return (Set_Iterator_t) NodeList_get_first_iter(set1->list);
}

/**Function********************************************************************

  Synopsis [Given an itarator of a set, returns the iterator pointing
  to the next chronological element in that set.]

  Description        [Returns the next iterator.
  Since sets are ordered, iterating through a set means to traverse
  the elements into the set in the same chronological ordering they
  have been previoulsy added to the set. If a set is changed, any
  previous stored iterator on that set might become invalid.]

  SideEffects        []

******************************************************************************/
Set_Iterator_t Set_GetNextIter(Set_Iterator_t iter)
{
  return (Set_Iterator_t) ListIter_get_next((ListIter_ptr) iter);
}

/**Function********************************************************************

  Synopsis [Returns true if the set iterator is at the end of the
  iteration]

  Description        []

  SideEffects        []

******************************************************************************/
boolean Set_IsEndIter(Set_Iterator_t iter)
{
  return ListIter_is_end((ListIter_ptr) iter);
}

/**Function********************************************************************

  Synopsis [Returns the element at given iterator]

  Description        []

  SideEffects        []

******************************************************************************/
Set_Element_t Set_GetMember(const Set_t set, Set_Iterator_t iter)
{
  nusmv_assert(!Set_IsEmpty(set));
  return NodeList_get_elem_at(set->list, (ListIter_ptr) iter);
}

/**Function********************************************************************

  Synopsis           [Given a set, returns the corresponding list]

  Description [Given a set, returns a corresponding list. Returned
  list belongs to self and must be NOT freed nor changed by the caller.]

  SideEffects        []

  SeeAlso            [Set_MakeSingleton]

******************************************************************************/
NodeList_ptr Set_Set2List(const Set_t set)
{
  set_check_list(set);
  if (set == NULL) {    
    nusmv_assert(set_empty_list != NODE_LIST(NULL) && 
                 NodeList_get_length(set_empty_list) == 0);
    return set_empty_list;
  }
  return set->list;
}

/**Function********************************************************************

  Synopsis           [Prints a set]

  Description [Prints a set to the specified file stream. Third
  parameter printer is a callback to be used when printing
  elements. If NULL, elements will be assumed to be node_ptr and
  print_node is called. printer_arg is an optional argument to be
  passed to the printer (can be NULL)]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Set_PrintSet(FILE * file, const Set_t s, 
                  void (*printer)(FILE* file, Set_Element_t el, void* arg), 
                  void* printer_arg)
{
  Set_Iterator_t iter;
  boolean keep;
  fprintf(file, "{");

  iter = Set_GetFirstIter(s);
  keep = !Set_IsEndIter(iter);
  while (keep) {
    if (printer != NULL) printer(file, Set_GetMember(s, iter), printer_arg);
    else print_node(file, (node_ptr) Set_GetMember(s, iter));
    iter = Set_GetNextIter(iter);
    keep = !Set_IsEndIter(iter);
    if (keep) fprintf(file, ", ");
  }
  fprintf(file, "}");
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Internal constructor]

  Description        [Internal constructor]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Set_t set_create(void)
{
  Set_t self = ALLOC(Set, 1);
  nusmv_assert(self != (Set_t) NULL);
  self->list = NODE_LIST(NULL);
  self->fam = (int*) NULL;

  return self;
}


/**Function********************************************************************

  Synopsis           [Internal copy constructor]

  Description        [Internal copy constructor]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Set_t set_copy(const Set_t self)
{  
  if (Set_IsEmpty(self)) return Set_MakeEmpty();

  if (self->fam != (int*) NULL) {
    /* it is frozen */
    *(self->fam) += 1;
    return self;
  }
  
  return set_copy_actual(self);
}


/**Function********************************************************************

  Synopsis           [Internal copy constructor]

  Description        [Internal copy constructor]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Set_t set_copy_actual(const Set_t self)
{
  Set_t copy;
  if (Set_IsEmpty(self)) return Set_MakeEmpty();

  copy = ALLOC(Set, 1);
  nusmv_assert(self != (Set_t) NULL);
  copy->list = NodeList_copy(self->list);
  copy->fam = (int*) NULL;

  return copy;  
}


/**Function********************************************************************

  Synopsis           [Internal destructor]

  Description        [Internal destructor]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void set_destroy(Set_t self)
{
  if (self != NULL) {
    if (self->fam == (int*) NULL || *(self->fam) == 1) {
      if (self->list != NODE_LIST(NULL)) {
        NodeList_destroy(self->list);
        self->list = NODE_LIST(NULL);
      }
      if (self->fam != (int*) NULL) {
        FREE(self->fam);
        self->fam = (int*) NULL;
      }
      FREE(self); 
    }
    else {
      *(self->fam) -= 1;
    }

  }
}


/**Function********************************************************************

  Synopsis           [This methods checks family counter and returns either a 
  new instance of self]

  Description        [Used internally by functions that change the instance
  to handle frozen set]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static Set_t set_check_frozen(Set_t self)
{
  if (self == NULL || self->fam == (int*) NULL) return self;
  
  /* copies as it is changing */
  *(self->fam) -= 1; /* deref self */
  return set_copy_actual(self);
}


/**Function********************************************************************

  Synopsis           [Fix the internal list if used actually]

  Description [This method is used internally to allow late allocation
  of the list]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void set_check_list(Set_t self)
{
  if (self != NULL && self->list == NULL) { 
    self->list = NodeList_create();
  }
}


/**Function********************************************************************

  Synopsis           [Given a node (possibly a UNION node) returns the 
  corresponding set]

  Description        [Auxiliary function for constructor from union nodes]

  SideEffects        [Given set will be added of found expressions]

  SeeAlso            []

******************************************************************************/
static void set_union_to_set_aux(node_ptr a, Set_t* set)
{    
  if (Nil == a) return;
  
  if (UNION == node_get_type(a)) {
    set_union_to_set_aux(car(a), set);
    set_union_to_set_aux(cdr(a), set);
    return;
  }

  /* if (TRUEEXP == node_get_type(a)) { */
  /*   *set = Set_AddMember(*set, find_node(NUMBER, NODE_PTR(1), Nil)); */
  /* } */
  /* else if (FALSEEXP == node_get_type(a)) { */
  /*   *set = Set_AddMember(*set, find_node(NUMBER, NODE_PTR(0), Nil)); */
  /* } */
  /* else */ 
  if (TWODOTS == node_get_type(a)) {
    int first, last;
    /* ranges may consist of constants NUMBERS only */
    nusmv_assert(NUMBER == node_get_type(car(a)) &&
                 NUMBER == node_get_type(cdr(a)));
    for (first = node_get_int(car(a)), last = node_get_int(cdr(a));
         first <= last; 
         first++) {
      *set = Set_AddMember(*set, find_node(NUMBER, NODE_FROM_INT(first), Nil));
    }
  }
  /* normal arbitrary expression */
  else {
    *set = Set_AddMember(*set, a);
  }

}
