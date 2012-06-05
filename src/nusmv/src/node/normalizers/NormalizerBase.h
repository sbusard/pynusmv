/**CHeaderFile*****************************************************************

  FileName    [PrinterBase.h]

  PackageName [node.normalizers]

  Synopsis    [Public interface of class 'NormalizerBase']

  Description []

  SeeAlso     [NormalizerBase.c]

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


******************************************************************************/


#ifndef __NORMALIZER_BASE_H__
#define __NORMALIZER_BASE_H__


#include "node/node.h"

#include "utils/object.h"
#include "utils/utils.h"


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class NormalizerBase]

  Description []

******************************************************************************/
typedef struct NormalizerBase_TAG*  NormalizerBase_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class NormalizerBase]

  Description [These macros must be used respectively to cast and to check
  instances of class NormalizerBase]

******************************************************************************/
#define NORMALIZER_BASE(self) \
         ((NormalizerBase_ptr) self)

#define NORMALIZER_BASE_CHECK_INSTANCE(self) \
         (nusmv_assert(NORMALIZER_BASE(self) != NORMALIZER_BASE(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN VIRTUAL node_ptr
NormalizerBase_normalize_node ARGS((NormalizerBase_ptr self,
                                    node_ptr n));



/**AutomaticEnd***************************************************************/


#endif /* __NORMALIZER_BASE_H__ */
