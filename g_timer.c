/*-----------------------------------------*
                 g_timer.c
            coded by H.B. Lee
                Feb/2007
             Revisioned May/2009
 *-----------------------------------------*
 *      routines for timers & watchdog     *
 *-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <pthread.h>
 
#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_pack.h"
#include "g_queue.h"
#include "g_util.h"
#include "g_gtway.h"

#define UDHCP_REFRESH_TIME 86400 // 60sec * 60min * 24hour

pthread_mutex_t     Mutx_timr = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      Cond_timr = PTHREAD_COND_INITIALIZER;
pthread_condattr_t  Attr_timr;

pthread_mutex_t     Mutx_stat = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      Cond_stat = PTHREAD_COND_INITIALIZER;
pthread_condattr_t  Attr_stat;

extern int    	    Pppd_onfrq_no;

void   wait_timr();
void   wait_stat();
void   wakeup_timr();
void   wakeup_stat();
void  *do_gtwy_timr(void *arg);
void  *do_gtwy_stat(void *arg);

/*-----------------------------------------*
 * sw timer triggered by node event or wdog*
 *-----------------------------------------*/
void *do_gtwy_timer(void *arg)
{
    QUEUE *q;
    char  *type;
    time_t tim;
	struct  timeval     current_time;
	long int before_time = 0;
    int    twc_slp, i;

    if (Node_igtr_sec && Node_sfmi_sec < Node_igtr_sec)
	    Node_igtr_sec = 0;

    if (!Srvr_conn_frq && Node_sfmi_sec < Node_igtr_sec + Cdsk_pppd_trt*2)
        Srvr_conn_frq = 1; 	     /* impossible twice connection    */

    if (!Srvr_conn_frq)
	    twc_slp = Node_sfmi_sec - Node_igtr_sec - 2;
    else
	    twc_slp = 0;

    pthread_cond_init(&Cond_timr, &Attr_timr);

    pppd_frq_init();	/* Pppd_onfrq_no, Pppd_shut_way */

	before_time = gettimeofday( &current_time, NULL );

    sleep(Node_sfmi_sec+1);
    while(1) {

    	wait_timr();
    	D01("\ngtwy timer:%d...........\n\n", Node_conn_cnt);
        time(&tim);
		gettimeofday( &current_time, NULL );

    	if (Node_igtr_sec > 0)
    	    sleep(Node_igtr_sec);

    	if (Pppd_onfrq_no && Node_conn_cnt % Srvr_conn_frq == Pppd_onfrq_no)
    	    wakeup_pppd(PPPD_CTRL_DMNON);
    	    
    	if (!Srvr_conn_frq || Node_conn_cnt % Srvr_conn_frq == 0)
    	    wakeup_srvr();

    	if (!Gtwy_stat_frq || Node_conn_cnt % Gtwy_stat_frq == 0)
    	    wakeup_stat();

    	if (twc_slp > 0)
    	    sleep(twc_slp);

    	if (!Srvr_conn_frq)
    	    wakeup_srvr();

		if (current_time.sec > (before_time + UDHCP_REFRESH_TIME))
		{
			D12("\n[UDHCPC] current_time.sec : %d, before_time : %d\n\n", current_time.sec, before_time);
			system(/usr/sbin/udhcpc -q -n -i eth0 -p /var/run/udhcpc.eth0.pid);
			before_time = current_time + UDHCP_REFRESH_TIME;
		}
		D12("\ncurrent_time.sec : %d, before_time : %d\n\n", current_time.sec, before_time);
    }
}

int	Wdog_chck_cnt = 0,
	Wdog_node_evt = 0,
	Wdog_srvr_evt = 0;
/*-----------------------------------------*
 *       for hw & sw watchdog timer        *
 *-----------------------------------------*/
void *do_wdog_timer(void *arg)
{
    time_t  ct;
    int     wd, node_conn_cnt, srvr_conn_cnt, hwdg_frq = 0;
    char    *dp;

    D21("__[%s]__\n", __FUNCTION__);
    node_conn_cnt = Node_conn_cnt;
    srvr_conn_cnt = Srvr_conn_cnt;

    for ( ; ;Wdog_chck_cnt++)
    { 
        sleep(Node_sfmi_sec + 1);

    	time(&ct);
    	dp = ctime(&ct);
    D22("do_wdog_timer: Node_conn_cnt=%d,node_conn_cnt =%d\n", Node_conn_cnt, node_conn_cnt);
    if (Node_conn_cnt > node_conn_cnt) {
    	    node_conn_cnt = Wdog_chck_cnt = Node_conn_cnt;
    	}
    	else {
    	    D22("do_wdog_timer: Wdog_node_evt=%d,%s", Wdog_node_evt, dp);
    	    L02("do_wdog_timer: Wdog_node_evt=%d,%s", Wdog_node_evt, dp);
    	    Wdog_node_evt++;
    	    node_conn_cnt = ++Node_conn_cnt;
    	    wakeup_timr();
    	}
    	    
    	D23("Wdog_chck_cnt=%d, Srvr_conn_frq=%d,srvr_conn_cnt=%d\n", Wdog_chck_cnt, Srvr_conn_frq,srvr_conn_cnt);
        if ((!Srvr_conn_frq) || (Wdog_chck_cnt % Srvr_conn_frq == 1)) {
    	    if (Srvr_conn_cnt > srvr_conn_cnt) {
    	    	srvr_conn_cnt = Srvr_conn_cnt;
    	    	//Wdog_srvr_evt = 0;
    	    }
    	    else {
    	        Wdog_srvr_evt++;
    	        D02("do_wdog_timer: Wdog_srvr_evt=%d,%s", Wdog_srvr_evt, dp);
    	        L02("do_wdog_timer: Wdog_srvr_evt=%d,%s", Wdog_srvr_evt, dp);
    	    }
    	}
    	
        D23("Gtwy_swdg_ndf = %d, Wdog_node_evt=%d, Node_conn_cnt=%d\n", Gtwy_swdg_ndf, Wdog_node_evt, Node_conn_cnt);
        D23("Gtwy_swdg_svf = %d, Wdog_srvr_evt=%d, Srvr_conn_cnt=%d\n", Gtwy_swdg_svf, Wdog_srvr_evt, Srvr_conn_cnt);
        if ((Gtwy_swdg_ndf && Wdog_node_evt > Gtwy_swdg_ndf) || 
    		(Gtwy_swdg_svf && Wdog_srvr_evt > Gtwy_swdg_svf)) {
    	    D01("do_wdog_timer: system reboot,%s", ctime(&ct));
    	    L01("do_wdog_timer: system reboot,%s", ctime(&ct));
                system("reboot");
    	}
    }
}

extern int Cdsk_con1_err[], Cdsk_con2_err[];

void *do_gtwy_stat(void *arg)
{
    QUEUE *q;
    char  *type, host[25];
    time_t tim;
    int    i;

    memset((void *)host, ' ', sizeof(host));
    host[sizeof(host) - 1] = 0;

    if((i = strlen(Cdsk_srv_addr)) > 20)
        i = 20;
    memcpy(host, Cdsk_srv_addr, i);
    sprintf(host + i, "|%d", Cdsk_srv_port);
    host[strlen(host)] = ' ';

    pthread_cond_init(&Cond_stat, &Attr_stat);

    while(1) {

        wait_stat();
        sleep(2);	/* do after server interface */
        time(&tim);

        SP0(" ====================================================================\n");
        SP4("     GID:[%d]:   [%s%s] %s", My_gateway_id, __TIME__, __DATE__, ctime(&tim) );
        SP0("     QUEUE         trcvd      tsend      tfail      taway      npack\n");
        SP0("  -----------  ---------  ---------  ---------  ---------  ---------\n");

        for (i = 0, q = Queue; i < N_OF_QUEUE; i++, q++) {
            switch(q->type) {
                case UP_UGNT_DTA :  type = "UP_UGNT_DTA"; break;
                case UP_NORM_CMD :  type = "UP_NORM_CMD"; break;
                case UP_NORM_DTA :  type = "UP_NORM_DTA"; break;
                case UP_UGNT_IG2 :  type = "UP_UGNT_IG2"; break;
                case UP_UGNT_IG1 :  type = "UP_UGNT_IG1"; break;
                case UP_NORM_IG2 :  type = "UP_NORM_IG2"; break;
                case UP_NORM_IG1 :  type = "UP_NORM_IG1"; break;
                case UP_UGNT_BD2 :  type = "UP_UGNT_BD2"; break;
                case UP_UGNT_BD1 :  type = "UP_UGNT_BD1"; break;
                case UP_NORM_BD2 :  type = "UP_NORM_BD2"; break;
                case UP_NORM_BD1 :  type = "UP_NORM_BD1"; break;
                case UP_TNOR_DTA :  type = "UP_TNOR_DTA"; break;
                case UP_TNOR_IG1 :  type = "UP_TNOR_IG1"; break;
                case DN_NORM_FMW :  type = "DN_NORM_FMW"; break;
                case DN_UGNT_CMD :  type = "DN_UGNT_CMD"; break;
                case DN_NORM_CMD :  type = "DN_NORM_CMD"; break;
                case DN_UNKN_CMD :  type = "DN_UNKN_CMD"; break;
                default:            type = "UNKNOWN_TYP"; break;
            }
            SP1("  %s :", type); 
            SP5("%9d  %9d  %9d  %9d  %9d\n", q->trcvd, q->tsend, q->tfail, q->taway, q->npack);
        }

            SP0(" ====================================================================\n");
        if (!Cdma_uart_use) {
            SP0("     SOCKET SERVER         pppdE sockE connE readE wrteE nodeW srvrW \n");
            SP0("  -----------------------  ----- ----- ----- ----- ----- ----- -----\n");
            SP1("  %s", host); 
            SP5(" %5d %5d %5d %5d %5d", Cdsk_pppd_err, Cdsk_sock_err, Cdsk_conn_err, Cdsk_read_err, Cdsk_wrte_err);
            SP2(" %5d %5d\n", Wdog_node_evt, Wdog_srvr_evt);
        }
            SP0(" ====================================================================\n");
    }
}

void wait_timr()
{
        pthread_mutex_lock(&Mutx_timr);
        pthread_cond_wait(&Cond_timr, &Mutx_timr);
        pthread_mutex_unlock(&Mutx_timr); 
}

void wakeup_timr()
{
    pthread_mutex_lock(&Mutx_timr);
    pthread_cond_signal(&Cond_timr);
    pthread_mutex_unlock(&Mutx_timr);
}

void wait_stat()
{
        pthread_mutex_lock(&Mutx_stat);
        pthread_cond_wait(&Cond_stat, &Mutx_stat);
        pthread_mutex_unlock(&Mutx_stat); 
}

void wakeup_stat()
{
    pthread_mutex_lock(&Mutx_stat);
    pthread_cond_signal(&Cond_stat);
    pthread_mutex_unlock(&Mutx_stat);
}
