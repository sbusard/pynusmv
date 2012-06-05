/**CHeaderFile*****************************************************************

  FileName    [TraceXmlDumper_private.h]

  PackageName [trace.plugins]

  Synopsis    [The private header file for the TraceXmldumper class.]

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
#ifndef __TRACE_XMLDUMPER_PRIVATE__H
#define __TRACE_XMLDUMPER_PRIVATE__H

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "pkg_traceInt.h"
#include "TracePlugin_private.h"

#include "TraceXmlDumper.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [This is a plugin that dumps the XML representation of a trace]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct TraceXmlDumper_TAG
{
  INHERITS_FROM(TracePlugin);

} TraceXmlDumper;

typedef struct XmlNodes_TAG* XmlNodes_ptr;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void trace_xmldumper_init ARGS((TraceXmlDumper_ptr self));

void trace_xmldumper_deinit ARGS((TraceXmlDumper_ptr self));

int trace_xmldumper_action ARGS((const TracePlugin_ptr plugin));

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/
void trace_xml_dumper_init ARGS((TraceXmlDumper_ptr self));

int trace_xml_dumper_action ARGS((TracePlugin_ptr plugin));

#endif /* __TRACE_XML_DUMPER_PRIVATE__H */

