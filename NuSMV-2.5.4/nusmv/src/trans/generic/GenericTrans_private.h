/**CHeaderFile*****************************************************************

  FileName    [GenericTrans_private.h]

  PackageName [trans.generic]

  Synopsis    [The private interface of class GenericTrans]

  Description [Private definition to be used by derived classes]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``trans.generic'' package of NuSMV version 2. 
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

  Revision    [$Id: ]

******************************************************************************/

#ifndef __TRANS_GENERIC_GENERIC_TRANS_PRIVATE_H__
#define __TRANS_GENERIC_GENERIC_TRANS_PRIVATE_H__


#include "GenericTrans.h"

#include "utils/utils.h"
#include "utils/object_private.h"


/**Struct**********************************************************************

  Synopsis    [Transition Relation Class "GenericTrans"]

  Description [ This class defines a prototype for a generic
  transition relation. 
  This class is virtual, and must be specialized.
  ]

  SeeAlso     []   
  
******************************************************************************/
typedef struct GenericTrans_TAG
{
  INHERITS_FROM(Object);

  /* ---------------------------------------------------------------------- */ 
  /*     Private members                                                    */
  /* ---------------------------------------------------------------------- */   
  TransType _type;

  /* ---------------------------------------------------------------------- */ 
  /*     Virtual Methods                                                    */
  /* ---------------------------------------------------------------------- */   
 
} GenericTrans; 


void generic_trans_init ARGS((GenericTrans_ptr self, 
                              const TransType trans_type));

void generic_trans_deinit ARGS((GenericTrans_ptr self));

void generic_trans_copy_aux ARGS((const GenericTrans_ptr self, 
                                  GenericTrans_ptr copy));


#endif /* __TRANS_GENERIC_GENERIC_TRANS_PRIVATE_H__ */
