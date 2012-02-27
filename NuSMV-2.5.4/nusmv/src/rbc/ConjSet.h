
/**CHeaderFile*****************************************************************

  FileName    [ConjSet.h]

  PackageName [rbc]

  Synopsis    [Public interface of class 'ConjSet']

  Description []

  SeeAlso     [ConjSet.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``rbc'' package of NuSMV version 2. 
  Copyright (C) 2007 by FBK-irst. 

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

  Revision    [$Id: ConjSet.h,v 1.1.2.5 2007-04-04 12:00:14 nusmv Exp $]

******************************************************************************/


#ifndef __CONJ_SET_H__
#define __CONJ_SET_H__

#include "rbc/rbc.h"

#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class ConjSet]

  Description [A ConjSet holds the associations between variables 
	and the corresponding expression each variable can be substituted with. 
	A ConjSet is internally used by RBC inlining. In particular it is used 
	by class InlineResult]

******************************************************************************/
typedef struct ConjSet_TAG*  ConjSet_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class ConjSet]

  Description [These macros must be used respectively to cast and to check
  instances of class ConjSet]

******************************************************************************/
#define CONJ_SET(self) \
         ((ConjSet_ptr) self)

#define CONJ_SET_CHECK_INSTANCE(self) \
         (nusmv_assert(CONJ_SET(self) != CONJ_SET(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN ConjSet_ptr ConjSet_create ARGS((Rbc_Manager_t* rbcm));
EXTERN void ConjSet_destroy ARGS((ConjSet_ptr self));
EXTERN ConjSet_ptr ConjSet_copy ARGS((const ConjSet_ptr self));

EXTERN void ConjSet_add_var_assign ARGS((ConjSet_ptr self, 
					 Rbc_t* var, Rbc_t* expr));

EXTERN void ConjSet_inherit_from ARGS((ConjSet_ptr self, 
				       const ConjSet_ptr other));

EXTERN void ConjSet_flattenize ARGS((ConjSet_ptr self));
EXTERN Rbc_t* ConjSet_substitute ARGS((ConjSet_ptr self, Rbc_t* f));
EXTERN Rbc_t* ConjSet_conjoin ARGS((ConjSet_ptr self, Rbc_t* f));


/**AutomaticEnd***************************************************************/



#endif /* __CONJ_SET_H__ */
