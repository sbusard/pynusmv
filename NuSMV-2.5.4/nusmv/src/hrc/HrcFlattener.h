
/**CHeaderFile*****************************************************************

  FileName    [HrcFlattener.h]

  PackageName [hrc]

  Synopsis    [Public interface of class 'HrcFlattener']

  Description []

  SeeAlso     [HrcFlattener.c]

  Author      [Alessandro Mariotti]

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

  Revision    [$Id: HrcFlattener.h,v 1.1.2.1 2009-09-16 16:06:14 nusmv Exp $]

******************************************************************************/


#ifndef __HRC_FLATTENER_H__
#define __HRC_FLATTENER_H__


#include "utils/utils.h" 
#include "HrcNode.h"
#include "compile/symb_table/SymbTable.h"
#include "compile/FlatHierarchy.h"
#include "fsm/sexp/SexpFsm.h"
#include "hrc.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class HrcFlattener]

  Description []

******************************************************************************/
typedef struct HrcFlattener_TAG*  HrcFlattener_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class HrcFlattener]

  Description [These macros must be used respectively to cast and to check
  instances of class HrcFlattener]

******************************************************************************/
#define HRC_FLATTENER(self) \
         ((HrcFlattener_ptr) self)

#define HRC_FLATTENER_CHECK_INSTANCE(self) \
         (nusmv_assert(HRC_FLATTENER(self) != HRC_FLATTENER(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/


EXTERN FlatHierarchy_ptr 
HrcToFlatHierarchy ARGS((HrcNode_ptr node, 
                         SymbTable_ptr symb_table,
                         SymbLayer_ptr layer));

EXTERN SexpFsm_ptr 
HrcToSexpFsm ARGS((HrcNode_ptr node, 
                   SymbTable_ptr symb_table,
                   SymbLayer_ptr layer));

EXTERN void
HrcPopulateSymbTable ARGS((HrcNode_ptr node, 
                     SymbTable_ptr symb_table,
                     SymbLayer_ptr layer));

EXTERN HrcFlattener_ptr HrcFlattener_create ARGS((HrcNode_ptr node,
                                                  SymbTable_ptr symb_table,
                                                  SymbLayer_ptr layer));

EXTERN void HrcFlattener_flatten_hierarchy ARGS((HrcFlattener_ptr self));

EXTERN void HrcFlattener_populate_symbol_table ARGS((HrcFlattener_ptr self));

EXTERN FlatHierarchy_ptr 
HrcFlattener_get_flat_hierarchy ARGS((HrcFlattener_ptr self));

EXTERN SymbTable_ptr 
HrcFlattener_get_symbol_table ARGS((HrcFlattener_ptr self));

EXTERN SymbLayer_ptr 
HrcFlattener_get_symbol_layer ARGS((HrcFlattener_ptr self));

EXTERN void 
HrcFlattener_write_flatten_model ARGS((HrcFlattener_ptr self,
                                       FILE* out));

EXTERN void HrcFlattener_destroy ARGS((HrcFlattener_ptr self));


/**AutomaticEnd***************************************************************/

#endif /* __HRC_FLATTENER_H__ */
