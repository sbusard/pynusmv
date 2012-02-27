/**CFile*****************************************************************

  FileName    [ParserOrd.c]

  PackageName [parser.ord]

  Synopsis    []

  Description []

  SeeAlso     [ParserOrd.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.ord'' package of NuSMV version 2. 
  Copyright (C) 2003 by FBK-irst.

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

#include "ordInt.h"
#include "ParserOrd.h"
#include "ParserOrd_private.h"

#include "utils/NodeList.h"
#include "utils/ustring.h"
#include "utils/error.h"

#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: ParserOrd.c,v 1.1.2.7.4.2.6.2 2009-06-03 19:52:29 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct ParserOrd_TAG 
{
  NodeList_ptr vars_list;
} ParserOrd;  


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void parser_ord_init ARGS((ParserOrd_ptr self));
static void parser_ord_deinit ARGS((ParserOrd_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ParserOrd_ptr ParserOrd_create()
{
  ParserOrd_ptr self = ALLOC(ParserOrd, 1);
  PARSER_ORD_CHECK_INSTANCE(self);

  parser_ord_init(self);  
  return self;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserOrd_destroy(ParserOrd_ptr self)
{
  PARSER_ORD_CHECK_INSTANCE(self);

  parser_ord_deinit(self);  
  FREE(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserOrd_parse_from_file(ParserOrd_ptr self, FILE* f)
{
  YY_BUFFER_STATE buf;

  PARSER_ORD_CHECK_INSTANCE(self);

  parser_ord_set_global_parser(self);

  if (f == (FILE*) NULL) parser_ord_in = nusmv_stdin;
  else parser_ord_in = f;

  buf = parser_ord__create_buffer(parser_ord_in, 16384);
  parser_ord__switch_to_buffer(buf);
  parser_ord_restart(parser_ord_in);
  parser_ord_parse();
  parser_ord__delete_buffer(buf);

  parser_ord_reset_global_parser(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserOrd_parse_from_string(ParserOrd_ptr self, const char* str)
{  
  YY_BUFFER_STATE buf;

  PARSER_ORD_CHECK_INSTANCE(self);
  
  parser_ord_set_global_parser(self);

  buf = parser_ord__scan_string(str);
  parser_ord_parse();
  parser_ord__delete_buffer(buf);

  parser_ord_reset_global_parser(self);
}


/**Function********************************************************************

  Synopsis           [Returns the list of variables read by the parser]

  Description        [Returned list is owned by self, and should not be 
  changed or destroyed]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr ParserOrd_get_vars_list(const ParserOrd_ptr self)
{
  PARSER_ORD_CHECK_INSTANCE(self);
  return self->vars_list;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserOrd_reset(ParserOrd_ptr self)
{
  PARSER_ORD_CHECK_INSTANCE(self);

  NodeList_destroy(self->vars_list);
  self->vars_list = NodeList_create();
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void parser_ord_add_var(ParserOrd_ptr self, node_ptr name)
{
  PARSER_ORD_CHECK_INSTANCE(self);

  if (NodeList_belongs_to(self->vars_list, name)) {
    warning_var_appear_twice_in_order_file(name);
  }
  else {
    NodeList_prepend(self->vars_list, name); 
  }
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_ord_mk_dot(ParserOrd_ptr self, node_ptr left, node_ptr right)
{
  PARSER_ORD_CHECK_INSTANCE(self);
  return find_node(DOT, left, right);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_ord_mk_array(ParserOrd_ptr self, node_ptr left, node_ptr right)
{
  PARSER_ORD_CHECK_INSTANCE(self);
  return find_node(ARRAY, left, right);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_ord_mk_bit(ParserOrd_ptr self, node_ptr left, int suffix)
{
  PARSER_ORD_CHECK_INSTANCE(self);
  return find_node(BIT, left, NODE_FROM_INT(suffix));
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_ord_mk_atom(ParserOrd_ptr self, const char* name) 
{
  node_ptr atom;
  PARSER_ORD_CHECK_INSTANCE(self);

  atom = find_node(ATOM, (node_ptr) find_string((char*) name), Nil);
  return atom;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_ord_mk_num(ParserOrd_ptr self, const int num) 
{
  PARSER_ORD_CHECK_INSTANCE(self);
  return find_node(NUMBER, NODE_FROM_INT(num), Nil);
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void parser_ord_init(ParserOrd_ptr self)
{
  self->vars_list = NodeList_create();
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void parser_ord_deinit(ParserOrd_ptr self)
{
  NodeList_destroy(self->vars_list);
}

