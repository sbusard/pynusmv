/**CHeaderFile*****************************************************************

   FileName    [NormalizerPsl_private.h]

   PackageName [node.normalizers]

   Synopsis    [Private and protected interface of class 'NormalizerPsl']

   Description [This file can be included only by derived and friend classes]

   SeeAlso     [NormalizerPsl.h]

   Author      [Alessandro Mariotti]

   Copyright   [
   This file is part of the ``node.normalizers'' package of NuSMV version 2.
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

******************************************************************************/


#ifndef __NORMALIZER_PSL_PRIVATE_H__
#define __NORMALIZER_PSL_PRIVATE_H__


#include "NormalizerPsl.h"
#include "NormalizerBase.h"
#include "NormalizerBase_private.h"
#include "utils/utils.h"


/**Struct**********************************************************************

   Synopsis    [NormalizerPsl class definition derived from
   class NormalizerBase]

   Description []

   SeeAlso     [Base class NormalizerBase]

******************************************************************************/
typedef struct NormalizerPsl_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(NormalizerBase);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */


  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} NormalizerPsl;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void
normalizer_psl_init ARGS((NormalizerPsl_ptr self, const char* name,
                          int low, size_t num));

EXTERN void normalizer_psl_deinit ARGS((NormalizerPsl_ptr self));

EXTERN node_ptr
normalizer_psl_normalize_node ARGS((NormalizerBase_ptr self, node_ptr n));

#endif /* __NORMALIZER_PSL_PRIVATE_H__ */
