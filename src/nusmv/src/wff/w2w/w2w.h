/**CHeaderFile*****************************************************************

  FileName    [w2w.h]

  PackageName [wff.w2w]

  Synopsis    [Public interface for well-formed-formula to
               well-formed-formula conversions.]

  Description []

  Author      [Alessandro Mariotti]

  Copyright   [
  This file is part of the ``wff.w2w'' package of NuSMV version 2.
  Copyright (C) 2011 by FBK-irst.

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


#ifndef __W_2_W_H__
#define __W_2_W_H__

#include "utils/utils.h"
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN node_ptr Wff2Nnf ARGS((node_ptr wff));
EXTERN void w2w_clear_wff2nnf_memoization ARGS((void));

#endif /* __W_2_W_H__ */
