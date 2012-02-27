/**CHeaderFile*****************************************************************

  FileName    [BoolEnc.h]

  PackageName [enc.bool]

  Synopsis    [Public interface of class 'BoolEnc']

  Description []

  SeeAlso     [BoolEnc.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bool'' package of NuSMV version 2.
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

  Revision    [$Id: BoolEnc.h,v 1.1.2.7.6.7 2009-06-15 12:47:48 nusmv Exp $]

******************************************************************************/


#ifndef __BOOL_ENC_H__
#define __BOOL_ENC_H__

#include "BitValues.h"
#include "enc/base/BaseEnc.h"

#include "compile/symb_table/SymbTable.h"
#include "utils/NodeList.h"
#include "node/node.h"
#include "set/set.h"

#include "utils/utils.h"
#include "utils/object.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class BoolEnc]

  Description []

******************************************************************************/
typedef struct BoolEnc_TAG*  BoolEnc_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class BoolEnc]

  Description [These macros must be used respectively to cast and to check
  instances of class BoolEnc]

******************************************************************************/
#define BOOL_ENC(self) \
         ((BoolEnc_ptr) self)

#define BOOL_ENC_CHECK_INSTANCE(self) \
         (nusmv_assert(BOOL_ENC(self) != BOOL_ENC(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN BoolEnc_ptr BoolEnc_create ARGS((SymbTable_ptr symb_table));

EXTERN VIRTUAL void BoolEnc_destroy ARGS((BoolEnc_ptr self));

EXTERN boolean
BoolEnc_is_var_bit ARGS((const BoolEnc_ptr self, node_ptr name));

EXTERN boolean
BoolEnc_is_var_scalar ARGS((const BoolEnc_ptr self, node_ptr name));

EXTERN node_ptr
BoolEnc_get_scalar_var_from_bit ARGS((const BoolEnc_ptr self, node_ptr name));

EXTERN node_ptr
BoolEnc_make_var_bit ARGS((const BoolEnc_ptr self, node_ptr name, int index));

EXTERN int
BoolEnc_get_index_from_bit ARGS((const BoolEnc_ptr self, node_ptr name));

EXTERN NodeList_ptr
BoolEnc_get_var_bits ARGS((const BoolEnc_ptr self, node_ptr name));

EXTERN node_ptr
BoolEnc_get_var_encoding ARGS((const BoolEnc_ptr self, node_ptr name));

EXTERN node_ptr
BoolEnc_get_values_bool_encoding ARGS((const BoolEnc_ptr self,
                                       node_ptr values,
                                       Set_t* bits));
EXTERN const char*
BoolEnc_scalar_layer_to_bool_layer ARGS((const char* layer_name));

EXTERN boolean
BoolEnc_is_bool_layer ARGS((const char* layer_name));

EXTERN node_ptr
BoolEnc_get_value_from_var_bits ARGS((const BoolEnc_ptr self,
                                      const BitValues_ptr bit_values));

EXTERN node_ptr
BoolEnc_get_var_mask ARGS((const BoolEnc_ptr self, node_ptr name));
/**AutomaticEnd***************************************************************/



#endif /* __BOOL_ENC_H__ */
