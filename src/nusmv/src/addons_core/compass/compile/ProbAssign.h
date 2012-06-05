
/**CHeaderFile*****************************************************************

  FileName    [ProbAssign.h]

  PackageName [compass.compile]

  Synopsis    [Public interface of class 'ProbAssign']

  Description []

  SeeAlso     [ProbAssign.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compass.compile'' package of NuSMV version 2. 
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

  Revision    [$Id: ProbAssign.h,v 1.1.2.1 2008-12-09 21:01:17 nusmv Exp $]

******************************************************************************/


#ifndef __PROB_ASSIGN_H__
#define __PROB_ASSIGN_H__


#include "node/node.h"
#include "utils/utils.h" 

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class ProbAssign]

  Description []

******************************************************************************/
typedef struct ProbAssign_TAG*  ProbAssign_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class ProbAssign]

  Description [These macros must be used respectively to cast and to check
  instances of class ProbAssign]

******************************************************************************/
#define PROB_ASSIGN(self) \
         ((ProbAssign_ptr) self)

#define PROB_ASSIGN_CHECK_INSTANCE(self) \
         (nusmv_assert(PROB_ASSIGN(self) != PROB_ASSIGN(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN ProbAssign_ptr 
ProbAssign_create ARGS((node_ptr assigns, node_ptr value));

EXTERN void ProbAssign_destroy ARGS((ProbAssign_ptr self));

EXTERN node_ptr 
ProbAssign_get_assigns_expr ARGS((const ProbAssign_ptr self));

EXTERN node_ptr ProbAssign_get_prob ARGS((const ProbAssign_ptr self));


/**AutomaticEnd***************************************************************/



#endif /* __PROB_ASSIGN_H__ */
