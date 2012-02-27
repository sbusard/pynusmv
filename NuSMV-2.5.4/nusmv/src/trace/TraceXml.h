/**CHeaderFile*****************************************************************

  FileName    [TraceXml.h]

  PackageName [trace]

  Synopsis    [The Trace xml header]

  Description []

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
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
#ifndef __TRACE_XML__H
#define __TRACE_XML__H

#include "utils/utils.h"
/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/**Struct**********************************************************************

  Synopsis    [Numeric values for all possible tags that can occur in the xml
               representation]

  Description []

  SeeAlso     []

******************************************************************************/
typedef enum TraceXmlTag_TAG
{
  TRACE_XML_INVALID_TAG = -1,
  TRACE_XML_CNTX_TAG    =  0,
  TRACE_XML_NODE_TAG,
  TRACE_XML_STATE_TAG,
  TRACE_XML_COMB_TAG,
  TRACE_XML_INPUT_TAG,
  TRACE_XML_VALUE_TAG,
  TRACE_XML_LOOPS_TAG
} TraceXmlTag;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

#define TRACE_XML_CNTX_TAG_STRING   "counter-example"
#define TRACE_XML_NODE_TAG_STRING   "node"
#define TRACE_XML_STATE_TAG_STRING  "state"
#define TRACE_XML_COMB_TAG_STRING   "combinatorial"
#define TRACE_XML_INPUT_TAG_STRING  "input"
#define TRACE_XML_VALUE_TAG_STRING  "value"
#define TRACE_XML_LOOPS_TAG_STRING  "loops"


/**AutomaticStart*************************************************************/

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

TraceXmlTag TraceXmlTag_from_string ARGS((const char* tag)); 
const char* TraceXmlTag_to_string ARGS((TraceXmlTag tag));

#endif /* __TRACE_XML__H */
