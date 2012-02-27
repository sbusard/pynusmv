/**CHeaderFile*****************************************************************

  FileName    [utils_io.h]

  PackageName [utils]

  Synopsis    [Header for the utils_io.c file.]

  Description []

  SeeAlso     []

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and FBK-irst. 

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

  Revision    [$Id: utils_io.h,v 1.1.2.1.4.1.6.1 2009-07-20 14:02:40 nusmv Exp $]

******************************************************************************/

#ifndef __UTILS_IO_H__
#define __UTILS_IO_H__

#include "utils.h"
#include "dd/dd.h"
#include "prop/Prop.h"

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN void inc_indent_size ARGS((void));
EXTERN void dec_indent_size ARGS((void));
EXTERN int get_indent_size ARGS((void));
EXTERN void reset_indent_size ARGS((void));
EXTERN void set_indent_size ARGS((int));

EXTERN void indent ARGS((FILE *));
EXTERN void indent_print ARGS((FILE *, const char *, ...));
EXTERN void indent_node ARGS((FILE *,char *, node_ptr, char *));
EXTERN void print_in_process ARGS((char *, node_ptr));
EXTERN void print_invar ARGS((FILE *, Prop_ptr));

#endif /* __UTILS_IO_H__ */
