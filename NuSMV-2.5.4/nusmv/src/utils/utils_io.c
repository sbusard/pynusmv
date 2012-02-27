/**CFile***********************************************************************

  FileName    [utils_io.c]

  PackageName [utils]

  Synopsis    [Simple pretty printing]

  Description [Routines that provides some simple pretty printing routines.]

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

******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"
#include "utils/utils.h"
#include "node/node.h"
#include "utils/assoc.h"
#include "dd/dd.h"
#include "rbc/rbc.h"
#include "set/set.h"
#include "parser/symbols.h"
#include "utils/utils_io.h"
#include "utils/error.h"
#include "compile/compile.h"

static char rcsid[] UTIL_UNUSED = "$Id: utils_io.c,v 1.1.2.2.4.1 2005-03-03 12:32:24 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE *nusmv_stderr;

/**Variable********************************************************************

  Synopsis    [Number of spaces to be printed at the beginning of the line.]

  Description [Number of spaces to be printed at the beginning of the
  line. It is used to represent recursive calls to the evaluator. Each
  time the evaluator recursively call itself, than this variable is
  incremented and after the call decremented.]

  SeeAlso     []

******************************************************************************/
static int indent_size = 0;
void inc_indent_size(void) { indent_size++;}
void dec_indent_size(void) { indent_size--;}
int get_indent_size(void) { return(indent_size);}
void reset_indent_size(void) { indent_size = 0;}
void set_indent_size(int n) { indent_size = n;}

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

void indent(FILE *stream)
{
  int i;

  for(i=0; i < get_indent_size(); i++) fprintf(stream, "  ");
}

void indent_node(FILE *stream, char *s1, node_ptr n, char *s2)
{
  indent(stream);
  fprintf(stream, "%s", s1);
  print_node(stream, n);
  fprintf(stream, "%s", s2);
}

/* works as usual fprintf but before 'fmt' prints the required number
   of spaces (controlled by ..._indent_size functions).
*/
void indent_print(FILE * stream, const char * fmt, ...)
{
  va_list args;

  indent(stream);
  va_start(args, fmt);
  (void) vfprintf(stream, fmt, args);
  va_end(args);
}

/*
  This function prints out string "s" followed by the process
  name. This function is only used if verbose mode is greter than zero.
*/
void print_in_process(char *s, node_ptr context)
{
  fprintf(nusmv_stderr, "%s", s);
  if(context != Nil) indent_node(nusmv_stderr, " in process ", context, "");
  fprintf(nusmv_stderr, "...\n");
}

