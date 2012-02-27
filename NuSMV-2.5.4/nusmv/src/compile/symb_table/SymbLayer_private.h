/**CHeaderFile*****************************************************************

  FileName    [SymbLayer_private.h]

  PackageName [compile.symb_table]

  Synopsis    [Private interface accessed by class SymbTable]

  Description []

  SeeAlso     [SymbolLayer.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.symb_table'' package of NuSMV
  version 2.  Copyright (C) 2004 by FBK-irst.

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

#ifndef __SYMB_LAYER_PRIVATE_H__
#define __SYMB_LAYER_PRIVATE_H__

#include "SymbLayer.h"
#include "SymbCache.h"


#include "utils/utils.h"


/* ---------------------------------------------------------------------- */
/*     Private methods                                                    */
/* ---------------------------------------------------------------------- */

EXTERN SymbLayer_ptr
SymbLayer_create ARGS((const char* name, const LayerInsertPolicy policy,
                       SymbCache_ptr cache));

EXTERN void SymbLayer_destroy ARGS((SymbLayer_ptr self));

EXTERN void SymbLayer_destroy_raw ARGS((SymbLayer_ptr self));

EXTERN void
SymbLayer_set_name ARGS((SymbLayer_ptr self, const char* new_name));

EXTERN LayerInsertPolicy
SymbLayer_get_insert_policy ARGS((const SymbLayer_ptr self));

EXTERN int
SymbLayer_compare_insert_policy ARGS((const SymbLayer_ptr self,
                                      const SymbLayer_ptr other));

EXTERN void
SymbLayer_committed_to_enc ARGS((SymbLayer_ptr self));

EXTERN void
SymbLayer_removed_from_enc ARGS((SymbLayer_ptr self));


#endif /* __SYMB_LAYER_PRIVATE_H__ */
