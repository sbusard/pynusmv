/**CHeaderFile*****************************************************************

  FileName    [TraceOpt.h]

  PackageName [trace]

  Synopsis    [Public interface of class 'TraceOpt']

  Description []

  SeeAlso     [TraceOpt.c]

  Author      [Alessandro Mariotti]

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

  Revision    [$Id: TraceOpt.h,v 1.1.2.3 2010-03-04 16:58:56 nusmv Exp $]

******************************************************************************/
#ifndef __TRACE_OPT_H__
#define __TRACE_OPT_H__

#include "utils/utils.h"
#include "opt/opt.h"

/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class TraceOpt]

  Description []

******************************************************************************/
typedef struct TraceOpt_TAG*  TraceOpt_ptr;


/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class TraceOpt]

  Description [These macros must be used respectively to cast and to check
  instances of class TraceOpt]

******************************************************************************/
#define TRACE_OPT(self) \
         ((TraceOpt_ptr) self)

#define TRACE_OPT_CHECK_INSTANCE(self) \
         (nusmv_assert(TRACE_OPT(self) != TRACE_OPT(NULL)))



/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN TraceOpt_ptr TraceOpt_create ARGS((void));
EXTERN TraceOpt_ptr TraceOpt_create_from_env ARGS((OptsHandler_ptr opt));

EXTERN void TraceOpt_update_from_env ARGS((TraceOpt_ptr self,
                                           OptsHandler_ptr opt));

EXTERN void TraceOpt_destroy ARGS((TraceOpt_ptr self));

EXTERN boolean TraceOpt_obfuscate ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_obfuscate ARGS((TraceOpt_ptr self, boolean obfuscate));

EXTERN boolean TraceOpt_show_defines ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_show_defines ARGS((TraceOpt_ptr self,
                                            boolean show_defines));

EXTERN boolean TraceOpt_show_defines_with_next ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_show_defines_with_next ARGS((TraceOpt_ptr self,
                                                      boolean show_next));

EXTERN boolean TraceOpt_eval_defines ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_eval_defines ARGS((TraceOpt_ptr self,
                                            boolean eval_defines));

EXTERN const char* TraceOpt_hiding_prefix ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_hiding_prefix ARGS((TraceOpt_ptr self,
                                             const char* hiding_prefix));

#if NUSMV_HAVE_REGEX_H
EXTERN regex_t* TraceOpt_regexp ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_regexp ARGS((TraceOpt_ptr self,
                                      regex_t* regexp));
#endif

EXTERN
boolean TraceOpt_xml_reader_halts_on_undefined_symbols ARGS((TraceOpt_ptr self));
EXTERN void
TraceOpt_set_xml_reader_halts_on_undefined_symbols ARGS((TraceOpt_ptr self,
                                                         boolean halt));
EXTERN
boolean TraceOpt_xml_reader_halts_on_wrong_section ARGS((TraceOpt_ptr self));
EXTERN void
TraceOpt_set_xml_reader_halts_on_wrong_section ARGS((TraceOpt_ptr self,
                                                     boolean halt));

EXTERN unsigned TraceOpt_from_here ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_from_here ARGS((TraceOpt_ptr self, unsigned index));

EXTERN unsigned TraceOpt_to_here ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_to_here ARGS((TraceOpt_ptr self, unsigned index));

EXTERN FILE* TraceOpt_output_stream ARGS((TraceOpt_ptr self));
EXTERN void TraceOpt_set_output_stream ARGS((TraceOpt_ptr self, FILE* out));

/**AutomaticEnd***************************************************************/



#endif /* __TRACE_OPT_H__ */
