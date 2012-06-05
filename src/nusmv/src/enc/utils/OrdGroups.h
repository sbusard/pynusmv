/**CHeaderFile*****************************************************************

  FileName    [OrdGroups.h]

  PackageName [enc.utils]

  Synopsis    [Public interface of class OrdGroups. ]

  Description [This class represents a set of groups of variables to
  be kept grouped]
                                               
  SeeAlso     [OrdGroups.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.utils'' package of NuSMV version 2. 
  Copyright (C) 2005 by FBK-irst. 

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

#ifndef __ENC_UTILS_ORD_GROUPS_H__
#define __ENC_UTILS_ORD_GROUPS_H__

#include "utils/utils.h"
#include "utils/NodeList.h"
#include "node/node.h"

typedef struct OrdGroups_TAG*  OrdGroups_ptr;

#define ORD_GROUPS(x)  \
        ((OrdGroups_ptr) (x))

#define ORD_GROUPS_CHECK_INSTANCE(x)  \
        (nusmv_assert(ORD_GROUPS(x) != ORD_GROUPS(NULL)))



EXTERN OrdGroups_ptr OrdGroups_create ARGS((void));
EXTERN OrdGroups_ptr OrdGroups_copy ARGS((const OrdGroups_ptr self));
EXTERN void OrdGroups_destroy ARGS((OrdGroups_ptr self));

EXTERN int OrdGroups_create_group ARGS((OrdGroups_ptr self));

EXTERN void 
OrdGroups_add_variable ARGS((OrdGroups_ptr self, node_ptr name, 
                             int group));

EXTERN void
OrdGroups_add_variables ARGS((OrdGroups_ptr self, NodeList_ptr vars, 
                              int group));

EXTERN NodeList_ptr 
OrdGroups_get_vars_in_group ARGS((const OrdGroups_ptr self, int group));

EXTERN int
OrdGroups_get_var_group ARGS((const OrdGroups_ptr self, node_ptr name));

EXTERN int OrdGroups_get_size ARGS((const OrdGroups_ptr self));

#endif /* __ENC_UTILS_ORD_GROUPS_H__ */
