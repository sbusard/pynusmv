/**CFile***********************************************************************

  FileName    [nodeWffPrint.c]

  PackageName [node]

  Synopsis    [Pretty printing of formulas represented using node struct.]

  Description [This file conatins the code to perform pretty printing
  of a formula represented with a node struct.]

  SeeAlso     [node.c]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``node'' package of NuSMV version 2.
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

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "nodeInt.h"
#include "node/printers/MasterPrinter.h"

#if NUSMV_HAVE_STRING_H
#include <string.h> /* for strdup */
#else
char* strdup(const char*); /* forward declaration */
#endif


static char rcsid[] UTIL_UNUSED = "$Id: nodeWffPrint.c,v 1.13.2.4.4.17.6.3 2009-08-05 13:57:59 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Pretty print a formula on a file]

  Description        [Pretty print a formula on a file]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int print_node(FILE *stream, node_ptr n)
{ 
  MasterPrinter_ptr mp = node_pkg_get_global_master_wff_printer();

  MasterPrinter_set_stream_type(
				mp, 
				STREAM_TYPE_FILE, 
				(StreamTypeArg) stream);

  MasterPrinter_reset_stream(mp, 0);
  
  return 
      MasterPrinter_print_node(mp, n)  &&
  
    /* ensure proper flushing */
    MasterPrinter_flush_stream(mp);
}

/**Function********************************************************************

  Synopsis           [Pretty print a formula into a string]

  Description        [Pretty print a formula into a string. The returned 
  string must be freed after using it. Returns NULL in case of failure.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char* sprint_node(node_ptr n)
{
  MasterPrinter_ptr mp = node_pkg_get_global_master_wff_printer();

  MasterPrinter_set_stream_type(
				mp, 
				STREAM_TYPE_STRING, 
				STREAM_TYPE_ARG_UNUSED);

  MasterPrinter_reset_stream(mp, 0);

  boolean success = 
    MasterPrinter_print_node(mp, n) &&
  
    MasterPrinter_flush_stream(mp);

  return (success) ? strdup(MasterPrinter_get_streamed_string(mp)) : NULL;
}

/**Function********************************************************************

  Synopsis           [Pretty print a formula on a file (indenting)]

  Description [Pretty print a formula on a file (indenting), starting
  at given offset.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int print_node_indent_at(FILE *stream, node_ptr n, int ofs)
{ 
  MasterPrinter_ptr mp = node_pkg_get_indenting_master_wff_printer();

  MasterPrinter_set_stream_type(
				mp, 
				STREAM_TYPE_FILE, 
				(StreamTypeArg) stream);

  MasterPrinter_reset_stream(mp, ofs);
  
  return 
    MasterPrinter_print_node(mp, n) &&
    
    /* ensure proper flushing */
    MasterPrinter_flush_stream(mp);
}

/**Function********************************************************************

  Synopsis           [Pretty print a formula into a string (indenting) ]

  Description [Pretty print a formula into a string (indenting),
  starting at given offset. The returned string must be freed after
  using it. Returns NULL in case of failure.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char* sprint_node_indent_at(node_ptr n, int ofs)
{
  MasterPrinter_ptr mp = node_pkg_get_indenting_master_wff_printer();

  MasterPrinter_set_stream_type(
				mp, 
				STREAM_TYPE_STRING, 
				STREAM_TYPE_ARG_UNUSED);

  MasterPrinter_reset_stream(mp, ofs);

  boolean success =
    MasterPrinter_print_node(mp, n) &&

    /* ensure proper flushing */
    MasterPrinter_flush_stream(mp);

  return (success) ? strdup(MasterPrinter_get_streamed_string(mp)) : NULL;
}

/**Function********************************************************************

  Synopsis           [Pretty print a formula on a file (indenting)]

  Description [Pretty print a formula on a file (indenting), starting
  at column 0.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int print_node_indent(FILE *stream, node_ptr n)
{ 
  return print_node_indent_at(stream, n, 0);
}

/**Function********************************************************************

  Synopsis           [Pretty print a formula into a string (indenting) ]

  Description [Pretty print a formula into a string (indenting),
  starting at column 0. The returned string must be freed after using
  it. Returns NULL in case of failure.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
char* sprint_node_indent(node_ptr n)
{
  return sprint_node_indent_at(n, 0);
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
