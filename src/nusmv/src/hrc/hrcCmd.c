/**CFile***********************************************************************

  FileName    [hrcCmd.c]

  PackageName [hrc]

  Synopsis    [Shell interface to the hrc package.]

  Description [This file contains the interface of the compile package
  with the interactive shell.]

  SeeAlso     []

  Author      [Sergio Mover]

  Copyright   [
  This file is part of the ``hrc'' package of NuSMV version 2.
  Copyright (C) 2009 by FBK.

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

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#include "hrc.h"
#include "hrcCmd.h"
#include "hrcInt.h"
#include "parser/ord/ParserOrd.h"
#include "cmd/cmd.h"
#include "compile/compile.h"
#include "compile/compileInt.h"
#include "parser/symbols.h"
#include "parser/parser.h"

#include "hrc/dumpers/HrcDumperDebug.h"
#include "hrc/dumpers/HrcDumperSmv.h"
#include "hrc/dumpers/HrcDumperXml.h"

static char rcsid[] UTIL_UNUSED = "$Id: hrcCmd.c,v 1.1.2.17 2010-03-05 07:56:36 nusmv Exp $";

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/
typedef HrcDumper_ptr (*HrcDumperFactory)(FILE* fout);


/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int UsageHrcWriteModel ARGS((void));
static int UsageHrcDumpModel ARGS((void));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the commands of the hrc package.]

  Description        []

  SideEffects        []

******************************************************************************/
void Hrc_init_cmd()
{
  Cmd_CommandAdd("hrc_write_model", CommandHrcWriteModel, 0, false);
  Cmd_CommandAdd("hrc_dump_model", CommandHrcDumpModel, 0, false);
}

/**Function********************************************************************

  Synopsis           [Removes the commands provided by the hrc package.]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
void Hrc_quit_cmd()
{
  Cmd_CommandRemove("hrc_write_model");

#if HRC_FLATTENER_TEST
  Cmd_CommandRemove("_test_hrc_flattener");
#endif
}

/**Function********************************************************************

  Synopsis           [Writes the SMV model contained in the root node
  of the hrc structure.]

  CommandName        [hrc_write_model]

  CommandSynopsis    [Writes the hrc structure from root node to a
  given SMV file]

  CommandArguments   [\[-h\] | \[-o "filename"\] \[-d\]]

  CommandDescription [Writes the currently loaded SMV model stored in
  hrc structure in the specified file. If no file is specified the

  standard output is used. <p>

  Command Options:
  <dl>
    <dt> <tt>-o "filename"</tt>
    <dd> Attempts to write the SMV model in "filename".

    <dt> <tt>-d</tt>
    <dd> Renames modules appending "_hrc" the the original module name.
  </dl>
 ]

******************************************************************************/
int CommandHrcWriteModel(int argc, char **argv)
{
  int c = 0;
  int rv = 0;
  char* output_file = NIL(char);
  FILE* ofileid = NIL(FILE);
  boolean bSpecifiedFilename = false;
  boolean append_suffix = false;

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "ho:d")) != EOF) {
    switch (c) {
    case 'h': return UsageHrcWriteModel();
    case 'o':
      output_file = ALLOC(char, strlen(util_optarg)+1);
      nusmv_assert(output_file);
      strcpy(output_file, util_optarg);
      bSpecifiedFilename = true;
      break;
    case 'd':
      append_suffix = true;
      break;
    default:
      break;
    }
  }

  if (argc != util_optind) return UsageHrcWriteModel();

  if (!cmp_struct_get_hrc_built(cmps)) {
    fprintf(nusmv_stdout,
            "The HRC structure was not built, use command flatten_hierarchy\n");
    return 1;
  }

  if (HRC_NODE(NULL) == mainHrcNode) {
    fprintf(nusmv_stdout,
            "The HRC structure is not available, cannot dump the model\n");
    return 1;
  }

  if (output_file == NIL(char)) {
    ofileid = nusmv_stdout;
  }
  else {
    ofileid = fopen(output_file, "w");
    if ((FILE *)NULL == ofileid) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", output_file);
      if (bSpecifiedFilename == true)  FREE(output_file);
      return 1;
    }
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Writing hrc model into file \"%s\"..",
      output_file == (char *)NULL ? "stdout" : output_file);
  }

  CATCH {
    /* Write the hrc model */
    Hrc_WriteModel(mainHrcNode, ofileid, append_suffix);

    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, ".. done.\n");
    }
  }
  FAIL {
    rv = 1;
  }
  fflush(ofileid);

  if (ofileid != nusmv_stdout) {
    fclose(ofileid);
    if (bSpecifiedFilename) FREE(output_file);
  }
  return rv;
}


/**Function********************************************************************

  Synopsis           [Writes the SMV model contained in the root node
  of the hrc structure.]

  CommandName        [hrc_dump_model]

  CommandSynopsis    [Writes the hrc structure from root node to a
  given SMV file]

  CommandArguments   [\[-h\] | \[-o "filename"\] \[-d\]]

  CommandDescription [Writes the currently loaded SMV model stored in
  hrc structure in the specified file. If no file is specified the

  standard output is used. <p>

  Command Options:
  <dl>
    <dt> <tt>-o "filename"</tt>
    <dd> Attempts to write the SMV model in "filename".

    <dt> <tt>-d</tt>
    <dd> Renames modules appending "_hrc" the the original module name.
  </dl>
 ]

******************************************************************************/
int CommandHrcDumpModel(int argc, char **argv)
{
  int c = 0;
  int rv = 0;
  char* dump_format = (char*) NULL;
  char* output_fname = (char*) NULL;
  FILE* ofileid = NIL(FILE);
  boolean append_suffix = false;
  boolean use_indent = true;
  HrcDumper_ptr dumper = HRC_DUMPER(NULL);

  util_getopt_reset();
  while ((c = util_getopt(argc, argv, "ho:f:di")) != EOF) {
    switch (c) {
    case 'h': goto CommandHrcDumpModel_exit_usage;
    case 'o':
      if ((char*) NULL != output_fname) { FREE(output_fname); }
      output_fname = util_strsav(util_optarg);
      nusmv_assert((char*) NULL != output_fname);
      break;

    case 'f':
      if ((char*) NULL != dump_format) { FREE(dump_format); }
      dump_format = util_strsav(util_optarg);
      nusmv_assert((char*) NULL != dump_format);
      break;

    case 'd':  append_suffix = true; break;

    case 'i':  use_indent = false; break;

    default: goto CommandHrcDumpModel_exit_usage;
    }
  }

  if (argc != util_optind) goto CommandHrcDumpModel_exit_usage;

  if (!cmp_struct_get_hrc_built(cmps)) {
    fprintf(nusmv_stdout,
            "The HRC structure was not built, use command flatten_hierarchy\n");
    return 1;
  }

  if (HRC_NODE(NULL) == mainHrcNode) {
    fprintf(nusmv_stdout,
            "The HRC structure is not available, cannot dump the model\n");
    rv = 1; goto CommandHrcDumpModel_exit;
  }

  if (output_fname == NIL(char)) {
    ofileid = nusmv_stdout;
  }
  else {
    ofileid = fopen(output_fname, "w");
    if ((FILE *) NULL == ofileid) {
      fprintf(nusmv_stderr, "Unable to open file \"%s\".\n", output_fname);
      rv = 1; goto CommandHrcDumpModel_exit;
    }
  }

  {
    /* based on the format, constructs the right dumper */
    struct {
      const char* format;
      HrcDumperFactory factory;
    }
    dumpers[] = { 
      {"debug", (HrcDumperFactory) HrcDumperDebug_create},
      {"smv", (HrcDumperFactory) HrcDumperSmv_create},
      {"xml", (HrcDumperFactory) HrcDumperXml_create},
    };
    int idx;
    
    if ((char*) NULL != dump_format) {
      /* check the format, and finds the corresponding dumper */
      for (idx=0; idx<sizeof(dumpers)/sizeof(dumpers[0]); ++idx) {
        if (Utils_strcasecmp(dumpers[idx].format, dump_format) == 0) {
          dumper = HRC_DUMPER(dumpers[idx].factory(ofileid));
          break;
        }
      }
    }

    if (HRC_DUMPER(NULL) == dumper || (char*) NULL == dump_format) {
      if ((char*) NULL == dump_format) {
        fprintf(nusmv_stderr, "A format must be specified.\n");
      }
      else {
        fprintf(nusmv_stderr, "Invalid format '%s'\n", dump_format);
      }
      fprintf(nusmv_stderr, "Valid formats are: ");
      for (idx=0; idx<sizeof(dumpers)/sizeof(dumpers[0]); ++idx) {
        fprintf(nusmv_stderr, "%s ", dumpers[idx].format);
      }
      fprintf(nusmv_stderr, "\n");
      rv = 1; goto CommandHrcDumpModel_exit;
    }

    HrcDumper_enable_mod_suffix(dumper, append_suffix);
    HrcDumper_enable_indentation(dumper, use_indent);
  }

  if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
    fprintf(nusmv_stderr, "Dumping hrc model in format '%s' into file '%s'...",
            dump_format,
            (char*) NULL == output_fname ? "stdout" : output_fname);
  }

  CATCH {
    /* Write the hrc model */
    Hrc_DumpModel(mainHrcNode, dumper);
    
    if (opt_verbose_level_gt(OptsHandler_get_instance(), 0)) {
      fprintf(nusmv_stderr, "  done.\n");
    }
    rv = 0;
  }
  FAIL {
    rv = 1;
  }

 CommandHrcDumpModel_exit:
  if (HRC_DUMPER(NULL) != dumper) HrcDumper_destroy(dumper);
  if ((char*) NULL != dump_format) { FREE(dump_format); }
  if ((char*) NULL != output_fname) { FREE(output_fname); }
  return rv;

 CommandHrcDumpModel_exit_usage:
  if (HRC_DUMPER(NULL) != dumper) HrcDumper_destroy(dumper);
  if ((char*) NULL != dump_format) { FREE(dump_format); }
  if ((char*) NULL != output_fname) { FREE(output_fname); }
  return UsageHrcDumpModel();
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Prints the usage of the command UsageHrcWriteModel]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int UsageHrcWriteModel(void)
{
  fprintf(nusmv_stderr, "usage: hrc_write_model [-h] [-o filename] [-d]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -o filename\tWrites output to \"filename\"\n");
  fprintf(nusmv_stderr, "  -d Renames every module name appending the " \
          "suffix \"_hrc\"\n");
  return 1;
}


/**Function********************************************************************

  Synopsis           [Prints the usage of the command UsageHrcDumpModel]

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static int UsageHrcDumpModel(void)
{
  fprintf(nusmv_stderr, "usage: hrc_dump_model [-h]  -f format [-o filename] [-d][-i]\n");
  fprintf(nusmv_stderr, "  -h \t\tPrints the command usage.\n");
  fprintf(nusmv_stderr, "  -f format\tDumps in the given format.\n");
  fprintf(nusmv_stderr, "  -o filename\tDumps output to \"filename\"\n");
  fprintf(nusmv_stderr, "  -d Renames every module name appending the " \
          "suffix \"_hrc\"\n");
  fprintf(nusmv_stderr, "  -i Disable indentation.\n");
  return 1;
}

