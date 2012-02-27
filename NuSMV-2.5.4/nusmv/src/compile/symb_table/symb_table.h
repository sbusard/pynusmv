/**CHeaderFile*****************************************************************

  FileName    [symb_table.h]

  PackageName [compile.symb_table]

  Synopsis    [Public interface of compile.symb_table package]

  Description [See symb_table.c file for more description]

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``compile.symb_table'' package of NuSMV version 2. 
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

  Revision    [$Id: symb_table.h,v 1.1.2.7.6.3 2007-11-08 19:06:29 nusmv Exp $]

******************************************************************************/
#ifndef __SYMB_TABLE_PKG_H__
#define __SYMB_TABLE_PKG_H__

#include "SymbType.h"
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

/* create simplified types with shared memory */
EXTERN SymbType_ptr SymbTablePkg_no_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_statement_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_boolean_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_pure_symbolic_enum_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_int_symbolic_enum_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_pure_int_enum_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_integer_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_real_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_unsigned_word_type ARGS((int width));
EXTERN SymbType_ptr SymbTablePkg_signed_word_type ARGS((int width));
EXTERN SymbType_ptr SymbTablePkg_wordarray_type ARGS((int awidth, int vwidth));
EXTERN SymbType_ptr SymbTablePkg_array_type ARGS((SymbType_ptr subtype,
                                                  int lower_bound,
                                                  int upper_bound));
EXTERN SymbType_ptr SymbTablePkg_string_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_boolean_set_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_integer_set_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_symbolic_set_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_integer_symbolic_set_type ARGS((void));
EXTERN SymbType_ptr SymbTablePkg_error_type ARGS((void));


#endif /* __SYMB_TABLE_PKG_H__ */
