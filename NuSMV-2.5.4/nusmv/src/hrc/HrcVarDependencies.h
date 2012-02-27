
/**CHeaderFile*****************************************************************

  FileName    [HrcVarDependencies.h]

  PackageName [hrc]

  Synopsis    [Public interface of class 'HrcVarDependencies']

  Description [Public interface of class 'HrcVarDependencies']

  SeeAlso     [HrcVarDependencies.c]

  Author      [Sergio Mover]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2. 
  Copyright (C) 2009 by FBK-irst. 

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

  Revision    [$Id: HrcVarDependencies.h,v 1.1.2.1 2009-08-05 15:51:24 nusmv Exp $]

******************************************************************************/


#ifndef __HRC_VAR_DEPENDENCIES_H__
#define __HRC_VAR_DEPENDENCIES_H__


#include "node/node.h"
#include "set/set.h"
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class HrcVarDependencies]

  Description []

******************************************************************************/
typedef struct HrcVarDependencies_TAG*  HrcVarDependencies_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class HrcVarDependencies]

  Description [These macros must be used respectively to cast and to check
  instances of class HrcVarDependencies]

******************************************************************************/
#define HRC_VAR_DEPENDENCIES(self) \
         ((HrcVarDependencies_ptr) self)

#define HRC_VAR_DEPENDENCIES_CHECK_INSTANCE(self) \
         (nusmv_assert(HRC_VAR_DEPENDENCIES(self) != HRC_VAR_DEPENDENCIES(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN HrcVarDependencies_ptr HrcVarDependencies_create ARGS((void));

EXTERN void HrcVarDependencies_destroy ARGS((HrcVarDependencies_ptr self));

EXTERN void 
HrcVarDependencies_add_variable ARGS((HrcVarDependencies_ptr self,
                                      node_ptr variable));

EXTERN void 
HrcVarDependencies_add_define ARGS((HrcVarDependencies_ptr self,
                                    node_ptr define));

EXTERN void 
HrcVarDependencies_add_parameter ARGS((HrcVarDependencies_ptr self,
                                       node_ptr formal,
                                       node_ptr actual));

EXTERN Set_t 
HrcVarDependencies_get_variables_set ARGS((HrcVarDependencies_ptr self));

EXTERN Set_t 
HrcVarDependencies_get_defines_set ARGS((HrcVarDependencies_ptr self));

EXTERN Set_t 
HrcVarDependencies_get_formal_par_set ARGS((HrcVarDependencies_ptr self));

EXTERN Set_t 
HrcVarDependencies_get_actual_par_set ARGS((HrcVarDependencies_ptr self));

EXTERN boolean 
HrcVarDependencies_has_formal_parameter ARGS((HrcVarDependencies_ptr self,
                                              node_ptr formal));

/**AutomaticEnd***************************************************************/



#endif /* __HRC_VAR_DEPENDENCIES_H__ */
