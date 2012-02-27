/**CHeaderFile*****************************************************************

  FileName    [node.h]

  PackageName [node]

  Synopsis    [The header file of the <tt>node</tt> package.]

  Description [The <tt>node</tt> package implements a data structure
  which offers constructs with e syntax and semantic lisp like.]

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
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA.

  For more information on NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

  Revision [$Id:node.h,v 1.10.2.2.2.1.2.7.4.15 2010-02-08 12:25:28 nusmv Exp $]

******************************************************************************/

#ifndef _node_h
#define _node_h

#include <stdio.h> /* for FILE */

#include "utils/defs.h"
#include "util.h" /* for EXTERN and ARGS */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define Nil ((node_ptr)0)
#define FAILURE_NODE ((node_ptr)(-1))
#define CLOSED_NODE (node_ptr)(-2)

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef union value_ node_val;
typedef struct node node_rec;
typedef struct node * node_ptr;
typedef node_ptr (*NPFN)(node_ptr);
typedef node_ptr (*NPFNN)(node_ptr, node_ptr);
typedef void  (*VPFN)(node_ptr);
typedef boolean (*BPFN)(node_ptr);

typedef int (*custom_print_sexp_t)(FILE *, node_ptr);
typedef int (*out_func_t)(void*, char*);
typedef int (*custom_print_node_t)(out_func_t, FILE *, node_ptr,
                                   int *, char **, int *, int *, int *);

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Possible value that a node can assume.]

  Description [The value of a node is generic. It may be an integer, a
  pointer to a node, a pointer to a string_ structure or a pointer to
  an ADD or BDD. This in order to have a behavior lisp like, in which
  a variable may be a n integer, an atom or a list.]

  SeeAlso     [string_]

******************************************************************************/
union value_ {
  int inttype;
  struct node *nodetype;
  struct string_ * strtype;
  void * bddtype;
};


/**Struct**********************************************************************

  Synopsis    [The <tt>node</tt> data structure.]

  Description [This data structure allows the implementation of a lisp
  like s-expression.
  <ul>
  <li><b>lineno</b> It is used to store the line number of the input
      file to which the stored data refers to.
  <li><b>type</b> It indicates the kind of node we are dealing
      with. I.e. if the node is <em>CONS</em> like, or a leaf, and in
      this case which kind of leaf (<em>NUMBER</em>, <em>ATOM</em>, ...).
  <li><b>left</b> It's the left branch of the s-expression.
  <li><b>right</b> It's the left branch of the s-expression.
  <li><b>link</b> It's a pointer used in the internal hash.
  ]

******************************************************************************/
struct node {
  struct node *link;
  /*char locked; */ /* this field should be a bit into the others
                       (e.g. in type) */
  short int type;
  int lineno;
  node_val left;
  node_val right;
  void* extra_data; /* added to handle rbc */
};

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
/**Macro**********************************************************************
  Synopsis     [Casts the given pointer to a node_ptr]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define NODE_PTR(x)                             \
  ((node_ptr) (x))

/**Macro**********************************************************************
  Synopsis     [Casts the given node_ptr to an int]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#define NODE_TO_INT(x)                          \
  (PTR_TO_INT(x))

/**Macro**********************************************************************
  Synopsis     [Casts the given int to a node_ptr]
  Description  []
  SideEffects  []
  SeeAlso      []
******************************************************************************/
#if NUSMV_SIZEOF_VOID_P == 8 && NUSMV_SIZEOF_INT == 4
#if NUSMV_SIZEOF_LONG == 8
#define NODE_FROM_INT(x)                                \
  (PTR_FROM_INT(node_ptr, (0x00000000FFFFFFFFL & ((nusmv_ptrint)x))))
#else
/* Long is 4 bytes, thus we need to use long long (Windows 64bits) */
#define NODE_FROM_INT(x)                                \
  (PTR_FROM_INT(node_ptr, (0x00000000FFFFFFFFLL & ((nusmv_ptrint)x))))
#endif
#else
#define NODE_FROM_INT(x)                        \
  (PTR_FROM_INT(node_ptr, (x)))
#endif

#define caar(_n_) car(car(_n_))
#define cadr(_n_) car(cdr(_n_))
#define cdar(_n_) cdr(car(_n_))
#define cddr(_n_) cdr(cdr(_n_))
#define node_get_type(_n_) (_n_)->type
#define node_get_lineno(_n_) (_n_)->lineno
#define node_get_lstring(_n_) (_n_)->left.strtype
#define node_get_int(_n_) (_n_)->left.inttype
#define node_bdd_setcar(_n_,_b_) \
{ /*nusmv_assert(!(_n_)->locked);*/ (_n_)->left.bddtype = (_b_); }

#define node_bdd_setcdr(_n_,_b_) \
{ /*nusmv_assert(!(_n_)->locked);*/ (_n_)->right.bddtype = (_b_); }

#define node_node_setcar(_n_,_b_)\
{ /*nusmv_assert(!(_n_)->locked);*/ (_n_)->left.nodetype = (_b_); }

#define node_node_setcdr(_n_,_b_)\
{ /*nusmv_assert(!(_n_)->locked);*/ (_n_)->right.nodetype = (_b_); }

#define node_int_setcar(_n_,_b_) \
{ /*nusmv_assert(!(_n_)->locked);*/ (_n_)->left.inttype = (_b_); }

#define node_int_setcdr(_n_,_b_) \
{ /*nusmv_assert(!(_n_)->locked);*/ (_n_)->right.inttype = (_b_); }

#define node_str_setcar(_n_,_b_) \
{ /*nusmv_assert(!(_n_)->locked);*/ (_n_)->left.strtype = (_b_);  }

#define node_str_setcdr(_n_,_b_) \
{ /*nusmv_assert(!(_n_)->locked);*/ (_n_)->right.strtype = (_b_); }



/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void node_pkg_init ARGS((void));
EXTERN void node_pkg_quit ARGS((void));

EXTERN void free_node ARGS((node_ptr));
EXTERN void swap_nodes ARGS((node_ptr *, node_ptr *));
EXTERN int node_is_failure ARGS((node_ptr node));
EXTERN int node_is_leaf ARGS((node_ptr node));

EXTERN void     walk ARGS((VPFN fun, node_ptr));
EXTERN node_ptr new_node ARGS((int, node_ptr, node_ptr));
EXTERN node_ptr new_lined_node ARGS((int, node_ptr, node_ptr, int));
EXTERN node_ptr cons ARGS((node_ptr, node_ptr));
EXTERN node_ptr car ARGS((node_ptr));
EXTERN node_ptr cdr ARGS((node_ptr));
EXTERN node_ptr new_list ARGS((void));
EXTERN node_ptr copy_list ARGS((node_ptr l));
EXTERN void     free_list ARGS((node_ptr l));
EXTERN int      is_list_empty ARGS((node_ptr));
EXTERN int      in_list ARGS((node_ptr, node_ptr));
EXTERN int      llength ARGS((node_ptr));
EXTERN node_ptr append ARGS((node_ptr, node_ptr));
EXTERN node_ptr append_ns ARGS((node_ptr, node_ptr));
EXTERN node_ptr reverse ARGS((node_ptr));
EXTERN node_ptr reverse_ns ARGS((node_ptr));
EXTERN node_ptr last ARGS((node_ptr));
EXTERN node_ptr map ARGS((NPFN fun, node_ptr));
EXTERN node_ptr map2 ARGS((NPFNN fun, node_ptr, node_ptr));
EXTERN node_ptr even_elements ARGS((node_ptr));
EXTERN node_ptr odd_elements ARGS((node_ptr));
EXTERN node_ptr node_subtract ARGS((node_ptr, node_ptr));
EXTERN node_ptr find_node ARGS((int, node_ptr, node_ptr));
EXTERN node_ptr find_atom ARGS((node_ptr));
EXTERN int print_sexp ARGS((FILE *, node_ptr));

EXTERN void print_array_type ARGS((FILE* output_stream, const node_ptr body));

EXTERN void normalize_nonboolean_case
ARGS((FILE *file, node_ptr node, custom_print_sexp_t cps_fun,
      int pred, node_ptr lhs));

EXTERN int print_node ARGS((FILE *, node_ptr));
EXTERN char* sprint_node ARGS((node_ptr));

EXTERN int print_node_indent_at ARGS((FILE *stream, node_ptr n, int ofs));
EXTERN char* sprint_node_indent_at ARGS((node_ptr n, int ofs));

EXTERN int print_node_indent ARGS((FILE *stream, node_ptr n));
EXTERN char* sprint_node_indent ARGS((node_ptr n));

EXTERN void setcar ARGS((node_ptr, node_ptr));
EXTERN void setcdr ARGS((node_ptr, node_ptr));
EXTERN void node_set_type ARGS((node_ptr, int));
EXTERN node_ptr node_normalize ARGS((node_ptr));
EXTERN node_ptr node_normalize_list ARGS((node_ptr sexp));

EXTERN struct MasterNormalizer_TAG*
node_pkg_get_global_master_normalizer ARGS((void));

EXTERN struct MasterPrinter_TAG*
node_pkg_get_global_master_wff_printer ARGS((void));

EXTERN struct MasterPrinter_TAG*
node_pkg_get_global_master_sexp_printer ARGS((void));

EXTERN struct MasterPrinter_TAG*
node_pkg_get_indenting_master_wff_printer ARGS((void));

#ifdef PROFILE_NODE
EXTERN void node_show_profile_stats ARGS((FILE* out));
#endif

#endif /* _node_h */

