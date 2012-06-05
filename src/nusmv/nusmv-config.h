#ifndef _NUSMV_CONFIG_H
#define _NUSMV_CONFIG_H 1
 
/* nusmv-config.h. Generated automatically at end of configure. */
/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Define to 1 if the `closedir' function returns void instead of `int'. */
/* #undef CLOSEDIR_VOID */

/* Executable file names suffix */
#ifndef NUSMV_EXEEXT
#define NUSMV_EXEEXT ""
#endif

/* Define to __attribute((noreturn)) or empty if not available */
#ifndef NUSMV_FUNCATTR_NORETURN
#define NUSMV_FUNCATTR_NORETURN __attribute__ ((noreturn))
#endif

/* Enables the named addon */
#ifndef NUSMV_HAVE_COMPASS
#define NUSMV_HAVE_COMPASS 1
#endif

/* Have preprocessor */
#ifndef NUSMV_HAVE_CPP
#define NUSMV_HAVE_CPP 1
#endif

/* If cudd-2.4 or greater is available */
#ifndef NUSMV_HAVE_CUDD_24
#define NUSMV_HAVE_CUDD_24 1
#endif

/* Define to 1 if you have the <dirent.h> header file, and it defines `DIR'.
   */
#ifndef NUSMV_HAVE_DIRENT_H
#define NUSMV_HAVE_DIRENT_H 1
#endif

/* Define to 1 if you have the <dlfcn.h> header file. */
#ifndef NUSMV_HAVE_DLFCN_H
#define NUSMV_HAVE_DLFCN_H 1
#endif

/* Define to 1 if you don't have `vprintf' but do have `_doprnt.' */
/* #undef HAVE_DOPRNT */

/* Define to 1 if you have the <errno.h> header file. */
#ifndef NUSMV_HAVE_ERRNO_H
#define NUSMV_HAVE_ERRNO_H 1
#endif

/* Define to 1 if you have the <float.h> header file. */
#ifndef NUSMV_HAVE_FLOAT_H
#define NUSMV_HAVE_FLOAT_H 1
#endif

/* Define to 1 if you have the `floor' function. */
#ifndef NUSMV_HAVE_FLOOR
#define NUSMV_HAVE_FLOOR 1
#endif

/* Define to 1 if you have the `getenv' function. */
#ifndef NUSMV_HAVE_GETENV
#define NUSMV_HAVE_GETENV 1
#endif

/* Define to 1 if you have the `getpid' function. */
#ifndef NUSMV_HAVE_GETPID
#define NUSMV_HAVE_GETPID 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#ifndef NUSMV_HAVE_INTTYPES_H
#define NUSMV_HAVE_INTTYPES_H 1
#endif

/* Define to 1 if you have the `isatty' function. */
#ifndef NUSMV_HAVE_ISATTY
#define NUSMV_HAVE_ISATTY 1
#endif

/* Disables expat usage */
#ifndef NUSMV_HAVE_LIBEXPAT
#define NUSMV_HAVE_LIBEXPAT 1
#endif

/* Define to 1 if you have the `m' library (-lm). */
#ifndef NUSMV_HAVE_LIBM
#define NUSMV_HAVE_LIBM 1
#endif

/* Uses the private readline */
#ifndef NUSMV_HAVE_LIBREADLINE
#define NUSMV_HAVE_LIBREADLINE 1
#endif

/* Define to 1 if you have the <limits.h> header file. */
#ifndef NUSMV_HAVE_LIMITS_H
#define NUSMV_HAVE_LIMITS_H 1
#endif

/* Define to 1 if your system has a GNU libc compatible `malloc' function, and
   to 0 otherwise. */
#ifndef NUSMV_HAVE_MALLOC
#define NUSMV_HAVE_MALLOC 1
#endif

/* Defined to 1 if the system provides malloc.h */
/* #undef HAVE_MALLOC_H */

/* Define to 1 if you have the `memmove' function. */
#ifndef NUSMV_HAVE_MEMMOVE
#define NUSMV_HAVE_MEMMOVE 1
#endif

/* Define to 1 if you have the <memory.h> header file. */
#ifndef NUSMV_HAVE_MEMORY_H
#define NUSMV_HAVE_MEMORY_H 1
#endif

/* Define to 1 if you have the `memset' function. */
#ifndef NUSMV_HAVE_MEMSET
#define NUSMV_HAVE_MEMSET 1
#endif

/* Define to 1 if you have the `mkstemp' function. */
#ifndef NUSMV_HAVE_MKSTEMP
#define NUSMV_HAVE_MKSTEMP 1
#endif

/* Define to 1 if you have the `mktemp' function. */
#ifndef NUSMV_HAVE_MKTEMP
#define NUSMV_HAVE_MKTEMP 1
#endif

/* Define to 1 if you have the <ndir.h> header file, and it defines `DIR'. */
/* #undef HAVE_NDIR_H */

/* Uses the old version of zchaff */
/* #undef HAVE_OLD_ZCHAFF */

/* Define to 1 if you have the `popen' function. */
#ifndef NUSMV_HAVE_POPEN
#define NUSMV_HAVE_POPEN 1
#endif

/* Define to 1 if you have the `pow' function. */
#ifndef NUSMV_HAVE_POW
#define NUSMV_HAVE_POW 1
#endif

/* Define to 1 if you have the `random' function. */
#ifndef NUSMV_HAVE_RANDOM
#define NUSMV_HAVE_RANDOM 1
#endif

/* Define to 1 if your system has a GNU libc compatible `realloc' function,
   and to 0 otherwise. */
#ifndef NUSMV_HAVE_REALLOC
#define NUSMV_HAVE_REALLOC 1
#endif

/* Define to 1 if you have the <regex.h> header file. */
#ifndef NUSMV_HAVE_REGEX_H
#define NUSMV_HAVE_REGEX_H 1
#endif

/* No sat solvers available */
#ifndef NUSMV_HAVE_SAT_SOLVER
#define NUSMV_HAVE_SAT_SOLVER 1
#endif

/* Define to 1 if you have the `setvbuf' function. */
#ifndef NUSMV_HAVE_SETVBUF
#define NUSMV_HAVE_SETVBUF 1
#endif

/* Define to 1 if you have the <signal.h> header file. */
#ifndef NUSMV_HAVE_SIGNAL_H
#define NUSMV_HAVE_SIGNAL_H 1
#endif

/* Disables Minisat */
#ifndef NUSMV_HAVE_SOLVER_MINISAT
#define NUSMV_HAVE_SOLVER_MINISAT 1
#endif

/* Disables ZChaff */
#ifndef NUSMV_HAVE_SOLVER_ZCHAFF
#define NUSMV_HAVE_SOLVER_ZCHAFF 0
#endif

/* Define to 1 if you have the `srandom' function. */
#ifndef NUSMV_HAVE_SRANDOM
#define NUSMV_HAVE_SRANDOM 1
#endif

/* Define to 1 if `stat' has the bug that it succeeds when given the
   zero-length file name argument. */
/* #undef HAVE_STAT_EMPTY_STRING_BUG */

/* Define to 1 if stdbool.h conforms to C99. */
#ifndef NUSMV_HAVE_STDBOOL_H
#define NUSMV_HAVE_STDBOOL_H 1
#endif

/* Define to 1 if you have the <stddef.h> header file. */
#ifndef NUSMV_HAVE_STDDEF_H
#define NUSMV_HAVE_STDDEF_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#ifndef NUSMV_HAVE_STDINT_H
#define NUSMV_HAVE_STDINT_H 1
#endif

/* Define to 1 if you have the <stdlib.h> header file. */
#ifndef NUSMV_HAVE_STDLIB_H
#define NUSMV_HAVE_STDLIB_H 1
#endif

/* Define to 1 if you have the `strcasecmp' function. */
#ifndef NUSMV_HAVE_STRCASECMP
#define NUSMV_HAVE_STRCASECMP 1
#endif

/* Define to 1 if you have the `strchr' function. */
#ifndef NUSMV_HAVE_STRCHR
#define NUSMV_HAVE_STRCHR 1
#endif

/* Define to 1 if you have the <strings.h> header file. */
#ifndef NUSMV_HAVE_STRINGS_H
#define NUSMV_HAVE_STRINGS_H 1
#endif

/* Define to 1 if you have the <string.h> header file. */
#ifndef NUSMV_HAVE_STRING_H
#define NUSMV_HAVE_STRING_H 1
#endif

/* Define to 1 if you have the `strrchr' function. */
#ifndef NUSMV_HAVE_STRRCHR
#define NUSMV_HAVE_STRRCHR 1
#endif

/* Define to 1 if you have the `strstr' function. */
#ifndef NUSMV_HAVE_STRSTR
#define NUSMV_HAVE_STRSTR 1
#endif

/* Define to 1 if you have the `strtol,' function. */
/* #undef HAVE_STRTOL_ */

/* Define to 1 if you have the `system' function. */
#ifndef NUSMV_HAVE_SYSTEM
#define NUSMV_HAVE_SYSTEM 1
#endif

/* Define to 1 if you have the <sys/dir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_DIR_H */

/* Define to 1 if you have the <sys/ioctl.h> header file. */
#ifndef NUSMV_HAVE_SYS_IOCTL_H
#define NUSMV_HAVE_SYS_IOCTL_H 1
#endif

/* Defined to 1 if the system provides sys/malloc.h */
#ifndef NUSMV_HAVE_SYS_MALLOC_H
#define NUSMV_HAVE_SYS_MALLOC_H 1
#endif

/* Define to 1 if you have the <sys/ndir.h> header file, and it defines `DIR'.
   */
/* #undef HAVE_SYS_NDIR_H */

/* Define to 1 if you have the <sys/param.h> header file. */
#ifndef NUSMV_HAVE_SYS_PARAM_H
#define NUSMV_HAVE_SYS_PARAM_H 1
#endif

/* Define to 1 if you have the <sys/resource.h> header file. */
#ifndef NUSMV_HAVE_SYS_RESOURCE_H
#define NUSMV_HAVE_SYS_RESOURCE_H 1
#endif

/* Define to 1 if you have the <sys/signal.h> header file. */
#ifndef NUSMV_HAVE_SYS_SIGNAL_H
#define NUSMV_HAVE_SYS_SIGNAL_H 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#ifndef NUSMV_HAVE_SYS_STAT_H
#define NUSMV_HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#ifndef NUSMV_HAVE_SYS_TIME_H
#define NUSMV_HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#ifndef NUSMV_HAVE_SYS_TYPES_H
#define NUSMV_HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the `tmpnam' function. */
#ifndef NUSMV_HAVE_TMPNAM
#define NUSMV_HAVE_TMPNAM 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#ifndef NUSMV_HAVE_UNISTD_H
#define NUSMV_HAVE_UNISTD_H 1
#endif

/* Define to 1 if you have the `vprintf' function. */
#ifndef NUSMV_HAVE_VPRINTF
#define NUSMV_HAVE_VPRINTF 1
#endif

/* Define to 1 if the system has the type `_Bool'. */
#ifndef NUSMV_HAVE__BOOL
#define NUSMV_HAVE__BOOL 1
#endif

/* Library bug message */
#ifndef NUSMV_LIBRARY_BUGREPORT
#define NUSMV_LIBRARY_BUGREPORT "Please report bugs to <nusmv-users@fbk.eu>"
#endif

/* Library build date */
#ifndef NUSMV_LIBRARY_BUILD_DATE
#define NUSMV_LIBRARY_BUILD_DATE "Mon Mar  5 15:55:46 UTC 2012"
#endif

/* Library Email */
#ifndef NUSMV_LIBRARY_EMAIL
#define NUSMV_LIBRARY_EMAIL "nusmv-users@list.fbk.eu"
#endif

/* Library Name */
#ifndef NUSMV_LIBRARY_NAME
#define NUSMV_LIBRARY_NAME "NuSMV"
#endif

/* Library Version */
#ifndef NUSMV_LIBRARY_VERSION
#define NUSMV_LIBRARY_VERSION "2.5.4"
#endif

/* Library Website */
#ifndef NUSMV_LIBRARY_WEBSITE
#define NUSMV_LIBRARY_WEBSITE "http://nusmv.fbk.eu"
#endif

/* Names of linked addons */
#ifndef NUSMV_LINKED_CORE_ADDONS
#define NUSMV_LINKED_CORE_ADDONS "compass "
#endif

/* Define to 1 if `lstat' dereferences a symlink specified with a trailing
   slash. */
/* #undef LSTAT_FOLLOWS_SLASHED_SYMLINK */

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#ifndef NUSMV_LT_OBJDIR
#define NUSMV_LT_OBJDIR ".libs/"
#endif

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Define to the address where bug reports for this package should be sent. */
#ifndef NUSMV_PACKAGE_BUGREPORT
#define NUSMV_PACKAGE_BUGREPORT "nusmv-users@fbk.eu"
#endif

/* Build date */
#ifndef NUSMV_PACKAGE_BUILD_DATE
#define NUSMV_PACKAGE_BUILD_DATE "Mon Mar  5 15:55:46 UTC 2012"
#endif

/* Define to the full name of this package. */
#ifndef NUSMV_PACKAGE_NAME
#define NUSMV_PACKAGE_NAME "NuSMV"
#endif

/* Define to the full name and version of this package. */
#ifndef NUSMV_PACKAGE_STRING
#define NUSMV_PACKAGE_STRING "NuSMV 2.5.4"
#endif

/* Define to the one symbol short name of this package. */
#ifndef NUSMV_PACKAGE_TARNAME
#define NUSMV_PACKAGE_TARNAME "nusmv"
#endif

/* Define to the version of this package. */
#ifndef NUSMV_PACKAGE_VERSION
#define NUSMV_PACKAGE_VERSION "2.5.4"
#endif

/* Preprocessor call string */
#ifndef NUSMV_PROG_CPP
#define NUSMV_PROG_CPP "gcc -E -x c"
#endif

/* Define as the return type of signal handlers (`int' or `void'). */
#ifndef NUSMV_RETSIGTYPE
#define NUSMV_RETSIGTYPE void
#endif

/* The size of `int', as computed by sizeof. */
#ifndef NUSMV_SIZEOF_INT
#define NUSMV_SIZEOF_INT 4
#endif

/* The size of `long', as computed by sizeof. */
#ifndef NUSMV_SIZEOF_LONG
#define NUSMV_SIZEOF_LONG 8
#endif

/* The size of `long long', as computed by sizeof. */
#ifndef NUSMV_SIZEOF_LONG_LONG
#define NUSMV_SIZEOF_LONG_LONG 8
#endif

/* The size of `void *', as computed by sizeof. */
#ifndef NUSMV_SIZEOF_VOID_P
#define NUSMV_SIZEOF_VOID_P 8
#endif

/* Define to 1 if you have the ANSI C header files. */
#ifndef NUSMV_STDC_HEADERS
#define NUSMV_STDC_HEADERS 1
#endif

/* Define to 1 if `lex' declares `yytext' as a `char *' by default, not a
   `char[]'. */
#ifndef NUSMV_YYTEXT_POINTER
#define NUSMV_YYTEXT_POINTER 1
#endif

/* Define to __FUNCTION__ or "" if `__func__' does not conform to ANSI C. */
/* #undef __func__ */

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to rpl_malloc if the replacement function should be used. */
/* #undef malloc */

/* Define to rpl_realloc if the replacement function should be used. */
/* #undef realloc */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */
 
/* once: _NUSMV_CONFIG_H */
#endif
