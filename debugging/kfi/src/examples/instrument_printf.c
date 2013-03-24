/*
 * Written by Der Herr Hofrat, der.herr@hofr.at
 * (C) 2004 OpenTech EDV Research GmbH
 * License: GPL Version 2
 */
/* Trivial example of using function instrumentation provided by gcc
 * compile: gcc -finstrument-function instrument_printf.c -o instrument_printf
 * run    : ./instrument_printf
 *        
 * calls are (only one in this case) are printed to stdout.
 */
#include <stdio.h>

void  __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *this_fn, void *call_site)
{
	printf("func_enter: function = %p, called by = %p\n",
		this_fn, 
		call_site);
}

void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this_fn, void *call_site)
{
	printf("func_exit: function = %p, called by = %p\n",
		this_fn, 
		call_site);
}

main(){
   printf("hello world\n");
   return 0;
}
