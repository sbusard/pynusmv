/**CHeaderFile*****************************************************************

  FileName    [TypeChecker_provate.h]

  PackageName [compile.type_checking]

  Synopsis    [Private interface of class 'TypeChecker']

  Description [ This interface is part of the private relationship 
  among the TypeChecker and the checkes it contains and controls. 
  ]

  SeeAlso     [TypeChecker.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.type_checking'' package of NuSMV 
  version 2. Copyright (C) 2006 by FBK-irst. 

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

  Revision    [$Id: TypeChecker_private.h,v 1.1.2.3.6.1 2007-04-04 12:00:33 nusmv Exp $]

******************************************************************************/


#ifndef __TYPE_CHECKER_PRIVATE_H__
#define __TYPE_CHECKER_PRIVATE_H__


#include "compile/symb_table/SymbType.h" 
#include "utils/utils.h" 

EXTERN SymbType_ptr 
tc_set_expression_type ARGS((TypeChecker_ptr self,
			     node_ptr expression, SymbType_ptr type));

EXTERN SymbType_ptr
tc_lookup_expr_type ARGS((TypeChecker_ptr self, node_ptr expression));


EXTERN SymbType_ptr 
type_checker_check_expression ARGS((TypeChecker_ptr self,
				    node_ptr expression, node_ptr context));


EXTERN void 
type_checker_print_error_message ARGS((TypeChecker_ptr self, 
				       node_ptr expr, boolean is_error));

EXTERN void type_checker_enable_memoizing ARGS((TypeChecker_ptr self, 
						boolean enabled));

EXTERN boolean 
type_checker_is_memoizing_enabled ARGS((const TypeChecker_ptr self));

#endif /* __TYPE_CHECKER_PRIVATE_H__ */
