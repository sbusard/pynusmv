/**CFile***********************************************************************

  FileName    [TraceXml.c]

  PackageName [trace]

  Synopsis    [Routines related to Trace xml dumpers and loaders.]

  Description []

  SeeAlso     []

  Author      [Marco Pensallorto]

  Copyright   [
  This file is part of the ``trace'' package of NuSMV version 2.
  Copyright (C) 2010 by FBK-irst.

  NuSMV version 2 is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  NuSMV version 2 is distributed in the hope that it will be
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
#if NUSMV_HAVE_CONFIG
# include "nusmv-config.h"
#endif

#include "TraceXml.h"

static char rcsid[] UTIL_UNUSED = "$Id: $";

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of external functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

Synopsis           [String to XML Tag converter.]

Description        [ Protected function that converts an string to
                     TraceXMLTag ]

SideEffects        []

SeeAlso            []

******************************************************************************/
TraceXmlTag TraceXmlTag_from_string(const char* tag)
{
  static char* tag_names[] = {
    TRACE_XML_CNTX_TAG_STRING,
    TRACE_XML_NODE_TAG_STRING,
    TRACE_XML_STATE_TAG_STRING,
    TRACE_XML_COMB_TAG_STRING,
    TRACE_XML_INPUT_TAG_STRING,
    TRACE_XML_VALUE_TAG_STRING,
    TRACE_XML_LOOPS_TAG_STRING
  };

  static TraceXmlTag tag_value[] = {
    TRACE_XML_CNTX_TAG,
    TRACE_XML_NODE_TAG,
    TRACE_XML_STATE_TAG,
    TRACE_XML_COMB_TAG,
    TRACE_XML_INPUT_TAG,
    TRACE_XML_VALUE_TAG,
    TRACE_XML_LOOPS_TAG,
    TRACE_XML_INVALID_TAG
  };

  TraceXmlTag ret_val = TRACE_XML_INVALID_TAG;
  int i;

  for (i = 0; i < sizeof(tag_names) / sizeof(tag_names[0]); i++) {
    if (strncmp(tag, tag_names[i], strlen(tag)) == 0) {
      ret_val = tag_value[i];
      break;
    }
  }

  return ret_val;
}


/**Function********************************************************************

Synopsis           [XML Tag converter to string converter.]

Description        [ Protected function that converts a TraceXMLTag to
                     a string ]

SideEffects        []

SeeAlso            []

******************************************************************************/
const char* TraceXmlTag_to_string(TraceXmlTag tag)
{
  static char* tag_names[] = {
    TRACE_XML_CNTX_TAG_STRING,
    TRACE_XML_NODE_TAG_STRING,
    TRACE_XML_STATE_TAG_STRING,
    TRACE_XML_COMB_TAG_STRING,
    TRACE_XML_INPUT_TAG_STRING,
    TRACE_XML_VALUE_TAG_STRING,
    TRACE_XML_LOOPS_TAG_STRING
  };

  return tag_names[tag];
}
