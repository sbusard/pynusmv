/**CFile***********************************************************************

  FileName    [rbcOutput.c]

  PackageName [rbc]

  Synopsis    [Formula output in various formats.]

  Description [External functions included in this module:
                <ul>
                <li> <b>Rbc_OutputDaVinci()</b>
                <li> <b>Rbc_OutputSexpr()<b>
                </ul>]

  SeeAlso     []

  Author      [Armando Tacchella]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by University of Genova.

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

#include "rbc/rbcInt.h"


static char rcsid[] UTIL_UNUSED = "$Id: rbcOutput.c,v 1.5.2.1.2.1.2.1.6.5 2010-02-18 10:00:03 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define LABEL_SZ 20

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************
  Synopsis      [Data passing in daVinci output DFS.]
  Description   [Data passing in daVinci output DFS.]
  SeeAlso       []
******************************************************************************/
struct DaVinciDfsData {
  int    label;
  FILE * outFile;
};

/**Struct**********************************************************************
  Synopsis      [Data passing in Sexpr output DFS.]
  Description   [Data passing in Sexpr output DFS.]
  SeeAlso       []
******************************************************************************/
struct SexprDfsData {
  FILE * outFile;
};

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct DaVinciDfsData DaVinciDfsData_t;
typedef struct SexprDfsData SexprDfsData_t;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int DaVinciSet(Rbc_t * f, char * daVinciData, nusmv_ptrint sign);
static void DaVinciFirst(Rbc_t * f, char * DaVinciData, nusmv_ptrint sign);
static void DaVinciBack(Rbc_t * f, char * daVinciData, nusmv_ptrint sign);
static void DaVinciLast(Rbc_t * f, char * daVinciData, nusmv_ptrint sign);

static int GdlSet(Rbc_t * f, char * GdlData, nusmv_ptrint sign);
static void GdlFirst(Rbc_t * f, char * GdlData, nusmv_ptrint sign);
static void GdlBack(Rbc_t * f, char * GdlData, nusmv_ptrint sign);
static void GdlLast(Rbc_t * f, char * GdlData, nusmv_ptrint sign);

static int SexprSet(Rbc_t * f, char * SexprData, nusmv_ptrint sign);
static void SexprFirst(Rbc_t * f, char * SexprData, nusmv_ptrint sign);
static void SexprBack(Rbc_t * f, char * SexprData, nusmv_ptrint sign);
static void SexprLast(Rbc_t * f, char * SexprData, nusmv_ptrint sign);

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Print out an rbc using DaVinci graph format.]

  Description [Print out an rbc using DaVinci graph format.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void
Rbc_OutputDaVinci(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * f,
  FILE          * outFile)
{
  Dag_DfsFunctions_t daVinciFunctions;
  DaVinciDfsData_t   daVinciData;

  /* Cleaning the user fields. */
  Dag_Dfs(f, &dag_DfsClean, NIL(char));

  /* Setting up the DFS functions. */
  daVinciFunctions.Set        = DaVinciSet;
  daVinciFunctions.FirstVisit = DaVinciFirst;
  daVinciFunctions.BackVisit  = DaVinciBack;
  daVinciFunctions.LastVisit  = DaVinciLast;

 /* Setting up the DFS data. */
  daVinciData.label   = 0;
  daVinciData.outFile = outFile;

  /* Initialize the graph and print a fake an ancestor. */
  fprintf(outFile, "[ ");
  fprintf(outFile,
          "l(\"root\",n(\"\",[a(\"OBJECT\",\"\"),a(\"_GO\",\"box\")],[ ");

  /* Calling DFS on f. */
  Dag_Dfs(f, &daVinciFunctions, (char*)(&daVinciData));

  /* Closing the fake ancestor (root) and the graph. */
  fprintf(outFile, "])) ]");

  return;

} /* End of Rbc_OutputDaVinci. */


/**Function********************************************************************

  Synopsis    [Print out an rbc using LISP S-expressions.]

  Description [Print out an rbc using LISP S-exrpressions.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void
Rbc_OutputSexpr(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * f,
  FILE          * outFile)
{
  Dag_DfsFunctions_t SexprFunctions;
  SexprDfsData_t     SexprData;

  /* Cleaning the user fields. */
  Dag_Dfs(f, &dag_DfsClean, NIL(char));

  /* Setting up the DFS functions. */
  SexprFunctions.Set        = SexprSet;
  SexprFunctions.FirstVisit = SexprFirst;
  SexprFunctions.BackVisit  = SexprBack;
  SexprFunctions.LastVisit  = SexprLast;

 /* Setting up the DFS data. */
  SexprData.outFile = outFile;

  /* Calling DFS on f. */
  Dag_Dfs(f, &SexprFunctions, (char*)(&SexprData));

  return;

} /* End of Rbc_OutputSexpr. */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Dfs Set for DaVinci output.]

  Description [Dfs Set for DaVinci output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
DaVinciSet(
 Rbc_t  * f,
 char   * daVinciData,
 nusmv_ptrint sign)
{

  static char      * symbols[RBCIFF + 1] = {"", "", "/\\\\", "<-->"};
  static char      * colors [RBCIFF + 1] = {"#CCCCCC", "#CCCCCC", "#EEEEFF", "#CCCCD7"};

  DaVinciDfsData_t * sd = (DaVinciDfsData_t*)daVinciData;

  /* Increment the label. */
  ++(sd -> label);

  /* Draw the incoming edge... */
  if (sign != 0) {
    /* Red edge if the current vertex is negated... */
    fprintf(sd -> outFile,
            "l(\"e_%d\",e(\"\",[a(\"EDGECOLOR\",\"#e0e0e0\"),a(\"EDGEPATTERN\",\"dashed\")],", sd -> label);
  } else {
    /* ... green otherwise. */
    fprintf(sd -> outFile,
            "l(\"e_%d\",e(\"\",[a(\"EDGECOLOR\",\"blue\")],", sd -> label);
  }

  /* Draw the node... */
  if (f -> gRef != NIL(char)) {
    /* This subtree was already visited: write the reference,
       close the edge and return. */
    fprintf(sd -> outFile, "r(\"%s\")", f -> gRef);
    fprintf(sd -> outFile, "))");
    return (1);
  } else {
    /* This subtree was not yet visited: compose a label and store it. */
    f -> gRef = ALLOC(char, LABEL_SZ);
    switch (f -> symbol) {
    case RBCVAR :
      sprintf(f -> gRef, "x%d", PTR_TO_INT(f -> data));
      break;
    case RBCAND :
      sprintf(f -> gRef, "and_%d", sd -> label);
      break;
    case RBCIFF :
      sprintf(f -> gRef, "iff_%d", sd -> label);
      break;
    default :
      sprintf(f -> gRef, "err");
    }
    /* Write the node. */
    if (f -> symbol == RBCVAR) {
      /* In the case of variables, also close the edge and return. */

      fprintf(sd -> outFile,
      "l(\"%s\",n(\"\",[a(\"OBJECT\",\"%s\"),a(\"COLOR\",\"#FFDDDD\"),a(\"BORDER\",\"double\"),a(\"_GO\",\"box\")],[]))",
      f -> gRef, f -> gRef);

      fprintf(sd -> outFile, "))");
      return (1);
    } else {
      /* In the case of formulas, leave the world list and the edge open. */
      fprintf(sd -> outFile,

      "l(\"%s\",n(\"\",[a(\"OBJECT\",\"%s\"),a(\"COLOR\",\"%s\"),a(\"FONTFAMILY\",\"lucida\"),a(\"_GO\",\"ellipse\")],[",
      f -> gRef, symbols[f -> symbol], colors[f -> symbol]);


      return (0);
    }
  }

} /* End of DaVinciSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for DaVinci output.]

  Description [Dfs FirstVisit for DaVinci output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
DaVinciFirst(
 Rbc_t  * f,
 char   * DaVinciData,
 nusmv_ptrint sign)
{
  /* Set the user-defined integer data to 1 to remember operands. */
  f -> iRef = (f->outList!=(Dag_Vertex_t**)NULL) ? f->numSons : 0;

  return;
} /* End of DaVinciFirst. */


/**Function********************************************************************

  Synopsis    [Dfs BackVisit for DaVinci output.]

  Description [Dfs BackVisit for DaVinci output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
DaVinciBack(
 Rbc_t  * f,
 char   * daVinciData,
 nusmv_ptrint sign)
{
  DaVinciDfsData_t * sd   = (DaVinciDfsData_t*)daVinciData;

  if (f -> iRef > 1) {
    fprintf(sd -> outFile, ",");
    --(f -> iRef);
  }

  return;

} /* End of DaVinciBack. */


/**Function********************************************************************

  Synopsis    [Dfs LastVisit for DaVinci output.]

  Description [Dfs LastVisit for DaVinci outputon.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
DaVinciLast(
 Rbc_t  * f,
 char   * daVinciData,
 nusmv_ptrint sign)
{
  DaVinciDfsData_t * sd   = (DaVinciDfsData_t*)daVinciData;

  fprintf(sd -> outFile, "]))))");

  return;

} /* End of DaVinciLast. */


/**Function********************************************************************

  Synopsis    [Dfs Set for Sexpr output.]

  Description [Dfs Set for Sexpr output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
SexprSet(
 Rbc_t  * f,
 char   * SexprData,
 nusmv_ptrint sign)
{

  /* Always visit (a simple expression is a tree). */
  return (-1);

} /* End of SexprSet. */


/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for Sexpr output.]

  Description [Dfs FirstVisit for Sexpr output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SexprFirst(
 Rbc_t  * f,
 char   * SexprData,
 nusmv_ptrint sign)
{
  SexprDfsData_t * sd   = (SexprDfsData_t*)SexprData;

  /* If the sign is negative print out a negation. */
  if (sign == RBC_FALSE) {
    fprintf(sd -> outFile, "(NOT ");
  }

  /* If this an operator print out its label. */
  switch (f -> symbol) {
  case RBCAND:
    fprintf(sd -> outFile, "(AND ");
    break;
  case RBCIFF:
    fprintf(sd -> outFile, "(IFF ");
    break;
  case RBCITE:
    fprintf(sd -> outFile, "(ITE ");
    break;
  case RBCVAR:
    fprintf(sd -> outFile, "X%d", PTR_TO_INT(f -> data));
    break;
  default:
    break;
  }

  /* Set the user-defined integer data to 1 to remember operands. */
  f -> iRef = 1;

  return;

} /* End of SexprFirst. */


/**Function********************************************************************

  Synopsis    [Dfs BackVisit for Sexpr output.]

  Description [Dfs BackVisit for Sexpr output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SexprBack(
 Rbc_t  * f,
 char   * SexprData,
 nusmv_ptrint sign)
{
  SexprDfsData_t * sd   = (SexprDfsData_t*)SexprData;

  if (f -> iRef == 1) {
    fprintf(sd -> outFile, " ");
    --(f -> iRef);
  }

  return;

} /* End of SexprBack. */


/**Function********************************************************************

  Synopsis    [Dfs LastVisit for Sexpr output.]

  Description [Dfs LastVisit for Sexpr outputon.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
SexprLast(
 Rbc_t  * f,
 char   * SexprData,
 nusmv_ptrint sign)
{
  SexprDfsData_t * sd   = (SexprDfsData_t*)SexprData;


  /* Close the operator. */
  if (f -> symbol != RBCVAR) {
    fprintf(sd -> outFile, ")");
  }

  /* Close the negation. */
  if (sign == RBC_FALSE) {
    fprintf(sd -> outFile, ")");
  }

  return;

} /* End of SexprLast. */









/**Struct**********************************************************************
  Synopsis      [Data passing in Gdl output DFS.]
  Description   [Data passing in Gdl output DFS.]
  SeeAlso       []
******************************************************************************/
struct GdlDfsData {
  int    label;
  char * fatherName;
  FILE * outFile;
};

typedef struct GdlDfsData GdlDfsData_t;






/**Function********************************************************************

  Synopsis    [Print out an rbc using Gdl graph format.]

  Description [Print out an rbc using Gdl graph format.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
void
Rbc_OutputGdl(
  Rbc_Manager_t * rbcManager,
  Rbc_t         * f,
  FILE          * outFile)
{
  Dag_DfsFunctions_t GdlFunctions;
  GdlDfsData_t   GdlData;

  /* Cleaning the user fields. */
  Dag_Dfs(f, &dag_DfsClean, NIL(char));

  /* Setting up the DFS functions. */
  GdlFunctions.Set        = GdlSet;
  GdlFunctions.FirstVisit = GdlFirst;
  GdlFunctions.BackVisit  = GdlBack;
  GdlFunctions.LastVisit  = GdlLast;

 /* Setting up the DFS data. */
  GdlData.label   = 0;
  GdlData.outFile = outFile;

  /* Initialize the graph and print a fake an ancestor. */
  fprintf(outFile, "graph: {\n");

  /* Calling DFS on f. */
  Dag_Dfs(f, &GdlFunctions, (char*)(&GdlData));

  /* Closing the fake ancestor (root) and the graph. */
  fprintf(outFile, "}");

  return;

} /* End of Rbc_OutputGdl. */

/**Function********************************************************************

  Synopsis    [Dfs Set for Gdl output.]

  Description [Dfs Set for Gdl output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int
GdlSet(
 Rbc_t  * f,
 char   * GdlData,
 nusmv_ptrint sign)
{

#if 0 /* this is disabled */
  static char      * symbols[RBCIFF + 1] = {"", "", "&", "<->"};
#endif
  GdlDfsData_t * sd = (GdlDfsData_t*)GdlData;

  /* Increment the label. */
  ++(sd -> label);

  /* Draw the incoming edge... */
#if 0  /* this is disabled */
  if (sign != 0) {
    /* Red edge if the current vertex is negated... */
    fprintf(sd -> outFile,
            "l(\"e_%d\",e(\"\",[a(\"EDGECOLOR\",\"blue\"),a(\"EDGEPATTERN\",\"dashed\")],", sd -> label);
  } else {
    /* ... green otherwise. */
    fprintf(sd -> outFile,
            "l(\"e_%d\",e(\"\",[a(\"EDGECOLOR\",\"blue\")],", sd -> label);
  }
#endif 

  /* Draw the node... */
  if (f -> gRef != NIL(char)) {
    /* This subtree was already visited: write the reference,
       close the edge and return. */
#if 0 /* this is disabled */
    fprintf(sd -> outFile, "r(\"%s\")", f -> gRef);
    fprintf(sd -> outFile, "))");
#endif
    if (sd->fatherName != NULL)
        fprintf(sd -> outFile,
		"edge: { sourcename: \"%s\" targetname: \"%s\" }\n", sd->fatherName,f ->gRef);

    return (1);
  } else {
    /* This subtree was not yet visited: compose a label and store it. */
    f -> gRef = ALLOC(char, LABEL_SZ);
    switch (f -> symbol) {
    case RBCVAR :
      sprintf(f -> gRef, "x%d", PTR_TO_INT(f -> data));
      break;
    case RBCAND :
      sprintf(f -> gRef, "and_%d", sd -> label);
      break;
    case RBCIFF :
      sprintf(f -> gRef, "iff_%d", sd -> label);
      break;
    default :
      sprintf(f -> gRef, "err");
    }

    /* Write the edge. */
    if (sd->fatherName != NULL)
        fprintf(sd -> outFile,
             "edge: { sourcename: \"%s\" targetname: \"%s\" }\n", sd->fatherName,f ->gRef);

    /* Write the node. */
    if (f -> symbol == RBCVAR) {
      /* In the case of variables, also close the edge and return. */
#if 0 /* this is disabled */
      fprintf(sd -> outFile,
      "l(\"%s\",n(\"\",[a(\"OBJECT\",\"%s\"),a(\"BORDER\",\"double\"),a(\"_GO\",\"box\")],[]))",
      f -> gRef, f -> gRef);
      fprintf(sd -> outFile, "))");
#endif

      fprintf(sd -> outFile,
              "    node: { title: \"%s\" }\n", f -> gRef);

      return (1);
    }
    else {
#if 0 /* this is disabled */
      /* In the case of formulas, leave the world list and the edge open. */
      fprintf(sd -> outFile,
      "l(\"%s\",n(\"\",[a(\"OBJECT\",\"%s\"),a(\"_GO\",\"ellipse\")],[",
      f -> gRef, symbols[f -> symbol]);
#endif

      fprintf(sd -> outFile, "    node: { title: \"%s\" }\n", f -> gRef);
      sd->fatherName = f -> gRef;

      return (0);
    }
  }


} /* End of GdlSet. */

/**Function********************************************************************

  Synopsis    [Dfs FirstVisit for Gdl output.]

  Description [Dfs FirstVisit for Gdl output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
GdlFirst(
 Rbc_t  * f,
 char   * GdlData,
 nusmv_ptrint sign)
{

  /* Set the user-defined integer data to 1 to remember operands. */
  f -> iRef = 1;

  return;

} /* End of GdlFirst. */

/**Function********************************************************************

  Synopsis    [Dfs BackVisit for Gdl output.]

  Description [Dfs BackVisit for Gdl output.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
GdlBack(
 Rbc_t  * f,
 char   * GdlData,
 nusmv_ptrint sign)
{
#if 0 /* this is disabled */
  GdlDfsData_t * sd   = (GdlDfsData_t*) GdlData;
#endif

  if (f -> iRef == 1) {
#if 0 /* this is disabled */
    fprintf(sd -> outFile, ",");
#endif
    --(f -> iRef);
  }

  return;

} /* End of GdlBack. */

/**Function********************************************************************

  Synopsis    [Dfs LastVisit for Gdl output.]

  Description [Dfs LastVisit for Gdl outputon.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static void
GdlLast(
 Rbc_t  * f,
 char   * GdlData,
 nusmv_ptrint sign)
{
#if 0 /* this is disabled */
  GdlDfsData_t * sd   = (GdlDfsData_t*)GdlData;

  fprintf(sd -> outFile, "}\n");
#endif

  return;

} /* End of GdlLast. */
