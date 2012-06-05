/**CHeaderFile*****************************************************************

  FileName    [HrcDumper_private.h]

  PackageName [hrc.dumpers]

  Synopsis    [Private and protected interface of class 'HrcDumper']

  Description [This file can be included only by derived and friend classes]

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


#ifndef __HRC_DUMPER_PRIVATE_H__
#define __HRC_DUMPER_PRIVATE_H__


#include "HrcDumper.h" 

#include "node/node.h"
#include "utils/object.h" 
#include "utils/object_private.h"
#include "utils/utils.h" 


/**Struct**********************************************************************

  Synopsis    [HrcDumper class definition derived from
               class Object]

  Description []

  SeeAlso     [Base class Object]   
  
******************************************************************************/
typedef struct HrcDumper_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(Object); 

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  FILE* fout;
  boolean use_indentation;
  int indent;
  size_t indent_size;
  boolean indent_pending; /* used to control indentation */
  unsigned int columns; 
  boolean use_mod_suffix;

  /* -------------------------------------------------- */
  /*                  Virtual methods                   */
  /* -------------------------------------------------- */
  void (*dump_snippet)(HrcDumper_ptr self,
                       HrcDumperSnippet snippet,
                       const HrcDumperInfo* info);

  void (*dump_comment)(HrcDumper_ptr self,
                       const char* msg);

  void (*dump_header)(HrcDumper_ptr self,
                      const char* msg);

} HrcDumper;


/* ---------------------------------------------------------------------- */
/* Macros                                                                 */
/* ---------------------------------------------------------------------- */

#define HRC_DEFAULT_COLUMNS 79
#define HRC_MODULE_SUFFIX "_hrc"


#define _HRC_DUMP_STR(x)          \
  {                               \
    hrc_dumper_dump_indent(self); \
    fprintf(self->fout, x);       \
  }

#define _HRC_DUMP_STR_NL(x)                                           \
  {                                                                   \
    hrc_dumper_dump_indent(self);                                     \
    fprintf(self->fout, x);                                           \
    hrc_dumper_nl(self);                                              \
  }

#define _HRC_DUMP_NL()                                                \
  {                                                                   \
    hrc_dumper_nl(self);                                              \
  }

#define _HRC_DUMP_NODE(x)         \
  {                               \
    hrc_dumper_dump_indent(self); \
    print_node(self->fout, x);    \
  }

#define _HRC_DUMP_COMMENT(x)      \
  {                               \
    self->dump_comment(self, x);  \
  }

#define _HRC_DUMP_HEADER(x)       \
  {                               \
    self->dump_header(self, x);   \
  }



/* ---------------------------------------------------------------------- */
/* Private methods to be used by derivated and friend classes only         */
/* ---------------------------------------------------------------------- */
EXTERN void hrc_dumper_init ARGS((HrcDumper_ptr self, FILE* fout));
EXTERN void hrc_dumper_deinit ARGS((HrcDumper_ptr self));

EXTERN void hrc_dumper_dump_snippet ARGS((HrcDumper_ptr self,
                                          HrcDumperSnippet snippet,
                                          const HrcDumperInfo* info));

EXTERN void hrc_dumper_dump_comment ARGS((HrcDumper_ptr self, 
                                          const char* msg));

EXTERN void hrc_dumper_dump_header ARGS((HrcDumper_ptr self, const char* msg));

EXTERN void hrc_dumper_dump_indent ARGS((HrcDumper_ptr self));
EXTERN void hrc_dumper_nl ARGS((HrcDumper_ptr self));

EXTERN void hrc_dumper_dump_var_type ARGS((HrcDumper_ptr self, node_ptr node));

#endif /* __HRC_DUMPER_PRIVATE_H__ */
