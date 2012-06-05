/**CHeaderFile*****************************************************************

  FileName    [type_checkingInt.h]

  PackageName [compile.type_checking]

  Synopsis    [The private interface of the type-checking package.]

  Description [This package contains the functions required to 
  the type checking package internally]

  Author      [Andrei Tchaltsev]

  Copyright   [
  This file is part of the ``compile.type_checking'' package of NuSMV version 2.
  Copyright (C) 2000-2005 by  FBK-irst. 

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

  Revision    [$Id: type_checkingInt.h,v 1.1.2.10 2006-04-04 09:41:58 nusmv Exp $]

******************************************************************************/
#ifndef __TYPE_CHECKING_INT_H__
#define __TYPE_CHECKING_INT_H__

#include "TypeChecker.h"
#include "checkers/checkersInt.h"

#include "compile/compile.h"
#include "opt/opt.h" /* for options type_checking_... */
#include "node/node.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

extern FILE * nusmv_stderr;
extern FILE * nusmv_stdout;
extern FILE * nusmv_stdin;

extern int yylineno;

extern node_ptr boolean_range;
extern node_ptr zero_number;
extern node_ptr one_number;


/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

#endif /* __TYPE_CHECKING_INT_H__ */
