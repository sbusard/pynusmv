/**CHeaderFile*****************************************************************

  FileName    [bmcCheck.h]

  PackageName [bmc]

  Synopsis    [Interface for module Check]

  Description [Contains function definition for propositional wff checking]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``bmc'' package of NuSMV version 2.
  Copyright (C) 2000-2001 by FBK-irst and University of Trento.

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

  Revision    [$Id: bmcCheck.h,v 1.7.4.1.2.1.2.1 2005-03-03 12:32:03 nusmv Exp $]

******************************************************************************/

#ifndef _BMC_CHECK__H
#define _BMC_CHECK__H


#include "utils/utils.h"
#include "node/node.h"


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/* matching function for iteration in lists of wffs */
typedef int (*BMC_PF_MATCH)(node_ptr,  int, void*);

/* answer function in case of match: */  
typedef void (*BMC_PF_MATCH_ANSWER)(node_ptr, int, void*);


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr 
Bmc_CheckFairnessListForPropositionalFormulae ARGS((node_ptr wffList)); 

EXTERN int 
Bmc_WffListMatchProperty ARGS((node_ptr wffList, BMC_PF_MATCH pCheck, 
			       void* pCheckOptArgument, int iMaxMatches, 
			       unsigned int* aiMatchedIndexes, 
			       BMC_PF_MATCH_ANSWER pAnswer, 
			       void* pAnswerOptArgument )); 

EXTERN boolean Bmc_IsPropositionalFormula ARGS((node_ptr wff)); 

/**AutomaticEnd***************************************************************/

#endif  /* _BMC_CHECK__H */

