/**CHeaderFile*****************************************************************

  FileName    [ucmd.h]

  PackageName [utils]

  Synopsis    [Header part of ucmd.c]

  Description [This file contains useful structures, macros and functions to 
  be used in command line processing]

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and FBK-irst. 

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

  Revision    [$Id: ucmd.h,v 1.8.4.1.4.1 2005-03-03 12:32:24 nusmv Exp $]

******************************************************************************/

#ifndef _U_CMD_H
#define _U_CMD_H

#include <stdlib.h> /* for strtol */
#include "utils/utils.h"
#include "opt/opt.h" /* for OPT_USER_POV_NULL_STRING */


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [SubstValue a service structure used by SubstString. Ignore.]

  Description []

  SeeAlso     [SubstString]

******************************************************************************/
typedef struct SubstValue_TAG
{
  enum { sv_string, sv_integer, sv_floating, sv_pointer, sv_undef } type;
  union 
  { 
    char*  string;
    int    integer;
    double floating;
    void*  pointer;
  } assign;
} SubstValue;


/**Struct**********************************************************************

  Synopsis    [SubstString is the structure passed to the function 
  apply_string_macro_expansion]

  Description [For your comfort you can use the SYMBOL_CREATE and 
  SYMBOL_ASSIGN macros in order to easily fill all fields of this data 
  structure]

  SeeAlso     [SYMBOL_CREATE, SYMBOL_ASSIGN, apply_string_macro_expansion]

******************************************************************************/
typedef struct SubstString_TAG
{
  char* symbol; 
  SubstValue value; 
  char* format;
} SubstString;
  

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**Macro***********************************************************************

  Synopsis     [SYMBOL_CREATE helps the static allocation of a new symbol
  usable with the function apply_string_macro_expansion]

  Description  [The macro parameter is the symbol string which will be 
  searched for substitution in the function apply_string_macro_expansion ]

  SideEffects  []

  SeeAlso      [SYMBOL_ASSIGN, apply_string_macro_expansion]

******************************************************************************/
#define SYMBOL_CREATE() \
  { "\0", {sv_undef, {NULL}}, "" } 



/**Macro***********************************************************************

  Synopsis     [SYMBOL_ASSIGN helps to fill the fields of the SubstString 
  structure, previously allocated with the SYMBOL_CREATE macro ]

  Description  [
  The first parameter is the variable assigned by SYMBOL_CREATE; <BR>
  The third parameter is the type of the value that will substitute
  the symbol. Can be: string, integer, floating or pointer. <BR>
  The fourth parameter is the format string (as the printf format string) 
  used to convert the value in a string. 
  The fifth parameter is the static value which will substitute all 
  occurences of the symbol.]

  SideEffects  [The structure passed as first parameter will change]

  SeeAlso      [SYMBOL_CREATE, apply_string_macro_expansion]

******************************************************************************/
#define SYMBOL_ASSIGN(_subst, _symbol, _type, _format, _value) \
_subst.symbol = _symbol; \
_subst.value.type = sv_##_type; \
_subst.format = _format;  \
_subst.value.assign._type = _value



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN void 
apply_string_macro_expansion ARGS((const SubstString* const subst, 
				   char* string, size_t buf_len));


EXTERN int util_str2int ARGS((const char* str, int* value));
EXTERN int util_str2int_inc ARGS((const char* str, char** endptr, int* out));
EXTERN int util_is_string_null ARGS((const char* string));
EXTERN int util_str2int_incr ARGS((const char* str, char **endptr, int* out));
/**AutomaticEnd***************************************************************/

#endif /* _U_CMD_H */







