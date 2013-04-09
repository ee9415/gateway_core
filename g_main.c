 /*-----------------------------------------*
	            g_main.c
		coded by H.B. Lee
  		    Feb/2007
	        Revisoned May/2009
 *-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

#include <pthread.h>
#include <sched.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_gtway.h"

main(int ac, char *av[])
{
    pthread_t th_node_listen, th_cdma_listen, th_cdsk_listen, th_cdsk_pppd,
              th_stwc_listen, th_gtwy_timer,  th_wdog_timer,  th_gtwy_stat, th_watchdog;
    UBUF    *ubp;
    int     rt_min, rt_max, hlp = 0, st;

    struct  sched_param sched;
    time_t  ct;

    if (ac > 1 && av[1][0] == '-') {
        if (av[1][1] != 'h') {
            D01("Usage : %s [-h]\n", av[0]);
            exit(1);
        }
        hlp = 1;
    } 

    par_init(hlp); /* no return when hlep */

    if (strlen(Log_file_name) > 0) {
    Log_dev = fopen(Log_file_name, "a");
    }

    time(&ct);
    L01("Gateway starts at %s", ctime(&ct));

    que_init();
    ubp_init();
    svr_init();

    rt_min = sched_get_priority_min(SCHED_FIFO);
    rt_max = sched_get_priority_max(SCHED_FIFO);

    if (pthread_create(&th_node_listen, NULL, do_node_listen, (void *)Node_uart_dev) < 0) {
        L00("main(2) : pthread_create(do_node_listen) fail...\n");
        exit(2);
    }
    sched.sched_priority = rt_max;
    pthread_setschedparam(th_node_listen, SCHED_FIFO, &sched);
    pthread_detach(th_node_listen);

    if (Cdma_uart_use) {
        if (pthread_create(&th_cdma_listen, NULL, do_cdma_listen, (void *)Cdma_uart_dev) < 0) {
            L00("main(4) : pthread_create(do_cdma_listen) fail...\n");
            exit(4);
        }
        sched.sched_priority = (rt_max + rt_min) / 2;
        pthread_setschedparam(th_cdma_listen, SCHED_FIFO, &sched);
        pthread_detach(th_cdma_listen);

    } 
    else { /* use cdsk */
        if (pthread_create(&th_cdsk_listen, NULL, do_cdsk_listen, NULL) < 0) {
            L00("main(6) : pthread_create(do_cdsk_listen) fail...\n");
            exit(6);
        }
        sched.sched_priority = (rt_max + rt_min) / 4;
        pthread_setschedparam(th_cdsk_listen, SCHED_FIFO, &sched);
        pthread_detach(th_cdsk_listen);
    }

    if (pthread_create(&th_gtwy_timer , NULL, do_gtwy_timer,  (void *)NULL) < 0) {
        L00("main(5) : pthread_create(do_gtwy_timer) fail...\n");
        exit(5);
    }
    sched.sched_priority = (rt_max + rt_min) / 2;
    pthread_setschedparam(th_gtwy_timer, SCHED_FIFO, &sched);
    pthread_detach(th_gtwy_timer);

    if (pthread_create(&th_gtwy_stat, NULL, do_gtwy_stat,  (void *)NULL) < 0) {
        L00("main(8) : pthread_create(do_statistics) fail...\n");
        exit(8);
    }
    sched.sched_priority = (rt_max + rt_min) / 6;
    pthread_setschedparam(th_gtwy_stat, SCHED_FIFO, &sched);
    pthread_detach(th_gtwy_stat);

    if (Gtwy_stwc_use) {
        if (pthread_create(&th_stwc_listen, NULL, do_stwc_listen,  (void *)Gtwy_stwc_dev) < 0) {
            L00("main(9) : pthread_create(do_stwc_listen) fail...\n");
            exit(9);
    }
        sched.sched_priority = (rt_max + rt_min) / 6;
        pthread_setschedparam(th_stwc_listen, SCHED_FIFO, &sched);
        pthread_detach(th_stwc_listen);
    }

    if (Gtwy_hwdg_use) {
        if (pthread_create(&th_watchdog, NULL, do_gtwy_hw_wdog,  (void *)Gtwy_hwdg_dev) < 0) {
            L00("main(9) : pthread_create(do_gtwy_hw_wdog) fail...\n");
            exit(9);
    }
        sched.sched_priority = (rt_max + rt_min) / 6;
        pthread_setschedparam(th_watchdog, SCHED_FIFO, &sched);
        pthread_detach(th_watchdog);
    }

    if (!Cdma_uart_use) {
        if (Cdsk_pppd_use) {
            if (pthread_create(&th_cdsk_pppd, NULL, do_pppd_control, (void *)NULL) < 0) {
                L00("main(9) : pthread_create(do_cdsk_pppd) fail...\n");
                exit(9);
            }
            sched.sched_priority = (rt_max + rt_min) / 6;
            pthread_setschedparam(th_cdsk_pppd, SCHED_FIFO, &sched);
            pthread_detach(th_cdsk_pppd);
        }
    }
    if (pthread_create(&th_wdog_timer, NULL, do_wdog_timer,  (void *)Gtwy_hwdg_dev) < 0) {
        L00("main(10) : pthread_create(do_wdog_timer) fail...\n");
        exit(10);
    }
    sched.sched_priority = (rt_max + rt_min) / 8;
    pthread_setschedparam(th_wdog_timer, SCHED_FIFO, &sched);
    pthread_detach(th_wdog_timer);

    pause();
}
