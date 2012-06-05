
/**CHeaderFile*****************************************************************

  FileName    [hrcPrefixUtils.h]

  PackageName [hrc]

  Synopsis    [Header of hrcPrefixUtils.c.]

  Description []

  SeeAlso     []

  Author      [Sergio Mover]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK.

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

  Revision    [$Id: hrcPrefixUtils.h,v 1.1.2.2 2009-09-24 15:36:58 nusmv Exp $]

******************************************************************************/

#ifndef _HRC_PREFIX_UTILS__
#define _HRC_PREFIX_UTILS__

#include "set/set.h"
#include "node/node.h"
#include "hrc/hrc.h"

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


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN Set_t hrc_prefix_utils_get_prefix_symbols ARGS((Set_t symbol_set,
                                                       node_ptr prefix));

EXTERN boolean
hrc_prefix_utils_is_subprefix ARGS((node_ptr subprefix, node_ptr prefix));

EXTERN node_ptr
hrc_prefix_utils_add_context ARGS((node_ptr context, node_ptr expression));

EXTERN node_ptr hrc_prefix_utils_get_first_subcontext ARGS((node_ptr symbol));

EXTERN node_ptr hrc_prefix_utils_remove_context ARGS((node_ptr identifier,
                                                      node_ptr context));

EXTERN node_ptr 
hrc_prefix_utils_assign_module_name ARGS((HrcNode_ptr instance,
                                          node_ptr instance_name));

EXTERN node_ptr hrc_prefix_utils_flatten_instance_name ARGS((HrcNode_ptr instance));

/**AutomaticEnd***************************************************************/

#endif /* _HRC_PREFIX_UTILS__ */
