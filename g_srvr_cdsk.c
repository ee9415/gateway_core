/*----------------------------------------*
              g_srvr_cdsk.c
            coded by H.B. Lee
                 Feb/2007
        Revisoned May/2009
 *-----------------------------------------*
 *       routines for server interface     *
 *-----------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <pthread.h>
#include <sys/signal.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_pack.h"
#include "g_io.h"
#include "g_util.h"
#include "g_gtway.h"

int   Cdsk_pppd_err;
int   Cdsk_sock_err;
int   Cdsk_conn_err;
int   Cdsk_con1_err[24];
int   Cdsk_con2_err[24];
int   Cdsk_read_err;
int   Cdsk_wrte_err;

void *do_cdsk_listen(void *arg);
void *do_cdsk_pppd(void *arg);



/***
int   cdsk_pppd_check();
int   cdsk_pppd_pwrctl(char pwr);
int   cdsk_pppd_dmnctl(char dmn);
***/

int   cdsk_ubp_read(UBUF *ubp);
int   cdsk_ubp_write(UBUF *ubp, BYTE *buf, int size);

/*******************************************/
/**** for server interface with socket *****/
/*******************************************/
void *do_cdsk_listen(void *arg)
{
    struct linger lgv;
    UBUF   *ubp;
    struct  sockaddr_in cli_addr;
    struct  timeval tv;
    int     sd, flag, i, n, first = 1;
    struct  hostent *p_hent;
    fd_set  wrset;

    time_t  ct;
    struct  tm *tm;



    SV_D02("do_cdsk_listen(0) : dev=cdsk(%s|%d)\n", Cdsk_srv_addr, Cdsk_srv_port);

    if (Cdsk_pppd_use) {
        sleep(3);
        wakeup_pppd(PPPD_CTRL_BTDON);
        sleep(Cdsk_pppd_pwt + Cdsk_pppd_dmt);
        if (!pppd_ison()) {
            sleep(Cdsk_pppd_dmt);
            if (!pppd_ison()) {
                SV_D00("do_cdsk_listen(1) : pppd is not running...\n");
                L00("do_cdsk_listen(1) : pppd is not running...\n");
                exit(1);
            }
        }
    }

    ubp = ubp_alloc(-1);

    while(1) {

        SV_D20("do_cdsk_listen(2) : sleep....\n");

    	if (!first) 
            wait_srvr();
    	else {
            if (++first > 5) {
            	SV_D00("do_cdsk_listen(1) : kickoff srvr fail...\n");
            	L00("do_cdsk_listen(1) : kickoff srvr fail...\n");
            	exit(2);
            }
            if (Cdsk_pppd_use) 
                sleep(Cdsk_pppd_dmt);
            else
                sleep(3);
        }

        SV_D20("do_cdsk_listen(2) : wakeup...\n");
    
    	if (Cdsk_pppd_use && !pppd_ison()) {   /* may be booting up	*/
            SV_D00("do_cdsk_listen(1) : pppd fail...\n");
            Cdsk_pppd_err++;
            continue;
        }

        if ((sd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            if (Cdsk_pppd_use) 
                pppd_wrapup(PPPD_WRUP_OPEN);
            L00("do_cdsk_listen(3) : socket() error...\n");
            SV_D00("do_cdsk_listen(3) : socket() error...\n");
            Cdsk_sock_err++;
            continue;
        }
        SV_D10("do_cdsk_listen(3) : socket() Open...\n");

        if ((p_hent = gethostbyname(Cdsk_srv_addr)) == NULL) {
            L01("do_cdsk_listen(3) : server(%s) not exist...\n", Cdsk_srv_addr);
            exit(1);
        }
        memset(&cli_addr, 0, sizeof(cli_addr));

        cli_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr *)p_hent->h_addr_list[0]));
        cli_addr.sin_family = AF_INET;
        cli_addr.sin_port = htons(Cdsk_srv_port);

        flag = fcntl(sd, F_GETFL, NULL);
        fcntl(sd, F_SETFL, flag | O_NONBLOCK);

        FD_ZERO(&wrset); FD_SET(sd, &wrset);
    	tv.tv_sec  = 5;
    	tv.tv_usec = 0;

        time(&ct); 
        tm = gmtime(&ct);

        SV_D11("gtwy-->srvr : REQ_CONNECT:%s", ctime(&ct));

        if (connect(sd,(struct sockaddr *)&cli_addr,sizeof(cli_addr)) < 0) {
            if (errno != EINPROGRESS && errno != EWOULDBLOCK) {
                close(sd); 
            	Cdsk_conn_err++; 
                Cdsk_con1_err[tm->tm_hour]++;
            	if (Cdsk_pppd_use) 
                    pppd_wrapup(PPPD_WRUP_CONN);
                L03("do_cdsk_listen(4) : cdsk(%s|%d) unavailable...%d\n", Cdsk_srv_addr, Cdsk_srv_port, errno);
                SV_D03("do_cdsk_listen(4) : cdsk(%s|%d) unavailable...%d\n", Cdsk_srv_addr, Cdsk_srv_port, errno);
                continue;
            }
        }
         if ((n = select(sd +1, NULL, &wrset, NULL, &tv)) <= 0) {
            close(sd); 
            Cdsk_conn_err++;
            Cdsk_con2_err[tm->tm_hour]++;
            if (Cdsk_pppd_use) 
                pppd_wrapup(PPPD_WRUP_CONN);
            L02("do_cdsk_listen(5) : cdsk(%s|%d) unavailable...%d\n", Cdsk_srv_addr, Cdsk_srv_port);
            SV_D02("do_cdsk_listen(5) : cdsk(%s|%d) unavailable...%d\n", Cdsk_srv_addr, Cdsk_srv_port);
            continue;

        }
    	fcntl(sd, F_SETFL, flag);

    	lgv.l_onoff = 1, lgv.l_linger = Cdsk_iot_msec/1000;
    	setsockopt(sd, SOL_SOCKET, SO_LINGER, (char *)&lgv, sizeof(lgv));

        SV_D10("gtwy<--srvr : RES_CONNECT\n");

        SV_D22("do_cdsk_listen(6) : cdsk(%s|%d) available...\n", Cdsk_srv_addr, Cdsk_srv_port);

        ubp->ufd = sd;
        ubp_flush(ubp);

        if (do_srvr_interface(ubp, first) > 0 && first) {
            first = 0;
            wakeup_node();
        }
        //Srvr_conn_cnt++;

        close(sd);
        SV_D10("do_cdsk_listen(3) : socket() Close...\n");
        
        if (Cdsk_pppd_use && !first){
            sleep(Cdsk_iot_msec/1000);
            pppd_wrapup(PPPD_WRUP_NORM);
        }

        SV_D22("do_cdsk_listen(7) : cdsk(%s|%d) close...\n", Cdsk_srv_addr, Cdsk_srv_port);

    }
}

/*******************************************/
/****** block and/or time expireation ******/
/*******************************************/
int  cdsk_ubp_read(UBUF *ubp)
{
    int tn, j;

    if (ubp->rn > 0) {
        if (ubp->sp > ubp->mp -1) { /* mode data to front */
            for (j = 0; j <= ubp->rn; j++)
                ubp->buf[j] = ubp->sp[j];
                ubp->sp = ubp->buf; ubp->cp = ubp->sp + ubp->rn;
        }
    }
    else
        ubp_flush(ubp);

    tn = ubp->ep - ubp->cp;
    if (tn > ubp->bsz / 2)
        tn = ubp->bsz/ 2;

    if (!tn) {
	SV_D03("???:%d-%d:%d\n", ubp->rn, ubp->sp - ubp->buf, ubp->cp - ubp->buf);
        //pause();
    }

    if ((tn = cdsk_read(ubp->ufd, ubp->cp, tn)) <= 0) {
	Cdsk_read_err++;
        ubp_flush(ubp);
        return(-1);
    }

    ubp->cp += tn, ubp->rn += tn, *ubp->cp = 0;
    return(tn);
}

int  cdsk_ubp_write(UBUF *ubp, BYTE *buf, int size)
{
    int	  n;

    n = cdsk_write(ubp->ufd, buf, size);
    if (n <= 0)
    	Cdsk_wrte_err++;
    return(n);
}

