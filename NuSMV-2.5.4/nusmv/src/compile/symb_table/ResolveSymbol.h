/**CHeaderFile*****************************************************************

  FileName    [ResolveSymbol.h]

  PackageName [compile]

  Synopsis    [Public interface of class 'ResolveSymbol']

  Description [Basic Routines for resolving a name]

  SeeAlso     [ResolveSymbol.c]

  Author      [Marco Roveri Alessandro Mariotti]

  Copyright   [
  This file is part of the ``compile'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

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

#ifndef __RESOLVE_SYMBOL_H__
#define __RESOLVE_SYMBOL_H__


#include "utils/utils.h"
#include "node/node.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class ResolveSymbol]

  Description []

******************************************************************************/
typedef struct ResolveSymbol_TAG*  ResolveSymbol_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class ResolveSymbol]

  Description [These macros must be used respectively to cast and to check
  instances of class ResolveSymbol]

******************************************************************************/
#define RESOLVE_SYMBOL(self) \
         ((ResolveSymbol_ptr) self)

#define RESOLVE_SYMBOL_CHECK_INSTANCE(self) \
         (nusmv_assert(RESOLVE_SYMBOL(self) != RESOLVE_SYMBOL(NULL)))

/* Forward declaration of the SymbTable structure, in order to avoid
   circular dependency  (ST needs RS, RS needs ST) */
struct SymbTable_TAG;


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN ResolveSymbol_ptr ResolveSymbol_create ARGS((void));

EXTERN void ResolveSymbol_destroy ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_undefined ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_defined ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_ambiguous ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_var ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_define ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_function ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_constant ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_parameter ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_array ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_array_def ARGS((ResolveSymbol_ptr self));

EXTERN boolean ResolveSymbol_is_error ARGS((ResolveSymbol_ptr self));

EXTERN char* ResolveSymbol_get_error_message ARGS((ResolveSymbol_ptr self));

EXTERN void ResolveSymbol_print_error_message ARGS((ResolveSymbol_ptr self,
                                                    FILE* stream));

EXTERN void ResolveSymbol_throw_error ARGS((ResolveSymbol_ptr self));

EXTERN node_ptr ResolveSymbol_get_resolved_name ARGS((ResolveSymbol_ptr self));

EXTERN node_ptr ResolveSymbol_resolve ARGS((ResolveSymbol_ptr self,
                                            struct SymbTable_TAG* st,
                                            node_ptr name,
                                            node_ptr context));

/**AutomaticEnd***************************************************************/



#endif /* __RESOLVE_SYMBOL_H__ */
