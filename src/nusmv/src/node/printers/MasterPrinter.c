/**CFile***********************************************************************

  FileName    [MasterPrinter.c]

  PackageName [node.printers]

  Synopsis    [Implementaion of class 'MasterPrinter', derived from
  MasterNodeWalker]

  Description []

  SeeAlso     [MasterPrinter.h]

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

  Revision    [$Id: MasterPrinter.c,v 1.1.2.6.4.4 2009-09-30 15:34:38 nusmv Exp $]

******************************************************************************/

#if HAVE_CONFIG_H
#include "nusmv-config.h"
#endif

#include "MasterPrinter.h"
#include "MasterPrinter_private.h"

#include "node/MasterNodeWalker_private.h"

#include "PrinterBase.h"
#include "PrinterBase_private.h"
#include "printersInt.h"

#include "opt/opt.h"
#include "utils/utils.h"
#include "utils/NodeList.h"
#include "utils/error.h"

#if NUSMV_HAVE_STRING_H
#include <string.h> /* for strdup */
#else
char* strdup(const char*); /* forward declaration */
#endif

static char rcsid[] UTIL_UNUSED = "$Id: MasterPrinter.c,v 1.1.2.6.4.4 2009-09-30 15:34:38 nusmv Exp $";


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

  Synopsis    [MasterPrinter class definition]

  Description []

  SeeAlso     []

******************************************************************************/
typedef struct MasterPrinter_TAG
{
  /* this MUST stay on the top */
  INHERITS_FROM(MasterNodeWalker);

  /* -------------------------------------------------- */
  /*                  Private members                   */
  /* -------------------------------------------------- */
  StreamType stream_type;
  StreamTypeArg stream_arg;

  NodeList_ptr indent_stack; /* internal indentation levels stack */
  int current_ofs;  /* current offset on the current line */

  /* string stream is handled by using these varaibles: */
  char* sstream;       /* string stream */
  size_t sstream_cap;  /* sstream allocation capacity */
  size_t sstream_len;  /* sstream current usage */
  size_t sstream_grow_sum; /* sstream grow sum */
  int sstream_grow_num; /* number of resizes */

  int (*inner_stream_print)(MasterPrinter_ptr self, const char* str);
  int (*external_stream_print)(void* stream, const char* str);

} MasterPrinter;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/
#define BLANK_STR " "
#define NEWLINE_CHR '\n'
#define NEWLINE_STR "\n"
#define TERM_CHR '\0'

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static void master_printer_init ARGS((MasterPrinter_ptr self));
static void master_printer_deinit ARGS((MasterPrinter_ptr self));

static void master_printer_finalize ARGS((Object_ptr object, void* dummy));

static int
master_printer_fprint ARGS((MasterPrinter_ptr self, const char* str));

static int
master_printer_sprint ARGS((MasterPrinter_ptr self, const char* str));

static int
master_printer_get_level(MasterPrinter_ptr self);

static void
master_printer_reset_string_stream(MasterPrinter_ptr self);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The MasterPrinter class constructor]

  Description        [The MasterPrinter class constructor]

  SideEffects        []

  SeeAlso            [MasterPrinter_destroy]

******************************************************************************/
MasterPrinter_ptr MasterPrinter_create()
{
  MasterPrinter_ptr self = ALLOC(MasterPrinter, 1);
  MASTER_PRINTER_CHECK_INSTANCE(self);

  master_printer_init(self);
  return self;
}


/**Function********************************************************************

  Synopsis           [Prints the given node on the stream currently set]

  Description        [If the stream is a string stream, the result can be
  obtained be calling MasterPrinter_get_streamed_string. Returns
  0 if an error occurred for some reason.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int MasterPrinter_print_node(MasterPrinter_ptr self, node_ptr n)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);
  return master_printer_print_node(self, n, 0);
}


/**Function********************************************************************

  Synopsis           [Prints the given string on the stream currently set]

  Description        [If the stream is a string stream, the result can be
  obtained be calling MasterPrinter_get_streamed_string. Returns
  0 if an error occurred for some reason.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int MasterPrinter_print_string(MasterPrinter_ptr self, const char* str)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  /* defaults to ok */
  int res = 1;

  /* a local copy of the string */
  char *s = strdup(str);

  /* beginning, end pointers to the current chunk */
  char *p = s, *q;

  /* if there are newlines in the string to be printed, cut the string where
   * first newline occurs, then print the fragment followed by a newline and
   * a number of spaces according to internal indentation stack head. Repeat
   * until all fragments have been printed.
   **/

  do {
    boolean pad = false;

    if ((q=strchr(p, NEWLINE_CHR))) {
      /* side effects on p */
      (*q) = TERM_CHR; q++;
      pad = true;
    }

    /* original printout */
    switch (self->stream_type) {
    case STREAM_TYPE_DEFAULT:
    case STREAM_TYPE_STDOUT:
    case STREAM_TYPE_STDERR:
    case STREAM_TYPE_FILE:
      if (!master_printer_fprint(self, p)) {
        res = 0; /* failure */
        goto leave;
      }
      break;

    case STREAM_TYPE_STRING:
      if (!master_printer_sprint(self, p)) {
        res = 0; /* failure */
        goto leave;
      }
      break;

    case STREAM_TYPE_FUNCTION:
      {
        void* arg = self->stream_arg.function.argument;
        if (!self->stream_arg.function.func_ptr(p, arg)) {
          res = 0; /* failure */
          goto leave;
        }
        break;
      }

    default:
      internal_error("MasterPrinter::print_string: Invalid stream type\n");
    }

    /* update current ofs */
    self->current_ofs += strlen(p);

    /* post-processing padding */
    if (pad) {
      int i, padding = master_printer_get_level(self);

      /* add a newline and #padding spaces */
      for (i=0; i<=padding; i++) {
        char *tmp = (!i) ? NEWLINE_STR : BLANK_STR;

        switch (self->stream_type) {
        case STREAM_TYPE_DEFAULT:
        case STREAM_TYPE_STDOUT:
        case STREAM_TYPE_STDERR:
        case STREAM_TYPE_FILE:
          if (!master_printer_fprint(self, tmp)) {
            res = 0; /* failure */
            goto leave;
          }
          break;

        case STREAM_TYPE_STRING:
          if (!master_printer_sprint(self, tmp)) {
            res = 0; /* failure */
            goto leave;
          }
          break;

        case STREAM_TYPE_FUNCTION:
          {
            void* arg = self->stream_arg.function.argument;
            if (!self->stream_arg.function.func_ptr(tmp, arg)) {
              res = 0; /* failure */
              goto leave;
            }
            break;
          }

        default:
          internal_error("MasterPrinter::print_string: Invalid stream type\n");
        }
      }

      /* update current ofs */
      self->current_ofs = padding;
    }

    p=q;
  } while (p); /* is there anything left to print? */

 leave:
  FREE(s);
  return res; /* 1 if no failures occurred */
}


/**Function********************************************************************

  Synopsis           [Returns the string that has been streamed]

  Description        [Returned string belongs to self, DO NOT free it.

  Warning: this method can be called only when the current
  stream type is STREAM_TYPE_STRING.]

  SideEffects        []

  SeeAlso            [master_printer_reset_string_stream]

******************************************************************************/
const char* MasterPrinter_get_streamed_string(const MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);
  nusmv_assert(self->stream_type == STREAM_TYPE_STRING);

  return (const char*) (self->sstream);
}


/**Function********************************************************************

  Synopsis [Sets the stream type to be used to produce a printing
  result]

  Description [When the given type requires an argument (for example,
  STREAM_TYPE_FILE requires a file), the argument must be passed by
  using the 'arg' parameter. When not required (for example
  STREAM_TYPE_STRING), the caller can pass STREAM_TYPE_ARG_UNUSED
  as argument.

  When STREAM_TYPE_FILE is used, the argument must be the handler of an open
  writable file.
  ]


  SideEffects        []

  SeeAlso            []

******************************************************************************/
void MasterPrinter_set_stream_type(MasterPrinter_ptr self,
                                   StreamType type, StreamTypeArg arg)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  switch (type) {
  case STREAM_TYPE_DEFAULT:
  case STREAM_TYPE_STDOUT:
    self->stream_arg.file = nusmv_stdout;
    break;

  case STREAM_TYPE_STDERR:
    self->stream_arg.file = nusmv_stderr;
    break;

  case STREAM_TYPE_STRING:
    if (self->stream_type != STREAM_TYPE_STRING) {
      /* we start with a new string stream, but only if it is not
         currently set a string stream */
      master_printer_reset_string_stream(self);
    }
    break;

  case STREAM_TYPE_FILE:
    self->stream_arg.file = arg.file;
    break;

  case STREAM_TYPE_FUNCTION:
    /*
      Copy the structure 'function' (Both function pointer and void* argument)
    */
    self->stream_arg.function = arg.function;
    break;

  default:
    internal_error("MasterPrinter::set_stream_type: Invalid stream type\n");
  }

  self->stream_type = type;
}

/**Function********************************************************************

  Synopsis           [Returns the currently set stream type]

  Description        [Returns the currently set stream type]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
StreamType MasterPrinter_get_stream_type(const MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);
  return self->stream_type;
}

/**Function********************************************************************

  Synopsis           [Reset the stream]

  Description [Set the indentation offset for this stream. Negative
  offsets are silently discarded.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void  MasterPrinter_reset_stream(MasterPrinter_ptr self, int initial_offset)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  self->current_ofs = (0<initial_offset) ? initial_offset : 0;

  /* clear stack (should be empty anyway) */
  ListIter_ptr p = NodeList_get_first_iter(self->indent_stack);
  while (NULL != p) {
    NodeList_remove_elem_at(self->indent_stack, p);
    p = NodeList_get_first_iter(self->indent_stack);
  }

  /* stream dependant operations */
  switch (self->stream_type) {

  case STREAM_TYPE_DEFAULT:
  case STREAM_TYPE_STDOUT:
  case STREAM_TYPE_STDERR:
  case STREAM_TYPE_FILE:
  case STREAM_TYPE_FUNCTION:
    break;

  case STREAM_TYPE_STRING:
    master_printer_reset_string_stream(self);
    break;

  default:
    error_unreachable_code(); /* no other possible cases */
  }
}

/**Function********************************************************************

  Synopsis           [Closes the current stream, if possible or applicable]

  Description [The currently set stream is closed (file) or reset
  (string) and the stream type is set to be STREAM_TYPE_DEFAULT.
  IMPORTANT: If the current stream is nusmv_std{out,err} the stream is
  not closed.

  This function is provided to allow the called to forget the set
  stream after setting it into the master printer.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void MasterPrinter_close_stream(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  switch (self->stream_type) {
  case STREAM_TYPE_DEFAULT:
  case STREAM_TYPE_STDOUT:
  case STREAM_TYPE_STDERR:
    break;

  case STREAM_TYPE_STRING:
    /* resets the string stream */
    master_printer_reset_string_stream(self);
    break;

  case STREAM_TYPE_FILE:
    /* closes the file stream if not nusmv_std{out,err} */
    if ((self->stream_arg.file != nusmv_stdout) &&
        (self->stream_arg.file != nusmv_stderr)) {
      fclose(self->stream_arg.file);
      break;
    }

  case STREAM_TYPE_FUNCTION:
    break;

  default:
    error_unreachable_code(); /* no other possible cases */
  }

  /* sets the default stream type */
  MasterPrinter_set_stream_type(self, STREAM_TYPE_DEFAULT,
                                STREAM_TYPE_ARG_UNUSED);
}

/**Function********************************************************************

  Synopsis [Flushes the current stream, if possible or applicable]

  Description [The currently set stream is flushed out (i.e. no
  unstreamed data remains afterwards.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int MasterPrinter_flush_stream(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  switch (self->stream_type) {
  case STREAM_TYPE_DEFAULT:
    /* not applicable here */
    break;

  case STREAM_TYPE_STDOUT:
    return !fflush(nusmv_stdout);
    break;

  case STREAM_TYPE_STDERR:
    return !fflush(nusmv_stderr);
    break;

  case STREAM_TYPE_STRING:
    /* not applicable here */
    break;

  case STREAM_TYPE_FILE:
    return !fflush(self->stream_arg.file);
    break;

  case STREAM_TYPE_FUNCTION:
    /* not applicable here */
    break;

  default:
    error_unreachable_code(); /* no other possible cases */
  }

  return 1;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis           [Internal version of the method print_node, callable
  internally and by printers]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int master_printer_print_node(MasterPrinter_ptr self,
                              node_ptr n, int priority)
{
  ListIter_ptr iter;
  iter = NodeList_get_first_iter(MASTER_NODE_WALKER(self)->walkers);
  while (!ListIter_is_end(iter)) {
    PrinterBase_ptr pr =
      PRINTER_BASE(NodeList_get_elem_at(MASTER_NODE_WALKER(self)->walkers,
                                        iter));

    if (NodeWalker_can_handle(NODE_WALKER(pr), n)) {

      return PrinterBase_print_node(pr, n, priority);
    }

    iter = ListIter_get_next(iter);
  }

  return 1;
}

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [The MasterPrinter class private initializer]

  Description        [The MasterPrinter class private initializer]

  SideEffects        []

  SeeAlso            [MasterPrinter_create]

******************************************************************************/
static void master_printer_init(MasterPrinter_ptr self)
{
  /* base class initialization */
  master_node_walker_init(MASTER_NODE_WALKER(self));

  /* allocates a minimal string stream */
  self->sstream_cap = 1;
  self->sstream_len = 0;
  self->sstream_grow_sum = 0;
  self->sstream_grow_num = 0;
  self->sstream = ALLOC(char, self->sstream_cap);
  self->sstream[0] = '\0';

  /* initialize internal structure for pretty printing */
  self->indent_stack = NodeList_create();
  self->current_ofs  = 0;

  /* default stream */
  MasterPrinter_set_stream_type(self,
                                STREAM_TYPE_DEFAULT,
                                STREAM_TYPE_ARG_UNUSED);

  /* virtual methods settings */
  OVERRIDE(Object, finalize) = master_printer_finalize;

}


/**Function********************************************************************

  Synopsis           [The MasterPrinter class private deinitializer]

  Description        [The MasterPrinter class private deinitializer]

  SideEffects        []
  SeeAlso            [MasterPrinter_destroy]

******************************************************************************/
static void master_printer_deinit(MasterPrinter_ptr self)
{
  NodeList_destroy(self->indent_stack);
  self->indent_stack = NODE_LIST(NULL);
  FREE(self->sstream);
  self->sstream = (char*) NULL;

  /* base class deinitialization */
  master_node_walker_deinit(MASTER_NODE_WALKER(self));
}


/**Function********************************************************************

  Synopsis    [The PrinterBase class virtual finalizer]

  Description [Called by the class destructor]

  SideEffects []

  SeeAlso     []

******************************************************************************/
static void master_printer_finalize(Object_ptr object, void* dummy)
{
  MasterPrinter_ptr self = MASTER_PRINTER(object);

  master_printer_deinit(self);
  FREE(self);
}

/**Function********************************************************************

  Synopsis           [Appends a string to the internal string stream]

  Description        [Warning: current stream type must be STREAM_TYPE_STRING]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int master_printer_sprint(MasterPrinter_ptr self, const char* str)
{
  int len;

  nusmv_assert(self->stream_type == STREAM_TYPE_STRING);

  /* ignore empty strings */
  len = strlen(str);
  if (0 != len) {
    self->sstream_len += len;

    if (self->sstream_len >= self->sstream_cap) {
      /* resizes the sstream */
      self->sstream_grow_sum += self->sstream_len;
      self->sstream_grow_num += 1;
      self->sstream_cap = self->sstream_len + 1 +
        (self->sstream_grow_sum / self->sstream_grow_num);

      self->sstream = (char*) REALLOC(char, self->sstream, self->sstream_cap);
      nusmv_assert(self->sstream != (char*) NULL);
    }

    strcat(self->sstream, str);
  }

  return 1;
}


/**Function********************************************************************

  Synopsis           [Stream the given string to the internally set file
  stream]

  Description        [Warning: current stream type must be compatible]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int master_printer_fprint(MasterPrinter_ptr self, const char* str)
{
  nusmv_assert( (self->stream_type == STREAM_TYPE_DEFAULT) ||
                (self->stream_type == STREAM_TYPE_STDOUT) ||
                (self->stream_type == STREAM_TYPE_STDERR) ||
                (self->stream_type == STREAM_TYPE_FILE));

  if (str[0] != '\0') {
    return fprintf(self->stream_arg.file, "%s", str);
  }

  else return 1;
}

/**Function********************************************************************

  Synopsis           [Pushes the current level of indentation]

  Description        []

  SideEffects        [The internal status of the master printer is changed]

  SeeAlso            [master_printer_deindent]

******************************************************************************/
int master_printer_indent(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  NodeList_prepend(self->indent_stack,
                   NODE_FROM_INT (self->current_ofs));

  /* never fails */
  return 1;
}


/**Function********************************************************************

  Synopsis           [Restore previous level of indentation]

  Description [Restore previous level of indentation. Raises an
  internal error if an inconsisten internal state is detected.]

  SideEffects        [The internal status of the master printer is changed]

  SeeAlso            [master_printer_indent]

******************************************************************************/
int master_printer_deindent(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  ListIter_ptr head =
    NodeList_get_first_iter(self->indent_stack);

  if (NULL == head) {
    internal_error("printout failure: empty indentation stack!");
  }

  NodeList_remove_elem_at(self->indent_stack, head);

  /* never fails */
  return 1;

}

/**Function********************************************************************

  Synopsis           [Get current level of indentation]

  Description [Returns an integer representing the current level of
  indentation (i.e. the number of whitespaces to pad the string
  with). If the internal stack is empty, assume indentation level is 0
  for backward compatibility.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int
master_printer_get_level(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  ListIter_ptr head =
    NodeList_get_first_iter(self->indent_stack);

  return (NULL != head)
    ? NODE_TO_INT(NodeList_get_elem_at(self->indent_stack, head))
    : 0 ;
}

/**Function********************************************************************

  Synopsis           [Cleans up the stream that have been read so far.
  Any previoulsy read stream will be lost]

  Description        []

  SideEffects        []

  SeeAlso            [get_streamed_string]

******************************************************************************/
static void master_printer_reset_string_stream(MasterPrinter_ptr self)
{
  MASTER_PRINTER_CHECK_INSTANCE(self);

  self->sstream[0] = '\0';
  self->sstream_len = 0;
}

/**AutomaticEnd***************************************************************/
