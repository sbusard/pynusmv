/**CHeaderFile*****************************************************************

  FileName    [encInt.h]

  PackageName [enc]

  Synopsis    [Internal API for the enc package]

  Description []

  SeeAlso     [enc.h]

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

#ifndef __ENC_INT_H__
#define __ENC_INT_H__

#include "enc/utils/OrdGroups.h"
#include "enc/bool/BoolEnc.h"

#include "utils/utils.h"
#include "node/node.h"
#include "opt/opt.h"
#include "dd/dd.h"

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure definitions                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN int yylineno;
EXTERN node_ptr boolean_range; 

EXTERN DdManager * dd_manager;

EXTERN FILE* nusmv_stdout;
EXTERN FILE* nusmv_stderr;


/*---------------------------------------------------------------------------*/
/* Functions declarations                                                    */
/*---------------------------------------------------------------------------*/
EXTERN OrdGroups_ptr 
enc_utils_parse_ordering_file ARGS((const char* order_filename, 
                                    const BoolEnc_ptr bool_enc));

EXTERN void enc_add_commands ARGS((void));

EXTERN void Enc_append_bit_to_sorted_list ARGS((SymbTable_ptr symb_table, 
                                                NodeList_ptr sorted_list,
                                                node_ptr var,
                                                node_ptr* sorting_cache));
EXTERN OrdGroups_ptr 
enc_utils_create_vars_ord_groups ARGS((BoolEnc_ptr bool_enc,
                                       NodeList_ptr vars));

#endif /* __ENC_INT_H__ */
