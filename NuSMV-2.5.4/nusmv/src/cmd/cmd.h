/**CHeaderFile*****************************************************************

  FileName    [cmd.h]

  PackageName [cmd]

  Synopsis    [Implements command line interface, and miscellaneous commands.]

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``cmd'' package of NuSMV version 2. 
  Copyright (C) 1998-2001 by CMU and FBK-irst. 

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

  Revision    [$Id: cmd.h,v 1.4.2.1.4.4.4.3 2007-12-20 17:12:02 nusmv Exp $]

******************************************************************************/

#ifndef _CMD
#define _CMD

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/
#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif 

#include "utils/utils.h"

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef int (*PFI)(int argc, char **argv);

 
/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

#if NUSMV_HAVE_LIBREADLINE
EXTERN char *readline(char *PROMPT);
EXTERN void add_history(char *line);
#endif
#if NUSMV_HAVE_SETVBUF
EXTERN int setvbuf(FILE*, char*, int mode, size_t size);
#endif
#ifdef PURIFY
EXTERN void purify_all_inuse();
#endif

EXTERN void Cmd_CommandAdd ARGS((char* name, PFI funcFp, int changes, 
                                 boolean reentrant));

EXTERN boolean Cmd_CommandRemove ARGS((const char* name));

EXTERN int Cmd_CommandExecute ARGS((char* command));
EXTERN int Cmd_SecureCommandExecute ARGS((char* command));

EXTERN FILE* Cmd_FileOpen ARGS((char* fileName, char* mode, 
                                char** realFileName_p, int silent));

EXTERN void Cmd_Init ARGS((void));
EXTERN void Cmd_End ARGS((void));

EXTERN FILE* CmdOpenPipe ARGS((int useMore));
EXTERN void CmdClosePipe ARGS((FILE* file));
EXTERN FILE* CmdOpenFile ARGS((const char* filename));
EXTERN void CmdCloseFile ARGS((FILE* file));

/**AutomaticEnd***************************************************************/

#endif /* _CMD */















