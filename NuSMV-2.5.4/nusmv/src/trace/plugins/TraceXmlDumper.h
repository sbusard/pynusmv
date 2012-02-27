/**CHeaderFile*****************************************************************

  FileName    [TraceXmlDumper.h]

  PackageName [trace.plugins]

  Synopsis    [The header file for the TraceXmlDumper class.]

  Description []

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace.plugins'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK.

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
#ifndef __TRACE_XML_DUMPER__H
#define __TRACE_XML_DUMPER__H

#include "TracePlugin.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef struct TraceXmlDumper_TAG* TraceXmlDumper_ptr;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define TRACE_XML_DUMPER(x) \
         ((TraceXmlDumper_ptr) x)

#define TRACE_XML_DUMPER_CHECK_INSTANCE(x) \
         (nusmv_assert(TRACE_XML_DUMPER(x) != TRACE_XML_DUMPER(NULL)))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN TraceXmlDumper_ptr TraceXmlDumper_create ARGS((void));

/**AutomaticEnd***************************************************************/

#endif /* __TRACE_XML_DUMPER__H */

