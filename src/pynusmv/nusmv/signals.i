%{
#include <setjmp.h>
#include <signal.h>

static sigjmp_buf timeout;

static void backout(int sig) {
  siglongjmp(timeout, sig);
}
%}

%include <exception.i>

%exception {
  if (!sigsetjmp(timeout, 1)) {
    signal(SIGINT,backout); // Check return?
    $action
  }
  else {
    // raise a Python exception
    SWIG_exception(SWIG_RuntimeError, "Interruption in $decl");
  }
}