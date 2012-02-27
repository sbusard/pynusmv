/**CHeaderFile*****************************************************************

  FileName    [MasterPrinter.h]

  PackageName [node.printers]

  Synopsis    [Public interface of class 'MasterPrinter']

  Description []

  SeeAlso     [MasterPrinter.c]

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``node.printers'' package of NuSMV version 2.
  Copyright (C) 2006 by FBK-irst.

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

  Revision    [$Id: MasterPrinter.h,v 1.1.2.4.6.1 2009-03-23 18:13:22 nusmv Exp $]

******************************************************************************/


#ifndef __MASTER_PRINTER_H__
#define __MASTER_PRINTER_H__

#include "node/node.h"
#include "node/MasterNodeWalker.h"

#include "utils/utils.h"


/**Type***********************************************************************

  Synopsis    [Definition of the public accessor for class MasterPrinter]

  Description []

******************************************************************************/
typedef struct MasterPrinter_TAG*  MasterPrinter_ptr;



/**Macros**********************************************************************

  Synopsis    [To cast and check instances of class MasterPrinter]

  Description [These macros must be used respectively to cast and to check
  instances of class MasterPrinter]

******************************************************************************/
#define MASTER_PRINTER(self) \
         ((MasterPrinter_ptr) self)

#define MASTER_PRINTER_CHECK_INSTANCE(self) \
         (nusmv_assert(MASTER_PRINTER(self) != MASTER_PRINTER(NULL)))


/**Type***********************************************************************

  Synopsis    [Definition of enumeration StreamType]

  Description [Values taken from this enum are used to set the stream
  type to be used by the MasterPrinter when producing a printing output]

******************************************************************************/
typedef enum StreamType_TAG {
  STREAM_TYPE_DEFAULT,  /* the default stream type (STREAM_TYPE_STDOUT) */
  STREAM_TYPE_STDOUT,
  STREAM_TYPE_STDERR,
  STREAM_TYPE_STRING,
  STREAM_TYPE_FILE,     /* This requires a parameter */
  STREAM_TYPE_FUNCTION  /* This requires a parameter */
} StreamType;



/**Type***********************************************************************

  Synopsis    [Function pointer for STREAM_TYPE_FUNCTION type]

  Description [When STREAM_TYPE_FUNCTION is set as stream type, the
  argument must be a function pointer whose prototype is defined by
  StreamTypeFunction_ptr

  NOTE: The argument 'arg' is a generic argument useful for passing
  information in a reentrant way]

******************************************************************************/
typedef int (*StreamTypeFunction_ptr)(const char* str, void* arg);


/**Type***********************************************************************

  Synopsis    [Definition of enumeration StreamType]

  Description [Values taken from this enum are used to set the stream
  type to be used by the MasterPrinter when producing a printing output]

******************************************************************************/
typedef union StreamTypeArg_TAG
{
  /* for STREAM_TYPE_FILE */
  FILE* file;

  /* for STREAM_TYPE_FUNCTION */
  struct {
    /* The function pointer */
    StreamTypeFunction_ptr func_ptr;
    /* The argument to pass to each function call */
    void* argument;
  } function;

} StreamTypeArg;



#define STREAM_TYPE_ARG_UNUSED ((StreamTypeArg) ((FILE*) NULL))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN MasterPrinter_ptr MasterPrinter_create ARGS((void));

EXTERN int
MasterPrinter_print_node ARGS((MasterPrinter_ptr self, node_ptr n));

EXTERN int
MasterPrinter_print_string ARGS((MasterPrinter_ptr self, const char* str));

EXTERN const char*
MasterPrinter_get_streamed_string ARGS((const MasterPrinter_ptr self));

EXTERN void
MasterPrinter_reset_stream ARGS((MasterPrinter_ptr self, int offs));

/* this may become static as its behavior has been duplicated into
   the more general MasterPrinter_reset_stream */
EXTERN void
MasterPrinter_reset_string_stream ARGS((MasterPrinter_ptr self));

EXTERN void
MasterPrinter_set_stream_type ARGS((MasterPrinter_ptr self,
                                    StreamType type, StreamTypeArg arg));

EXTERN StreamType
MasterPrinter_get_stream_type ARGS((const MasterPrinter_ptr self));

EXTERN int
MasterPrinter_flush_stream ARGS((MasterPrinter_ptr self));

EXTERN void
MasterPrinter_close_stream ARGS((MasterPrinter_ptr self));

/**AutomaticEnd***************************************************************/


#endif /* __MASTER_PRINTER_H__ */
