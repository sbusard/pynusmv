/**CFile***********************************************************************

  FileName    [sbmcNodeStack.c]

  PackageName [bmc.sbmc]

  Synopsis    [A stack of node_ptr]

  Description [A stack of node_ptr]

  SeeAlso     []

  Author      [Timo Latvala]

  Copyright   [
  This file is part of the ``bmc.sbmc'' package of NuSMV version 2. 
  Copyright (C) 2004 by Timo Latvala <timo.latvala@tkk.fi>. 

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

  For more information of NuSMV see <http://nusmv.fbk.eu>
  or email to <nusmv-users@fbk.eu>.
  Please report bugs to <nusmv-users@fbk.eu>.

  To contact the NuSMV development board, email to <nusmv@fbk.eu>. ]

******************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include "sbmcNodeStack.h"
#include "utils/utils.h"
#include "utils/error.h"

static char rcsid[] UTIL_UNUSED = "$Id: sbmcNodeStack.c,v 1.1.2.5.4.1 2010-03-01 14:38:46 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define STACK_SIZE 127

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN FILE* nusmv_stderr;

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Create a new stack]

  Description        [Create a new stack]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
Bmc_Stack_ptr Bmc_Stack_new_stack()
{
  unsigned i;
  Bmc_Stack_ptr thestack = (Bmc_Stack_ptr)ALLOC(struct nodeStack, 1);
  thestack->alloc = STACK_SIZE;
  thestack->first_free = 0;
  thestack->table = (node_ptr *) ALLOC(node_ptr, thestack->alloc);
  if (thestack->table == NULL) {
    internal_error("Bmc_Stack_new_stack: Out of Memory");
  }

  for (i=0; i < thestack->alloc; ++i) {
    thestack->table[i] = NULL;
  }
  return thestack;
}

/**Function********************************************************************

  Synopsis           [Push a node unto the stack]

  Description        [Push a node unto the stack]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void Bmc_Stack_push(Bmc_Stack_ptr thestack, node_ptr node) 
{ 
  if (thestack->first_free >= thestack->alloc) { /**The stack needs to grow*/
    unsigned i;
    thestack->alloc = 2*thestack->alloc;
    node_ptr *temp = ALLOC(node_ptr, thestack->alloc); 
    nusmv_assert(temp != NULL);
    for (i = thestack->first_free; i--; ) {
      temp[i] = thestack->table[i];
    }
    FREE(thestack->table);
    thestack->table = temp;    
  }
  /**Put node on stack*/
  thestack->table[thestack->first_free] = node;
  thestack->first_free++;
  return;
}

/**Function********************************************************************

  Synopsis           [Return the number of occupied slots]

  Description        [Return the number of occupied slots]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
unsigned Bmc_Stack_size(Bmc_Stack_ptr thestack) 
{  
  return thestack->first_free;
}

/**Function********************************************************************

  Synopsis           [Pop an element from the stack]

  Description        [Pop an element from the stack]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Stack_pop (Bmc_Stack_ptr thestack)
{
  nusmv_assert(thestack->first_free > 0);
  thestack->first_free--;
  return thestack->table[thestack->first_free];
}

/**Function********************************************************************

  Synopsis           [Delete the stack]

  Description        [Delete the stack]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void Bmc_Stack_delete(Bmc_Stack_ptr thestack)
{
  FREE(thestack->table);
  FREE(thestack);
}

/**Function********************************************************************

  Synopsis           [Return the top element of the stack]

  Description        [Return the top element of the stack]

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
node_ptr Bmc_Stack_top(Bmc_Stack_ptr thestack)
{
  nusmv_assert(thestack->first_free > 0);
  return thestack->table[thestack->first_free - 1];
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
