/* LINTLIBRARY */

#include <stdio.h>
#include "util.h"

#if defined(BSD) && !defined(__MINGW32__) 
#include <pwd.h>
#endif

/* NuSMV: add begin */
static char* get_user_home(const char* username);

#define HOME_LEN  256
#define USER_LEN  256

/* Substitutes tilde, and '/' with '\\' under windows. 
   Returned string must be destroyed */
char* util_tilde_expand(char* fname)
{
  char username[USER_LEN];
  char* filename = (char*) NULL;
  char* home;
  size_t len;
  register int i, j;

  len = strlen(fname) + HOME_LEN + USER_LEN;

  filename = ALLOC(char, len);
  assert(filename != (char*) NULL);
  filename[0] = '\0';
  i = 0;

  if (fname[i] == '~') {
    j = 0; ++i;
    while ((fname[i] != '\0') && (fname[i] != '/') && (fname[i] != '\\')) {
      username[j++] = fname[i++];
    }
    username[j] = '\0';

    home = get_user_home(username);
    if (strlen(home) > 0) {
      strncpy(filename, home, (len-1) * sizeof(char));
      len -= strlen(home);
    }
    else i = 0;
    FREE(home);
  }

  /* Concantenate remaining portion of file name */
  strncat(filename, fname + i, (len-1) * sizeof(char));
  
# if defined(__MINGW32__)
  {
    /* substitute all '/' with '\\' */
    char* iter = filename;
    while(*iter != '\0') {
      if ((*iter) == '/') *iter='\\';
      ++iter;
    }
  }
# endif /* __MINGW32__ */
  
  return filename;
}


/* Returns the user home path. If user is NULL, or the empty string,
   then the current user is taken into account. An empty string is
   returned if no home is found. String must be freed */
static char* get_user_home(const char* username)
{
  char home[HOME_LEN];
  char* var;

  home[0] = '\0';
  
# if defined(BSD) && !defined(__MINGW32__)
  var = getenv("HOME");
  if ((char*) NULL == var) return util_strsav("");

  if (((char*) NULL != username) && ('\0' != username[0])) {
    /* substitutes the given user within var */
    int i;
    int start_user_name;

    /* gets rid of all final '/' */
    for (i=strlen(var)-1; i>=0 && '/' == var[i]; --i);

    /* continues searching the previous '/' */
    for (; i>=0 && '/' != var[i]; --i);
    start_user_name = i; /* notice that this can be -1 */
    
    /* copies 'til the '/' before the username */
    strncpy(home, var, start_user_name+1); 
    /* substitutes the username */
    strncpy(home+start_user_name+1, username, HOME_LEN-start_user_name-1);
    /* appends the terminator for safety */ 
    home[HOME_LEN-start_user_name-2] = '\0';
  }
  else {
    strncpy(home, var, HOME_LEN-1);
     home[HOME_LEN-1] = '\0'; /* appends the terminator for safety */ 
  }

 # elif defined(__MINGW32__) && !defined(UNDER_CE) 
   var = getenv("HOME");
   if (var != (char*) NULL) {	
     strncpy(home, var, (HOME_LEN-1) * sizeof(char));
   } 
   else {
     var = getenv("USERPROFILE");
     if (var != (char*) NULL) {
       strncpy(home, var, (HOME_LEN-1) * sizeof(char));
     }
     else {
       char* homepath;
       homepath = getenv("HOMEPATH");
       if (homepath != (char*) NULL) {
	 char* drive;
	 drive = getenv("HOMEDRIVE");
	 if (drive != (char*) NULL) {
	   snprintf(home, HOME_LEN-1, "%s\\%s", drive, homepath); 
	 }
       }
     }
   }
   home[HOME_LEN-1] = '\0'; /* null terminates for safety */

   if ((username != (char*) NULL) && (username[0] != '\0')) {
     if (home[0] != '\0') {
       /* tries to substitute the user name with the specified username */
       char* curr_username;

       curr_username = getenv("USERNAME");
       if (curr_username == (char*) NULL) {
	 /* tries to retrieve the user name from the home path */
	 curr_username = strrchr(home, '\\');
	 if (curr_username == (char*) NULL) curr_username = strrchr(home, '/');

	 /* skips the '\\' or '/' */
	 if (curr_username != (char*) NULL) ++curr_username; 
       }
       if (curr_username != (char*) NULL) {
	 char* last = NULL;
	 char* pos = home;
	 /* searches for the username part */
	 while(1) {
	   pos = strstr(pos, curr_username); 
	   if (pos != (char*) NULL) {
	     last = pos;
	     ++pos;
	   }
	   else break;
	 }

	 /* last contains here the position of curr_username, substitute */
	 if (last != (char*) NULL) {
	   strncpy(last, username, ((HOME_LEN-1) * sizeof(char)) - (last-home));
	   home[HOME_LEN-1] = '\0'; /* terminates for safety */
	 }
       }
     }
   } /* username specified */

 #elif defined(__MINGW32__) && defined(UNDER_CE)
   return util_strsav("");
 #endif

   return util_strsav(home);
}
/* NuSMV: add end */
