/**CFile*****************************************************************

  FileName    [ParserIdList.c]

  PackageName [parser.idlist]

  Synopsis    []

  Description []

  SeeAlso     [ParserIdList.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``parser.idlist'' package of NuSMV version 2. 
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

******************************************************************************/

#include "idlist_int.h"
#include "ParserIdList.h"
#include "ParserIdList_private.h"

#include "utils/NodeList.h"
#include "utils/ustring.h"
#include "utils/error.h"

#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: ParserIdList.c,v 1.1.2.3 2007-03-20 19:30:13 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct ParserIdList_TAG 
{
  NodeList_ptr id_list;
} ParserIdList;  


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void parser_id_list_init ARGS((ParserIdList_ptr self));
static void parser_id_list_deinit ARGS((ParserIdList_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ParserIdList_ptr ParserIdList_create()
{
  ParserIdList_ptr self = ALLOC(ParserIdList, 1);
  PARSER_ID_LIST_CHECK_INSTANCE(self);

  parser_id_list_init(self);  
  return self;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserIdList_destroy(ParserIdList_ptr self)
{
  PARSER_ID_LIST_CHECK_INSTANCE(self);

  parser_id_list_deinit(self);  
  FREE(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserIdList_parse_from_file(ParserIdList_ptr self, FILE* f)
{
  YY_BUFFER_STATE buf;

  PARSER_ID_LIST_CHECK_INSTANCE(self);

  parser_idlist_set_global_parser(self);

  if (f == (FILE*) NULL) parser_idlist_in = nusmv_stdin;
  else parser_idlist_in = f;

  buf = parser_idlist__create_buffer(parser_idlist_in, 16384);
  parser_idlist__switch_to_buffer(buf);
  parser_idlist_restart(parser_idlist_in);

  CATCH { parser_idlist_parse(); }
  FAIL { 
    parser_idlist__delete_buffer(buf);
    parser_idlist_reset_global_parser(self);
    nusmv_exit(1);
  }

  parser_idlist__delete_buffer(buf);
  parser_idlist_reset_global_parser(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserIdList_parse_from_string(ParserIdList_ptr self, const char* str)
{  
  YY_BUFFER_STATE buf = NULL;

  PARSER_ID_LIST_CHECK_INSTANCE(self);
  
  parser_idlist_set_global_parser(self);

  CATCH {
    buf = parser_idlist__scan_string(str);
    parser_idlist_parse();
  }
  FAIL { 
    if (((YY_BUFFER_STATE) NULL) != buf) {
        parser_idlist__delete_buffer(buf);
      }
    parser_idlist_reset_global_parser(self);
    nusmv_exit(1);
  }

  parser_idlist__delete_buffer(buf);
  parser_idlist_reset_global_parser(self);
}


/**Function********************************************************************

  Synopsis           [Returns the list of variables read by the parser]

  Description        [Returned list is owned by self, and should not be 
  changed or destroyed]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr ParserIdList_get_id_list(const ParserIdList_ptr self)
{
  PARSER_ID_LIST_CHECK_INSTANCE(self);
  return self->id_list;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserIdList_reset(ParserIdList_ptr self)
{
  PARSER_ID_LIST_CHECK_INSTANCE(self);

  parser_id_list_deinit(self);
  parser_id_list_init(self);
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
void parser_id_list_add_id(ParserIdList_ptr self, node_ptr name)
{
  PARSER_ID_LIST_CHECK_INSTANCE(self);

  if (NodeList_belongs_to(self->id_list, name)) {
    warning_id_appears_twice_in_idlist_file(name);
  }
  else {
    NodeList_prepend(self->id_list, name); 
  }
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_id_list_mk_dot(ParserIdList_ptr self, node_ptr left, 
			       node_ptr right)
{
  PARSER_ID_LIST_CHECK_INSTANCE(self);
  return find_node(DOT, left, right);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_id_list_mk_array(ParserIdList_ptr self, node_ptr left, 
				 node_ptr right)
{
  PARSER_ID_LIST_CHECK_INSTANCE(self);
  return find_node(ARRAY, left, right);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_id_list_mk_bit(ParserIdList_ptr self, node_ptr left, 
			       int suffix)
{
  PARSER_ID_LIST_CHECK_INSTANCE(self);
  return find_node(BIT, left, NODE_FROM_INT(suffix));
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_id_list_mk_atom(ParserIdList_ptr self, const char* name) 
{
  node_ptr atom;
  PARSER_ID_LIST_CHECK_INSTANCE(self);

  atom = find_node(ATOM, (node_ptr) find_string((char*) name), Nil);
  return atom;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_id_list_mk_num(ParserIdList_ptr self, const int num) 
{
  PARSER_ID_LIST_CHECK_INSTANCE(self);
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
static void parser_id_list_init(ParserIdList_ptr self)
{
  self->id_list = NodeList_create();
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void parser_id_list_deinit(ParserIdList_ptr self)
{
  NodeList_destroy(self->id_list);
}

