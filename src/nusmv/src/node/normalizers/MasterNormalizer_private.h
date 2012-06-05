/**CHeaderFile*****************************************************************

  FileName    [MasterNormalizer_private.h]

  PackageName [node.normalizers]

  Synopsis    [Private interface of class 'MasterNormalizer', to be used by normalizers]

  Description []

  SeeAlso     [MasterNormalizer.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``node.normalizers'' package of NuSMV version 2.
  Copyright (C) 2006 by FBK-irst.

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

  Revision    [$Id: MasterNormalizer_private.h,v 1.1.2.1.6.1 2009-03-23 18:13:22 nusmv Exp $]

******************************************************************************/


#ifndef __MASTER_NORMALIZER_PRIVATE_H__
#define __MASTER_NORMALIZER_PRIVATE_H__

#include "MasterNormalizer.h"
#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr
master_normalizer_normalize_node ARGS((MasterNormalizer_ptr self, node_ptr n));

/**AutomaticEnd***************************************************************/

#endif /* __MASTER_NORMALIZER_PRIVATE_H__ */
