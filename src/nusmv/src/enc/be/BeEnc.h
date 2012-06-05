
/**CHeaderFile*****************************************************************

  FileName    [BeEnc.h]

  PackageName [enc.be]

  Synopsis    [Public interface of class 'BeEnc']

  Description []

  SeeAlso     [BeEnc.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``enc.be'' package of NuSMV version 2. 
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

  Revision    [$Id: BeEnc.h,v 1.1.2.6.6.2 2007-06-18 16:50:56 nusmv Exp $]

******************************************************************************/


#ifndef __BE_ENC_H__
#define __BE_ENC_H__


#include "enc/base/BoolEncClient.h" 
#include "enc/bool/BoolEnc.h"
#include "compile/symb_table/SymbTable.h"
#include "be/be.h"
#include "node/node.h"

#include "utils/object.h"
#include "utils/utils.h" 


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class BeEnc]

  Description []

******************************************************************************/
typedef struct BeEnc_TAG*  BeEnc_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class BeEnc]

  Description [These macros must be used respectively to cast and to check
  instances of class BeEnc]

******************************************************************************/
#define BE_ENC(self) \
         ((BeEnc_ptr) self)

#define BE_ENC_CHECK_INSTANCE(self) \
         (nusmv_assert(BE_ENC(self) != BE_ENC(NULL)))


/**Macros**********************************************************************

  Synopsis    [A constant representing the time a current
  untimed variable belong to.]

  Description [The value of the constant is guaranteed to be out of the range
  of legal time numbers, i.e. out of \[0, BeEnc_get_max_time()\].
  
  This constant, for example, is returned by BeEnc_index_to_time
  given a frozen variable (which can be only untimed).]

******************************************************************************/
#define BE_CURRENT_UNTIMED -1


/**Enum************************************************************************

  Synopsis    [The category which a be variable belongs to.]

  Description [Used to classify a be variable within 3 main categories: 
  - current state variables
  - frozen variables
  - input variables
  - next state variables 

  These values can be combined when for example the iteration is
  performed.  In this way it is possible to iterate through the set of
  current and next state vars, skipping the inputs.
  ]

  SeeAlso     []

******************************************************************************/
typedef enum BeVarType_TAG {
  BE_VAR_TYPE_CURR   = 0x1 << 0, 
  BE_VAR_TYPE_FROZEN = 0x1 << 1, 
  BE_VAR_TYPE_INPUT  = 0x1 << 2, 
  BE_VAR_TYPE_NEXT   = 0x1 << 3,

  /* a shorthand for all legal types */
  BE_VAR_TYPE_ALL    = BE_VAR_TYPE_CURR | BE_VAR_TYPE_FROZEN | 
                       BE_VAR_TYPE_INPUT | BE_VAR_TYPE_NEXT,
  /* this value is used internally to represent erroneous situations */
  BE_VAR_TYPE_ERROR  = 0x1 << 4,
} BeVarType;

/**AutomaticStart*************************************************************/


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN BeEnc_ptr BeEnc_create ARGS((SymbTable_ptr symb_table, 
                                    BoolEnc_ptr bool_enc));

EXTERN VIRTUAL 
void BeEnc_destroy ARGS((BeEnc_ptr self));

/* Getters: */

EXTERN Be_Manager_ptr BeEnc_get_be_manager ARGS((const BeEnc_ptr self));

EXTERN int BeEnc_get_state_vars_num ARGS((const BeEnc_ptr self)); 
EXTERN int BeEnc_get_frozen_vars_num ARGS((const BeEnc_ptr self)); 
EXTERN int BeEnc_get_input_vars_num ARGS((const BeEnc_ptr self)); 
EXTERN int BeEnc_get_vars_num ARGS((const BeEnc_ptr self)); 
EXTERN int BeEnc_get_max_time ARGS((const BeEnc_ptr self));


/* Conversion of name to something */
EXTERN be_ptr 
BeEnc_name_to_untimed ARGS((const BeEnc_ptr self, const node_ptr var_name));

EXTERN int 
BeEnc_name_to_index ARGS((const BeEnc_ptr self, const node_ptr name));

EXTERN be_ptr 
BeEnc_name_to_timed ARGS((const BeEnc_ptr self, 
                          const node_ptr name, const int time));


/* Conversion of index to something */
EXTERN node_ptr 
BeEnc_index_to_name ARGS((const BeEnc_ptr self, const int index));


EXTERN be_ptr 
BeEnc_index_to_var ARGS((const BeEnc_ptr self, const int index));

EXTERN be_ptr 
BeEnc_index_to_timed ARGS((const BeEnc_ptr self, const int index, 
                           const int time));

EXTERN int BeEnc_index_to_time ARGS((const BeEnc_ptr self, const int index));

EXTERN int 
BeEnc_index_to_untimed_index ARGS((const BeEnc_ptr self, const int index));

/* Conversion of be variable to something */
EXTERN node_ptr 
BeEnc_var_to_name ARGS((const BeEnc_ptr self, be_ptr be_var));

EXTERN int 
BeEnc_var_to_index ARGS((const BeEnc_ptr self, const be_ptr var));

EXTERN be_ptr 
BeEnc_var_to_timed ARGS((const BeEnc_ptr self, const be_ptr var, 
                         const int time));

EXTERN be_ptr 
BeEnc_var_to_untimed ARGS((const BeEnc_ptr self, const be_ptr var));

EXTERN be_ptr 
BeEnc_var_curr_to_next ARGS((const BeEnc_ptr self, const be_ptr curr)); 

EXTERN be_ptr 
BeEnc_var_next_to_curr ARGS((const BeEnc_ptr self, const be_ptr next)); 


/* Shifting of expressions */
EXTERN be_ptr 
BeEnc_shift_curr_to_next ARGS((BeEnc_ptr self, const be_ptr exp));

EXTERN be_ptr 
BeEnc_untimed_expr_to_timed ARGS((BeEnc_ptr self, const be_ptr exp, 
                                  const int time));

EXTERN be_ptr 
BeEnc_untimed_expr_to_times ARGS((BeEnc_ptr self, const be_ptr exp, 
                                  const int ctime, 
                                  const int ftime, 
                                  const int itime, 
                                  const int ntime));

EXTERN be_ptr 
BeEnc_untimed_to_timed_and_interval ARGS((BeEnc_ptr self, 
                                          const be_ptr exp, 
                                          const int from, const int to));

EXTERN be_ptr 
BeEnc_untimed_to_timed_or_interval ARGS((BeEnc_ptr self, 
                                         const be_ptr exp, 
                                         const int from, const int to));


/* Tests on type of be variables' indices */
EXTERN boolean 
BeEnc_is_index_state_var ARGS((const BeEnc_ptr self, const int index));

EXTERN boolean 
BeEnc_is_index_frozen_var ARGS((const BeEnc_ptr self, const int index));

EXTERN boolean 
BeEnc_is_index_input_var ARGS((const BeEnc_ptr self, const int index));

EXTERN boolean 
BeEnc_is_index_untimed ARGS((const BeEnc_ptr self, const int index));

EXTERN boolean 
BeEnc_is_index_untimed_curr ARGS((const BeEnc_ptr self, const int index));

EXTERN boolean 
BeEnc_is_index_untimed_frozen ARGS((const BeEnc_ptr self, const int index));

EXTERN boolean 
BeEnc_is_index_untimed_input ARGS((const BeEnc_ptr self, const int index));

EXTERN boolean 
BeEnc_is_index_untimed_curr_frozen_input ARGS((const BeEnc_ptr self, 
                                               const int index));

EXTERN boolean 
BeEnc_is_index_untimed_next ARGS((const BeEnc_ptr self, const int index));


/* Iteration over set of untimed variables */
EXTERN int 
BeEnc_get_first_untimed_var_index ARGS((const BeEnc_ptr self, BeVarType type));

EXTERN int 
BeEnc_get_next_var_index ARGS((const BeEnc_ptr self, 
                               int var_index, BeVarType type));

EXTERN int 
BeEnc_get_var_index_with_offset ARGS((const BeEnc_ptr self, 
                                      int from_index, int offset, 
                                      BeVarType type));

EXTERN boolean 
BeEnc_is_var_index_valid ARGS((const BeEnc_ptr self, int var_index));




/**AutomaticEnd***************************************************************/



#endif /* __BE_ENC_H__ */
