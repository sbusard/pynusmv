/**CHeaderFile*****************************************************************

  FileName    [BitValues.h]

  PackageName [enc.bool]

  Synopsis    [Public interface of class 'BitValues']

  Description [BitValues is a structured array of values of
  bits.  bits are boolean variable that are used to encode a scalar
  variable.  A BitValues is used when extracting scalar value
  of a variable from the assigments to its bits. A bit value can be
  BIT_VAL_TRUE, BIT_VAL_FALSE or BIT_VAL_DONTCARE]

  SeeAlso     [BitValues.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.bool'' package of NuSMV version 2. 
  Copyright (C) 2008 by FBK-irst. 

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

  Revision    [$Id: BitValues.h,v 1.1.2.1 2008-10-03 13:02:50 nusmv Exp $]

******************************************************************************/

#ifndef __BIT_VALUES_H__
#define __BIT_VALUES_H__

#include "node/node.h"
#include "utils/utils.h" 
#include "utils/NodeList.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class BitValues]

  Description []

******************************************************************************/
typedef struct BitValues_TAG*  BitValues_ptr;


/**Type***********************************************************************

  Synopsis    [BitValue is the set of possible values a bit can take]

  Description []

******************************************************************************/
typedef enum BitValue_TAG { 
  BIT_VALUE_FALSE,
  BIT_VALUE_TRUE,
  BIT_VALUE_DONTCARE,
} BitValue;
  


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class BitValues]

  Description [These macros must be used respectively to cast and to check
  instances of class BitValues]

******************************************************************************/
#define BIT_VALUES(self) \
         ((BitValues_ptr) self)

#define BIT_VALUES_CHECK_INSTANCE(self) \
         (nusmv_assert(BIT_VALUES(self) != BIT_VALUES(NULL)))



/**AutomaticStart*************************************************************/

struct BoolEnc_TAG; /* a forward declaration */

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN BitValues_ptr 
BitValues_create ARGS((struct BoolEnc_TAG* enc, node_ptr var));

EXTERN void BitValues_destroy ARGS((BitValues_ptr self));

EXTERN node_ptr BitValues_get_scalar_var ARGS((const BitValues_ptr self));
EXTERN size_t BitValues_get_size ARGS((const BitValues_ptr self));
EXTERN NodeList_ptr BitValues_get_bits ARGS((const BitValues_ptr self));

EXTERN void BitValues_reset ARGS((BitValues_ptr self));

EXTERN BitValue BitValues_get ARGS((const BitValues_ptr self, size_t index));

EXTERN BitValue 
BitValues_get_value_from_expr ARGS((const BitValues_ptr self, node_ptr expr));

EXTERN void BitValues_set ARGS((BitValues_ptr self, 
                                size_t index, BitValue val));

EXTERN void BitValues_set_from_expr ARGS((BitValues_ptr self, 
                                          size_t index, node_ptr expr));

EXTERN void BitValues_set_from_values_list ARGS((BitValues_ptr self, 
                                                 struct BoolEnc_TAG* enc,
                                                 node_ptr vals));

/**AutomaticEnd***************************************************************/



#endif /* __BIT_VALUES_H__ */
