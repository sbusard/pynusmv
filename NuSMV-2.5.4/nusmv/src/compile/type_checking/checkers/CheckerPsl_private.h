/**CHeaderFile*****************************************************************

  FileName    [CheckerPsl_private.h]

  PackageName [compile.type_checking.checkers]

  Synopsis    [Private and protected interface of class 'CheckerPsl']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [CheckerPsl.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.type_checking.checkers'' package of NuSMV version 2. 
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

  Revision    [$Id: CheckerPsl_private.h,v 1.1.2.1.6.1 2007-04-04 12:00:33 nusmv Exp $]

******************************************************************************/


#ifndef __CHECKER_PSL_PRIVATE_H__
#define __CHECKER_PSL_PRIVATE_H__


#include "CheckerPsl.h" 

#include "CheckerBase.h" 
#include "CheckerBase_private.h" 

#include "utils/utils.h" 



/**Struct**********************************************************************

  Synopsis    [CheckerPsl class definition derived from
               class CheckerBase]

  Description []

  SeeAlso     [Base class CheckerBase]   
  
******************************************************************************/
typedef struct CheckerPsl_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(CheckerBase); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} CheckerPsl;



/**Macro***********************************************************************

  Synopsis           [Short way of calling type_checking_check_expression]

  Description        [Use this macro to recursively recall 
  type_checking_check_expression. Must be used from within a method. This 
  can be used when expressions that are PslNode_ptr]

  SeeAlso            []
  
******************************************************************************/
#undef _THROW
#define _THROW(exp, ctx)                                                     \
   (NodeWalker_can_handle(NODE_WALKER(self), exp) ?                          \
    CHECKER_BASE(self)->check_expr(self, exp, ctx) :                         \
   type_checker_check_expression(TYPE_CHECKER(NODE_WALKER(self)->master),    \
                                 PslNode_convert_to_node_ptr(exp),           \
                                 PslNode_convert_to_node_ptr(ctx)))


/**Macro***********************************************************************

  Synopsis           [Short way of calling tc_set_expression_type]

  Description        [Use this macro to set an expression type within the 
  master type checker. Must be used from within a method. This 
  can be used when expressions that are PslNode_ptr]

  SeeAlso            []
  
******************************************************************************/
#define _SET_TYPE(expr, type)                                        \
   tc_set_expression_type(TYPE_CHECKER(NODE_WALKER(self)->master),   \
			  PslNode_convert_to_node_ptr(expr), type)


/**Macro***********************************************************************

  Synopsis           [Short way of calling tc_lookup_expr_type]

  Description        [Use this macro to get an expression type from the 
  master type checker. Must be used from within a method. This 
  can be used when expressions that are PslNode_ptr]

  SeeAlso            []
  
******************************************************************************/
#define _GET_TYPE(expr)                                           \
   tc_lookup_expr_type(TYPE_CHECKER(NODE_WALKER(self)->master),   \
		       PslNode_convert_to_node_ptr(expr))


/**Macro***********************************************************************

  Synopsis           [Short way of calling checker_base_manage_violation]

  Description        [Use this macro to manage a violation at the master 
  type checker level. Must be used from within a method. This 
  can be used when expressions that are PslNode_ptr]

  SeeAlso            []
  
******************************************************************************/
#define _VIOLATION(viol_id, expr)                                   \
   checker_base_manage_violation(CHECKER_BASE(self), viol_id,       \
				 PslNode_convert_to_node_ptr(expr))



/**Macro***********************************************************************

  Synopsis           [Short way of calling type_checker_print_error_message]

  Description        [Use this macro to recursively call
  type_checking_check_expression into violation handlers. This 
  can be used when expressions that are PslNode_ptr]

  SeeAlso            []
  
******************************************************************************/
#define _PRINT_ERROR_MSG(exp, is_error)                                      \
   type_checker_print_error_message(TYPE_CHECKER(NODE_WALKER(self)->master), \
                                 PslNode_convert_to_node_ptr(exp),           \
				 is_error)



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void checker_psl_init ARGS((CheckerPsl_ptr self));
EXTERN void checker_psl_deinit ARGS((CheckerPsl_ptr self));



#endif /* __CHECKER_PSL_PRIVATE_H__ */
