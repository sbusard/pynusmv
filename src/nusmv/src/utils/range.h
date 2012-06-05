/**CHeaderFile*****************************************************************

  FileName    [range.h]

  PackageName [utils]

  Synopsis    [External header of the utils package]

  Description [External header of the utils package.]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2004 by FBK-irst. 

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

  Revision    [$Id: range.h,v 1.1.2.1.4.2 2005-04-27 08:50:16 nusmv Exp $]

******************************************************************************/

#ifndef __UTILS_RANGE_H__
#define __UTILS_RANGE_H__

#include "utils.h"
#include "node/node.h"


EXTERN void Utils_set_data_for_range_check ARGS((node_ptr var,
                                                 node_ptr range));
EXTERN void Utils_set_mode_for_range_check ARGS((boolean is_fatal));

EXTERN void Utils_range_check ARGS((node_ptr n));
EXTERN void Utils_failure_node_check ARGS((node_ptr n));

EXTERN boolean Utils_is_in_range ARGS((node_ptr s, node_ptr d));

EXTERN boolean Utils_check_subrange ARGS((node_ptr subrange));
EXTERN boolean Utils_check_subrange_not_negative ARGS((node_ptr subrange));


#endif /* __UTILS_RANGE_H__ */
