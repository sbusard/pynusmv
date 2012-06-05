/**CHeaderFile*****************************************************************

  FileName    [PrinterIWffCore.h]

  PackageName [node.printers]

  Synopsis    [Public interface of class 'PrinterIWffCore']

  Description [An indenting subclass of PrinterWffCore]

  SeeAlso     [PrinterWffCore.c]

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

  Revision    [$Id: PrinterIWffCore.h,v 1.1.2.1 2009-03-23 18:13:22 nusmv Exp $]

******************************************************************************/

#ifndef __PRINTER_IWFF_CORE_H__
#define __PRINTER_IWFF_CORE_H__

#include "PrinterBase.h"
#include "utils/utils.h" 

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class PrinterIWffCore]

  Description []

******************************************************************************/
typedef struct PrinterIWffCore_TAG*  PrinterIWffCore_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class PrinterWffCore]

  Description [These macros must be used respectively to cast and to check
  instances of class PrinterIWffCore]

******************************************************************************/
#define PRINTER_IWFF_CORE(self) \
         ((PrinterIWffCore_ptr) self)

#define PRINTER_IWFF_CORE_CHECK_INSTANCE(self) \
         (nusmv_assert(PRINTER_IWFF_CORE(self) != PRINTER_IWFF_CORE(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN PrinterIWffCore_ptr PrinterIWffCore_create ARGS((const char* name));

/**AutomaticEnd***************************************************************/

#endif /* __PRINTER_IWFF_CORE_H__ */
