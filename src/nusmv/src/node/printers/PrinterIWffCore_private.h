/**CHeaderFile*****************************************************************

  FileName    [PrinterIWffCore_private.h]

  PackageName [node.printers]

  Synopsis    [Private and protected interface of class 'PrinterIWffCore']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [PrinterIWffCore.h]

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

  Revision    [$Id: PrinterIWffCore_private.h,v 1.1.2.1 2009-03-23 18:13:22 nusmv Exp $]

******************************************************************************/


#ifndef __PRINTER_IWFF_CORE_PRIVATE_H__
#define __PRINTER_IWFF_CORE_PRIVATE_H__


#include "PrinterIWffCore.h" 

#include "PrinterWffCore.h" 
#include "PrinterWffCore_private.h"

#include "PrinterBase.h"
#include "PrinterBase_private.h"

#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis [PrinterIWffCore class definition derived from class
  PrinterWffCore]

  Description []

  SeeAlso     [Base class PrinterIWffCore]   
  
******************************************************************************/
typedef struct PrinterIWffCore_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(PrinterWffCore); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */


  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} PrinterIWffCore;


/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only        */
/* ---------------------------------------------------------------------- */
EXTERN void 
printer_iwff_core_init ARGS((PrinterIWffCore_ptr self, const char* name, 
			    int low, size_t num));

EXTERN void
printer_iwff_core_deinit ARGS((PrinterIWffCore_ptr self));

EXTERN int 
printer_iwff_core_print_node ARGS((PrinterBase_ptr self, node_ptr n, 
				  int priority));

#endif /* __PRINTER_IWFF_CORE_PRIVATE_H__ */
