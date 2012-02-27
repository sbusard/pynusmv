/**CFile***********************************************************************

  FileName    [HrcDumper.c]

  PackageName [hrc.dumpers]

  Synopsis    [Implementation of class 'HrcDumper']

  Description []

  SeeAlso     [HrcDumper.h]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``hrc.dumpers'' package of NuSMV version 2. 
  Copyright (C) 2011 by FBK-irst. 

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

  Revision    [$Id: $]

******************************************************************************/

#include "HrcDumper.h" 
#include "HrcDumper_private.h" 
#include "parser/symbols.h"
#include "utils/utils.h" 
#include "utils/error.h" 

static char rcsid[] UTIL_UNUSED = "$Id: $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'HrcDumper_private.h' for class 'HrcDumper' definition. */ 

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define HRC_DEFAULT_INDENT_SIZE 4


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void hrc_dumper_finalize ARGS((Object_ptr object, void* dummy));

static void 
hrc_dumper_dump_scalar_type ARGS((HrcDumper_ptr self, node_ptr node));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The HrcDumper class constructor]

  Description [The HrcDumper class constructor. Parameter fout
  belongs to self.]

  SideEffects        []

  SeeAlso            [HrcDumper_destroy]

******************************************************************************/
HrcDumper_ptr HrcDumper_create(FILE* fout)
{
  HrcDumper_ptr self = ALLOC(HrcDumper, 1);
  HRC_DUMPER_CHECK_INSTANCE(self);

  hrc_dumper_init(self, fout);
  return self;
}


/**Function********************************************************************

  Synopsis           [The HrcDumper class destructor]

  Description [The HrcDumper class destructor. This can be used
  also by all derivated classes.]

  SideEffects        []

  SeeAlso            [HrcDumper_create]

******************************************************************************/
void HrcDumper_destroy(HrcDumper_ptr self)
{
  HRC_DUMPER_CHECK_INSTANCE(self);

  Object_destroy(OBJECT(self), NULL);
}


/**Function********************************************************************

  Synopsis           [Makes the dumper dump the given snippet]

  Description        [This is a virtual method]

  SideEffects        []

  SeeAlso            [HrcDumper_destroy]

******************************************************************************/
VIRTUAL void HrcDumper_dump_snippet(HrcDumper_ptr self,
                                    HrcDumperSnippet snippet,
                                    const HrcDumperInfo* info)
{
  HRC_DUMPER_CHECK_INSTANCE(self);
  self->dump_snippet(self, snippet, info);
}


/**Function********************************************************************

  Synopsis           [Enables/disables the indentation]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcDumper_enable_indentation(HrcDumper_ptr self, boolean flag)
{
  HRC_DUMPER_CHECK_INSTANCE(self);
  self->use_indentation = flag;
}


/**Function********************************************************************

  Synopsis           [Increments the indent level]

  Description        [Increments the indent level]

  SideEffects        []

  SeeAlso            [HrcDumper_dec_indent]

******************************************************************************/
void HrcDumper_inc_indent(HrcDumper_ptr self)
{
  HRC_DUMPER_CHECK_INSTANCE(self);
  nusmv_assert(self->indent >= 0);
  self->indent += 1;
}


/**Function********************************************************************

  Synopsis           [Decrements the indent level]

  Description [Decrements the indent level. Each call must
  correspond to a call to inc_indent. An assertion fails if called
  when the indent level is zero.]

  SideEffects        []

  SeeAlso            [HrcDumper_inc_indent]

******************************************************************************/
void HrcDumper_dec_indent(HrcDumper_ptr self)
{
  HRC_DUMPER_CHECK_INSTANCE(self);
  nusmv_assert(self->indent > 0);
  self->indent -= 1;
}


/**Function********************************************************************

  Synopsis [Controls if module names must be dumped with a
  (default) suffix or not.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void HrcDumper_enable_mod_suffix(HrcDumper_ptr self, boolean flag)
{
  HRC_DUMPER_CHECK_INSTANCE(self);
  self->use_mod_suffix = flag;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The HrcDumper class private initializer]

  Description        [The HrcDumper class private initializer]

  SideEffects        []

  SeeAlso            [HrcDumper_create]

******************************************************************************/
void hrc_dumper_init(HrcDumper_ptr self, FILE* fout)
{
  /* base class initialization */
  object_init(OBJECT(self));

  /* members initialization */
  self->fout = fout;
  self->use_indentation = true;
  self->indent_pending = false;
  self->indent = 0;
  self->indent_size = HRC_DEFAULT_INDENT_SIZE;
  self->columns = HRC_DEFAULT_COLUMNS;
  self->use_mod_suffix = false;

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = hrc_dumper_finalize;
  OVERRIDE(HrcDumper, dump_snippet) = hrc_dumper_dump_snippet;
  OVERRIDE(HrcDumper, dump_comment) = hrc_dumper_dump_comment;
  OVERRIDE(HrcDumper, dump_header) = hrc_dumper_dump_header;
}


/**Function********************************************************************

  Synopsis           [The HrcDumper class private deinitializer]

  Description        [The HrcDumper class private deinitializer]

  SideEffects        []

  SeeAlso            [HrcDumper_destroy]

******************************************************************************/
void hrc_dumper_deinit(HrcDumper_ptr self)
{
  extern FILE *nusmv_stderr, *nusmv_stdout;

  /* members deinitialization */
  if (self->fout && self->fout != nusmv_stderr && self->fout != nusmv_stdout) {
    fclose(self->fout);
  }

  /* base class deinitialization */
  object_deinit(OBJECT(self));
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_dump_snippet(HrcDumper_ptr self,
                             HrcDumperSnippet snippet,
                             const HrcDumperInfo* info)
{
  switch (snippet) {
  case HDS_HRC_TOP:
    /* Available information:
       info->hrcNode
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_MODS:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_MOD:
    /* Available information:
       info->hrcNode
       info->n1.name : name of module
       info->n2.lineno : line number of MODULE definition
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_MOD_NAME:
    /* Available information:
       info->hrcNode
       info->n1.name : name of module
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_MOD_FORMAL_PARAMS:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_MOD_FORMAL_PARAM:
    /* Available information:
       info->hrcNode
       info->n1.name : name of param
       info->n2.type : type of param (can be Nil)
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_MOD_INSTANCES:
    /* Available information:
       info->hrcNode
       info->symb_cat : the var kind of the instances
       info->list_is_empty
   */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_MOD_INSTANCE:
    /* Available information:
       info->hrcNode
       info->n1.name : the instance name
       info->n2.type : the module instance name
       info->symb_cat : the var kind of the instance
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_MOD_INSTANCE_VARNAME:
    /* Available information:
       info->hrcNode
       info->n1.name : name of the instance
       info->symb_cat : the var kind of the instance
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_MOD_INSTANCE_MODNAME:
    /* Available information:
       info->hrcNode
       info->n1.name : name of the instance's type (module)
       info->symb_cat : the var kind of the instance
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_MOD_INSTANCE_ACTUAL_PARAMS:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_MOD_INSTANCE_ACTUAL_PARAM:
    /* Available information:
       info->hrcNode
       info->n1.value : value of the actual param
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_SYMBOLS:
    /* Available information:
       info->hrcNode
       info->symb_cat : the var/define/constant kind of the symbols list
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_SYMBOL:
    /* Available information:
       info->hrcNode
       info->n1.name : name of the symbol
       info->n2.type : type of variable (for vars)
       info->n2.body : body of define (for defines)
       info->symb_cat : the var/define/constant kind of the symbol
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_ASSIGNS:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_ASSIGN_INIT:
    /* Available information:
       info->hrcNode
       info->n1.name : name of the variable
       info->n2.expr : expression of the assignment
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_ASSIGN_INVAR:
    /* Available information:
       info->hrcNode
       info->n1.name : name of the variable
       info->n2.expr : expression of the assignment
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_ASSIGN_NEXT:
    /* Available information:
       info->hrcNode
       info->n1.name : name of the variable
       info->n2.expr : expression of the assignment
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_CONSTRAINTS:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_CONSTRAINT_INIT:
    /* Available information:
       info->hrcNode
       info->n1.expr : expression of the constraint
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_CONSTRAINT_INVAR:
    /* Available information:
       info->hrcNode
       info->n1.expr : expression of the constraint
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_CONSTRAINT_TRANS:
    /* Available information:
       info->hrcNode
       info->n1.expr : expression of the constraint
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_FAIRNESS:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_JUSTICE:
    /* Available information:
       info->hrcNode
       info->n1.expr : expression of the justice
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_COMPASSION:
    /* Available information:
       info->hrcNode
       info->n1.expr : first expression of the compassion
       info->n2.expr : second expression of the compassion
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_SPECS:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_SPEC:
    /* Available information:
       info->hrcNode
       info->spec_type : PropType of the specification
       info->n1.name : name of the spec (or Nil)
       info->n2.expr : expression of the specification
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_COMPILER_INFO:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_LIST_SYNTAX_ERRORS:
    /* Available information:
       info->hrcNode
       info->list_is_empty
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  case HDS_ERROR:
    /* Available information:
       info->hrcNode
       info->error.filename
       info->error.lineno
       info->error.message
       info->error.token
       info->last_in_list
    */
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
    }
    break;

  default:
    internal_error("Unexpected node %d", snippet);
  }
}


/**Function********************************************************************

  Synopsis           [Dumps a comment]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_dump_comment(HrcDumper_ptr self, const char* msg)
{
  error_unreachable_code(), "TBI";
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_dump_header(HrcDumper_ptr self, const char* msg)
{
  error_unreachable_code(), "TBI";
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_dump_indent(HrcDumper_ptr self)
{
  if (self->use_indentation & self->indent_pending) {
    const size_t spaces = self->indent * self->indent_size;
    size_t i;
    for (i=0; i<spaces; ++i) {
      fprintf(self->fout, " ");
    }
    self->indent_pending = false;
  }
}


/**Function********************************************************************

  Synopsis           [Implements indentation of a newline]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_nl(HrcDumper_ptr self)
{
  fprintf(self->fout, "\n");
  self->indent_pending = true;
}


/**Function********************************************************************

  Synopsis           [Prints the type of a variable.]

  Description        [Prints the type of a variable. The printers used
  in compileWrite.c in compile package cannot be used in hrc, unless
  symbol table is used.

  The printer manages the following types: BOOLEAN, INTEGER, REAL,
  UNSIGNED_WORD, SIGNED_WORD, SCALAR, WORD_ARRAY and  ARRAY_TYPE.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_dump_var_type(HrcDumper_ptr self, node_ptr node)
{
  int node_type = node_get_type(node);

  switch (node_type) {
  case BOOLEAN:
  case TWODOTS: /* range */
    _HRC_DUMP_NODE(node);
    break;

  case INTEGER:
    _HRC_DUMP_STR("integer");
    break;

  case REAL:
    _HRC_DUMP_STR("real");
    break;

  case SIGNED_WORD:
    _HRC_DUMP_STR("signed word[");
    _HRC_DUMP_NODE(car(node));
    _HRC_DUMP_STR("]");
    break;

  case UNSIGNED_WORD:
    _HRC_DUMP_STR("unsigned word[");
    _HRC_DUMP_NODE(car(node));
    _HRC_DUMP_STR("]");
    break;

  case SCALAR:
    hrc_dumper_dump_scalar_type(self, node);
    break;

  case WORDARRAY:
    _HRC_DUMP_STR("array word[");
    _HRC_DUMP_NODE(car(node));
    _HRC_DUMP_STR("]");

    _HRC_DUMP_STR(" of word[");
    _HRC_DUMP_NODE(cdr(node));
    _HRC_DUMP_STR("]");
    break;

  case ARRAY_TYPE:
    _HRC_DUMP_STR("array ");

    /* Prints subrange of array n..m */
    _HRC_DUMP_NODE(car(node));

    _HRC_DUMP_STR(" of ");

    /* recursively prints the array type */
    hrc_dumper_dump_var_type(self, cdr(node));
    break;

  default:
    internal_error("Type %d not supported by hrc emitter.\n", node_type);
  }

  return;
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The HrcDumper class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void hrc_dumper_finalize(Object_ptr object, void* dummy) 
{
  HrcDumper_ptr self = HRC_DUMPER(object);

  hrc_dumper_deinit(self);
  FREE(self);
}


/**Function********************************************************************

  Synopsis           [Dumps the scalar type of a variable.]

  Description        [Dumps the scalar type of a variable. The
  printer takes care of reversing the CONS list that contains the
  enumeration to preserve the order of the literals in the source
  model.]

  SideEffects        []

  SeeAlso            [hrc_dumper_dump_var_type]

******************************************************************************/
static void hrc_dumper_dump_scalar_type(HrcDumper_ptr self, node_ptr node)
{
  int node_type;
  node_ptr reversed_literals;
  node_ptr iterator;
  boolean first_literal;

  node_type = node_get_type(node);
  nusmv_assert(SCALAR == node_type);

  _HRC_DUMP_STR("{");

  /* reverse the literals of the enumerations to preserve their
     original order */
  reversed_literals = reverse_ns(car(node));

  iterator = reversed_literals;
  first_literal = true;
  while (Nil != iterator) {
    node_ptr literal;

    literal = car(iterator);
    nusmv_assert(Nil != literal);

    if (! first_literal) {
      _HRC_DUMP_STR(", ");
    }
    _HRC_DUMP_NODE(literal);

    first_literal = false;
    iterator = cdr(iterator);
  }

  _HRC_DUMP_STR("}");

  free_list(reversed_literals);
}



/**AutomaticEnd***************************************************************/
