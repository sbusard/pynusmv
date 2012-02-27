/**CHeaderFile*****************************************************************

  FileName    [cmdInt.h]

  PackageName [cmd]

  Synopsis    [Internal declarations for command package.]

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

  Revision    [$Id: cmdInt.h,v 1.3.4.1.2.1.2.3 2005-11-16 12:04:40 nusmv Exp $]

******************************************************************************/

#ifndef _CMDINT
#define _CMDINT

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "cinit/cinit.h"
#include "cmd.h"
#include "opt/opt.h"
#include "dd/dd.h"
#include "utils/utils.h"
#include "utils/array.h"
#include "utils/avl.h"


#if NUSMV_STDC_HEADERS
#  include <string.h>
#  include <stdlib.h>
#else
 void free();
# if NUSMV_HAVE_STRING_H
#  include <string.h>
# else
char *strncpy();
# endif
#endif

/*
 * This is for Solaris -- it needs to be convinced that we're actually
 * using BSD-style calls in sys/ioctl.h, otherwise it doesn't find
 * "ECHO" "CRMOD" and "TIOCSTI" when compiling cmdFile.c
 */
#define BSD_COMP

#if IOCTL_WITH_TERMIOS
#  include <sys/ioctl.h>
#  include <sys/termios.h>
#else
#  if NUSMV_HAVE_SYS_IOCTL_H
#    include <sys/ioctl.h>
#  else
#    if NUSMV_HAVE_SYS_TERMIOS_H
#      include <sys/termios.h>
#    endif
#  endif
#endif

/* Linux and its wacky header files... */
#if NUSMV_HAVE_BSD_SGTTY_H
#  include <bsd/sgtty.h>
#endif

#if NUSMV_HAVE_SYS_SIGNAL_H
#  include <sys/signal.h>
#endif
#if NUSMV_HAVE_SIGNAL_H
#  include <signal.h>
#endif

/*
 * No unix system seems to be able to agree on how to access directories,
 * which cmdFile.c needs to do.  This solution, suggested by the autoconf
 * distribution, seems to handle most of the nonsense.
 */

#if NUSMV_HAVE_DIRENT_H
# if NUSMV_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
#  include <dirent.h>
#  define NAMLEN(dirent) strlen((dirent)->d_name)
#else
#  define dirent direct
#  define NAMLEN(dirent) (dirent)->d_namlen
#  if NUSMV_HAVE_SYS_NDIR_H
#    include <sys/ndir.h>
#  endif
#  if NUSMV_HAVE_SYS_DIR_H
#    include <sys/dir.h>
#  endif
#  if NUSMV_HAVE_NDIR_H
#    include <ndir.h>
#  endif
#endif

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/
typedef struct CmdAliasDescrStruct {
  char *name;
  int argc;
  char **argv;
} CmdAliasDescr_t;

typedef struct CommandDescrStruct {
  char *name;
  PFI command_fp;
  int changes_hmgr;
  boolean reentrant;
} CommandDescr_t;

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern avl_tree *cmdCommandTable;
extern avl_tree *cmdAliasTable;
extern array_t  *cmdCommandHistoryArray;

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN boolean Cmd_CommandDefined(const char* name);
EXTERN CommandDescr_t *Cmd_CommandGet(const char* name);
EXTERN void CmdCommandFree(char * value);
EXTERN CommandDescr_t * CmdCommandCopy(CommandDescr_t * value);
EXTERN char * CmdFgetsFilec(char * buf, int size, FILE * stream, char * prompt);
EXTERN char * CmdFgetsFilec(char * buf, int size, FILE * stream, char * prompt);
EXTERN char * CmdHistorySubstitution(char * line, int * changed);
EXTERN void CmdFreeArgv(int argc, char ** argv);
EXTERN void CmdAliasFree(char * value);

/**AutomaticEnd***************************************************************/

#endif /* _CMDINT */
