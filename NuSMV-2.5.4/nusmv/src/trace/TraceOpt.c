/**CFile***********************************************************************

  FileName    [TraceOpt.c]

  PackageName [trace]

  Synopsis    [Implementation of class 'TraceOpt']

  Description []

  SeeAlso     [TraceOpt.h]

  Author      [Alessandro Mariotti, Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

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

  Revision    [$Id: TraceOpt.c,v 1.1.2.3 2010-03-04 16:58:56 nusmv Exp $]

******************************************************************************/

#include "TraceOpt.h"
#include "opt/opt.h"
#include "utils/utils.h"
#include "trace/pkg_traceInt.h"

static char rcsid[] UTIL_UNUSED = "$Id: TraceOpt.c,v 1.1.2.3 2010-03-04 16:58:56 nusmv Exp $";


/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [TraceOpt class definition]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct TraceOpt_TAG
{
  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  boolean obfuscate;
  boolean show_defines;
  boolean show_defines_with_next;

  unsigned from_here;
  unsigned to_here;

  FILE* output_stream;
  char* hiding_prefix;

#if NUSMV_HAVE_REGEX_H
  regex_t* regexp;
#endif
} TraceOpt;



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

static void trace_opt_init ARGS((TraceOpt_ptr self));
static void trace_opt_deinit ARGS((TraceOpt_ptr self));


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The TraceOpt class constructor]

  Description        [The TraceOpt class constructor]

  SideEffects        []

  SeeAlso            [TraceOpt_destroy]

******************************************************************************/
TraceOpt_ptr TraceOpt_create()
{
  TraceOpt_ptr self = ALLOC(TraceOpt, 1);
  TRACE_OPT_CHECK_INSTANCE(self);

  trace_opt_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt class constructor]

  Description        [The TraceOpt class constructor]

  SideEffects        []

  SeeAlso            [TraceOpt_destroy]

******************************************************************************/
TraceOpt_ptr TraceOpt_create_from_env(OptsHandler_ptr env)
{
  TraceOpt_ptr self = TraceOpt_create();
  TRACE_OPT_CHECK_INSTANCE(self);

  TraceOpt_update_from_env(self, env);
  return self;
}


/**Function********************************************************************

  Synopsis           [Updates trace options struct with current values in env]

  Description        [Updates trace options struct with current values in env]

  SideEffects        []

  SeeAlso            [TraceOpt_destroy]

******************************************************************************/
void TraceOpt_update_from_env(TraceOpt_ptr self, OptsHandler_ptr env)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  self->output_stream = NIL(FILE);
  self->show_defines = opt_show_defines_in_traces(env);
  self->show_defines_with_next = opt_backward_comp(env) ?
    false : opt_show_defines_with_next(env);
  self->hiding_prefix = (char*)opt_traces_hiding_prefix(env);

#if NUSMV_HAVE_REGEX_H
    {
      const char* pattern = opt_traces_regexp(env);

      /* free previous regexp contents if any */
      if ((regex_t *)(NULL) != self->regexp) {
        regfree(self->regexp); FREE(self->regexp);
        self->regexp = (regex_t*)(NULL);
      }

      /* replace regexp if any has been defined */
      if (NIL(char) != pattern) {
        self->regexp = ALLOC(regex_t, 1);
        if (0 != regcomp(self->regexp, pattern, REG_EXTENDED|REG_NOSUB)) {
          internal_error("%s:%d:%s: processing regular expression: %s",
                         __FILE__, __LINE__, __func__, pattern);
        }
      }
    }
#endif
}


/**Function********************************************************************

  Synopsis           [The TraceOpt class destructor]

  Description        [The TraceOpt class destructor]

  SideEffects        []

  SeeAlso            [TraceOpt_create]

******************************************************************************/
void TraceOpt_destroy(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);

  trace_opt_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [The TraceOpt obfuscate field getter]

  Description        [The TraceOpt obfuscate field getter]

  SideEffects        []

  SeeAlso            [TraceOpt_set_obfuscate]

******************************************************************************/
boolean TraceOpt_obfuscate(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  return self->obfuscate;
}

/**Function********************************************************************

  Synopsis           [The TraceOpt obfuscate field setter]

  Description        [The TraceOpt obfuscate field setter]

  SideEffects        []

  SeeAlso            [TraceOpt_obfuscate]

******************************************************************************/
void TraceOpt_set_obfuscate(TraceOpt_ptr self, boolean obfuscate)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  self->obfuscate = obfuscate;
}

/**Function********************************************************************

  Synopsis           [The TraceOpt from_here field getter]

  Description        [The TraceOpt from_here field getter]

  SideEffects        []

  SeeAlso            [TraceOpt_set_from_here]

******************************************************************************/
unsigned TraceOpt_from_here(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  return self->from_here;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt from_here field setter]

  Description        [The TraceOpt from_here field setter]

  SideEffects        []

  SeeAlso            [TraceOpt_from_here]

******************************************************************************/
void TraceOpt_set_from_here(TraceOpt_ptr self, unsigned index)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  self->from_here = index;
}

/**Function********************************************************************

  Synopsis           [The TraceOpt to_here field getter]

  Description        [The TraceOpt to_here field getter]

  SideEffects        []

  SeeAlso            [TraceOpt_set_to_here]

******************************************************************************/
unsigned TraceOpt_to_here(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  return self->to_here;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt to_here field setter]

  Description        [The TraceOpt to_here field setter]

  SideEffects        []

  SeeAlso            [TraceOpt_to_here]

******************************************************************************/
void TraceOpt_set_to_here(TraceOpt_ptr self, unsigned index)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  self->to_here = index;
}

/**Function********************************************************************

  Synopsis           [The TraceOpt output_stream field getter]

  Description        [The TraceOpt output_stream field getter]

  SideEffects        []

  SeeAlso            [TraceOpt_set_output_stream]

******************************************************************************/
FILE* TraceOpt_output_stream(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  return (NIL(FILE) != self->output_stream) \
    ? self->output_stream : nusmv_stdout;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt output_stream field setter]

  Description        [The TraceOpt output_stream field setter]

  SideEffects        []

  SeeAlso            [TraceOpt_output_stream]

******************************************************************************/
void TraceOpt_set_output_stream(TraceOpt_ptr self, FILE* out)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  self->output_stream = out;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt show_defines field getter]

  Description        [The TraceOpt show_defines field getter]

  SideEffects        []

  SeeAlso            [TraceOpt_set_show_defines]

******************************************************************************/
boolean TraceOpt_show_defines(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  return self->show_defines;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt show_defines field setter]

  Description        [The TraceOpt show_defines field setter]

  SideEffects        []

  SeeAlso            [TraceOpt_show_defines]

******************************************************************************/
void TraceOpt_set_show_defines(TraceOpt_ptr self, boolean show_defines)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  self->show_defines = show_defines;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt show_defines_with_next field getter]

  Description        [The TraceOpt show_defines_with_next field getter]

  SideEffects        []

  SeeAlso            [TraceOpt_set_show_defines_with_next]

******************************************************************************/
boolean TraceOpt_show_defines_with_next(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  return self->show_defines_with_next;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt show_defines_with_next field setter]

  Description        [The TraceOpt show_defines_with_next field setter]

  SideEffects        []

  SeeAlso            [TraceOpt_show_defines_with_next]

******************************************************************************/
void TraceOpt_set_show_defines_with_next(TraceOpt_ptr self, boolean show_defines_with_next)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  self->show_defines_with_next = show_defines_with_next;
}


/**Function********************************************************************

  Synopsis           [The TraceOpt hiding_prefix field getter]

  Description        [The TraceOpt hiding_prefix field getter]

  SideEffects        []

  SeeAlso            [TraceOpt_set_hiding_prefix]

******************************************************************************/
const char* TraceOpt_hiding_prefix(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  return self->hiding_prefix;
}

/**Function********************************************************************

  Synopsis           [The TraceOpt hiding_prefix field setter]

  Description        [The TraceOpt hiding_prefix field setter]

  SideEffects        []

  SeeAlso            [TraceOpt_hiding_prefix]

******************************************************************************/
void TraceOpt_set_hiding_prefix(TraceOpt_ptr self, const char* hiding_prefix)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  self->hiding_prefix = util_strsav(hiding_prefix);
}


#if NUSMV_HAVE_REGEX_H
/**Function********************************************************************

  Synopsis           [The TraceOpt regexp field getter]

  Description        [The TraceOpt regexp field getter]

  SideEffects        []

  SeeAlso            [TraceOpt_regexp]

******************************************************************************/
regex_t* TraceOpt_regexp(TraceOpt_ptr self)
{
  TRACE_OPT_CHECK_INSTANCE(self);
  return self->regexp;
}

#endif

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The TraceOpt class private initializer]

  Description        [The TraceOpt class private initializer]

  SideEffects        []

  SeeAlso            [TraceOpt_create]

******************************************************************************/
static void trace_opt_init(TraceOpt_ptr self)
{
  /* members initialization */
  self->obfuscate = false;
  self->show_defines = true;
  self->show_defines_with_next = true;

  self->from_here = 0;
  self->to_here = 0;

  self->output_stream = NIL(FILE);
  self->hiding_prefix = (char*)NULL;

#if NUSMV_HAVE_REGEX_H
  self->regexp = (regex_t*)NULL;
#endif
}


/**Function********************************************************************

  Synopsis           [The TraceOpt class private deinitializer]

  Description        [The TraceOpt class private deinitializer]

  SideEffects        []

  SeeAlso            [TraceOpt_destroy]

******************************************************************************/
static void trace_opt_deinit(TraceOpt_ptr self)
{
  /* members deinitialization */
  if ((char*)NULL == self->hiding_prefix) {
    FREE(self->hiding_prefix);
  }
}



/**AutomaticEnd***************************************************************/

