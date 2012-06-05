/**CHeaderFile*****************************************************************

   FileName    [NormalizerPsl.h]

   PackageName [node.normalizers]

   Synopsis    [Public interface of class 'NormalizerPsl']

   Description []

   SeeAlso     [NormalizerPsl.c]

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


#ifndef __NORMALIZER_PSL_H__
#define __NORMALIZER_PSL_H__


#include "NormalizerBase.h"
#include "utils/utils.h"


/**Type***********************************************************************

   Synopsis    [Definition of the public accessor for class NormalizerPsl]

   Description []

******************************************************************************/
typedef struct NormalizerPsl_TAG*  NormalizerPsl_ptr;


/**Macros**********************************************************************

   Synopsis    [To cast and check instances of class NormalizerPsl]

   Description [These macros must be used respectively to cast and to check
   instances of class NormalizerPsl]

******************************************************************************/
#define NORMALIZER_PSL(self)                    \
  ((NormalizerPsl_ptr) self)

#define NORMALIZER_PSL_CHECK_INSTANCE(self)                     \
  (nusmv_assert(NORMALIZER_PSL(self) != NORMALIZER_PSL(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN NormalizerPsl_ptr NormalizerPsl_create ARGS((const char* name));


/**AutomaticEnd***************************************************************/



#endif /* __NORMALIZER_PSL_H__ */
