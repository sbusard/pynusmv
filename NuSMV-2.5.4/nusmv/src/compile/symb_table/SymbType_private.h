/**CHeaderFile*****************************************************************

  FileName    [SymbType_private.h]

  PackageName [compile.symb_table]

  Synopsis    [Private interface accessed by class SymbTable]

  Description []
                                               
  SeeAlso     [SymbType.h]

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``compile.symb_table'' package of NuSMV
  version 2.  Copyright (C) 2005 by FBK-irst.

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

#ifndef __SYMB_TYPE_PRIVATE_H__
#define __SYMB_TYPE_PRIVATE_H__

#include "SymbType.h"


/* ---------------------------------------------------------------------- */
/*     Private methods                                                    */
/* ---------------------------------------------------------------------- */

EXTERN SymbType_ptr 
SymbType_create_memory_sharing_type ARGS((SymbTypeTag tag, node_ptr body));
EXTERN SymbType_ptr 
SymbType_create_memory_sharing_array_type ARGS((SymbType_ptr subtype,
                                                int lower_bound,
                                                int higher_bound));

EXTERN void SymbType_destroy_memory_sharing_type ARGS((SymbType_ptr self));

#endif /* __SYMB_TYPE_PRIVATE_H__ */
