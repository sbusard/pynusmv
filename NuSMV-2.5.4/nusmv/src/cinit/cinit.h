/**CHeaderFile*****************************************************************

  FileName    [cinit.h]

  PackageName [cinit]

  Synopsis    ["Main" package of NuSMV ("cinit" = core init).]

  Author      [Adapted to NuSMV by Marco Roveri]

  Copyright   [
  This file is part of the ``cinit'' package of NuSMV version 2.
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

  Revision    [$Id: sm.h,v 1.4.2.4.4.2.6.1 2006-09-18 13:23:32 nusmv Exp $]

******************************************************************************/

#ifndef _CINIT
#define _CINIT

/*---------------------------------------------------------------------------*/
/* Nested includes                                                           */
/*---------------------------------------------------------------------------*/
#include "util.h"
#include "utils/utils.h"
#include "opt/OptsHandler.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#ifndef NUSMV_LIBRARY_NAME
#define NUSMV_LIBRARY_NAME "NuSMV"
#endif

#ifndef NUSMV_LIBRARY_VERSION
#define NUSMV_LIBRARY_VERSION "2.5.0"
#endif

#ifndef NUSMV_LIBRARY_BUILD_DATE
#define NUSMV_LIBRARY_BUILD_DATE "<compile date not supplied>"
#endif

#ifndef NUSMV_LIBRARY_EMAIL
#define NUSMV_LIBRARY_EMAIL "nusmv-users@list.fbk.eu"
#endif

#ifndef NUSMV_LIBRARY_WEBSITE
#define NUSMV_LIBRARY_WEBSITE "http://nusmv.fbk.eu"
#endif

#ifndef NUSMV_LIBRARY_BUGREPORT
#define NUSMV_LIBRARY_BUGREPORT "Please report bugs to <nusmv-users@fbk.eu>"
#endif


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/
extern FILE *nusmv_stderr;
extern FILE *nusmv_stdout;
extern FILE *nusmv_stdin;
extern FILE *nusmv_historyFile;
extern FILE *nusmv_stdpipe;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/

typedef void (*FP_V_V)(void);

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Function prototypes                                                       */
/*---------------------------------------------------------------------------*/

EXTERN char * CInit_NuSMVReadVersion ARGS((void));
EXTERN char * CInit_NuSMVObtainLibrary ARGS((void));
EXTERN void CInit_NuSMVInitPrintMore ARGS((void));
EXTERN int CInit_NuSMVEndPrintMore ARGS((void));

EXTERN void CInit_BannerPrint ARGS((FILE * file));
EXTERN void CInit_BannerPrintLibrary ARGS((FILE * file));

EXTERN void CInit_BannerPrint_nusmv_library ARGS((FILE * file));
EXTERN void CInit_BannerPrint_cudd ARGS((FILE * file));
EXTERN void CInit_BannerPrint_minisat ARGS((FILE * file));
EXTERN void CInit_BannerPrint_zchaff ARGS((FILE * file));
EXTERN int CInit_NusmvrcSource ARGS((void));

EXTERN void save_nusmv_stdout ARGS((void));
EXTERN void restore_nusmv_stdout ARGS((void));
EXTERN void set_nusmv_stdout ARGS((const char* name));
EXTERN FILE* get_nusmv_stdout ARGS((void));
EXTERN void print_nusmv_stdout ARGS((const char* fmt, ...));
EXTERN void close_nusmv_stdout ARGS((void));
EXTERN void save_nusmv_stderr ARGS((void));
EXTERN void restore_nusmv_stderr ARGS((void));
EXTERN void set_nusmv_stderr ARGS((const char* name));
EXTERN FILE* get_nusmv_stderr ARGS((void));
EXTERN void print_nusmv_stderr ARGS((const char* fmt, ...));
EXTERN void close_nusmv_stderr ARGS((void));

EXTERN void nusmv_FREE ARGS((void* ptr));

EXTERN char* get_preprocessor_call ARGS((const char* name));
EXTERN char* get_preprocessor_filename ARGS((const char* name));
EXTERN char* get_preprocessor_names ARGS((void));
EXTERN int get_preprocessors_num ARGS((void));

EXTERN char* NuSMVCore_get_tool_name ARGS((void));
EXTERN char* NuSMVCore_get_tool_rc_file_name ARGS((void));
EXTERN void NuSMVCore_set_tool_name ARGS((char* tool_name));
EXTERN char* NuSMVCore_get_tool_version ARGS((void));
EXTERN void NuSMVCore_set_tool_version ARGS((char* tool_version));
EXTERN char* NuSMVCore_get_build_date ARGS((void));
EXTERN void NuSMVCore_set_build_date ARGS((char* build_date));
EXTERN char* NuSMVCore_get_prompt_string ARGS((void));
EXTERN void NuSMVCore_set_prompt_string ARGS((char* prompt_string));
EXTERN char* NuSMVCore_get_email ARGS((void));
EXTERN void NuSMVCore_set_email ARGS((char* email));
EXTERN char* NuSMVCore_get_website ARGS((void));
EXTERN void NuSMVCore_set_website ARGS((char* website));
EXTERN char* NuSMVCore_get_bug_report_message ARGS((void));
EXTERN void NuSMVCore_set_bug_report_message ARGS((char* bug_report_message));
EXTERN char* NuSMVCore_get_linked_addons ARGS((void));
EXTERN void NuSMVCore_set_linked_addons ARGS((char* linked_addons));

EXTERN char* NuSMVCore_get_library_name ARGS((void));
EXTERN char* NuSMVCore_get_library_version ARGS((void));
EXTERN char* NuSMVCore_get_library_build_date ARGS((void));
EXTERN char* NuSMVCore_get_library_email ARGS((void));
EXTERN char* NuSMVCore_get_library_website ARGS((void));
EXTERN char* NuSMVCore_get_library_bug_report_message ARGS((void));

EXTERN void NuSMVCore_set_banner_print_fun ARGS((void (*banner_print_fun)(FILE*)));
EXTERN void NuSMVCore_set_init_fun ARGS((void (*init_fun)(void)));
EXTERN void NuSMVCore_set_quit_fun ARGS((void (*quit_fun)(void)));
EXTERN void NuSMVCore_set_reset_init_fun ARGS((void (*reset_fun)(void)));
EXTERN void NuSMVCore_set_reset_quit_fun ARGS((void (*reset_fun)(void)));
EXTERN void NuSMVCore_set_batch_fun ARGS((void (*batch_fun)(void)));

EXTERN void NuSMVCore_add_env_command_line_option ARGS((char* name,
                                                 char* usage,
                                                 char* parameter,
                                                 char* env_var,
                                                 boolean is_deprecated,
                                                 boolean is_public,
                                                 char* dependency,
                                                 char* conflict));

EXTERN void
NuSMVCore_add_command_line_option ARGS((char* name,
                                 char* usage,
                                 char* parameter,
                                 boolean (*check_and_apply)(OptsHandler_ptr, char*),
                                 boolean is_deprecated,
                                 boolean is_public,
                                 char* dependency,
                                 char* conflict));

EXTERN void NuSMVCore_init_data ARGS((void));
EXTERN void NuSMVCore_init_cmd_options ARGS((void));
EXTERN void NuSMVCore_init ARGS((FP_V_V fns[][2], int));
EXTERN boolean NuSMVCore_main ARGS((int argc, char ** argv, int* status));
EXTERN void NuSMVCore_reset ARGS((void));
EXTERN void NuSMVCore_quit ARGS((void));

/**AutomaticEnd***************************************************************/

#endif /* _CINIT */
