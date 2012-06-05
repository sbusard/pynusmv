/**CFile***********************************************************************

  FileName    [node.c]

  PackageName [node]

  Synopsis    [The main routines of the <tt>node</tt> package.]

  Description [This file provides an abstract data type a la
  s-expression in LISP.]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``node'' package of NuSMV version 2.
  Copyright (C) 1998-2001 by CMU and FBK-irst.
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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/
#include "nodeInt.h"
#include "utils/WordNumber.h"

#include <stdlib.h>
#include "normalizers/MasterNormalizer.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/* Threashold for the reallocation of internal hash. It is the
   overaproximated ratio between the average entries size and the
   total number of hashed entries. Value 0.85 proved to be good in
   average.*/
#define HASH_REALLOC_THREASHOLD 0.85


/* Define the internal list organization. Only one has to be
   defined.
   SIMPLE: new nodes a always prepended to the hash lists.

   PROXIMITY: most frequenlty searched nodes are kept close to the
              beginning of lists in the hash.

   SORTED: hash lists are kept sorted to optimize searches.
 */
#define INSERT_NODE_SIMPLE 0
#define INSERT_NODE_PROXIMITY 1 /* this proved to be experimentally better */
#define INSERT_NODE_SORTED 0


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
/**Variable********************************************************************

  Synopsis    [The variable used to store the memory manager of the
  <tt>node</tt> package.]

  Description [The variable used to store the memory manager of the
  <tt>node</tt> package.
  We avoid declaring a global variable to store the node manager, and
  to pass the node_manager as an argument to all the node manipulation
  functions.]

******************************************************************************/
static node_mgr_ *node_mgr;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static inline unsigned int node_hash_fun ARGS((node_ptr node));
static unsigned node_eq_fun ARGS((node_ptr node1, node_ptr node2));
static int node_cmp_fun ARGS((node_ptr node1, node_ptr node2));
static node_ptr node_alloc ARGS((void));
static node_ptr insert_node ARGS((node_ptr node));
static void _node_realloc_nodelist ARGS((void));

#ifdef PROFILE_NODE
static int profile_info_cmp ARGS((const void *a, const void *b));
#endif

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Quits the <tt>node</tt> manager.]

  Description        [Quits the <tt>node</tt> manager. All the
  memory allocated it's freed.]

  SideEffects        [All the memory allocated by the <tt>node</tt>
  manager are left to the operating system.]

******************************************************************************/

#ifdef PROFILE_NODE
/**Function********************************************************************

  Synopsis           [Prints a summary of <tt>node</tt> resources usage]

  Description        [For debug and profiling purposes only]

  SideEffects        [none]

******************************************************************************/
void node_show_profile_stats(FILE* out)
{
  const unsigned int csize = node_mgr->nodelist_size;

  fprintf(out, "\nnode profiling results:\n");
  fprintf(out, "----------------------------------------------\n");

  node_profile_info* profile_info = ALLOC(node_profile_info, csize);

  unsigned long current_load=0;
  double load_avg = 0.0;
  double load_var = 0.0;
  int i; node_ptr p;

  /* collect load information and calculate stats */
  for (i=0; i<csize; ++i) {
    p = node_mgr->nodelist[i];

    profile_info[i].bucket = node_mgr->nodelist + i;
    profile_info[i].load = 0;
    profile_info[i].index = i;

    while (Nil != p) {
      ++ profile_info[i].load,  p = p->link;
    }

    current_load += profile_info[i].load;
  }

  /* calculate load average */
  load_avg = (double) current_load / (double) csize;

  /* calculate load variance */
  for (i=0;i<csize; ++i) {
    double var = (double) profile_info[i].load - load_avg;
    var *= var;

    load_var += var;
  }

  fprintf(
          out,
          "current load: %ld (%.3f%%)\n",
          current_load,
          (double) current_load / csize * 100);

  fprintf(out, "Avg (load): %.3f\n", load_avg);
  fprintf(out, "Var (load): %.3f\n", load_var );

  /* show 10 most loaded buckets */
  qsort(profile_info,
        csize,
        sizeof(node_profile_info),
        profile_info_cmp);

  fprintf(out, "\n10 most loaded buckets:\n");
  fprintf(out, "----------------------------------------------\n");
  for (i=0;i<10;i++) {
    fprintf(
            out,
            "%2d. index = %5d (@%8lx), load = %ld\n",
            i+1,
            (&profile_info[i])->index,
            (unsigned long) (&profile_info[i])->bucket,
            (&profile_info[i])->load);
  }

  FREE (profile_info);
}
#endif


/**Function********************************************************************

  Synopsis           [Free a node of the <tt>node<tt> manager.]

  Description        [Free a node of the <tt>node<tt> manager. The
  node is available for next node allocation.]

  SideEffects        [None]

******************************************************************************/
void free_node(node_ptr node)
{
  const unsigned int csize = node_mgr->nodelist_size;
  /* Check whether the node is in the hash. If it is, it should not
     be freed. */

  /*if (node->locked) return; */ /* hashed */

  {
    node_ptr * nodelist;
    node_ptr looking;
    unsigned int pos;

    nodelist = node_mgr->nodelist;

    pos = node_hash_fun(node) % csize;
    looking = nodelist[pos];
    while ((node_ptr) NULL != looking) {
      if (node == looking) return;
#ifdef DEBUG_FREE_NODE
      /* This tests that entries in the hash position have not
         been changed.  It might be the case that this test fails
         if an entry was previously changed by calling setcar or
         setcdr that should be not used with hashed nodes. */
      nusmv_assert(node_hash_fun(looking) % csize == pos);
#endif

#if INSERT_NODE_SORTED
      /* if sorted, we can exploit the ordering to decide that a
         node cannot be in the list */
      if (0 > node_cmp_fun(node, looking)) break; /* found limit */
#endif

      looking = looking->link;
    }
  }

  /*
    The node is not in the hash, so it can be freed.
  */
  /*nusmv_assert(!node->locked);*/
  node->link = node_mgr->nextFree;
  node_mgr->nextFree = node;

  /* This is a debugging feature to detect double freeing of the same
     node_ptr. This feature may considerably slowdown NuSMV.
     To use this feature compile NuSMV with "-DDEBUG_FREE_NODE" added to CFLAGS.
     For example, run
        make "CFLAGS=-Wall -g -DDEBUG_FREE_NODE"
     Do not forget to "touch src/node/node.c" before that.
  */
#ifdef DEBUG_FREE_NODE
#warning "Debugging of free_node is enabled"
 {
   node_ptr iter;
   for (iter = node_mgr->nextFree->link; iter != Nil; iter = iter->link) {
     if (iter == node) {
       print_sexp(nusmv_stderr, node);
       internal_error("The same node_ptr is freed twice.");
     }
   }
 }
#endif /* DEBUG_FREE_NODE */
}

/**Function********************************************************************

  Synopsis           [Creates a new node.]

  Description        [A new <tt>node</tt> of type <tt>type</tt> and
  left and right branch <tt>left<tt> and <tt>right</tt> respectively
  is created. The returned node is not stored in the <tt>node</tt> hash.]

  SideEffects        [None]

  SeeAlso            [find_node]

******************************************************************************/
node_ptr new_node(int type, node_ptr left, node_ptr right)
{
  extern int yylineno;
  node_ptr node;

  node = node_alloc();
  /*node -> locked         = 0;*/
  node -> type           = type;
  node -> lineno         = yylineno;
  node -> left.nodetype  = left;
  node -> right.nodetype = right;
  return node;
}

/**Function********************************************************************

  Synopsis           [Creates a new node.]

  Description        [The same as new_node except the line number
  is explicitly proved. A new <tt>node</tt> of type <tt>type</tt>, with
  left and right branch <tt>left<tt> and <tt>right</tt> respectively
  and on the line number <tt>lineno</tt> is created.
  The returned node is not stored in the <tt>node</tt> hash.]

  SideEffects        [None]

  SeeAlso            [new_node, find_node]

******************************************************************************/
node_ptr new_lined_node(int type, node_ptr left, node_ptr right, int lineno)
{
  node_ptr node;

  node = node_alloc();
  /*node -> locked         = 0;*/
  node -> type           = type;
  node -> lineno         = lineno;
  node -> left.nodetype  = left;
  node -> right.nodetype = right;
  return node;
}

/**Function********************************************************************

  Synopsis           [Creates a new node.]

  Description        [A new <tt>node</tt> of type <tt>type</tt> and
  left and right branch <tt>left<tt> and <tt>right</tt> respectively
  is created. The returned node is stored in the <tt>node</tt> hash.]

  SideEffects        [The <tt>node</tt> hash is modified.]

  SeeAlso            [new_node]

******************************************************************************/
node_ptr find_node(int type, node_ptr left, node_ptr right)
{
  extern int yylineno;
  node_rec node;

  /*node.locked = 0;*/
  node.type = type;
  node.lineno = yylineno;
  node.left.nodetype = left;
  node.right.nodetype = right;

  return insert_node(&node);
}

/**Function********************************************************************

  Synopsis           [Search the <tt>node</tt> hash for a given node.]

  Description        [Search the <tt>node</tt> hash for a given
  node. If the node is not <tt>Nil</tt>, and the node is not stored in
  the hash, the new node is created, stored in the hash and then returned.]

  SideEffects        [The node <tt>hash</tt> may change.]

  SeeAlso            [find_node]

******************************************************************************/
node_ptr find_atom(node_ptr a)
{
  if (a == Nil) return(a);
  return find_node(a->type, a->left.nodetype, a->right.nodetype);
}


/**Function********************************************************************

  Synopsis           [Conses two nodes.]

  Description        [Conses two nodes.]

  SideEffects        [None]

  SeeAlso            [car cdr]

******************************************************************************/
node_ptr cons(node_ptr x, node_ptr y)
{ return new_node(CONS,x,y); }

/**Function********************************************************************

  Synopsis           [Returns the left branch of a node.]

  Description        [Returns the left branch of a node.]

  SideEffects        [None]

  SeeAlso            [cdr cons]

******************************************************************************/
node_ptr car(node_ptr x)
{ return(x->left.nodetype);}

/**Function********************************************************************

  Synopsis           [Returns the right branch of a node.]

  Description        [Returns the right branch of a node.]

  SideEffects        [None]

  SeeAlso            [car cons]

******************************************************************************/
node_ptr cdr(node_ptr x)
{ return(x->right.nodetype); }

/**Function********************************************************************

  Synopsis           [Replaces the car of X with Y]

  Description        [Replaces the car of X with Y]

  SideEffects        [The car of X is replaced by Y.]

  SeeAlso            [car cdr cons setcdr]

******************************************************************************/
void setcar(node_ptr x, node_ptr y)
{
  /*nusmv_assert(!x->locked);*/
  x->left.nodetype = y;
}

/**Function********************************************************************

  Synopsis           [Replaces the cdr of X with Y]

  Description        [Replaces the cdr of X with Y]

  SideEffects        [The cdr of X is replaced by Y.]

  SeeAlso            [car cdr cons setcar]

******************************************************************************/
void setcdr(node_ptr x, node_ptr y)
{
  /*nusmv_assert(!x->locked);*/
  x->right.nodetype = y;
}

/**Function********************************************************************

  Synopsis           [Replaces the type of the node]

  Description        [Replaces the type of the node]

  SideEffects        [Replaces the type of the node]

  SeeAlso            [car cdr cons setcar node_get_type]

******************************************************************************/
void node_set_type (node_ptr x, int type)
{
  /*nusmv_assert(!x->locked);*/
  x->type = type;
}

/**Function********************************************************************

  Synopsis           [Returns 0 if given node is not a FAILURE node]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int node_is_failure(node_ptr x) { return FAILURE == node_get_type(x); }


/**Function********************************************************************

   Synopsis           [Tells if the given node is a numeric/boolean leaf]

   Description [Returns 0 if the given node is not a numeric/boolean/failure
   constant.  This is done a purely syntactic manner. To know if a
   *symbol* is constant declared within a symbol tablea, use method
   SymbTable_is_symbol_constant instead.]

   SideEffects        []

   SeeAlso            []

******************************************************************************/
int node_is_leaf(node_ptr node)
{
  return ((NUMBER == node_get_type(node)) ||
          (TRUEEXP == node_get_type(node)) ||
          (FALSEEXP == node_get_type(node)) ||
          (NUMBER_SIGNED_WORD == node_get_type(node)) ||
          (NUMBER_UNSIGNED_WORD == node_get_type(node)) ||
          (NUMBER_FRAC == node_get_type(node)) ||
          (NUMBER_REAL == node_get_type(node)) ||
          (NUMBER_EXP == node_get_type(node)) ||
          node_is_failure(node));
}


/**Function********************************************************************

  Synopsis           [Returns a new empty list]

  Description        []

  SideEffects        [None]

******************************************************************************/
node_ptr new_list()  { return Nil; }


/**Function********************************************************************

  Synopsis           [Returns a copy of a list]

  Description        [An invoker should free the returned list.]

  SideEffects        [free_list]

******************************************************************************/
node_ptr copy_list(node_ptr list)
{
  node_ptr new_list;

  /* create a reversed copy of the list */
  for (new_list = Nil; list != Nil; list = cdr(list)) {
    new_list = cons(car(list), new_list);
  }

  /* reverse the created list */
  new_list = reverse(new_list);
  return new_list;
}


/**Function********************************************************************

  Synopsis           [Frees all the elements of the list.]

  Description        [Frees all the elements of the list for further use.]

  SideEffects        [None]

  SeeAlso            [car]

******************************************************************************/
void free_list(node_ptr l) {
  while(l != Nil) {
    node_ptr tmp = l;

    l = cdr(l);
    free_node(tmp);
  }
}


/**Function********************************************************************

  Synopsis           [Returns 1 is the list is empty, 0 otherwise]

  Description        []

  SideEffects        [None]

******************************************************************************/
int is_list_empty(node_ptr list) { return list == Nil; }


/**Function********************************************************************

  Synopsis           [Checks list R to see if it contains the element N.]

  Description        [Checks list R to see if it contains the element N.]

  SideEffects        [None]

  SeeAlso            [node_subtract]

******************************************************************************/
int in_list(node_ptr n, node_ptr list)
{
  while (list) {
    if (car(list) == n) return(1);
    list = cdr(list);
  }
  return(0);
}


/**Function********************************************************************

  Synopsis           [Returns the length of list r.]

  Description        [Returns the length of list r.]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
int llength(node_ptr r)
{
  int l = 0;

  while (r) {
    l++;
    r = cdr(r);
  }
  return(l);
}


/**Function********************************************************************

  Synopsis           [Appends two lists and returns the result.]

  Description        [Constructs a new list by concatenating its arguments.]

  SideEffects        [The modified list is returned. Side effects on
  the returned list were performed. It is equivalent to the lisp NCONC]

******************************************************************************/
node_ptr append(node_ptr x, node_ptr y)
{
  if(x==Nil)return(y);
  x->right.nodetype = append(x->right.nodetype,y);
  return(x);
}


/**Function********************************************************************

  Synopsis           [Appends two lists and returns the result.]

  Description        [Constructs a new list by concatenating its arguments.]

  SideEffects        [The modified list is returned. No side effects on
  the returned list were performed.]

******************************************************************************/
node_ptr append_ns(node_ptr x, node_ptr y)
{
  if(x==Nil)return(copy_list(y));
  return(cons(car(x), append_ns(cdr(x), y)));
}


/**Function********************************************************************

  Synopsis           [Reverse a list.]

  Description        [Returns a new sequence containing the same
  elements as X but in reverse order.]

  SideEffects        [The orignial list is modified]

  SeeAlso            [last car cons append]

******************************************************************************/
node_ptr reverse(node_ptr x)
{
  node_ptr y=Nil;

  while (x != Nil) {
    node_ptr z = x->right.nodetype;

    x->right.nodetype = y;
    y = x;
    x = z;
  }
  return(y);
}


/**Function********************************************************************

  Synopsis           [reverses the list with no side-effect]

  Description        [Returns a reversed version of the given list.
  The original list is NOT modified]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
node_ptr reverse_ns(node_ptr l)
{
  node_ptr res = Nil;
  node_ptr iter;

  for (iter = l; iter != Nil; iter = cdr(iter)) {
    res = cons(car(iter), res);
  }

  return res;
}


/**Function********************************************************************

  Synopsis           [Returns the last cons in X.]

  Description        [Returns the last cons in X.]

  SideEffects        [None]

  SeeAlso            [car]

******************************************************************************/
node_ptr last(node_ptr x)
{
  nusmv_assert((node_ptr) NULL != x);

  if (!cdr(x)) return car(x);
  return last(cdr(x));
}


/**Function********************************************************************

  Synopsis           [Applies FUN to successive cars of LISTs and
  returns the results as a list.]

  Description        [Applies FUN to successive cars of LISTs and
  returns the results as a list.]

  SideEffects        [None]

  SeeAlso            [map2 walk]

******************************************************************************/
node_ptr map(NPFN fun, node_ptr l)
{
  node_ptr t;

  if (l == Nil) return(Nil);
  t = (node_ptr)(*fun)(car(l));
  return(cons(t,map(fun,cdr(l))));
}


/**Function********************************************************************

  Synopsis           [Applies FUN to successive cars of LISTs and
  returns the results as a list. Lists l1 and l2 are traversed in parallel.]

  Description        [Applies FUN to successive cars of LISTs and
  returns the results as a list. l1 and l2 must have the same length]

  SideEffects        [None]

  SeeAlso            [map walk]

******************************************************************************/
node_ptr map2(NPFNN fun, node_ptr l1, node_ptr l2)
{
  node_ptr res = Nil;

  while (l1 != Nil) {
    node_ptr t;
    nusmv_assert(l2 != Nil);

    t = (node_ptr)(*fun)(car(l1), car(l2));
    res = cons(t, res);

    l1 = cdr(l1); l2 = cdr(l2);
  }

  nusmv_assert(l2 == Nil);

  return reverse(res);
}


/**Function********************************************************************

  Synopsis           [Extracts odd elements of list L.]

  Description        [Extracts odd elements of list L.]

  SideEffects        [None]

  SeeAlso            [even_elements]

******************************************************************************/
node_ptr odd_elements(node_ptr l)
{
  if (l == Nil) return(Nil);
  return(cons(car(l),even_elements(cdr(l))));
}


/**Function********************************************************************

  Synopsis           [Extracts even elements of list L.]

  Description        [Extracts even elements of list L.]

  SideEffects        [None]

  SeeAlso            [odd_elements]

******************************************************************************/
node_ptr even_elements(node_ptr l)
{
  if(l == Nil)return(Nil);
  return(odd_elements(cdr(l)));
}


/**Function********************************************************************

  Synopsis           [Applies FUN to successive cars of LISTs.]

  Description        [Applies FUN to successive cars of LISTs.]

  SideEffects        [None]

  SeeAlso            [map]

******************************************************************************/
void walk(VPFN fun, node_ptr l)
{
  if (l == Nil) return;
  (void)(*fun)(car(l));
  walk(fun,cdr(l));
}

/**Function********************************************************************

  Synopsis           [Deletes from list set2 the elements of list set1.]

  Description        [Deletes elements of list set1 from list set2
  without doing side effect. The resulting list is returned.]

  SideEffects        [None]

******************************************************************************/
node_ptr node_subtract(node_ptr set1, node_ptr set2)
{
  if (set2 == Nil) return(Nil);
  if (in_list(car(set2),set1) == 1) return(node_subtract(set1,cdr(set2)));
  return(cons(car(set2),node_subtract(set1,cdr(set2))));
}

/**Function********************************************************************

  Synopsis           [Swaps two nodes.]

  Description        [Swaps two nodes.]

  SideEffects        [The two nodes are swapped.]

******************************************************************************/
void swap_nodes(node_ptr *n1, node_ptr *n2)
{
  node_ptr temp = *n1;

  *n1 = *n2;
  *n2 = temp;
}

/**Function********************************************************************

  Synopsis [Traverses the tree, and returns a possibly new tree that
  is a normalized copy of the first. Use for constant-time comparison
  of two trees]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/

/*       [MR2??]: elements out of the node package introducing circular dependencies. */
node_ptr node_normalize(node_ptr sexp)
{
  node_ptr memo = Nil;
  MasterNormalizer_ptr mn;

  if (sexp == Nil) return Nil;

  mn = node_pkg_get_global_master_normalizer();
  memo = MasterNormalizer_normalize_node(mn, sexp);

  return memo;
}

/**Function********************************************************************

  Synopsis    [Does the same thing as node_normalize but
  do it more efficiently if sexp is a list]

  Description [node_normalize is 100% recursive.
  This function instead expects the input to be a list (right
  directional and of AND or CONS) which will be processed in a loop
  instead of recursively.  For some examples this allowed to avoid
  stack overflow.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
node_ptr node_normalize_list(node_ptr sexp)
{
  node_ptr memo;
  node_ptr new_list;
  node_ptr iter;
  int nodetype;

  /* currently the list is expected to be of AND or CONS, but this can
     be changed when required */
  if (sexp == Nil ||
      (node_get_type(sexp) != AND &&
       node_get_type(sexp) != CONS)) {
    return node_normalize(sexp);
  }

  /* only the whole list is checked for being already processed.
     Sub-lists are not checked for being processed */

  nodetype = node_get_type(sexp);

  for(iter = sexp, new_list = Nil;
      Nil != iter && node_get_type(iter) == nodetype;
      iter = cdr(iter)){
    new_list = cons(node_normalize(car(iter)), new_list);
  }

  /* process the last element (it may/may not be Nil) */
  memo = node_normalize(iter);

  /* revert the order of the list, find_node it and free
     the nodes created earlier */
  while (new_list != Nil) {
    node_ptr tmp = new_list;
    memo = find_node(nodetype, car(new_list), memo);
    new_list = cdr(new_list);
    free_node(tmp);
  }

  return memo;

}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the <tt>node</tt> manager.]

  Description        [The <tt>node</tt> manager is initialized.]

  SideEffects        [None]

******************************************************************************/
void node_init()
{
  node_mgr = (node_mgr_ *)ALLOC(node_mgr_, 1);
  if (node_mgr == (node_mgr_*) NULL) {
    internal_error("node_init: Out of Memory in allocating the node manager\n");
  }
  node_mgr->allocated  = 0;
  node_mgr->hashed     = 0;
  node_mgr->memused    = 0;
  node_mgr->nodelist   = (node_ptr*) NULL;
  node_mgr->nodelist_size = 0;
  node_mgr->nodelist_size_idx = 0;
  node_mgr->memoryList = (node_ptr*) NULL;
  node_mgr->nextFree   = (node_ptr) NULL;

  { /* first allocation of nodelist */
    unsigned int newsize = node_primes[node_mgr->nodelist_size_idx];

    node_mgr->nodelist = (node_ptr*) ALLOC(node_ptr, newsize);
    if ((node_ptr*) NULL == node_mgr->nodelist) {
      internal_error("node_init: Out of Memory in allocating the node hash\n");
    }

    memset(node_mgr->nodelist, 0, newsize * sizeof(node_ptr));
    node_mgr->nodelist_size = newsize;
    node_mgr->nodelist_size_idx += 1;
  }

  node_mgr->subst_hash = new_assoc();
}


/**Function********************************************************************

  Synopsis           [De-initializes the <tt>node</tt> manager.]

  Description        [The <tt>node</tt> manager is de-initialized.]

  SideEffects        [None]

******************************************************************************/
void node_quit()
{
#ifdef PROFILE_NODE
  node_show_profile_stats(nusmv_stderr);
#endif

#ifdef DEBUG_FREE_NODE
  /* Check that shared nodes have not been modified (see also
     free_node). */
  {
    const unsigned int csize = node_mgr->nodelist_size;
    int i;
    node_ptr p;

    for (i = 0; i < csize; ++i) {
      p =  node_mgr->nodelist[i];
      while (Nil != p) {
        nusmv_assert(node_hash_fun(p) % csize == i);
        p = p->link;
      }
    }
  }
#endif

  /* Shut down the node manager */
  node_ptr * next;
  node_ptr * memlist = node_mgr->memoryList;

  while(memlist != NULL) {
    next = (node_ptr *) memlist[0];
    FREE(memlist);
    memlist = next;
  }
  node_mgr->nextFree = (node_ptr)NULL;
  node_mgr->memoryList = (node_ptr *)NULL;
  clear_assoc(node_mgr->subst_hash);
  free_assoc(node_mgr->subst_hash);
  node_mgr->subst_hash = (hash_ptr)NULL;

  FREE(node_mgr->nodelist);
  FREE(node_mgr);
  node_mgr = (node_mgr_ *)NULL;
}


/* This is used for debugging purposes, it is exported (not static)
   but not exposed in the interface */
void _node_self_check(boolean check_repeated)
{
  const unsigned int csize = node_mgr->nodelist_size;

  unsigned int i;
  for (i = 0; i < csize; ++i) {
    register node_ptr el;
    node_ptr prev;

    for (el = node_mgr->nodelist[i], prev = (node_ptr) NULL;
         (node_ptr) NULL != el;
         prev = el, el=el->link) {
      int j;

      /* ordering is guaranteed in each list */
      if (prev != (node_ptr) NULL) nusmv_assert(node_cmp_fun(prev, el) < 0);

      if (check_repeated) {
        /* there are no repeated elements in the hash */
        for (j=i+1; j < csize; ++j) {
          register node_ptr el2;
          for (el2 = node_mgr->nodelist[j]; (node_ptr) NULL != el2; el2=el2->link) {
            nusmv_assert(node_cmp_fun(el, el2) != 0);
          }
        }
      }
    }
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Inserts a node in the <tt>node</tt> hash.]

  Description        [Checks if node is in the cache, if it is the
  case then the hashed value is returned, else a new one is created,
  stored in the hash and returned.]

  SideEffects        [None]

  SeeAlso            [find_node]

******************************************************************************/
static node_ptr insert_node(node_ptr node)
#if INSERT_NODE_SIMPLE
{
  node_ptr* nodelist;
  node_ptr looking;
  unsigned int pos;

  if ((((double) node_mgr->hashed) / node_mgr->nodelist_size) >
      HASH_REALLOC_THREASHOLD) {
    _node_realloc_nodelist();
  }

  nodelist = node_mgr->nodelist;

  pos = node_hash_fun(node) % node_mgr->nodelist_size;
  looking = nodelist[pos];

  while ((node_ptr) NULL != looking) {
    if (node_eq_fun(node, looking)) return looking;
    looking = looking->link;
  }

  /* The node is not in the hash, it is created and then inserted
     in it. */
  looking = node_alloc();
  if (looking == (node_ptr) NULL) {
    internal_error("insert_node: Out of Memory\n");
  }
  /*looking->locked = 1;*/
  looking->type = node->type;
  looking->lineno = node->lineno;
  looking->left.nodetype = node->left.nodetype;
  looking->right.nodetype = node->right.nodetype;
  looking->link = nodelist[pos];
  looking->extra_data = NULL;
  nodelist[pos] = looking;

  node_mgr->hashed += 1;
  return looking;
}


#elif INSERT_NODE_PROXIMITY
{
  node_ptr* nodelist;
  node_ptr looking;
  node_ptr prev, pprev;
  unsigned int pos;

  if ((((double) node_mgr->hashed) / node_mgr->nodelist_size) >
      HASH_REALLOC_THREASHOLD) {
    _node_realloc_nodelist();
  }

  nodelist = node_mgr->nodelist;

  pos = node_hash_fun(node) % node_mgr->nodelist_size;
  looking = nodelist[pos];
  pprev = prev = (node_ptr) NULL; /* for swapping */
  while ((node_ptr) NULL != looking) {
    if (node_eq_fun(node, looking)) {
      if ((node_ptr) NULL != prev) {
        prev->link = looking->link;
        looking->link = prev;

        if ((node_ptr) NULL != pprev) pprev->link = looking;
        else { /* looking becomes the head */
          nodelist[pos] = looking;
        }
      }
      return looking;
    }
    pprev = prev; prev = looking;
    looking = looking->link;
  }

  /* The node is not in the hash, it is created and then inserted
     in it. */
  looking = node_alloc();
  if ((node_ptr) NULL == looking) {
    internal_error("insert_node: Out of Memory\n");
  }
  /*looking->locked = 1;*/
  looking->type = node->type;
  looking->lineno = node->lineno;
  looking->left.nodetype = node->left.nodetype;
  looking->right.nodetype = node->right.nodetype;
  looking->link = nodelist[pos];
  looking->extra_data = NULL;
  nodelist[pos] = looking;

  node_mgr->hashed += 1;
  return looking;
}


#elif INSERT_NODE_SORTED
{
  node_ptr* nodelist;
  node_ptr looking;
  node_ptr prev;
  unsigned int pos;

  if ((((double) node_mgr->hashed) / ((double) node_mgr->nodelist_size)) >
      HASH_REALLOC_THREASHOLD) {
    _node_realloc_nodelist();
  }

  nodelist = node_mgr->nodelist;

  pos = node_hash_fun(node) % node_mgr->nodelist_size;
  looking = nodelist[pos];
  prev = (node_ptr) NULL; /* for inserting after it */
  while ((node_ptr) NULL != looking) {
    const int cmp = node_cmp_fun(node, looking);
    if (0 == cmp) return looking;
    if (0 > cmp) break; /* found greater element, insert before it */
    prev = looking;
    looking = looking->link;
  }

  /* The node is not in the hash, it is created and then inserted
     in it (either at prev position, or prepended. */
  /*nusmv_assert(!node->locked);*/ /* node cannot be locked here */
  looking = node_alloc();
  if ((node_ptr) NULL == looking) {
    internal_error("insert_node: Out of Memory\n");
  }
  /*looking->locked = 1;*/
  looking->type = node->type;
  looking->lineno = node->lineno;
  looking->left.nodetype = node->left.nodetype;
  looking->right.nodetype = node->right.nodetype;
  looking->extra_data = NULL;

  if ((node_ptr) NULL != prev) { /* insert point is after prev */
    looking->link = prev->link;
    prev->link = looking;
  }
  else {
    /* appends to the head */
    looking->link = nodelist[pos];
    nodelist[pos] = looking;
  }

  node_mgr->hashed += 1;
  return looking;
}
#else
# error "Insert node type unrecognized"
#endif

/**Function********************************************************************

  Synopsis           [Hash function for <tt>node</tt>s.]

  SideEffects        [None]

  SeeAlso            [node_eq_fun]

******************************************************************************/
static inline unsigned int node_hash_fun(node_ptr node)
{
#if 1 /* new little-endian, seems to perform better with pointer-based
         ordering of sub-formulas (See Expr.c) */
  return (unsigned int) (((((size_t)  node->type )  + 31  )  +
                          (((size_t)  node->left.nodetype ) << 2)  +
                          (((size_t)  node->right.nodetype) << 1)));
#elif 0 /* little-endian */
  return (unsigned int) (((((size_t)  node->type )     )  +
                          (((size_t)  node->left.nodetype ) << 1)  +
                          (((size_t)  node->right.nodetype) << 2)));
#elif 0 /* big-endian 0 (TO BE TESTED) */
  return (unsigned int)
    (((((size_t)  node->type )     )  +
      (((size_t)  node->left.nodetype ) >> (NUSMV_SIZEOF_VOID * 4 - 1))  +
      (((size_t)  node->right.nodetype) << (NUSMV_SIZEOF_VOID * 4 - 2))));

#else /* big-endian 1 (TO BE TESTED) */
  return (unsigned int)
    ((((size_t)  node->type ))  +
     ((((size_t)  node->left.nodetype) << 17) ^ (((size_t)  node->left.nodetype) >> 16)) +
     ((((size_t)  node->right.nodetype) << 17) ^ (((size_t)  node->right.nodetype) >> 16)));

#endif
}


/**Function********************************************************************

  Synopsis           [Equality function for <tt>node</tt> hash.]

  SideEffects        [None]

  SeeAlso            [node_hash_fun]

******************************************************************************/
static unsigned node_eq_fun(node_ptr node1, node_ptr node2)
{
  return((node1->left.nodetype == node2->left.nodetype) &&
         (node1->right.nodetype == node2->right.nodetype) &&
         (node1->type == node2->type));
}


/**Function********************************************************************

  Synopsis [Comparison function for <tt>node</tt> sorted insertion.
  Returns is < 0 if node1 < node2, 0 if node1 == node2, and > 0 if
  node1 > node2]

  SideEffects        [None]

  SeeAlso            [node_hash_fun]

******************************************************************************/
static int node_cmp_fun(node_ptr node1, node_ptr node2)
{
  if (node1->type == node2->type) {
    if (node1->left.nodetype == node2->left.nodetype) {
      if (node1->right.nodetype == node2->right.nodetype) return 0;
      return (node1->right.nodetype > node2->right.nodetype) ? 1 : -1;
    }
    return (node1->left.nodetype > node2->left.nodetype) ? 1 : -1;
  }

  return (node1->type > node2->type) ? 1 : -1;
}


/**Function********************************************************************

  Synopsis           [Allocates NODE_MEM_CHUNK records and stores them
  in the free list of the <tt>node</tt> manager.]

  Description        [Allocates NODE_MEM_CHUNK records and stores them
  in the free list of the <tt>node</tt> manager.]

  SideEffects        [The free list of the <tt>node</tt> manager is
  updated by appending the new allocated nodes.]

******************************************************************************/
static node_ptr node_alloc()
{
  int i;
  node_ptr node;

  if (node_mgr->nextFree == (node_ptr) NULL) { /* memory is full */
    node_ptr list;
    node_ptr* mem = (node_ptr*) ALLOC(node_rec, NODE_MEM_CHUNK + 1);

    if (mem == (node_ptr*) NULL) { /* out of memory */
      fprintf(nusmv_stderr, "node_alloc: out of memory\n");
      internal_error("Memory in use for nodes = %ld\n", node_mgr->memused);
    }
    else { /* Adjust manager data structure */
      node_mgr->memused += (NODE_MEM_CHUNK + 1) * sizeof(node_rec);
      mem[0] = (node_ptr) node_mgr->memoryList;
      node_mgr->memoryList = mem;

      list = (node_ptr) mem;
      /* Link the new set of allocated node together */
      i = 1;
      do {
        list[i].link = &list[i+1];
      } while (++i < NODE_MEM_CHUNK);
      list[NODE_MEM_CHUNK].link = (node_ptr) NULL;

      node_mgr->nextFree = &list[1];
    }
  }

  /* Now the list of nextFree is not empty */
  node_mgr->allocated++;
  node = node_mgr->nextFree; /* Takes the first free available node */
  node_mgr->nextFree = node->link;
  node->link = (node_ptr) NULL;
  return node;
}


/**Function********************************************************************

  Synopsis           [Reallocation of the hash]

  Description [If possible (i.e. upper limit not reached yet)
  reallcoates the hash table of nodes. There are two strategies:
  the first try allocating a new bunch of memory, the second (if
  the former fails due to low memory) tries to enarge the existing
  hash. The hash is reallocated when a given load threashold is
  reached, when inserting nodes in the hash.]

  SideEffects        [None]

******************************************************************************/
static void _node_realloc_nodelist()
{
  node_ptr* nodelist;
  unsigned int newsize;

  if (node_mgr->nodelist_size_idx >= NODE_PRIMES_SIZE) {
    /* reached the size limit, sorry size remains untouched */
    return;
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "Node: Reallocating buckets... ");
  }

  newsize = node_primes[node_mgr->nodelist_size_idx];
  nodelist = (node_ptr*) ALLOC(node_ptr, newsize);

  if ((node_ptr*) NULL != nodelist) {

    memset(nodelist, 0, newsize * sizeof(node_ptr));

    {
      const unsigned int csize = node_mgr->nodelist_size;
      node_ptr* cnodelist = node_mgr->nodelist; /* alias */
      register node_ptr el;
      unsigned int newpos;
      unsigned int i;
      for (i = 0; i < csize; ++i) {
        for (el = cnodelist[i]; (node_ptr) NULL != el; el = cnodelist[i]) {
          cnodelist[i] = el->link; /* removes the element */
          newpos = node_hash_fun(el) % newsize;

#if INSERT_NODE_SIMPLE
          /* This algorithm is 'unstable', as it reverses the list
             alternatively */
          el->link = nodelist[newpos];
          nodelist[newpos] = el;

#elif INSERT_NODE_PROXIMITY
          { /* inserts at the end to keep stability of the list */
            register node_ptr el2 = nodelist[newpos];
            if ((node_ptr) NULL != el2) {
              while ((node_ptr) NULL != el2->link) el2 = el2->link;
              el2->link = el; /* append */
            }
            else { /* head of the (empty) list */
              nodelist[newpos] = el;
            }
            el->link = (node_ptr) NULL; /* last element */
          }

#elif INSERT_NODE_SORTED
          { /* similar to proximity, but searches the insertion
               point in the new list instead of appending */
            register node_ptr el2 = nodelist[newpos];
            node_ptr prev = (node_ptr) NULL;
            while ((node_ptr) NULL != el2) {
              const int cmp = node_cmp_fun(el, el2);
              nusmv_assert(0 != cmp); /* el cannot be in the list already */
              if (0 > cmp) break; /* found limit */
              prev = el2;
              el2 = el2->link;
            }

            if ((node_ptr) NULL != prev) { /* in the middle, after prev */
              el->link = prev->link;
              prev->link = el;
            }
            else { /* head of the list (possibly empty) */
              el->link = nodelist[newpos];
              nodelist[newpos] = el;
            }
          }
#else
# error "Insert node type unrecognized"
#endif
        }
      }

      FREE(cnodelist);
    }
  }
  else {
    /* not enough memory: fallback try with realloc */
    nodelist = (node_ptr*) REALLOC(node_ptr, node_mgr->nodelist, newsize);
    nusmv_assert((node_ptr*) NULL != nodelist);

    /* succeeded: resets the added part */
    memset(nodelist+node_mgr->nodelist_size, 0,
           (newsize - node_mgr->nodelist_size) * sizeof(node_ptr));

    { /* now moves elements in buckets' lists */
      const unsigned int csize = node_mgr->nodelist_size; /* alias */
      unsigned int i;

      for (i=0; i<csize; ++i) {
        node_ptr pel = (node_ptr) NULL;
        register node_ptr el;

        for (el = nodelist[i]; (node_ptr) NULL != el;) {
          unsigned int newpos = node_hash_fun(el) % newsize;

          if (newpos == i) { /* position remains untouched */
            /* the list keeps its ordering properties */
            pel = el;
            el = el->link;
          }
          else { /* element has to be moved into another bucket */
            node_ptr tmp = el->link; /* tmp points to the next
                                        element in the input
                                        list */

            /* removes the element 'el' from the old bucket */
            if ((node_ptr) NULL != pel) pel->link = tmp;
            else nodelist[i] = tmp;

#if INSERT_NODE_SIMPLE
            /* This algorithm is 'unstable', as it reverses the
               list alternatively */
            el->link = nodelist[newpos];
            nodelist[newpos] = el;

#elif INSERT_NODE_PROXIMITY
            { /* inserts at the end to keep stability of the list */
              register node_ptr el2 = nodelist[newpos];
              if ((node_ptr) NULL != el2) {
                while ((node_ptr) NULL != el2->link) el2 = el2->link;
                el2->link = el; /* append */
              }
              else { /* head of the (empty) list */
                nodelist[newpos] = el;
              }
              el->link = (node_ptr) NULL; /* last element */
            }

#elif INSERT_NODE_SORTED
            { /* similar to proximity, but searches the insertion
                 point in the new list instead of appending */
              register node_ptr el2 = nodelist[newpos];
              node_ptr prev = (node_ptr) NULL;

              while ((node_ptr) NULL != el2) {
                const int cmp = node_cmp_fun(el, el2);
                nusmv_assert(0 != cmp); /* el cannot be in the list already */
                if (0 > cmp) break; /* found limit */
                prev = el2;
                el2 = el2->link;
              }

              if ((node_ptr) NULL != prev) { /* in the middle, after prev */
                el->link = prev->link;
                prev->link = el;
              }
              else { /* head of the list (possibly empty) */
                el->link = nodelist[newpos];
                nodelist[newpos] = el;
              }
            }
#else
# error "Insert node type unrecognized"
#endif
            el = tmp;
          }
        }
      }
    } /* block */
  }

  node_mgr->nodelist = nodelist;
  node_mgr->nodelist_size = newsize;
  node_mgr->nodelist_size_idx += 1;


  /*_node_self_check(true);*/
  if (opt_verbose_level_gt(OptsHandler_get_instance(), 3)) {
    fprintf(nusmv_stderr, "Done. Size is now %d\n", newsize);
  }
}


#ifdef PROFILE_NODE
/**Function********************************************************************

  Synopsis           [Comparison function used for profiling]

******************************************************************************/
static
int profile_info_cmp(const void *a, const void *b)
{
  long res =
    (*((node_profile_info_ptr) b)).load -
    (*((node_profile_info_ptr) a)).load;

  return (int) res;
}
#endif
