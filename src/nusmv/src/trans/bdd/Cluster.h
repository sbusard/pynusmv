/**CHeaderFile*****************************************************************

  FileName    [Cluster.h]

  PackageName [trans.bdd]

  Synopsis    [The header file of trans cluster class.]

  Description []

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

  Revision    [$Id: Cluster.h,v 1.1.2.3.4.1.6.1 2007-04-30 16:41:18 nusmv Exp $]

******************************************************************************/

#ifndef __TRANS_BDD_CLUSTER_H__
#define __TRANS_BDD_CLUSTER_H__

#include "ClusterOptions.h"
#include "utils/object.h" 
#include "utils/utils.h" /* for EXTERN adn ARGS */
#include "dd/dd.h"


typedef struct Cluster_TAG*  Cluster_ptr;

typedef struct ClusterIwls95_TAG*  ClusterIwls95_ptr;

#define CLUSTER(x)    \
          ((Cluster_ptr) x)

#define CLUSTER_CHECK_INSTANCE(x)  \
          (nusmv_assert(CLUSTER(x) != CLUSTER(NULL)))

#define CLUSTER_IWLS95(x)    \
          ((ClusterIwls95_ptr) x)

#define CLUSTER_IWLS95_CHECK_INSTANCE(x)  \
          (nusmv_assert(CLUSTER_IWLS95(x) != CLUSTER_IWLS95(NULL)))



/* ---------------------------------------------------------------------- */
/*    Public interface                                                    */
/* ---------------------------------------------------------------------- */

EXTERN Cluster_ptr Cluster_create ARGS((DdManager* dd));

EXTERN boolean 
Cluster_is_equal ARGS((const Cluster_ptr self, const Cluster_ptr other));

EXTERN bdd_ptr Cluster_get_trans ARGS((const Cluster_ptr self));

EXTERN void 
Cluster_set_trans ARGS((Cluster_ptr self, DdManager* dd, bdd_ptr current));

EXTERN bdd_ptr 
Cluster_get_quantification_state_input ARGS((const Cluster_ptr self));

EXTERN void 
Cluster_set_quantification_state_input ARGS((Cluster_ptr self, 
                                             DdManager* dd, bdd_ptr new_val));

EXTERN bdd_ptr 
Cluster_get_quantification_state ARGS((const Cluster_ptr self));

EXTERN void 
Cluster_set_quantification_state ARGS((Cluster_ptr self, 
                                       DdManager* dd, bdd_ptr new_val));


/* ClusterIwls95 inherits from Cluster: */
EXTERN ClusterIwls95_ptr 
ClusterIwls95_create ARGS((DdManager* dd, 
                           const ClusterOptions_ptr trans_options, 
                           const double v_c, 
                           const double w_c, 
                           const double x_c, 
                           const double y_c, 
                           const double z_c, 
                           const double min_c, 
                           const double max_c)); 


EXTERN double 
ClusterIwls95_get_benefit ARGS((const ClusterIwls95_ptr self));


#endif /* __TRANS_BDD_CLUSTER_H__ */
