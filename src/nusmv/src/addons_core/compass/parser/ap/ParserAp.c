/**CFile*****************************************************************

  FileName    [ParserAp.c]

  PackageName [compass.parser.ap]

  Synopsis    []

  Description []

  SeeAlso     [ParserAp.h]

  Author      [Marco Roveri]

  Copyright   [
  This file is part of the ``compass.parser.ap'' package of NuSMV version 2. 
  Copyright (C) 2008 by FBK-irst.

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
  Please report bugs to <nusmv-users@fbk.eu>.]

******************************************************************************/

#include "apInt.h"
#include "ParserAp.h"
#include "ParserAp_private.h"

#include "utils/NodeList.h"
#include "utils/ustring.h"
#include "utils/error.h"

#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: ParserAp.c,v 1.1.2.1 2008-12-29 14:57:15 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct ParserAp_TAG 
{
  NodeList_ptr ap_list;
} ParserAp;  


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void parser_ap_init ARGS((ParserAp_ptr self));
static void parser_ap_deinit ARGS((ParserAp_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ParserAp_ptr ParserAp_create()
{
  ParserAp_ptr self = ALLOC(ParserAp, 1);
  PARSER_AP_CHECK_INSTANCE(self);

  parser_ap_init(self);  
  return self;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserAp_destroy(ParserAp_ptr self)
{
  PARSER_AP_CHECK_INSTANCE(self);

  parser_ap_deinit(self);  
  FREE(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserAp_parse_from_file(ParserAp_ptr self, FILE* f)
{
  YY_BUFFER_STATE buf;

  PARSER_AP_CHECK_INSTANCE(self);

  parser_ap_set_global_parser(self);

  if (f == (FILE*) NULL) parser_ap_in = nusmv_stdin;
  else parser_ap_in = f;

  buf = parser_ap__create_buffer(parser_ap_in, 16384);
  parser_ap__switch_to_buffer(buf);
  parser_ap_restart(parser_ap_in);
  parser_ap_parse();
  parser_ap__delete_buffer(buf);

  parser_ap_reset_global_parser(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserAp_parse_from_string(ParserAp_ptr self, const char* str)
{  
  YY_BUFFER_STATE buf;

  PARSER_AP_CHECK_INSTANCE(self);
  
  parser_ap_set_global_parser(self);

  buf = parser_ap__scan_string(str);
  parser_ap__delete_buffer(buf);

  parser_ap_reset_global_parser(self);
}


/**Function********************************************************************

  Synopsis           [Returns the list of ap read by the parser]

  Description        [Returned list is owned by self, and should not be 
  changed or destroyed]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr ParserAp_get_ap_list(const ParserAp_ptr self)
{
  PARSER_AP_CHECK_INSTANCE(self);
  return self->ap_list;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserAp_reset(ParserAp_ptr self)
{
  PARSER_AP_CHECK_INSTANCE(self);

  parser_ap_deinit(self);
  self->ap_list = NodeList_create();
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
void parser_ap_add(ParserAp_ptr self, node_ptr ap)
{
  node_ptr apass;

  PARSER_AP_CHECK_INSTANCE(self);
  
  nusmv_assert(COLON == node_get_type(ap));
  apass = cons(car(ap), cdr(ap));

  NodeList_prepend(self->ap_list, (node_ptr) apass); 
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_ap_mk_ap(ParserAp_ptr self, 
                         node_ptr label, node_ptr ap)                            
{
  PARSER_AP_CHECK_INSTANCE(self);
  return new_node(COLON, label, ap);
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
static void parser_ap_init(ParserAp_ptr self)
{
  self->ap_list = NodeList_create();
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void parser_ap_deinit(ParserAp_ptr self)
{
  ListIter_ptr iter;
  NODE_LIST_FOREACH(self->ap_list, iter) {
    free_node(NodeList_get_elem_at(self->ap_list, iter));
  }
  NodeList_destroy(self->ap_list);
  self->ap_list = NODE_LIST(NULL);
}

