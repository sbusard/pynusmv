/**CHeaderFile*****************************************************************

  FileName    [MasterNormalizer.h]

  PackageName [node.normalizers]

  Synopsis    [Public interface of class 'MasterNormalizer']

  Description []

  SeeAlso     [MasterNormalizer.c]

  Author      [Alessandro Mariotti]

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

  Revision    [$Id: MasterNormalizer.h,v 1.1.2.4.6.1 2009-03-23 18:13:22 nusmv Exp $]

******************************************************************************/


#ifndef __MASTER_NORMALIZER_H__
#define __MASTER_NORMALIZER_H__

#include "node/node.h"
#include "node/MasterNodeWalker.h"

#include "utils/utils.h"


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class MasterNormalizer]

  Description []

******************************************************************************/
typedef struct MasterNormalizer_TAG*  MasterNormalizer_ptr;



/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class MasterNormalizer]

  Description [These macros must be used respectively to cast and to check
  instances of class MasterNormalizer]

******************************************************************************/
#define MASTER_NORMALIZER(self) \
         ((MasterNormalizer_ptr) self)

#define MASTER_NORMALIZER_CHECK_INSTANCE(self) \
         (nusmv_assert(MASTER_NORMALIZER(self) != MASTER_NORMALIZER(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN MasterNormalizer_ptr MasterNormalizer_create ARGS((void));

EXTERN node_ptr
MasterNormalizer_normalize_node ARGS((MasterNormalizer_ptr self, node_ptr n));

EXTERN node_ptr
MasterNormalizer_lookup_cache ARGS((MasterNormalizer_ptr self, node_ptr n));

EXTERN void
MasterNormalizer_insert_cache ARGS((MasterNormalizer_ptr self, node_ptr n,
                                    node_ptr find));

EXTERN void
MasterNormalizer_destroy ARGS((MasterNormalizer_ptr self));


/**AutomaticEnd***************************************************************/


#endif /* __MASTER_NORMALIZER_H__ */
