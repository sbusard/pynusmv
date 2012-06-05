/**CFile***********************************************************************

  FileName    [nodePrint.c]

  PackageName [node]

  Synopsis    [Pretty prints a node struct.]

  Description [This function pretty print a node struct, in a way
  similar to a s-expression in LISP.]

  SeeAlso     []

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
#include "nodeInt.h"
#include "utils/ustring.h"
#include "utils/WordNumber.h" /* for WordNumber_ptr printing */
#include "node/printers/MasterPrinter.h"

static char UTIL_UNUSED rcsid[] = "$Id: nodePrint.c,v 1.7.4.3.2.1.2.21.4.16 2010-01-11 15:07:54 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void print_array_type_rec ARGS((FILE* out, const node_ptr body));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/
int print_sexp(FILE *file, node_ptr node)
{
  MasterPrinter_ptr mp = node_pkg_get_global_master_sexp_printer();

  MasterPrinter_set_stream_type(mp, STREAM_TYPE_FILE,
                                (StreamTypeArg) file);

  MasterPrinter_reset_stream(mp, 0);

  return
    MasterPrinter_print_node(mp, node)  &&
    /* ensure proper flushing */
    MasterPrinter_flush_stream(mp);
}


/**Function********************************************************************

  Synopsis           [Print an ARRAY_TYPE structure in smv]

  Description        []

  SideEffects        []

  SeeAlso            [print_sexp]

******************************************************************************/
void print_array_type(FILE* output_stream, const node_ptr body) 
{
  nusmv_assert(ARRAY_TYPE == node_get_type(body));
  print_array_type_rec(output_stream, body);
}


/**Function********************************************************************

  Synopsis           [Private function of print_array_type]

  Description        []

  SideEffects        []

  SeeAlso            [print_array_type]

******************************************************************************/
static void print_array_type_rec(FILE* out, const node_ptr body) 
{
  switch (node_get_type(body)) {
  case ARRAY_TYPE:
    fprintf(out, "array ");
    print_array_type_rec(out, car(body));
    fprintf(out, " of ");
    print_array_type_rec(out, cdr(body));
    break;
    
  case TWODOTS:
    print_node(out, car(body));
    fprintf(out, " .. ");
    print_node(out, cdr(body));
    break;
    
  case BOOLEAN:
    fprintf(out, "boolean");
    break;

  case UNSIGNED_WORD:
    fprintf(out, "word[");
    print_node(out, car(body));
    fprintf(out, "]");
    break;

  case INTEGER:
    fprintf(out, "integer");
    break;

  case REAL:
    fprintf(out, "real");
    break;

  case SCALAR:
    fprintf(out, "{ ");
    print_array_type_rec(out, car(body));
    fprintf(out, " }");
    break;

  case CONS:
    print_array_type_rec(out, car(body));                
    if (cdr(body) != Nil) {
      fprintf(out, ", ");
      print_array_type_rec(out, cdr(body));
    }
    break;

  default:
    print_node(out, body);
    break;
  }  
}
