
/**CFile***********************************************************************

  FileName    [PrinterBase.c]

  PackageName [node.printers]

  Synopsis    [Implementaion of class 'PrinterBase']

  Description []

  SeeAlso     [PrinterBase.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``node.printers'' package of NuSMV version 2. 
  Copyright (C) 2006 by FBK-irst. 

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

  Revision    [$Id: PrinterBase.c,v 1.1.2.5.4.1 2009-03-23 18:13:22 nusmv Exp $]

******************************************************************************/

#include "PrinterBase.h" 
#include "PrinterBase_private.h" 

#include "MasterPrinter.h"
#include "MasterPrinter_private.h"

#include "node/MasterNodeWalker.h"
#include "utils/utils.h" 
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: PrinterBase.c,v 1.1.2.5.4.1 2009-03-23 18:13:22 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'PrinterBase_private.h' for class 'PrinterBase' definition. */ 

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

static void printer_base_finalize ARGS((Object_ptr object, void* dummy));

static int 
printer_base_print_node ARGS((PrinterBase_ptr self, node_ptr n, int priority));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Creates and initializes a printer.
  To be usable, the printer will have to be registered to a MasterPrinter.]

  Description        [To each printer is associated a partition of
  consecutive indices over the symbols set. The lowest index of the
  partition is given through the parameter low, while num is the
  partition size. Name is used to easily identify printer instances. 

  This constructor is private, as this class is virtual]

  SideEffects        []

  SeeAlso            [PrinterBase_destroy]   
  
******************************************************************************/
PrinterBase_ptr 
PrinterBase_create(const char* name, int low, size_t num)
{
  PrinterBase_ptr self = ALLOC(PrinterBase, 1);
  PRINTER_BASE_CHECK_INSTANCE(self);

  printer_base_init(self, name, low, num, false);
  return self;
}


/**Function********************************************************************

  Synopsis           [Prints the given node]

  Description [This is virtual method. BEfore calling, please ensure
  the given node can be handled by self, by calling
  PrinterBase_can_handle. 

  Note: This method will be never called by the user]

  SideEffects        []

  SeeAlso            [PrinterBase_can_handle]   
  
******************************************************************************/
VIRTUAL int 
PrinterBase_print_node(PrinterBase_ptr self, node_ptr n, int priority)
{
  PRINTER_BASE_CHECK_INSTANCE(self);
  
  return self->print_node(self, n, priority);
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The PrinterBase class private initializer]

  Description        [The PrinterBase class private initializer]

  SideEffects        []

  SeeAlso            [PrinterBase_create]   
  
******************************************************************************/
void printer_base_init(PrinterBase_ptr self, const char* name, 
                       int low, size_t num, boolean can_handle_null)
{
  /* base class initialization */
  node_walker_init(NODE_WALKER(self), name, low, num, can_handle_null);
  
  /* members initialization */

  /* virtual methods settings */  
  OVERRIDE(Object, finalize) = printer_base_finalize;
  OVERRIDE(PrinterBase, print_node) = printer_base_print_node;
}


/**Function********************************************************************

  Synopsis           [The PrinterBase class private deinitializer]

  Description        [The PrinterBase class private deinitializer]

  SideEffects        []

  SeeAlso            [PrinterBase_destroy]   
  
******************************************************************************/
void printer_base_deinit(PrinterBase_ptr self)
{
  /* members deinitialization */  

  /* base class initialization */
  node_walker_deinit(NODE_WALKER(self));
}


/**Function********************************************************************

  Synopsis           [This method must be called by the virtual method 
  print_node to recursively print sub nodes]

  Description        []

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
int printer_base_throw_print_node(PrinterBase_ptr self, node_ptr n, int prior)
{ 
  if (NodeWalker_can_handle(NODE_WALKER(self), n)) {
    /* checks if self can handle the node without need of re-throw
       to the master */
    return PrinterBase_print_node(self, n, prior);
  }    
  return master_printer_print_node(
           MASTER_PRINTER(NODE_WALKER(self)->master), n, prior);
}


/**Function********************************************************************

  Synopsis [Use this method to print a string to the stream currently
  set]

  Description        []

  SideEffects        []

  SeeAlso            []
  
******************************************************************************/
int printer_base_print_string(PrinterBase_ptr self, const char* str)
{ 
  return MasterPrinter_print_string(
      MASTER_PRINTER(NODE_WALKER(self)->master), str); 
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The PrinterBase class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void printer_base_finalize(Object_ptr object, void* dummy) 
{
  PrinterBase_ptr self = PRINTER_BASE(object);

  printer_base_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis    [Virtual menthod that prints the given node]

  Description [This is a pure virtual method, to be implemented by derived 
  class, and cannot be called]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static int 
printer_base_print_node(PrinterBase_ptr self, node_ptr n, int priority)
{
  internal_error("PrinterBase: Pure virtual method print_node " \
                 "not implemented\n");
  return 0;
}

/**AutomaticEnd***************************************************************/
