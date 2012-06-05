/**CFile***********************************************************************

  FileName    [PrinterIWffCore.c]

  PackageName [node.printers]

  Synopsis    [Implementation of class 'PrinterIWffCore']

  Description []

  SeeAlso     [PrinterWffCore.h]

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``node.printers'' package of NuSMV version 2. 
  Copyright (C) 2009 by FBK.

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


  All Rights Reserved. This software is for educational purposes only.
  Permission is given to academic institutions to use, copy, and modify
  this software and its documentation provided that this introductory
  message is not removed, that this software and its documentation is
  used for the institutions' internal research and educational purposes,
  and that no monies are exchanged. No guarantee is expressed or implied
  by the distribution of this code.
  Send bug-reports and/or questions to: nusmv-users@fbk.eu. ]

  Revision    [$Id: PrinterIWffCore.c,v 1.1.2.1 2009-03-23 18:13:22 nusmv Exp $]

******************************************************************************/

#include "PrinterIWffCore.h" 
#include "PrinterIWffCore_private.h" 
#include "MasterPrinter_private.h"
#include "parser/symbols.h"

#include "utils/WordNumber.h"
#include "utils/utils.h" 
#include "utils/ustring.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: PrinterIWffCore.c,v 1.1.2.1 2009-03-23 18:13:22 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PrinterIWffCore_private.h' for class 'PrinterIWffCore' definition. */ 

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/**Macro***********************************************************************

  Synopsis           [Short way of calling printer_base_throw_print_node]

  Description        [Use this macro to recursively recall print_node]

  SeeAlso            []
  
******************************************************************************/
#define _THROW(n, p)  printer_base_throw_print_node(PRINTER_BASE(self), n, p) 


/**Macro***********************************************************************

  Synopsis           [Short way of calling printer_base_print_string]

  Description [Use to print a string (that will be redirected to the
  currently used stream)]

  SeeAlso            []
  
******************************************************************************/
#define _PRINT(str)  printer_base_print_string(PRINTER_BASE(self), str)
#define _NEWLINE()   printer_base_print_string(PRINTER_BASE(self), "\n")

/**Macro***********************************************************************

  Synopsis           [Short way of calling master_printer_indent]

  Description        [Use to augment current level of indentation]

  SeeAlso            []
  
******************************************************************************/
#define _INDENT()							\
  master_printer_indent(						\
			MASTER_PRINTER(NODE_WALKER(self)->master))

/**Macro***********************************************************************

  Synopsis           [Short way of calling master_printer_deindentt]

  Description [Use to revert to previous level of indentation]

  SeeAlso            []
  
******************************************************************************/
#define _DEINDENT(x)                                                    \
  master_printer_deindent(                                              \
			MASTER_PRINTER(NODE_WALKER(self)->master))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void printer_iwff_core_finalize ARGS((Object_ptr object, void* dummy));

static int 
printer_iwff_core_print_case ARGS((PrinterIWffCore_ptr self, node_ptr n));

static int 
printer_iwff_core_print_case_body ARGS((PrinterIWffCore_ptr self, node_ptr n));

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The PrinterIWffCore class constructor]

  Description        [The PrinterIWffCore class constructor]

  SideEffects        []

  SeeAlso            [PrinterIWffCore_destroy]   
  
******************************************************************************/
PrinterIWffCore_ptr PrinterIWffCore_create(const char* name)
{
  PrinterIWffCore_ptr self = ALLOC(PrinterIWffCore, 1);
  PRINTER_IWFF_CORE_CHECK_INSTANCE(self);

  printer_iwff_core_init(self, name, 
                        NUSMV_CORE_SYMBOL_FIRST, 
                        NUSMV_CORE_SYMBOL_LAST - NUSMV_CORE_SYMBOL_FIRST);
  return self;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The PrinterIWffCore class private initializer]

  Description        [The PrinterIWffCore class private initializer]

  SideEffects        []

  SeeAlso            [PrinterIWffCore_create]   
  
******************************************************************************/
void printer_iwff_core_init(PrinterIWffCore_ptr self, 
                            const char* name, 
                            int low, size_t num)
{
  /* base class initialization */
  printer_wff_core_init(PRINTER_WFF_CORE(self), name, low, num);
  
  /* members initialization */

  /* virtual methods settings */  
  OVERRIDE(Object, finalize) = printer_iwff_core_finalize;
  OVERRIDE(PrinterBase, print_node) = printer_iwff_core_print_node;
}


/**Function********************************************************************

  Synopsis           [The PrinterIWffCore class private deinitializer]

  Description        [The PrinterIWffCore class private deinitializer]

  SideEffects        []

  SeeAlso            [PrinterIWffCore_destroy]   
  
******************************************************************************/
void printer_iwff_core_deinit(PrinterIWffCore_ptr self)
{
  /* members deinitialization */

  /* base class initialization */
  printer_wff_core_deinit(PRINTER_WFF_CORE(self));
}


/**Function********************************************************************

  Synopsis    [Virtual menthod that prints the given node 
  (only indentantion capable nodes are handled here)]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int printer_iwff_core_print_node(PrinterBase_ptr self, node_ptr n, int priority)
{
  switch (node_get_type(n)) {
  case CASE: 
  case IFTHENELSE:
    return printer_iwff_core_print_case(PRINTER_IWFF_CORE(self), n);
    
    /* for all any other case delegate responsibility to ancestor class */
  default:
    return printer_wff_core_print_node(PRINTER_BASE(self), n, priority);
  }
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The PrinterWffCore class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void printer_iwff_core_finalize(Object_ptr object, void* dummy) 
{
  PrinterIWffCore_ptr self = PRINTER_IWFF_CORE(object);

  printer_iwff_core_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int printer_iwff_core_print_case(PrinterIWffCore_ptr self, node_ptr n)
{
  return
    _PRINT("case")
    && _INDENT()
    && _NEWLINE()

    /* recursively inside the case */
    && printer_iwff_core_print_case_body(self, n)

    /* newline *after* the DEINDENT */
    && _DEINDENT()
    && _NEWLINE()

    && _PRINT("esac");
}

/**Function********************************************************************

  Synopsis           [required]

  Description        [optional]

  SideEffects        [required]

  SeeAlso            [optional]

******************************************************************************/
static int 
printer_iwff_core_print_case_body(PrinterIWffCore_ptr self, node_ptr n)
{
  int res;

  nusmv_assert(n != Nil);
  res = _THROW(car(car(n)), 0) && _PRINT(" : ") &&
    _THROW(cdr(car(n)), 0) && _PRINT(";\n");

  if (res == 0) return 0; /* previous error */

  nusmv_assert(cdr(n) != Nil); /* Now there is always a last(default) case */ 

  /* continue to print the tail of the case list */
  if (node_get_type(cdr(n)) == CASE || node_get_type(cdr(n)) == IFTHENELSE) {
    return printer_iwff_core_print_case_body(self, cdr(n));
  }
  /* print the last(default) element. Do not print artificial FAILURE node */
  else if (node_get_type(cdr(n)) != FAILURE) {
    return _PRINT("TRUE : ") && /* the last (default) element */
      _THROW(cdr(n), 0) && _PRINT(";");
  }

  return res; /* the last element is FAILURE node */
}

/**AutomaticEnd***************************************************************/

