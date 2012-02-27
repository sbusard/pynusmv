/**CFile***********************************************************************

  FileName    [dagStat.c]

  PackageName [dag]

  Synopsis    [DAG manager statistics.]

  Description [External procedures included in this module:
		<ul>
		<li> <b>Dag_GetStats()</b> Get statistics;
		<li> <b>Dag_PrintStats()</b> Print statistics;
		</ul>]
		
  SeeAlso     [dagManager]

  Copyright   [
  This file is part of the ``dag'' package of NuSMV version 2. 
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

  Revision    [v. 1.0]

******************************************************************************/

#include "math.h"

#include "dagInt.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


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


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Prints various statistics.]

  Description [Prints the following:
               <ul>
               <li> the number of entries found in every chunk of
                    `clustSz' bins (if `clustSz' is 1 then the number
                    of entries per bin is given, if `clustSz' is 0 no
		    such information is displayed);
               <li> the number of shared vertices, i.e., the number
                    of v's such that v -> mark > 1;
	       <li> the average entries per bin and the variance;
	       <li> min and max entries per bin.
               </ul>]


  SideEffects [none]

  SeeAlso     []

******************************************************************************/
void
Dag_PrintStats(
  Dag_Manager_t * dagManager,            
  int             clustSz,
  FILE          * outFile)
{

  int               min, max, runSz, totClust;
  int               i, j = 0;
  st_table_entry *  b;

  int               shared = 0;
  float             total = 0.0;
  float             variance = 0.0;
  float             mean = 0.0;
  int               numBins = dagManager -> vTable -> num_bins;
  st_table_entry ** theHash = dagManager -> vTable -> bins;

  /* First pass: calculating the total, shared vertices and (eventually) 
     printing the bin's data. */
  runSz = clustSz;
  totClust = 0;
  for (i = 0; i < numBins; i++) {
    /* Scan each bin entries. */
    for (j = 0, b = theHash[i]; b != NIL(st_table_entry); b = b -> next, j++) {
      /* For each entry, check if the vertex is shared. */
      if (((Dag_Vertex_t*)(b -> key)) -> mark > 1) {
	shared += 1;
      }
    }
    total += (float)j;
    totClust += j;
    runSz -= 1;
    if (runSz == 0) {
      /* clustSz bins were visited, print out the information. */
      fprintf(outFile, "%6d\n", totClust);
      runSz = clustSz;
      totClust = 0;
    }
  } 
  if (clustSz > 0) {
    fprintf(outFile, "%6d\n", totClust);
  }
  mean = total / (float)numBins;

  /* Min and max initialized to the last value read in the first pass. */
  min = max = j;

  /* Second pass: variance, min and max. */
  for (i = 0; i < numBins; i++) {
    for (j = 0, b = theHash[i]; b != NIL(st_table_entry); b = b -> next, j++);
    variance += pow(((float)j - mean), 2.0);
    if (j > max) {
      max = j;
    } 
    if (j < min) {
      min = j;
    }
  } 
  variance = variance / (float)(numBins - 1);

  fprintf(outFile, "Mean     %10.3f\n", mean);
  fprintf(outFile, "Variance %10.3f\n", variance);
  fprintf(outFile, "Min      %10d\n", min);
  fprintf(outFile, "Max      %10d\n", max); 

  fprintf(outFile, "Total    %10.0f\n", total);
  fprintf(outFile, "Shared   %10d\n", shared);

  return;

} /* End of Dag_PrintStats. */


