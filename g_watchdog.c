#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <pthread.h>
//#include <linux/watchdog.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_pack.h"
#include "g_util.h"
#include "g_gtway.h"

void *do_gtwy_hw_wdog(void *arg);
#define WATCHDOG_TIME 30


/******************************************/
/***** for Gateway Watchdog interface *****/
/******************************************/
void *do_gtwy_hw_wdog(void *arg)
{
    char *wdog_dev = (char *)arg;
    int wd;
    
    if(Gtwy_hwdg_sec <= 6) {
        D20("Watchdog Sec Error 'Over 7' !!! \n");
        exit(1);
    }

    if(Gtwy_hwdg_use) {
        if((wd = open(wdog_dev, O_RDWR)) < 0) {
            L01("Watchdog dev(%s) open error!\n", wdog_dev);
            exit(1);
        }
    }

    while( 1 )
    {
        D00( "\nWatchdog 20 sec\n\n" );
        sleep(20);

        if ( Gtwy_hwdg_use ) {
            write(wd, "\n", 1);
            fsync(wd);
        }
    }
    close(wd);
}

