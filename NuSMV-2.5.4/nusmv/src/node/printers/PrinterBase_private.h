
/**CHeaderFile*****************************************************************

  FileName    [PrinterBase_private.h]

  PackageName [node.printers]

  Synopsis    [Private and protected interface of class 'PrinterBase']

  Description [This file can be included only by derived and friend classes]

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

  Revision    [$Id: PrinterBase_private.h,v 1.1.2.3 2006-04-13 09:52:32 nusmv Exp $]

******************************************************************************/


#ifndef __PRINTER_BASE_PRIVATE_H__
#define __PRINTER_BASE_PRIVATE_H__


#include "PrinterBase.h" 
#include "node/NodeWalker_private.h"

#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis    [PrinterBase class definition derived from
               class NodeWalker]

  Description []

  SeeAlso     [Base class Object]   
  
******************************************************************************/
typedef struct PrinterBase_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(NodeWalker); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
  int (*print_node)(PrinterBase_ptr self, node_ptr n, int priority);

} PrinterBase;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */
EXTERN PrinterBase_ptr 
PrinterBase_create ARGS((const char* name, int low, size_t num));

EXTERN void 
printer_base_init ARGS((PrinterBase_ptr self, const char* name, 
                        int low, size_t num, boolean can_handle_null));

EXTERN void printer_base_deinit ARGS((PrinterBase_ptr self));

EXTERN int 
printer_base_throw_print_node ARGS((PrinterBase_ptr self, 
                                    node_ptr n, int priority));

EXTERN int 
printer_base_print_string ARGS((PrinterBase_ptr self, const char* str));


#endif /* __PRINTER_BASE_PRIVATE_H__ */
