/**CFile***********************************************************************

  FileName    [CheckerBase.c]

  PackageName [compile.type_checking.checkers]

  Synopsis    [Implementaion of class 'CheckerBase']

  Description []

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

  Revision    [$Id: CheckerBase.c,v 1.1.2.4.4.3 2008-05-26 11:46:02 nusmv Exp $]

******************************************************************************/

#include "CheckerBase.h" 
#include "CheckerBase_private.h" 
#include "compile/type_checking/TypeChecker_private.h"

#include "utils/utils.h" 
#include "utils/error.h" 

static char rcsid[] UTIL_UNUSED = "$Id: CheckerBase.c,v 1.1.2.4.4.3 2008-05-26 11:46:02 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'CheckerBase_private.h' for class 'CheckerBase' definition. */ 

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void checker_base_finalize ARGS((Object_ptr object, void* dummy));

static SymbType_ptr
checker_base_check_expr ARGS((CheckerBase_ptr self,
			      node_ptr expression, node_ptr context));

static boolean 
checker_base_viol_handler ARGS((CheckerBase_ptr checker, 
				TypeSystemViolation violation, 
				node_ptr expression));



/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Creates and initializes a checker.
  To be usable, the checker will have to be registered to a TypeChecker.]

  Description        [To each checker is associated a partition of
  consecutive indices over the symbols set. The lowest index of the
  partition is given through the parameter low, while num is the
  partition size. Name is used to easily identify checker instances. 

  This method is private, as this class is virtual]

  SideEffects        []

  SeeAlso            [NodeWalker_destroy]   
  
******************************************************************************/
CheckerBase_ptr 
CheckerBase_create(const char* name, int low, size_t num)
{
  CheckerBase_ptr self = ALLOC(CheckerBase, 1);
  CHECKER_BASE_CHECK_INSTANCE(self);

  checker_base_init(self, name, low, num);
  return self;
}


/**Function********************************************************************

  Synopsis           [Checks the given node]

  Description [This is virtual method. Before calling, please ensure
  the given node can be handled by self, by calling
  CheckerBase_can_handle. 

  Note: This method will be never called by the user]

  SideEffects        []

  SeeAlso            [CheckerBase_can_handle]   
  
******************************************************************************/
VIRTUAL SymbType_ptr
CheckerBase_check_expr(CheckerBase_ptr self, node_ptr exp, node_ptr ctx)
{
  CHECKER_BASE_CHECK_INSTANCE(self);
  
  return self->check_expr(self, exp, ctx);
}



/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The CheckerBase class private initializer]

  Description        [The CheckerBase class private initializer]

  SideEffects        []

  SeeAlso            [CheckerBase_create]   
  
******************************************************************************/
void checker_base_init(CheckerBase_ptr self, const char* name, 
		       int low, size_t num)
{
  /* base class initialization */
  node_walker_init(NODE_WALKER(self), name, low, num, 
		   false /* not handle NULL case */); 
  
  /* virtual methods settings */  
  OVERRIDE(Object, finalize) = checker_base_finalize;
  OVERRIDE(CheckerBase, check_expr) = checker_base_check_expr;
  OVERRIDE(CheckerBase, viol_handler) = checker_base_viol_handler;
}


/**Function********************************************************************

  Synopsis           [The CheckerBase class private deinitializer]

  Description        [The CheckerBase class private deinitializer]

  SideEffects        []

  SeeAlso            [CheckerBase_destroy]   
  
******************************************************************************/
void checker_base_deinit(CheckerBase_ptr self)
{
  /* members deinitialization */  

  /* base class initialization */
  node_walker_deinit(NODE_WALKER(self));
}



/**Function********************************************************************

  Synopsis    [Virtual protected method for violation handling]

  Description [This virtual method is called by checkers to handle 
  their violations. Tipically this method is not called directly, but 
  throw the macro _VIOLATION that casts its arguments and improves 
  readability (at least as main tentative idea)]

  SideEffects []

  SeeAlso     []

******************************************************************************/
VIRTUAL boolean checker_base_manage_violation(CheckerBase_ptr self, 
                                              TypeSystemViolation violation, 
                                              node_ptr expression)
{
  return self->viol_handler(self, violation, expression);
}




/**Static function*************************************************************

  Synopsis           [Just prints out the type of an expression to
  output_stream]

  Description        [
  The expr must be a correct expression which has been already type checked 
  by 'checker'.
  The last parameter 'context' is the context where the expression
  has been found.
  The function is used in printing of an error messages only.]

  SideEffects        []

  SeeAlso            []
******************************************************************************/
void checker_base_print_type(CheckerBase_ptr self, FILE* output_stream, 
			     node_ptr expression, node_ptr context)
{
  SymbType_ptr type = 
    TypeChecker_get_expression_type(TYPE_CHECKER(NODE_WALKER(self)->master), 
				    expression, context);

  /* this expression must have been checked by this type checker. and
     it must not be 0 since this is an sub-expression of a problematic
     expression */
  if (type == SYMB_TYPE(NULL)) {
    internal_error("check_base_print_type: the type checker was not able \n" \
		   "to retrieve the given expression type");
  }

  SymbType_print(type, output_stream);
  return;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The CheckerBase class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void checker_base_finalize(Object_ptr object, void* dummy) 
{
  CheckerBase_ptr self = CHECKER_BASE(object);

  checker_base_deinit(self);
  FREE(self);
}



/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static SymbType_ptr
checker_base_check_expr(CheckerBase_ptr self,
			node_ptr expression, node_ptr context)
{
  internal_error("CheckerBase: Pure virtual method check_expression " \
		 "not implemented\n");
  return 0;
}



/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static boolean 
checker_base_viol_handler(CheckerBase_ptr checker, 
			  TypeSystemViolation violation, node_ptr expression)
{
  internal_error("CheckerBase: Pure virtual method viol_handler " \
		 "not implemented\n");  
  return 0;
}


/**AutomaticEnd***************************************************************/

