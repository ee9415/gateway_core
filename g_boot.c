 /*-----------------------------------------*
	            g_boot.c
		coded by H.B. Lee
  		    Feb/2007
	        Revisoned May/2009
 *-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

#include "g_macros.h"
#include "g_config.h"

char	*av[3];

main(int ac, char *av[])
{
    FILE   *lp;
    int	    st;
    time_t  ct;

    lp = fopen("/tmp/mb3/Eboot.log", "a");
    //lp = fopen("./Eboot.log", "a");
    //lp = fopen(Log_file_nam2, "a");

    av[0] = "Egtwy";
    av[1] = NULL;

    time(&ct);
    if(lp)
        fprintf(lp, "Gateway starts at %s", ctime(&ct));

    for( ; ; ) {

	    sleep(3);

        if (fork() > 0) { /* parent */

            wait(&st);

    	    time(&ct);

            if(WIFEXITED(st)) {
                D02("Gateway restarts(exit status=%d) at %s", WEXITSTATUS(st), ctime(&ct));
                if(lp)
    	    	    fprintf(lp, "Gateway restarts(exit status=%d) at %s", WEXITSTATUS(st), ctime(&ct));
            }
            else if(WIFSIGNALED(st)) {
                D02("Gateway restarts(signal=%d) at %s", WTERMSIG(st), ctime(&ct));
                if(lp)
    	    	    fprintf(lp, "Gateway restarts(signal=%d) at %s", WTERMSIG(st), ctime(&ct));
            }
            else {
                D01("Gateway restarts(cause unknown) at %s", ctime(&ct));
                if(lp)
    	    	    fprintf(lp, "Gateway restarts(cause unknown) at %s", ctime(&ct));
            }
	        if(lp) {
	            D01("Gateway at %s", ctime(&ct));
	            fflush(lp);
	        }
        }
        else {
            D01("Gateway restarts at %s",  ctime(&ct));
            execv("/tmp/mb3/Egtwy", av);
            //execv("./Egtwy", av);
            //execv(Execute_file, av);
            exit(0);
	    }
    }
}

