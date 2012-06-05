/**CFile***********************************************************************

  FileName    [TimerBench.c]

  PackageName [utils]

  Synopsis    [Implementation of class TimerBench]

  Description []

  SeeAlso     []

  Author      [Roberto Cavada]

  Copyright   [
  This file is part of the ``utils'' package of NuSMV version 2. 
  Copyright (C) 2008 by FBK-irst. 

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

#include "utils/TimerBench.h"

static char rcsid[] UTIL_UNUSED = "$Id: TimerBench.c,v 1.1.2.2 2009-08-05 13:58:00 nusmv Exp $";


typedef struct TimerBench_TAG {
  long start_time;
  long acc_time;
  long laps;

  char* name;
} TimerBench;


#define TIMER_BENCH_STOPPED -1

TimerBench_ptr TimerBench_create(const char* name)
{
  TimerBench_ptr self = (TimerBench_ptr) ALLOC(TimerBench, 1);
  TIMER_BENCH_CHECK_INSTANCE(self);
  
  self->name = util_strsav((char*) name);
  self->start_time = TIMER_BENCH_STOPPED;
  self->acc_time = 0;
  self->laps = 0;
  return self;
}

void TimerBench_destroy(TimerBench_ptr self)
{
  TIMER_BENCH_CHECK_INSTANCE(self);
  FREE(self->name);
  FREE(self);
}


void TimerBench_start(TimerBench_ptr self)
{
  long now = util_cpu_time();

  TIMER_BENCH_CHECK_INSTANCE(self);
  nusmv_assert(!TimerBench_is_running(self));

  self->start_time = now;
  self->laps += 1;
}

void TimerBench_stop(TimerBench_ptr self)
{
  long now = util_cpu_time();

  TIMER_BENCH_CHECK_INSTANCE(self);
  nusmv_assert(TimerBench_is_running(self));

  self->acc_time += (now - self->start_time);
  self->start_time = TIMER_BENCH_STOPPED;
}


void TimerBench_reset(TimerBench_ptr self)
{
  long now = util_cpu_time();

  TIMER_BENCH_CHECK_INSTANCE(self);

  self->acc_time = 0;
  self->laps = 0;
  if (TimerBench_is_running(self)) self->start_time = now;
}

boolean TimerBench_is_running(const TimerBench_ptr self)
{
  TIMER_BENCH_CHECK_INSTANCE(self);  
  return (self->start_time != TIMER_BENCH_STOPPED);
}

long TimerBench_get_time(const TimerBench_ptr self)
{
  TIMER_BENCH_CHECK_INSTANCE(self);  

  if (TimerBench_is_running(self)) {
    long now = util_cpu_time();
    return (now - self->start_time + self->acc_time);
  }

  return self->acc_time;
}

long TimerBench_get_laps(const TimerBench_ptr self)
{
  TIMER_BENCH_CHECK_INSTANCE(self);  
  return self->laps;
}


/** Prints:
    TIMER name # msg # time # laps # status 
    msg can be NULL */
void TimerBench_print(const TimerBench_ptr self, FILE* file, 
                      const char* msg)
{
  long time;
  
  TIMER_BENCH_CHECK_INSTANCE(self);
  
  time = TimerBench_get_time(self);
  if (msg != NULL) {
    fprintf(file, "TIMER %s # %s # %ld # ", self->name, msg, time);
  }
  else fprintf(file, "TIMER %s # # %ld # ", self->name, time);
  
  fprintf(file, "%ld laps # ", TimerBench_get_laps(self));

  if (TimerBench_is_running(self)) fprintf(file, "Running\n");
  else fprintf(file, "Stopped\n");
}

