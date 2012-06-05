/**CHeaderFile*****************************************************************

  FileName    [TypeChecker.h]

  PackageName [compile.type_checking]

  Synopsis    [Public interface of class 'TypeChecker']

  Description [ This class contains the functions performing type checking. 
  The class uses type_checking_violation_handler to deal with 
  type system violations (which may result in an error or warning)
  in the input files.
  After the type checking is performed, use function
  TypeChecker_get_expression_type to get the type of a particular
  expression.
  After the type checking is performed it is possible to obtain the
  type of an expression. Only memory-sharing types (SymbTablePkg_..._type)
  are used, so you can compare pointers instead of the type's contents.
  ]

  SeeAlso     [TypeChecker.c]

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``compile.type_checking'' package of NuSMV 
  version 2. 
  Copyright (C) 2005 by FBK-irst. 

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

  Revision    [$Id: TypeChecker.h,v 1.1.2.17.6.4 2009-11-06 19:03:34 nusmv Exp $]

******************************************************************************/


#ifndef __TYPE_CHECKER_H__
#define __TYPE_CHECKER_H__

#include "node/MasterNodeWalker.h"

#include "compile/symb_table/SymbType.h" 
#include "compile/symb_table/SymbLayer.h" 
#include "utils/utils.h" 



/*---------------------------------------------------------------------------*/
/* Type definitions                                                          */
/*---------------------------------------------------------------------------*/


/**Type***********************************************************************

  Synopsis    [Definition of the public type for class TypeChecker]

  Description []

******************************************************************************/
typedef struct TypeChecker_TAG*  TypeChecker_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class TypeChecker]

  Description [These macros must be used respectively to cast and to check
  instances of class TypeChecker]

******************************************************************************/
#define TYPE_CHECKER(self) \
         ((TypeChecker_ptr) self)

#define TYPE_CHECKER_CHECK_INSTANCE(self) \
         (nusmv_assert(TYPE_CHECKER(self) != TYPE_CHECKER(NULL)))



/* forward declaration, to avoid  circular dependency */
struct Prop_TAG;
struct SymbTable_TAG;


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
EXTERN TypeChecker_ptr 
TypeChecker_create ARGS((struct SymbTable_TAG* symbolTable));

EXTERN TypeChecker_ptr TypeChecker_create_with_default_checkers 
ARGS((struct SymbTable_TAG* symbolTable));

EXTERN void TypeChecker_destroy ARGS((TypeChecker_ptr self));

EXTERN struct SymbTable_TAG*
TypeChecker_get_symb_table ARGS((TypeChecker_ptr self));

EXTERN boolean 
TypeChecker_check_layer ARGS((TypeChecker_ptr self,
                              SymbLayer_ptr layer));

EXTERN boolean
TypeChecker_check_constrains ARGS((TypeChecker_ptr self,
                                   node_ptr init, node_ptr trans, 
                                   node_ptr invar, node_ptr assign, 
                                   node_ptr justice, node_ptr compassion));

EXTERN boolean 
TypeChecker_check_property ARGS((TypeChecker_ptr self, 
                                 struct Prop_TAG* property));


EXTERN boolean 
TypeChecker_is_expression_wellformed ARGS((TypeChecker_ptr self,
                                           node_ptr expression, 
                                           node_ptr context));

EXTERN boolean 
TypeChecker_is_specification_wellformed ARGS((TypeChecker_ptr self, 
                                              node_ptr expression));

EXTERN boolean 
TypeChecker_is_type_wellformed ARGS((TypeChecker_ptr self,
                                     SymbType_ptr type, node_ptr varName));

EXTERN SymbType_ptr 
TypeChecker_get_expression_type ARGS((TypeChecker_ptr self,
                                      node_ptr expression,
                                      node_ptr context));

EXTERN boolean
TypeChecker_is_expression_type_checked ARGS((TypeChecker_ptr self,
                                             node_ptr expression,
                                             node_ptr context));
  
/**AutomaticEnd***************************************************************/


#endif /* __TYPE_CHECKER_H__ */
