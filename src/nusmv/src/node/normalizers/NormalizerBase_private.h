/**CHeaderFile*****************************************************************

  FileName    [NormalizerBase_private.h]

  PackageName [node.normalizers]

  Synopsis    [Private and protected interface of class 'NormalizerBase']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [NormalizerBase.h]

  Author      [Mariotti Alessandro]

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

******************************************************************************/


#ifndef __NORMALIZER_BASE_PRIVATE_H__
#define __NORMALIZER_BASE_PRIVATE_H__


#include "NormalizerBase.h"
#include "node/NodeWalker_private.h"

#include "utils/utils.h"


/**Struct**********************************************************************

  Synopsis    [NormalizerBase class definition derived from
               class NodeWalker]

  Description []

  SeeAlso     [Base class Object]

******************************************************************************/
typedef struct NormalizerBase_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(NodeWalker);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
  node_ptr (*normalize_node)(NormalizerBase_ptr self, node_ptr n);

} NormalizerBase;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */
EXTERN NormalizerBase_ptr
NormalizerBase_create ARGS((const char* name, int low, size_t num));

EXTERN void
normalizer_base_init ARGS((NormalizerBase_ptr self, const char* name,
                        int low, size_t num, boolean can_handle_null));

EXTERN void normalizer_base_deinit ARGS((NormalizerBase_ptr self));

EXTERN node_ptr
normalizer_base_throw_normalize_node ARGS((NormalizerBase_ptr self,
                                           node_ptr n));


#endif /* __NORMALIZER_BASE_PRIVATE_H__ */
