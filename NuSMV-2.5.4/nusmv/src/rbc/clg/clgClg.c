/**CFile***********************************************************************

  FileName    [clgClg.c]

  PackageName [clg]

  Synopsis    [Clause graphs - main file]

  Description [Manage clause graphs]

  SeeAlso     []

  Author      [Dan Sheridan and Marco Roveri]

  Copyright [This file is part of the ``rbc.clg'' package 
  of NuSMV version 2. Copyright (C) 2007 by FBK-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  as published by the Free Software Foundation; either version 2 of
  the License, or (at your option) any later version.

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

  To contact the NuSMV development board, email to <nusmv@fbk.eu>.]

******************************************************************************/

#include "clgInt.h"

#include "node/node.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: clgClg.c,v 1.1.2.2 2009-12-04 01:06:33 nusmv Exp $";



/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

static int * clause;         /* Currently handled clause. Statically declared
				to avoid time-consuming memory allocations and
				de-allocations */
static int max_clause_size;  /* Maximum size clause for *clause */

static clause_graph * clgs;  /* All of the clause graphs. Need to
				change over to a "manager" style
				system like the rest of NuSMV */
static int clg_count=0;      /* How many clgs have been allocated */
static int max_clg_count;    /* size of *clgs */


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void Extract(clause_graph head, node_ptr follow, int clause_size, 
		    int type, Clg_Commit commit, void *data);

static int AddToClause(int pos_lit, int neg_lit, int size);
static clause_graph new_clg(void);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/






/**Function********************************************************************

  Synopsis           [Create a CLG representing a single literal]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
clause_graph Clg_Lit(int literal)
{
  clause_graph lit;

  nusmv_assert(0 != literal);

  lit = new_clg();
  lit->label = literal;
  lit->size = 1;
  lit->left = NULL;
  lit->right = NULL;

  return lit;
}

/**Function********************************************************************

  Synopsis           [Create a CLG representing a conjunction of two CLGs]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
clause_graph Clg_Conj(clause_graph left, clause_graph right)
{
  clause_graph vtx;
  if (left == NULL) return right;
  if (right == NULL) return left;
  vtx = new_clg();
  vtx->label = CLG_CONJ;
  vtx->size = left->size + right->size;
  vtx->left = left;
  vtx->right = right;

  return vtx;
}

/**Function********************************************************************

  Synopsis           [Create a CLG representing a disjunction of two CLGs]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
clause_graph Clg_Disj(clause_graph left, clause_graph right)
{
  clause_graph vtx;
  if (left == NULL) return right;
  if (right == NULL) return left;
  vtx = new_clg();
  vtx->label = CLG_DISJ;
  vtx->size = left->size * right->size;
  vtx->left = left;
  vtx->right = right;

  return vtx;
}

/**Function********************************************************************

  Synopsis           [Return the number of clauses stored in the CLG]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Clg_Size(clause_graph graph)
{
  nusmv_assert(graph != (clause_graph)NULL);
  return graph->size;
}


/**Function********************************************************************

  Synopsis           [Free all CLGs]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Clg_Free(clause_graph graph)
{
  for ( ; clg_count>0 ; clg_count--) {
    FREE(clgs[clg_count-1]);
    clgs[clg_count-1] = (clause_graph)NULL;
  }
  FREE(clgs);
  clgs = (clause_graph *)NULL;
}

/**Function********************************************************************

  Synopsis           [Extract the real clauses from the CLG]

  Description        [Calls commit with each extracted clause as an argument.
                      type indicates the style of clause (eg, ZChaff 
		      all-positive integer format); *data is passed to commit
		      as an extra argument.

		      Clauses have duplicated literals suppressed and
		      clauses with both positive and negative
		      occurrences of the same literal are skipped.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Clg_Extract(clause_graph head, int type, Clg_Commit commit, void *data)
{
  /* Allocate space for a biggish clause. Allocate once and avoid
     reallocating -- it only wastes time. */
  clause = ALLOC(int, 256);

  nusmv_assert(clause != (int *)NULL);

  max_clause_size = 256;

  Extract(head, Nil, 0, type, commit, data);
  FREE(clause);
  clause = (int *)NULL;
  max_clause_size = 0;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Extract the clauses, passing them to commit]

  Description        [Walk the data structure from the head, creating clauses 
                      each time one is seen complete. See Footnote 936 for 
		      details of algorithm]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void Extract(clause_graph head, node_ptr follow, 
		    int clause_size, int type, Clg_Commit commit, void *data)
{

  nusmv_assert(head != NULL);

  /* Check for leaf node */
  /* literals are such that both left and right are NULL. A better
     approach would be to add a field type or kind to the clg_graph so
     that it can be LITERAL, CONJ, DISJ */
  if (head->left == NULL && head->right == NULL) {

    switch (type) {
    case CLG_DIMACS:
    case CLG_NUSMV:
      clause_size = AddToClause(head->label, -(head->label), clause_size);
      break;

    case CLG_ZCHAFF:
      if (head->label < 0) {
	clause_size = AddToClause(1-(head->label)*2, -(head->label)*2, 
				  clause_size);	
      } else {
	clause_size = AddToClause((head->label)*2, 1+(head->label)*2, 
				  clause_size);	
      }
      break;

    default:
      internal_error("Clg_Extract: Bad extract type\n");
    }

    if (clause_size == 0) {
      /* All clauses from here on would be redundant, so backtrack now */
      return;
    }
    if (follow == Nil) {
      /* Nothing else to do: commit this clause! */
      commit(data, clause, clause_size);
    } else {
      /* Follow the follow list */
      Extract((clause_graph)car(follow), cdr(follow), clause_size, 
	      type, commit, data);
    }
  } else if (head->label == CLG_CONJ) {

    /* Internal conjunction node: branch */
    Extract(head->left, follow, clause_size, type, commit, data);
    Extract(head->right, follow, clause_size, type, commit, data);
    
  } else if (head->label == CLG_DISJ) {
    node_ptr nfollow;

    /* Internal disjunction node: chain */
    /* Heuristic to speed up search */
    nusmv_assert(head->left != (clause_graph)NULL);
    nusmv_assert(head->right != (clause_graph)NULL);

    if (head->left->size < head->right->size) {
      nfollow = cons((node_ptr)(head->right), follow);
      Extract(head->left, nfollow, clause_size, type, commit, data);
    } else {
      nfollow = cons((node_ptr)(head->left), follow);
      Extract(head->right, nfollow, clause_size, type, commit, data);
    }
    free_node(nfollow);
  } else {
    /* Something's wrong */
    internal_error("Clg_Extract: Nonsense clause graph vertex\n");
  }
}


/**Function********************************************************************

  Synopsis           [Insert a literal into the current clause]

  Description [Insert pos_lit, where neg_lit is the corresponding
               literal of opposite polarity. If neg_lit is already in
               the clause then the clause is cancelled; if pos_lit is
               already in the clause then it is not reinserted.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int AddToClause(int pos_lit, int neg_lit, int size)
{
  int i;

  nusmv_assert(max_clause_size > 0);

  /* Check size and reallocate if necessary */
  if (size+1 == max_clause_size) {
      max_clause_size *= 2;
      nusmv_assert(clause != (int *)NULL);
      clause = REALLOC(int, clause, max_clause_size);
      nusmv_assert(clause != (int *)NULL);
  }

  /* O(n) method for inserting... but who cares? */
  for (i=0; i<size; i++) {
    if (clause[i]==pos_lit) {
      return size;
    }
    if (clause[i]==neg_lit) {
      return 0;
    }
  }
  clause[i]=pos_lit;
  return size+1;
}
      


/**Function********************************************************************

  Synopsis           [Allocate a new CLG node.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static clause_graph new_clg(void)
{
  clause_graph result;

  if (clg_count == 0) {
    clgs = ALLOC(clause_graph, 4096);
    nusmv_assert(clgs != (clause_graph *) NULL);
    max_clg_count = 4096;
  }
  if (clg_count+1 >= max_clg_count) {
    max_clg_count *= 2;
    clgs = REALLOC(clause_graph, clgs, max_clg_count);
    nusmv_assert(clgs != (clause_graph *) NULL);
  }

  result = ALLOC(struct Clg_Vertex, 1);
  nusmv_assert(result != (struct Clg_Vertex *) NULL);

  clgs[clg_count] = result;
  clg_count++;

  return result;
}
