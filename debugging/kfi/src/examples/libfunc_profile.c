/*
 * Written by Der Herr Hofrat, <der.herr@hofr.at>
 * Copyright (C) 2004 OpenTech EDV Research GmbH
 * License: GPL Version 2
 */
/*
 * Compile as shared library with:
 * gcc -fPIC -Wall -g -O2 -shared  -o libfunc_profile.so.0 libfunc_profile.c
 *
 * Log all function calls to to the logfile in /tmp/func.log (default)
 * the format is chosen to allow usage of kfiresolver.py to reverse the log
 * entriew 
 */

#include <stdio.h>     /* fprintf */
#include <unistd.h>    /* exit , getpid*/
#include <sys/types.h> /* getpid */
#include <stdlib.h>    /* getenv */

#define _FCNTL_H
#include <bits/fcntl.h>

/* initialize and cleanup logfile(s) on load/unload of lib */
void __func_profile_init(void) __attribute((constructor));
void __func_profile_exit(void) __attribute((destructor));

long long int start,last,now;
FILE *logfile;
char default_fname[]="/tmp/func.log";
char *logfile_name;

__inline__ unsigned long long int hwtime(void)
{
	unsigned long long int x;
	__asm__ __volatile__("rdtsc\n\t"
		:"=A" (x));
	return x;
}

void  __attribute__((__no_instrument_function__))
__func_profile_init(void)
{
   if ((logfile_name = getenv("PROFILE_LOG")) != 0) {
      printf("using %s\n",logfile_name);
	} else {
		logfile_name=default_fname;
      printf("using %s (no PROFILE_LOG set in environment)\n",logfile_name);
   }

	if((logfile=fopen(logfile_name,"a+")) == NULL )
	{
		perror("Cannot open logfile\n");
		exit(-1);
	}

    /* logfile header */
    fprintf(logfile,
	" Entry      Delta       PID      Function    Caller\n");
    fprintf(logfile,
	 "--------   --------   --------   --------   --------\n");

    /* initialize time stamp */
    start=hwtime();
    last=start;
}

void  __attribute__((__no_instrument_function__))
__func_profile_exit(void)
{
	fclose(logfile);
}

void  __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
	unsigned long long delta;
	pid_t pid=getpid();
	delta=0LL;

	now=hwtime();
	delta=now-last;
	last=now;

	fprintf(logfile, "%8lu   %8lu   %7d    %08x   %08x\n",
		(unsigned long)(now-start),
		(unsigned long)delta,
		pid,
		(unsigned int)this_fn,
		(unsigned int)call_site);
}

void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
	unsigned long long delta;
	pid_t pid=getpid();
	delta=0LL;

	now=hwtime();
	delta=now-last;
	last=now;

	fprintf(logfile, "%8lu   %8lu   %7d    %08x   %08x\n",
		(unsigned long)(now-start),
		(unsigned long)delta,
		pid,
		(unsigned int)this_fn,
		(unsigned int)call_site);
}
