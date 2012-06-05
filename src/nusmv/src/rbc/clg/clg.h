/**CFile***********************************************************************

  FileName    [clg.c]

  PackageName [clg]

  Synopsis    [Clause graphs]

  Description [Compact data structure for representing sets of clauses with
               sharing of common structure. The data structure is a graph of 
	       conjunctions and disjunctions which are converted using the 
	       standard (exponential-size) CNF conversion to obtain the required
	       clauses.]

  SeeAlso     []

  Author      [Dan Sheridan & Marco Roveri]

  Copyright   [This file is part of the ``rbc.clg'' package 
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

  Revision    [$Id: clg.h,v 1.1.2.1 2007-01-30 17:46:23 nusmv Exp $]

******************************************************************************/

#ifndef _CLG__H
#define _CLG__H

#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define CLG_DIMACS 20 /* Create clauses suitable for a DIMACS file */
#define CLG_ZCHAFF 21 /* Create clauses suitable for feeding to ZChaff directly */
#define CLG_NUSMV  22 /* Create clauses suitable for feeding to NuMSV */

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct Clg_Vertex* clause_graph;

typedef void(*Clg_Commit)(void*, int*, int);

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN clause_graph Clg_Lit ARGS((int literal));
EXTERN clause_graph Clg_Conj ARGS((clause_graph left, clause_graph right));
EXTERN clause_graph Clg_Disj ARGS((clause_graph left, clause_graph right));

EXTERN void Clg_Extract ARGS((clause_graph head, int type, Clg_Commit commit, 
			      void *data));

EXTERN int Clg_Size ARGS((clause_graph graph));
EXTERN void Clg_Free ARGS((clause_graph graph));

/**AutomaticEnd***************************************************************/

#endif /* _CLG__H */

