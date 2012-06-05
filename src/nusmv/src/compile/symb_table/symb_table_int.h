/**CHeaderFile*****************************************************************

  FileName    [symb_table_int.h]

  PackageName [compile.symb_table]

  Synopsis    [The private interface of the package compile.symb_table]

  Description []
                                               
  SeeAlso     [SymbCache.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``compile.symb_table'' package of NuSMV
  version 2.  Copyright (C) 2004 by FBK-irst.

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

******************************************************************************/

#ifndef __COMPILE_SYMB_TABLE_INT_H__
#define __COMPILE_SYMB_TABLE_INT_H__

#include "utils/utils.h"
#include "opt/opt.h"


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

EXTERN FILE* nusmv_stderr;
EXTERN FILE* nusmv_stdout;
EXTERN int yylineno;

EXTERN node_ptr boolean_range;
EXTERN node_ptr zero_number;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void SymbTablePkg_init ARGS((void));
EXTERN void SymbTablePkg_quit ARGS((void));


#endif /* __COMPILE_SYMB_TABLE_INT_H__ */
