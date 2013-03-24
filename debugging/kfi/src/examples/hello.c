/* gcc -finstrument-functions hello.c -o hello -lfunc_profile */

#include <stdio.h>

void dummy(void){
    printf("dummy called\n");
}

main(){
   dummy();
   return 0;
}
