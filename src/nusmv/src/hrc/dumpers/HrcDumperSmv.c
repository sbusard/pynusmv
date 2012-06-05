/**CFile***********************************************************************

  FileName    [HrcDumperSmv.c]

  PackageName [hrc.dumpers]

  Synopsis    [Implementation of class 'HrcDumperSmv']

  Description []

  SeeAlso     [HrcDumperSmv.h]

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

#include "HrcDumperSmv.h" 
#include "HrcDumperSmv_private.h" 

#include "utils/utils.h" 
#include "utils/error.h" 

static char rcsid[] UTIL_UNUSED = "$Id: $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

const char* HRC_DUMPER_SMV_COMMENT_PREFIX = "-- ";
const size_t HRC_DUMPER_SMV_COMMENT_PREFIX_LEN = 3;

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
/* See 'HrcDumperSmv_private.h' for class 'HrcDumperSmv' definition. */ 

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void hrc_dumper_smv_finalize ARGS((Object_ptr object, void* dummy));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The HrcDumperSmv class constructor]

  Description        [The HrcDumperSmv class constructor]

  SideEffects        []

  SeeAlso            [HrcDumper_destroy]   
  
******************************************************************************/
HrcDumperSmv_ptr HrcDumperSmv_create(FILE* fout)
{
  HrcDumperSmv_ptr self = ALLOC(HrcDumperSmv, 1);
  HRC_DUMPER_SMV_CHECK_INSTANCE(self);

  hrc_dumper_smv_init(self, fout);
  return self;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The HrcDumperSmv class private initializer]

  Description        [The HrcDumperSmv class private initializer]

  SideEffects        []

  SeeAlso            [HrcDumperSmv_create]   
  
******************************************************************************/
void hrc_dumper_smv_init(HrcDumperSmv_ptr self, FILE* fout)
{
  /* base class initialization */
  hrc_dumper_init(HRC_DUMPER(self), fout);
  
  /* members initialization */

  /* virtual methods settings */  
  OVERRIDE(Object, finalize) = hrc_dumper_smv_finalize;
  OVERRIDE(HrcDumper, dump_snippet) = hrc_dumper_smv_dump_snippet;
  OVERRIDE(HrcDumper, dump_comment) = hrc_dumper_smv_dump_comment;
  OVERRIDE(HrcDumper, dump_header) = hrc_dumper_smv_dump_header; 
}


/**Function********************************************************************

  Synopsis           [The HrcDumperSmv class private deinitializer]

  Description        [The HrcDumperSmv class private deinitializer]

  SideEffects        []

  SeeAlso            [HrcDumper_destroy]   
  
******************************************************************************/
void hrc_dumper_smv_deinit(HrcDumperSmv_ptr self)
{
  /* members deinitialization */

  /* base class deinitialization */
  hrc_dumper_deinit(HRC_DUMPER(self));
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_smv_dump_snippet(HrcDumper_ptr self,
                                 HrcDumperSnippet snippet,
                                 const HrcDumperInfo* info)
{
  HRC_DUMPER_CHECK_INSTANCE(self);

  switch (snippet) {
  case HDS_HRC_TOP:
    break;

  case HDS_LIST_MODS:
    break;

  case HDS_MOD:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_HEADER("");
      _HRC_DUMP_STR("MODULE ");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_HEADER("End of module")
    }
    break;

  case HDS_MOD_NAME:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_NODE(info->n1.name);
      if (self->use_mod_suffix && 
          /* top level name must not be changed */
          HRC_NODE(NULL) != HrcNode_get_parent(info->hrcNode)) {
        _HRC_DUMP_STR(HRC_MODULE_SUFFIX);
      }
    }
    break;

  case HDS_LIST_MOD_FORMAL_PARAMS:    
    if (! info->list_is_empty) {
      if (info->stage & HRC_STAGE_BEGIN) { 
        _HRC_DUMP_STR("(");
      }
      if (info->stage & HRC_STAGE_END) { 
        _HRC_DUMP_STR(")");
      }
    }
    if (info->stage & HRC_STAGE_END) { 
      _HRC_DUMP_NL();
    }
    break;

  case HDS_MOD_FORMAL_PARAM:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_NODE(info->n1.name);
      nusmv_assert(Nil == info->n2.type); /* type not supported yet! */

      if (! info->last_in_list) { _HRC_DUMP_STR(", "); }
    }
    break;

  case HDS_LIST_MOD_INSTANCES:
    if (info->list_is_empty) break;
    if (info->stage & HRC_STAGE_BEGIN) {
      switch (info->symb_cat) {
      case SYMBOL_STATE_VAR: _HRC_DUMP_STR_NL("VAR"); break;
      case SYMBOL_INPUT_VAR: _HRC_DUMP_STR_NL("IVAR"); break;
      case SYMBOL_FROZEN_VAR: _HRC_DUMP_STR_NL("FROZENVAR"); break;
      default: internal_error("Unexpected type of list of mod instances");
      }
    }
    break;

  case HDS_MOD_INSTANCE:
    if (info->stage & HRC_STAGE_BEGIN) {
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_MOD_INSTANCE_VARNAME:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_NODE(info->n1.name);
    }
    break;

  case HDS_MOD_INSTANCE_MODNAME:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR(" : "); 
      _HRC_DUMP_NODE(info->n1.name);
      if (self->use_mod_suffix) {
        _HRC_DUMP_STR(HRC_MODULE_SUFFIX);
      }
    }
    break;

  case HDS_LIST_MOD_INSTANCE_ACTUAL_PARAMS:
    if (!info->list_is_empty) {
      _HRC_DUMP_STR((info->stage & HRC_STAGE_BEGIN) ? "(" : ")"); 
    }
    break;

  case HDS_MOD_INSTANCE_ACTUAL_PARAM:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_NODE(info->n1.value);
      if (!info->last_in_list) { _HRC_DUMP_STR(", "); }
    }
    break;

  case HDS_LIST_SYMBOLS:
    if (info->list_is_empty) break;

    if (info->stage & HRC_STAGE_BEGIN) {
      switch (info->symb_cat) {
      case SYMBOL_STATE_VAR: _HRC_DUMP_STR_NL("VAR"); break;
      case SYMBOL_INPUT_VAR: _HRC_DUMP_STR_NL("IVAR"); break;
      case SYMBOL_FROZEN_VAR: _HRC_DUMP_STR_NL("FROZENVAR"); break;
      case SYMBOL_CONSTANT: _HRC_DUMP_STR_NL("CONSTANTS"); break;
      case SYMBOL_DEFINE: _HRC_DUMP_STR_NL("DEFINE"); break;
      default: internal_error("Unexpected type of list of symbols");
      }
    }
    if (info->stage & HRC_STAGE_END) {
      switch (info->symb_cat) {
      case SYMBOL_CONSTANT: _HRC_DUMP_STR_NL(";"); break;
      default: break;
      }
    }
    break;

  case HDS_SYMBOL:
    if (info->stage & HRC_STAGE_BEGIN) {
      switch (info->symb_cat) {
      case SYMBOL_STATE_VAR:
      case SYMBOL_INPUT_VAR:
      case SYMBOL_FROZEN_VAR:
        _HRC_DUMP_NODE(info->n1.name);
        _HRC_DUMP_STR(" : ");
        hrc_dumper_dump_var_type(self, info->n2.type);
        break;

      case SYMBOL_CONSTANT:
        _HRC_DUMP_NODE(info->n1.name);
        break;

      case SYMBOL_DEFINE:
        _HRC_DUMP_NODE(info->n1.name);
        _HRC_DUMP_STR(" := ");
        _HRC_DUMP_NODE(info->n2.body);
        break;

      default: internal_error("Unexpected symbol type");
      }
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break; /* end of case HDS_SYMBOL */

  case HDS_LIST_ASSIGNS:
    if (info->list_is_empty) break;
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR_NL("ASSIGN");
    }
    break;

  case HDS_ASSIGN_INIT:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("init(");
      _HRC_DUMP_NODE(info->n1.name);
      _HRC_DUMP_STR(") := ");
      _HRC_DUMP_NODE(info->n2.expr);
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_ASSIGN_INVAR:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_NODE(info->n1.name);
      _HRC_DUMP_STR(" := ");
      _HRC_DUMP_NODE(info->n2.expr);
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_ASSIGN_NEXT:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("next(");
      _HRC_DUMP_NODE(info->n1.name);
      _HRC_DUMP_STR(") := ");
      _HRC_DUMP_NODE(info->n2.expr);
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;


  case HDS_LIST_CONSTRAINTS:
    /* nothing to do for smv */
    break;

  case HDS_CONSTRAINT_INIT:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("INIT ");
      _HRC_DUMP_NODE(info->n1.expr);
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_CONSTRAINT_INVAR:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("INVAR ");
      _HRC_DUMP_NODE(info->n1.expr);
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_CONSTRAINT_TRANS:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("TRANS ");
      _HRC_DUMP_NODE(info->n1.expr);
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_LIST_FAIRNESS:
    /* nothing to do for smv */
    break;

  case HDS_JUSTICE:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("JUSTICE ");
      _HRC_DUMP_NODE(info->n1.expr);
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_COMPASSION:
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_STR("COMPASSION ");
      _HRC_DUMP_STR("(");
      _HRC_DUMP_NODE(info->n1.expr);
      _HRC_DUMP_STR(", ");
      _HRC_DUMP_NODE(info->n2.expr);
      _HRC_DUMP_STR(")");
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_LIST_SPECS:
    /* nothing to do for smv */
    break;

  case HDS_SPEC:
    if (info->stage & HRC_STAGE_BEGIN) {
      switch (info->spec_type) {
      case Prop_Ctl:
        _HRC_DUMP_STR("CTLSPEC");
        break;
      case Prop_Ltl:
        _HRC_DUMP_STR("LTLSPEC");
        break;
      case Prop_Psl:
        _HRC_DUMP_STR("PSLSPEC");
        break;
      case Prop_Invar:
        _HRC_DUMP_STR("INVARSPEC");
        break;
      case Prop_Compute:
        _HRC_DUMP_STR("COMPUTE");
        break;
      default:
        internal_error("Invalid property type");
      }

      _HRC_DUMP_STR(" ");
      if (Nil != info->n1.name) {
        _HRC_DUMP_STR("NAME ");
        _HRC_DUMP_NODE(info->n1.name);
        _HRC_DUMP_STR(" := ");
      }
      _HRC_DUMP_NODE(info->n2.expr);
    }

    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_STR_NL(";");
    }
    break;

  case HDS_LIST_COMPILER_INFO:
    break;

  case HDS_LIST_SYNTAX_ERRORS:
    if (info->list_is_empty) break;
    if (info->stage & HRC_STAGE_BEGIN) {
      _HRC_DUMP_HEADER("Syntactic Errors") 
    }
    if (info->stage & HRC_STAGE_END) {
      _HRC_DUMP_HEADER("End of Syntactic Errors")
    }
    break;

  case HDS_ERROR:
    if (info->stage & HRC_STAGE_BEGIN) {
      char buf[12];
      int c;

      _HRC_DUMP_COMMENT("");
      _HRC_DUMP_STR("File ");
      if ((const char*) NULL != info->error.filename) {
        _HRC_DUMP_STR(info->error.filename);
      } 
      else {
        _HRC_DUMP_STR("stdin");
      }
      c = snprintf(buf, 12, "%d", info->error.lineno);
      SNPRINTF_CHECK(c, 12);

      _HRC_DUMP_STR(": line ");
      _HRC_DUMP_STR(buf);
      _HRC_DUMP_STR(": ");
      if ((const char*) NULL != info->error.token) {
        _HRC_DUMP_STR("at token \"");
        _HRC_DUMP_STR(info->error.token);
        _HRC_DUMP_STR("\"");
      }
      _HRC_DUMP_STR(": ");
      _HRC_DUMP_STR_NL(info->error.message);
    }
    break;

  default:
    /* not handled here, try with the base */
    hrc_dumper_dump_snippet(HRC_DUMPER(self), snippet, info);
  }
}



/**Function********************************************************************

  Synopsis           [Dumps a comment]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_smv_dump_comment(HrcDumper_ptr self, const char* msg)
{
  _HRC_DUMP_STR(HRC_DUMPER_SMV_COMMENT_PREFIX);
  _HRC_DUMP_STR(msg);
}


/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void hrc_dumper_smv_dump_header(HrcDumper_ptr self, const char* msg)
{
  const int len = self->columns - (strlen(msg) + 
                                   HRC_DUMPER_SMV_COMMENT_PREFIX_LEN);
  int i;

  _HRC_DUMP_NL();

  /* dumps -- =============== ... */
  _HRC_DUMP_COMMENT("");
  for (i=0; i < self->columns; ++i) {
    _HRC_DUMP_STR("=");
  }
  _HRC_DUMP_NL();

  if (strlen(msg) == 0) return;

  /* dumps the message centered */
  _HRC_DUMP_COMMENT("");
  for (i=0; i < len/2 - 1; ++i) {
    _HRC_DUMP_STR(" ");
  }
  _HRC_DUMP_STR(msg);
  _HRC_DUMP_NL();

  /* dumps -- =============== ... */
  _HRC_DUMP_COMMENT("");
  for (i=0; i < self->columns; ++i) {
    _HRC_DUMP_STR("=");
  }
  _HRC_DUMP_NL();
}


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The HrcDumperSmv class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void hrc_dumper_smv_finalize(Object_ptr object, void* dummy) 
{
  HrcDumperSmv_ptr self = HRC_DUMPER_SMV(object);

  hrc_dumper_smv_deinit(self);
  FREE(self);
}



/**AutomaticEnd***************************************************************/

