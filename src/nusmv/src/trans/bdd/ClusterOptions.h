/**CHeaderFile*****************************************************************

  FileName    [ClusterOptions.h]

  PackageName [trans.bdd]

  Synopsis    [The header file of ClusterOptions class.]

  Description [ ]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.bdd'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst. 

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

  Revision    [$Id: ClusterOptions.h,v 1.1.2.2.4.1.6.3 2007-05-03 15:48:21 nusmv Exp $]

******************************************************************************/

#ifndef __TRANS_BDD_CLUSTER_OPTIONS_H__
#define __TRANS_BDD_CLUSTER_OPTIONS_H__


#include "utils/utils.h" /* for EXTERN and ARGS */
#include "opt/OptsHandler.h"


typedef struct ClusterOptions_TAG* ClusterOptions_ptr;

#define CLUSTER_OPTIONS(x) \
             ((ClusterOptions_ptr) x)

#define CLUSTER_OPTIONS_CHECK_INSTANCE(x) \
             nusmv_assert(CLUSTER_OPTIONS(x) != CLUSTER_OPTIONS(NULL))


EXTERN ClusterOptions_ptr 
ClusterOptions_create ARGS((OptsHandler_ptr options));

EXTERN void ClusterOptions_destroy ARGS((ClusterOptions_ptr self));

EXTERN int ClusterOptions_get_threshold ARGS((const ClusterOptions_ptr self));

EXTERN boolean ClusterOptions_is_affinity ARGS((const ClusterOptions_ptr self));

EXTERN boolean 
ClusterOptions_clusters_appended ARGS((const ClusterOptions_ptr self));

EXTERN boolean 
ClusterOptions_is_iwls95_preorder ARGS((const ClusterOptions_ptr self));

EXTERN int 
ClusterOptions_get_cluster_size ARGS((const ClusterOptions_ptr self));

EXTERN int 
ClusterOptions_get_w1 ARGS((const ClusterOptions_ptr self));

EXTERN int 
ClusterOptions_get_w2 ARGS((const ClusterOptions_ptr self));

EXTERN int 
ClusterOptions_get_w3 ARGS((const ClusterOptions_ptr self));

EXTERN int 
ClusterOptions_get_w4 ARGS((const ClusterOptions_ptr self));

EXTERN void ClusterOptions_print 
ARGS((const ClusterOptions_ptr self, FILE* file));

#endif /* __TRANS_BDD_CLUSTER_OPTIONS_H__ */
