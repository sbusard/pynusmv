/**CHeaderFile*****************************************************************

  FileName    [HrcDumperXml_private.h]

  PackageName [hrc.dumpers]

  Synopsis    [Private and protected interface of class 'HrcDumperXml']

  Description [This file can be included only by derived and friend classes]

  SeeAlso     [HrcDumperXml.h]

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


#ifndef __HRC_DUMPER_XML_PRIVATE_H__
#define __HRC_DUMPER_XML_PRIVATE_H__


#include "HrcDumperXml.h" 
#include "HrcDumper.h" /* fix this */ 
#include "HrcDumper_private.h" /* fix this */ 
#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis    [HrcDumperXml class definition derived from
               class HrcDumper]

  Description []

  SeeAlso     [Base class HrcDumper]   
  
******************************************************************************/
typedef struct HrcDumperXml_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(HrcDumper); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */


  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */

} HrcDumperXml;



/* ---------------------------------------------------------------------- */
/* Constants                                                              */
/* ---------------------------------------------------------------------- */
const char* SMV_XSD_NS = "http://es.fbk.eu/xsd";


/* ---------------------------------------------------------------------- */
/* Macros                                                                 */
/* ---------------------------------------------------------------------- */
#undef _HRC_DUMP_STR_NL
#define _HRC_DUMP_STR_NL(x)                                           \
  {                                                                   \
    hrc_dumper_dump_indent(self);                                     \
    fprintf(self->fout, x);                                           \
    if (self->use_indentation) {                                      \
      hrc_dumper_nl(self);                                            \
    }                                                                 \
  }

#undef _HRC_DUMP_NL
#define _HRC_DUMP_NL()                                                \
  {                                                                   \
    if (self->use_indentation) {                                      \
      hrc_dumper_nl(self);                                            \
    }                                                                 \
  }

#define _HRC_DUMP_XML_TAG_BEGIN(t) \
  {                                \
  _HRC_DUMP_STR("<");              \
  _HRC_DUMP_STR(t);                \
  _HRC_DUMP_STR(">");              \
  }

#define _HRC_DUMP_XML_TAG_END(t)   \
  {                                \
    _HRC_DUMP_STR("</");           \
    _HRC_DUMP_STR(t);              \
    _HRC_DUMP_STR_NL(">");         \
  }

#define _HRC_DUMP_XML_TAG_BEGIN_END(t, s)       \
  {                                             \
    _HRC_DUMP_XML_TAG_BEGIN(t);                 \
    if ((char*) NULL != (char*) s) {            \
      _HRC_DUMP_STR(s);                         \
    }                                           \
    _HRC_DUMP_XML_TAG_END(t);                   \
  }

#define _HRC_DUMP_XML_NODE(n)                               \
  hrc_dumper_xml_dump_escaped_node(HRC_DUMPER_XML(self), n)

#define _HRC_DUMP_XML_NODE_BEGIN_END(t, n) \
  {                                        \
    _HRC_DUMP_STR("<");                    \
    _HRC_DUMP_STR(t);                      \
    _HRC_DUMP_STR(">");                    \
    _HRC_DUMP_XML_NODE(n);                 \
    _HRC_DUMP_STR("</");                   \
    _HRC_DUMP_STR(t);                      \
    _HRC_DUMP_STR_NL(">");                 \
  }


/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void hrc_dumper_xml_init ARGS((HrcDumperXml_ptr self, FILE* fout));
EXTERN void hrc_dumper_xml_deinit ARGS((HrcDumperXml_ptr self));

EXTERN void hrc_dumper_xml_dump_snippet ARGS((HrcDumper_ptr self,
                                              HrcDumperSnippet snippet,
                                              const HrcDumperInfo* info));

EXTERN void hrc_dumper_xml_dump_comment ARGS((HrcDumper_ptr self, 
                                              const char* msg));


#endif /* __HRC_DUMPER_XML_PRIVATE_H__ */
