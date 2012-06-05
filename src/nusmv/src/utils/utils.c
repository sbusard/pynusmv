/**CFile***********************************************************************

  FileName    [utils.c]

  PackageName [utils]

  Synopsis    [Contains useful functions and structures]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2.
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

******************************************************************************/

#if HAVE_CONFIG_H
# include "nusmv-config.h"
#endif

#if NUSMV_HAVE_DIRENT_H
# if NUSMV_HAVE_SYS_TYPES_H
#  include <sys/types.h>
# endif
# include <dirent.h>
#endif

#include <stdio.h>
#include <strings.h>
#include <limits.h>

#include "utils/utils.h"
#include "utils/error.h"
#include "parser/symbols.h"

#if !NUSMV_HAVE_STRCASECMP
# include <ctype.h>
#endif

#include "utils/TimerBench.h"

#if NUSMV_HAVE_UNISTD_H
#include <unistd.h>
#endif

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

typedef void (*hash_timers_DESTROY)(TimerBench_ptr);

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

static hash_ptr hash_timers = (hash_ptr)NULL;

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static void freeListOfLists_aux ARGS((lsList list));

static void hash_timers_init ARGS((void));
static void hash_timers_quit_fun ARGS((hash_timers_DESTROY));
static TimerBench_ptr hash_timers_lookup ARGS((const char* key));
static void hash_timers_insert ARGS((const char* key, TimerBench_ptr val));

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Initializes the utils package]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void Utils_pkg_init()
{
  hash_timers_init();
}


/**Function********************************************************************

  Synopsis           [De-initializes the utils package]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
void Utils_pkg_quit()
{
  hash_timers_quit_fun(TimerBench_destroy);
}


/**Function********************************************************************

  Synopsis           [Returns pathname without path prefix]

  Description        []

  SideEffects        [None]

  SeeAlso            []

******************************************************************************/
const char* Utils_StripPath(const char* pathfname)
{
  const char* old_pos = pathfname;

  while ((*pathfname) != '\0') {
    if ((*pathfname) == '/') old_pos = pathfname + 1;
    ++pathfname;
  }
  return old_pos;
}


/**Function********************************************************************

  Synopsis           [Returns filename without path and extension]

  Description        [Example: given "~/.../test.smv", "test" will be returned.
  filename must be a string whose length is large enought to contain the "pure"
  filename]

  SideEffects        [the string pointed by 'filename' changes]

  SeeAlso            []

******************************************************************************/
void Utils_StripPathNoExtension(const char* fpathname, char* filename)
{
  char* szExt = NULL;
  const char* szNoPath = Utils_StripPath(fpathname);

  szExt = strstr(szNoPath, ".");
  if (szExt != NULL) {
    strncpy(filename, szNoPath, (size_t)(szExt - szNoPath));
    *(filename+(size_t)(szExt - szNoPath)) = '\0'; /* terminates the string */
  }
  else {
    strcpy(filename, szNoPath);
  }
}


/**Function********************************************************************

  Synopsis           [Returns directory part of fpathname without filename and
                      extension]

  Description        [dirname must be a string whose length is large enough to
                      contain the directory part]

  SideEffects        [The string pointed to by 'dirname' changes]

  SeeAlso            [Utils_StripPathNoExtension, Utils_StripPath]

******************************************************************************/
void Utils_StripPathNoFilenameNoExtension(const char* fpathname, char* dirname)
{
  int pos = 0;
  int pos_last_sep = 0;

  nusmv_assert(fpathname != (char *) NULL);
  nusmv_assert(dirname != (char *) NULL);

  while(fpathname[pos] != '\0') {
    if (fpathname[pos] == '/') {
      pos_last_sep = pos;
    }
    ++pos;
  }

  strncpy(dirname, fpathname, pos_last_sep);
  dirname[pos_last_sep] = '\0';
}


/**Function********************************************************************

  Synopsis           [Destroys a list of list]

  Description        [This function can be used to destroy lists of list. The
  contained set of lists is removed from memory as the top level list.
  More than two levels are not handled at the moment.]

  SideEffects        [Lists are deallocated]

  SeeAlso            [lsDestroy]

******************************************************************************/
void Utils_FreeListOfLists(lsList list_of_lists)
{
  lsDestroy(list_of_lists, &freeListOfLists_aux);
}




/**Function********************************************************************

  Synopsis           [Return a string to be used as temporary file]

  Description [This functions gets a template parameter for the file
  name, with 6 'X' that will be substituted by an unique id. See
  mkstemp for further info. Ensures that the filename is not already
  in use in the given directory. If NULL is passed as the directory,
  then the standard temporary directory is used instead. Returned
  string must be freed. Returtns NULL if the filename cannot be found
  or if we do not have write priviledges in the specified directory.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
#define _TEMPDIR "/tmp"
char* Utils_get_temp_filename_in_dir(const char* dir, const char* templ)
{
  char* dirname = (char*) NULL;
  char* name = (char*) NULL;
  char* var;
  int len;

# if NUSMV_HAVE_MKSTEMP
  int fn;
# endif

# if defined(__MINGW32__)
  char dir_separator = '\\';
# else
  char dir_separator = '/';
# endif


  if (dir == NULL) {
    /* 1) Search for the directory */
#   if defined(__MINGW32__)
    var = (char*) NULL;
#   if NUSMV_HAVE_GETENV
    var = getenv("TEMP");
    if (var == (char*) NULL) var = getenv("TMP");
#   endif /* NUSMV_HAVE_GETENV */

    if (var != (char*) NULL) dirname = util_strsav(var);
    else dirname = util_strsav(".");

#   else /* ! defined __MINGW32__ */
    var = (char*) NULL;
#   if NUSMV_HAVE_GETENV
    var = getenv("TEMPDIR");
#   endif /* NUSMV_HAVE_GETENV */

    if (var != (char*) NULL) dirname = util_strsav(var);
    else dirname = util_strsav(_TEMPDIR);
#   endif /* __MINGW32__ */
  }
  else {
    dirname = ALLOC(char, strlen(dir) + 1);
    strcpy(dirname, dir);
    if (dir[strlen(dir) - 1] == dir_separator) {
      dirname[strlen(dir) - 1] = '\0';
    }
  }

  nusmv_assert(dirname != (char*) NULL);


  /* 2) Tries to open the file: */
  len = strlen(dirname) + 1 + strlen(templ) + 1;
  name = ALLOC(char, len);
  nusmv_assert(name != (char*) NULL);

  snprintf(name, len, "%s%c%s", dirname, dir_separator, templ);
  FREE(dirname);

# if NUSMV_HAVE_MKSTEMP
  fn = mkstemp(name);
  if (fn == -1) {
    /* tries with the current dir */
    sprintf(name, "%s", templ);
    fn = mkstemp(name);
    if (fn == -1) {
      /* no way */
      FREE(name);
      name = (char*) NULL;
    }
  }

  if (name != (char*) NULL) {
    nusmv_assert(fn != -1);
    close(fn);
    /* the file created needs to be removed */
    if (remove(name) == -1) {
      rpterr("error deleting temporary file \"%s\"", name);
    }
  }

#elif NUSMV_HAVE_MKTEMP
  if (mktemp(name) == (char*) NULL) {
    /* tries with the current dir */
    sprintf(name, "%s", templ);
    if (mktemp(name) == (char*) NULL) {
      /* no way */
      FREE(name);
      name = (char*) NULL;
    }
  }
#elif NUSMV_HAVE_TMPNAM
  #include <stdio.h>
  if (tmpnamp(name) == (char*) NULL) {
    /* tries with the current dir */
    sprintf(name, "%s", templ);
    if (mktemp(name) == (char*) NULL) {
      /* no way */
      FREE(name);
      name = (char*) NULL;
    }
  }
#else /* no support from OS */
  #warning "Utils_get_temp_filename_in_dir provides a poor support"
  snprintf(name, len, "TMP%d", utils_random());
#endif

  return name;
}

/**Function********************************************************************

  Synopsis           [Checks a list of directories for a given file.]

  Description        [The list of directories (delimited by the charaters given)
  are checked for the existence of the file.]

  SideEffects        []

******************************************************************************/
boolean Utils_file_exists_in_paths(const char* filename,
                                   const char* paths,
                                   const char* delimiters)
{
  char pathscopy[strlen(paths) + 1];
  char* dir;
  boolean result = false;

  strcpy(pathscopy, paths);
  dir = strtok(pathscopy, delimiters);

  while ((!result) && (dir != NULL)) {
    result = Utils_file_exists_in_directory(filename, dir);
    dir = strtok(NULL, delimiters);
  }
  return result;
}

/**Function********************************************************************

  Synopsis           [Checks for the existence of a file within a directory.]

  Description        []

  SideEffects        []

******************************************************************************/
boolean Utils_file_exists_in_directory(const char* filename, char* directory)
{
  boolean fileexists = false;

#if NUSMV_HAVE_DIRENT_H
  struct dirent *dirfile;
  int l1 = strlen(filename);

  DIR* dir = opendir(directory);

  if (dir != NULL){
    while ((!fileexists) && (dirfile = readdir(dir))) {
      if (strlen(dirfile->d_name) == l1) {
        fileexists = (strcmp(filename, dirfile->d_name) == 0);
      }
    }
    (void) closedir(dir);
  }
#else
#warning "Utils_file_exists_in_directory is not supported"
#endif

  return fileexists;
}


/**Function********************************************************************

  Synopsis           [An abstraction over BSD strcasecmp]

  Description        [Compares the two strings s1 and s2,
  ignoring the case of the characters.]

  SideEffects        []

******************************************************************************/
int Utils_strcasecmp(const char* s1, const char* s2)
{
#if NUSMV_HAVE_STRCASECMP
  return strcasecmp(s1, s2);
#else
{
  int res = 0;

  while (1) {
    int c1 = tolower(*s1);
    int c2 = tolower(*s2);

    if (c1 != c2) {
      if (c1 < c2) return -1;
      return 1;
    }

    if (*s1 == '\0') break;
    s1 += 1; s2 += 1;
  }

  return 0;
#endif
}


/**Function********************************************************************

  Synopsis           [Starts a timer whose name is given]

  Description [If the timer does not exist, it will be created and
  started. If already started an error occurs.]

  SideEffects        []

******************************************************************************/
void Utils_start_timer(const char* name)
{
  TimerBench_ptr timer = hash_timers_lookup(name);

  if (timer == TIMER_BENCH(NULL)) {
    // timer must be created
    timer = TimerBench_create(name);
    hash_timers_insert(name, timer);
  }

  TimerBench_start(timer);
}


/**Function********************************************************************

  Synopsis           [Stops a timer whose name is given]

  Description [The timer must be already existing and running.]

  SideEffects        []

******************************************************************************/
void Utils_stop_timer(const char* name)
{
  TimerBench_ptr timer = hash_timers_lookup(name);
  TIMER_BENCH_CHECK_INSTANCE(timer);

  TimerBench_stop(timer);
}

/**Function********************************************************************

  Synopsis           [Resets a timer whose name is given]

  Description [The timer must be already existing.]

  SideEffects        []

******************************************************************************/
void Utils_reset_timer(const char* name)
{
  TimerBench_ptr timer = hash_timers_lookup(name);
  TIMER_BENCH_CHECK_INSTANCE(timer);

  TimerBench_reset(timer);
}

/**Function********************************************************************

  Synopsis           [prints info about a timer whose name is given]

  Description [The timer must be already existing. msg can be NULL]

  SideEffects        []

******************************************************************************/
void Utils_print_timer(const char* name, const char* msg)
{
  extern FILE* nusmv_stderr;
  TimerBench_ptr timer = hash_timers_lookup(name);
  TIMER_BENCH_CHECK_INSTANCE(timer);

  TimerBench_print(timer, nusmv_stderr, msg);
}


/**Function********************************************************************

  Synopsis           [Escapes all characters in given string, and dumps them 
  into the xml file]

  Description        []

  SideEffects        []

******************************************************************************/
void Utils_str_escape_xml_file(const char* str, FILE* file)
{
  /* this table is used to associate a character to a string, for
     character escaping 

     IMPORTANT!! This table if changed has to be updated along with
     escape_table_begin and escape_table_end
  */
  static char escape_table_begin = '\t';
  static char escape_table_end = '>';
  static char* escape_table[] = {
    "&#009;",  /* '\t' */
    "&#010;",  /* '\n' */
    "&#011;",  /* '\v' */
    "&#012;",  /* '\f' */
    "&#013;",  /* '\r' */
    "&#014;",  /* SO   */
    "&#015;",  /* SI   */
    "&#016;",  /* DLE  */
    "&#017;",  /* DC1  */
    "&#018;",  /* DC2  */
    "&#019;",  /* DC3  */
    "&#020;",  /* DC4  */
    "&#021;",  /* NAK  */
    "&#022;",  /* SYN  */
    "&#023;",  /* ETB  */
    "&#024;",  /* CAN  */
    "&#025;",  /* EM   */
    "&#026;",  /* SUB  */
    "&#027;",  /* ESC  */
    "&#028;",  /* FS   */
    "&#029;",  /* GS   */
    "&#030;",  /* RS   */
    "&#031;",  /* US   */
    "&#032;",  /* ' '  */
    "&#033;",  /* '!'  */
    "&quot;",  /* '"'  */
    "#",       /* '#'  */
    "$",       /* '$'  */
    "%",       /* '%'  */
    "&amp;",   /* '&'  */
    "&apos;",  /* '''  */
    "(",       /* '('  */
    ")",       /* ')'  */
    "*",       /* '*'  */
    "+",       /* '+'  */
    ",",       /* ','  */
    "-",       /* '-'  */
    ".",       /* '.'  */
    "/",       /* '/'  */
    "0",       /* '0'  */
    "1",       /* '1'  */
    "2",       /* '2'  */
    "3",       /* '3'  */
    "4",       /* '4'  */
    "5",       /* '5'  */
    "6",       /* '6'  */
    "7",       /* '7'  */
    "8",       /* '8'  */
    "9",       /* '9'  */
    ":",       /* ':'  */
    ";",       /* ';'  */
    "&lt;",    /* '<'  */
    "=",       /* '='  */
    "&gt;",    /* '>'  */
  };

  if ((char*) NULL != str) {
    const char* iter;   
    char c;
    for (iter=str, c=*iter; c != '\0'; c=*(++iter)) {
      if (escape_table_begin <= c && c <= escape_table_end) {
        /* in table */
        fputs(escape_table[c-escape_table_begin], file);
      }
      else { /* not in table */
        fputc(c, file);
      }
    }
  }
}


/**Function********************************************************************

  Synopsis           [Computes the log2 of the given unsigned argument
                      rounding the result to the closest upper
                      integer. 0 gives 1 as result.]

  Description [This function can be used to calculate the number of
  bits needed to represent a value.]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
int Utils_log2_round(unsigned long long int a)
{
  int res;

  if (0 == a) return 1; /* special case */

  res = 0;
  while (a != 0) {
    a >>= 1;
    ++res;
  }
  return res;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis           [Private service for Utils_FreeListOfLists]

  SideEffects        []

  SeeAlso            [Utils_FreeListOfLists]

******************************************************************************/
static void freeListOfLists_aux(lsList list)
{
  lsDestroy(list, NULL);
}

/**Function********************************************************************

  Synopsis           [Initializes the hash_timers hash]

  Description        [Initializes the hash_timers hash]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hash_timers_init(void)
{
  nusmv_assert((hash_ptr)NULL == hash_timers);
  hash_timers = new_assoc();
  nusmv_assert((hash_ptr)NULL != hash_timers);
}

/**Function********************************************************************

  Synopsis           []

  Description        []

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static enum st_retval hash_timers_quit_fun_aux(char*k, char* e, char* a)
{
  hash_timers_DESTROY fun = (hash_timers_DESTROY)e;
  fun((TimerBench_ptr)e);
  return ASSOC_DELETE;
}

/**Function********************************************************************

  Synopsis           [Deinitializes the hash_timers hash]

  Description        [Deinitializes the hash_timers hash]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hash_timers_quit_fun(hash_timers_DESTROY fun)
{
  nusmv_assert((hash_ptr)NULL != hash_timers);
  clear_assoc_and_free_entries_arg(hash_timers,
                                   hash_timers_quit_fun_aux,
                                   (char*)fun);
  free_assoc(hash_timers);
  hash_timers = (hash_ptr)NULL;
}

/**Function********************************************************************

  Synopsis           [Looks up in the hash_timers hash]

  Description        [Looks up in the hash_timers hash]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static TimerBench_ptr hash_timers_lookup(const char* key)
{
  nusmv_assert((hash_ptr)NULL != hash_timers);
  return (TimerBench_ptr)find_assoc(hash_timers, (node_ptr)key);
}

/**Function********************************************************************

  Synopsis           [Inserts into the hash_timers hash]

  Description        [Inserts into the hash_timers hash]

  SideEffects        []

  SeeAlso            []

******************************************************************************/
static void hash_timers_insert(const char* key, TimerBench_ptr val)
{
  nusmv_assert((hash_ptr)NULL != hash_timers);
  insert_assoc(hash_timers, (node_ptr)key, (node_ptr)val);
}


/* ---------------------------------------------------------------------- */

