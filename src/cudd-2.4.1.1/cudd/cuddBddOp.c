/**CFile***********************************************************************

  FileName    [cuddBddOp.c]

  PackageName [cudd]

  Synopsis    [Some useful functions for manipulating BDD.]

  Description [Some useful functions for manipulating BDD.]

  Author      [Marco Roveri]

  Copyright   [Copyright (C) 1998 by FBK-irst
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.

  Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in the
  documentation and/or other materials provided with the distribution.

  Neither the name of the University of Colorado nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
  POSSIBILITY OF SUCH DAMAGE.]

******************************************************************************/

#include    "util.h"
#include    "st.h"
#include    "cuddInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

#ifndef lint
static char rcsid[] DD_UNUSED = "$Id: cuddBddOp.c,v 1.1.2.1 2010-02-04 10:41:15 nusmv Exp $";
#endif


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/
/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static int bddCheckPositiveCube (DdManager *, DdNode *);
static void ddClearFlag (DdNode *);
static CUDD_VALUE_TYPE Cudd_fatal_error (DdManager *, const char *);

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
/**Function********************************************************************

  Synopsis    [Computes the difference between two BDD cubes.]

  Description [Computes the set difference between two BDD cubes.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
Cudd_bddCubeDiff(
DdManager * dd,
DdNode * a,
DdNode * b)
{
  DdNode * res;

  if (bddCheckPositiveCube(dd, a) == 0) {
    (void) fprintf(dd->err,"Error: (arg_1) Can only abstract positive cubes\n");
    return(NULL);
  }
  if (bddCheckPositiveCube(dd, b) == 0) {
    (void) fprintf(dd->err,"Error: (arg_2) Can only abstract positive cubes\n");
    return(NULL);
  }
  do {
    dd->reordered = 0;
    res = cudd_bddCubeDiffRecur(dd, a, b);
  } while (dd->reordered == 1);
  return(res);
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int 
Cudd_BddGetLowestVar(
DdManager *dd,
DdNode * N)
{
  int res = Cudd_BddGetLowestVarRecur(dd, N, 0);
  ddClearFlag(N);
  return(Cudd_ReadInvPerm(dd, res));
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Cudd_bddCubeDiff.]

  Description [Performs the recursive step of Cudd_bddCubeDiff.
  Returns a pointer to the result if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
DdNode *
cudd_bddCubeDiffRecur(
DdManager * dd,
DdNode    * f,
DdNode    * g)
{
  DdNode       * t,  * e , * res;
  DdNode       * tf, * tg;
  unsigned int topf, topg;
  DdNode       * _true  = DD_TRUE(dd);
  DdNode       * _false = Cudd_Not(_true);
  
  if ((f == _false) || (g == _false))
    Cudd_fatal_error(dd, "cudd_bddCubeDiff: f == FALSE || g == FALSE");
  if (f == _true) return(f);

  topf = cuddI(dd,f->index);
  topg = cuddI(dd,g->index);

  if (topf < topg) {
    e = _false;
    cuddRef(e);
    tf = cuddT(f);
    t = cudd_bddCubeDiffRecur(dd, tf, g);
    if (t == NULL) {
      cuddDeref(e);
      return(NULL);
    }
    cuddRef(t);
    res = (t == e) ? t : cuddUniqueInter(dd, f->index, t, e);
    if (res == NULL) {
      cuddDeref(e);
      Cudd_RecursiveDeref(dd, t);
      return(NULL);
    }
    cuddDeref(t);
    cuddDeref(e);
    return(res);
  }
  else if (topf == topg) {
    tf = cuddT(f);
    tg = cuddT(g);
    res = cudd_bddCubeDiffRecur(dd, tf, tg);
    return(res);
  }
  else {
    tg = cuddT(g);
    res = cudd_bddCubeDiffRecur(dd, f, tg);
    return(res);
  }
}

/**Function********************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int 
Cudd_BddGetLowestVarRecur(
DdManager *dd,
DdNode * N,
int level)
{
  int i; 
  DdNode * RN = Cudd_Regular(N);
  /* if Cudd_IsComplement(RN->next) is true then the node has already been visited */
  if (Cudd_IsComplement(RN->next) || cuddIsConstant(RN)) return(level);
  RN->next = Cudd_Not(RN->next);
  i = cuddI(dd, RN->index);
  if (i > level) level = i;
  return(Cudd_BddGetLowestVarRecur(dd, cuddT(RN),
                              Cudd_BddGetLowestVarRecur(dd, cuddE(RN), level)));
}

/**Function********************************************************************

  Synopsis           [Returns the array of All Possible Minterms]

  Description        [Takes a minterm and returns an array of all its
  terms, according to variables specified in the array vars[].  Notice
  that the array of the result has to be previously allocated, and its
  size must be greater or equal the number of the minterms of the "minterm"
  function. The array contains referenced BDD so it is necessary to
  dereference them after their use.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int 
Cudd_PickAllTerms(
DdManager * dd      /* manager */,
DdNode *    minterm /* minterm from which to pick  all term */,
DdNode **   vars    /* The array of the vars to be put in the returned array */,
int         n       /* The size of the above array */,
DdNode **   result  /* The array used as return value */)
{
  CUDD_VALUE_TYPE value;
  DdGen * gen;
  int * cube;
  int q;
  int pos = 0;
  int reorder_status = 0;
  Cudd_ReorderingType Reorder_Method;

  if (result == NULL) {
    (void) fprintf(dd->err, "Cudd_PickAllTerms: result == NULL\n");
    return 1;
  }
  
  /*
    Check the dynamic reordering status. If enabled, then the status
    is saved in order to restore it after the operation. 
  */
  if (Cudd_ReorderingStatus(dd, &Reorder_Method)) {
    Cudd_AutodynDisable(dd);
    reorder_status = 1;
  }
  
  Cudd_ForeachCube(dd, minterm, gen, cube, value) {
    /* number of indifferent boolean variables */
    int nd = 0;
    /* We build the cube of the minterm. */

    for(q = 0; q < n; q++) {
      switch(cube[vars[q]->index]) {
      case 0:
      case 1:
        break;
      default:
        nd += 1;
        break;
      }
    }
    { /* For each cube we expand the terms, i.e. all the assignments. */
      int ** matrix;
      int lim = 0;
      int num_of_mint = (int)pow(2.0,(double)nd);

      matrix = ALLOC(int *, num_of_mint+1);
      if (matrix == NULL) {
        fprintf(dd->err, "Cudd_PickAllTerms: Unable to allocate matrix[]\n");
        return 1;
      }
      {
        int i;
        for (i = 0; i <= num_of_mint; i++) {
          matrix[i] = ALLOC(int, n+1); 
          if (matrix[i] == NULL) {
            fprintf(dd->err, "Cudd_PickAllTerms: Unable to allocate matrix[%d][]\n", i);
            return 1;
          }
        }
      }

      lim = 1;
      for(q = 0; q < n; q++) {
        switch(cube[vars[q]->index]) {
        case 0: {
          int i;
          for(i = 0; i < num_of_mint; i++) matrix[i][q] = 0;
          break;
        }
        case 1: {
          int i;
          for(i = 0; i < num_of_mint; i++) matrix[i][q] = 1;
          break;
        }
        case 2: {
          /*
            The current variable is indifferent. It has to be expanded
            considering the cases in which it is assigned a true value
            and the cases in which it is assigned a false value.
          */
          int i, j;
          i = 0;
          j = (1<<(lim-1));
          for (; j <= num_of_mint; ) {
            for(; i < j  && i < num_of_mint; i++) matrix[i][q] = 1;
            j += (1<<(lim-1));
            for(; i < j && i < num_of_mint; i++) matrix[i][q] = 0;
            j += (1<<(lim-1));
          }
          lim++;
          break;
        }
        default:
          (void) fprintf(dd->err,"\nCudd_PickAllTerms: unexpected switch value %d\n", cube[vars[q]->index]);
          return 1;
        }
      }
      {
        int i, j;

        for (j = 0; j < num_of_mint; j++) {
          result[pos] = Cudd_ReadTrue(dd);
          cuddRef(result[pos]);

          /* sometimes it is better to start building the cube from
             the higher order variables to the lower order variable to
             avoid traversal of the BDD so far build to add the last
             variable. Usually the orders of variables in vars
             corresponds to the order of bdd variables */
          for(i = n - 1; i >= 0; i--) {
            DdNode * New;  

            switch(matrix[j][i]) {
            case 0: {
              New = Cudd_bddAnd(dd, result[pos], Cudd_Not(vars[i]));
              break;
            }
            case 1: {
              New = Cudd_bddAnd(dd, result[pos], vars[i]);
              break;
            }
            default: {
              int k;

              fprintf(dd->err, "Cudd_PickAllTerms: unexpected switch value %d\n", matrix[j][i]);
              for(k = 0; k < num_of_mint; k++) FREE(matrix[k]);
              FREE(matrix);
              for(k = 0; k <= pos; k++) Cudd_RecursiveDeref(dd, result[k]);
              return 1;
            }
            }
              
            if (New == NULL) {
              int k;

              for(k = 0; k < num_of_mint; k++) FREE(matrix[k]);
              FREE(matrix);
              for(k = 0; k <= pos; k++) Cudd_RecursiveDeref(dd, result[k]);
              return 1;
            }
            cuddRef(New);
            Cudd_RecursiveDeref(dd, result[pos]);
            result[pos] = New;
          }
          pos++;
        }
      }
      {
        int k;

        for(k = 0; k <= num_of_mint; k++) FREE(matrix[k]);
        FREE(matrix);
      }
    }
  } /* End of Cudd_ForeachCube */

  /* If the case than the dynamic variable ordering is restored. */
  if (reorder_status) Cudd_AutodynEnable(dd, Reorder_Method);
  return 0;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs a DFS from f, clearing the LSB of the next
  pointers.]

  Description []

  SideEffects [None]

  SeeAlso     [ddSupportStep ddDagInt]

******************************************************************************/
static void
ddClearFlag(
DdNode *f)
{
    if (!Cudd_IsComplement(f->next)) {
	return;
    }
    /* Clear visited flag. */
    f->next = Cudd_Regular(f->next);
    if (cuddIsConstant(f)) {
	return;
    }
    ddClearFlag(cuddT(f));
    ddClearFlag(Cudd_Regular(cuddE(f)));
    return;

} /* end of ddClearFlag */

/**Function********************************************************************

  Synopsis [Checks whether cube is an BDD representing the product of
  positive literals.]

  Description [Returns 1 in case of success; 0 otherwise.]

  SideEffects [None]

******************************************************************************/
static int
bddCheckPositiveCube(
DdManager *manager,
DdNode	  *cube)
{
  if (Cudd_IsComplement(cube)) return(0);
  if (cube == DD_TRUE(manager)) return(1);
  if (cuddIsConstant(cube)) return(0);
  if (cuddE(cube) == Cudd_Not(DD_TRUE(manager))) {
    return(bddCheckPositiveCube(manager, cuddT(cube)));
  }
  return(0);
} /* end of bddCheckPositiveCube */

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static CUDD_VALUE_TYPE Cudd_fatal_error(DdManager * dd, const char * message)
{
  start_parsing_err();
  fprintf(dd->err,"\nFatal error: %s\n", message);
  finish_parsing_err();
  return(NULL);
}


