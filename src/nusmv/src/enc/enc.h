/**CHeaderFile*****************************************************************

  FileName    [enc.h]

  PackageName [enc]

  Synopsis    [Public API for the enc package. Basically methods for
  accessing global encodings are provided here]

  Description []

  SeeAlso     [enc.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst. 

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

#ifndef __ENC_H__
#define __ENC_H__

#include "utils/utils.h"

#include "enc/bool/BoolEnc.h"
#include "enc/bdd/BddEnc.h"
#include "enc/be/BeEnc.h"


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure definitions                                                     */
/*---------------------------------------------------------------------------*/
/* possible Variable Ordering Types */
typedef enum  {
  VARS_ORD_INPUTS_BEFORE,
  VARS_ORD_INPUTS_AFTER, 
  VARS_ORD_TOPOLOGICAL, 
  VARS_ORD_INPUTS_BEFORE_BI, /* default */
  VARS_ORD_INPUTS_AFTER_BI, 
  VARS_ORD_TOPOLOGICAL_BI, 
  VARS_ORD_UNKNOWN

} VarsOrdType;

/* possible BDD Static Ordering Heuristics */
typedef enum {
  BDD_STATIC_ORDER_HEURISTICS_NONE,
  BDD_STATIC_ORDER_HEURISTICS_BASIC,
  BDD_STATIC_ORDER_HEURISTICS_ERROR, /* means an error has happened*/
} BddSohEnum;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Functions declarations                                                    */
/*---------------------------------------------------------------------------*/

EXTERN void Enc_init_encodings ARGS((void));

EXTERN void Enc_init_bool_encoding ARGS((void));
EXTERN void Enc_init_bdd_encoding ARGS((void));
EXTERN void Enc_init_be_encoding ARGS((void));

EXTERN void Enc_quit_encodings ARGS((void));

EXTERN void Enc_add_commands ARGS((void));

EXTERN BoolEnc_ptr Enc_get_bool_encoding ARGS((void));
EXTERN BddEnc_ptr Enc_get_bdd_encoding ARGS((void));
EXTERN BeEnc_ptr Enc_get_be_encoding ARGS((void));

EXTERN void Enc_set_bool_encoding ARGS((BoolEnc_ptr benc));
EXTERN void Enc_set_bdd_encoding ARGS((BddEnc_ptr enc));
EXTERN void Enc_set_be_encoding ARGS((BeEnc_ptr enc));

EXTERN const char* Enc_vars_ord_to_string ARGS((VarsOrdType));
EXTERN VarsOrdType Enc_string_to_vars_ord ARGS((const char*));
EXTERN const char* Enc_get_valid_vars_ord_types ARGS((void));

EXTERN const char* Enc_bdd_static_order_heuristics_to_string ARGS((BddSohEnum));
EXTERN BddSohEnum Enc_string_to_bdd_static_order_heuristics ARGS((const char*));
EXTERN const char* Enc_get_valid_bdd_static_order_heuristics ARGS((void));

#endif /* __ENC_H__ */
