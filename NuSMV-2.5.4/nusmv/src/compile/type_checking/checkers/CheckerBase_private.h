/**CHeaderFile*****************************************************************

  FileName    [CheckerBase_private.h]

  PackageName [compile.type_checking.checkers]

  Synopsis    [Private and protected interface of class 'CheckerBase']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [CheckerBase.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.type_checking.checkers'' package 
  of NuSMV version 2. Copyright (C) 2006 by FBK-irst. 

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

  Revision    [$Id: CheckerBase_private.h,v 1.1.2.3.6.1 2007-04-04 12:00:33 nusmv Exp $]

******************************************************************************/


#ifndef __CHECKER_BASE_PRIVATE_H__
#define __CHECKER_BASE_PRIVATE_H__


#include "CheckerBase.h" 
#include "checkersInt.h"

#include "node/NodeWalker_private.h"
#include "compile/type_checking/TypeChecker.h"
#include "compile/type_checking/TypeChecker_private.h"

#include "utils/utils.h" 


/**Macro***********************************************************************

  Synopsis           [Short way of calling type_checking_check_expression]

  Description        [Use this macro to recursively recall 
  type_checking_check_expression. Must be used from within a method. This 
  can be used when expressions that are node_ptr]

  SeeAlso            []
  
******************************************************************************/
#define _THROW(exp, ctx)                                                  \
   (NodeWalker_can_handle(NODE_WALKER(self), exp) ?                       \
    CHECKER_BASE(self)->check_expr(self, exp, ctx) :                      \
   type_checker_check_expression(TYPE_CHECKER(NODE_WALKER(self)->master), \
                                 exp, ctx))

/**Struct**********************************************************************

  Synopsis    [CheckerBase class definition derived from
               class node.NodeWalker]

  Description []

  SeeAlso     [Base class Object]   
  
******************************************************************************/
typedef struct CheckerBase_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(NodeWalker); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  TypeCheckingViolationHandler_ptr viol_handler;

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
  SymbType_ptr (*check_expr)(CheckerBase_ptr self, node_ptr exp, node_ptr ctx);

} CheckerBase;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN CheckerBase_ptr CheckerBase_create ARGS((const char* name, 
                                                int low, size_t num));

EXTERN void 
checker_base_init ARGS((CheckerBase_ptr self, const char* name, 
                        int low, size_t num));

EXTERN void checker_base_deinit ARGS((CheckerBase_ptr self));


EXTERN VIRTUAL boolean 
checker_base_manage_violation ARGS((CheckerBase_ptr self, 
                                    TypeSystemViolation violation, 
                                    node_ptr expression));

EXTERN void 
checker_base_print_type ARGS((CheckerBase_ptr self, FILE* output_stream, 
                              node_ptr expression, node_ptr context));


#endif /* __CHECKER_BASE_PRIVATE_H__ */
