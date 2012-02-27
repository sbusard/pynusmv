/**CFile*****************************************************************

  FileName    [ParserProb.c]

  PackageName [parser.ord]

  Synopsis    []

  Description []

  SeeAlso     [ParserProb.h]

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

#include "probInt.h"
#include "ParserProb.h"
#include "ParserProb_private.h"

#include "addons_core/compass/compile/ProbAssign.h"

#include "utils/NodeList.h"
#include "utils/ustring.h"
#include "utils/error.h"

#include "parser/symbols.h"

static char rcsid[] UTIL_UNUSED = "$Id: ParserProb.c,v 1.1.2.2 2009-02-04 20:02:12 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef struct ParserProb_TAG 
{
  NodeList_ptr prob_list;
} ParserProb;  


/*---------------------------------------------------------------------------*/
/* macro declarations                                                        */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void parser_prob_init ARGS((ParserProb_ptr self));
static void parser_prob_deinit ARGS((ParserProb_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
ParserProb_ptr ParserProb_create()
{
  ParserProb_ptr self = ALLOC(ParserProb, 1);
  PARSER_PROB_CHECK_INSTANCE(self);

  parser_prob_init(self);  
  return self;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserProb_destroy(ParserProb_ptr self)
{
  PARSER_PROB_CHECK_INSTANCE(self);

  parser_prob_deinit(self);  
  FREE(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserProb_parse_from_file(ParserProb_ptr self, FILE* f)
{
  YY_BUFFER_STATE buf;

  PARSER_PROB_CHECK_INSTANCE(self);

  parser_prob_set_global_parser(self);

  if (f == (FILE*) NULL) parser_prob_in = nusmv_stdin;
  else parser_prob_in = f;

  buf = parser_prob__create_buffer(parser_prob_in, 16384);
  parser_prob__switch_to_buffer(buf);
  parser_prob_restart(parser_prob_in);
  parser_prob_parse();
  parser_prob__delete_buffer(buf);

  parser_prob_reset_global_parser(self);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserProb_parse_from_string(ParserProb_ptr self, const char* str)
{  
  YY_BUFFER_STATE buf;

  PARSER_PROB_CHECK_INSTANCE(self);
  
  parser_prob_set_global_parser(self);

  buf = parser_prob__scan_string(str);
  parser_prob__delete_buffer(buf);

  parser_prob_reset_global_parser(self);
}


/**Function********************************************************************

  Synopsis           [Returns the list of prob read by the parser]

  Description        [Returned list is owned by self, and should not be 
  changed or destroyed]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
NodeList_ptr ParserProb_get_prob_list(const ParserProb_ptr self)
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return self->prob_list;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void ParserProb_reset(ParserProb_ptr self)
{
  PARSER_PROB_CHECK_INSTANCE(self);

  parser_prob_deinit(self);
  self->prob_list = NodeList_create();
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
void parser_prob_add(ParserProb_ptr self, node_ptr prob)
{
  ProbAssign_ptr probass;
  PARSER_PROB_CHECK_INSTANCE(self);
  
  nusmv_assert(COLON == node_get_type(prob));
  probass = ProbAssign_create(car(prob), cdr(prob));

  NodeList_prepend(self->prob_list, (node_ptr) probass); 
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_prob(ParserProb_ptr self, 
                           node_ptr assigns, node_ptr prob)                            
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(COLON, assigns, prob);
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_var_assign(ParserProb_ptr self, 
                                  node_ptr var, node_ptr val)
                            
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(EQUAL, var, val);
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_var_assigns(ParserProb_ptr self, 
                                   node_ptr left, node_ptr right)
                            
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(AND, left, right);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_dot(ParserProb_ptr self, node_ptr left, node_ptr right)
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(DOT, left, right);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_array(ParserProb_ptr self, node_ptr left, node_ptr right)
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(ARRAY, left, right);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_atom(ParserProb_ptr self, const char* name) 
{
  node_ptr atom;
  PARSER_PROB_CHECK_INSTANCE(self);

  atom = find_node(ATOM, (node_ptr) find_string((char*) name), Nil);
  return atom;
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_num(ParserProb_ptr self, const int num) 
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(NUMBER, NODE_FROM_INT(num), Nil);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_true(ParserProb_ptr self) 
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(TRUEEXP, Nil, Nil);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_false(ParserProb_ptr self) 
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(FALSEEXP, Nil, Nil);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
node_ptr parser_prob_mk_real(ParserProb_ptr self, const char* real_text) 
{
  PARSER_PROB_CHECK_INSTANCE(self);
  return find_node(NUMBER_REAL, 
                   (node_ptr) find_string((char*) real_text), Nil);
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
static void parser_prob_init(ParserProb_ptr self)
{
  self->prob_list = NodeList_create();
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void parser_prob_deinit(ParserProb_ptr self)
{
  ListIter_ptr iter;
  NODE_LIST_FOREACH(self->prob_list, iter) {
    ProbAssign_destroy(PROB_ASSIGN(NodeList_get_elem_at(self->prob_list, iter)));
  }
  NodeList_destroy(self->prob_list);
  self->prob_list = NODE_LIST(NULL);
}

