
/**CHeaderFile*****************************************************************

  FileName    [PrinterPsl_private.h]

  PackageName [node.printers]

  Synopsis    [Private and protected interface of class 'PrinterPsl']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [PrinterPsl.h]

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

  Revision    [$Id: PrinterPsl_private.h,v 1.1.2.1 2006-03-13 09:47:47 nusmv Exp $]

******************************************************************************/


#ifndef __PRINTER_PSL_PRIVATE_H__
#define __PRINTER_PSL_PRIVATE_H__


#include "PrinterPsl.h" 
#include "PrinterBase.h" /* fix this */ 
#include "PrinterBase_private.h" /* fix this */ 
#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis    [PrinterPsl class definition derived from
               class PrinterBase]

  Description []

  SeeAlso     [Base class PrinterBase]   
  
******************************************************************************/
typedef struct PrinterPsl_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(PrinterBase); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */


  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} PrinterPsl;



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void printer_psl_init ARGS((PrinterPsl_ptr self, const char* name, 
			    int low, size_t num));

EXTERN void printer_psl_deinit ARGS((PrinterPsl_ptr self));

EXTERN int 
printer_psl_print_node ARGS((PrinterBase_ptr self, node_ptr n, 
			     int priority));


#endif /* __PRINTER_PSL_PRIVATE_H__ */
