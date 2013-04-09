 /*-----------------------------------------*
	            g_stwc.c
		coded by Y.S. Park
  		    Feb/2007
 *-----------------------------------------*
 *    routines for stopwatch interface     *
 *-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/fcntl.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_pack.h"
#include "g_queue.h"
#include "g_io.h"
#include "g_util.h"
#include "g_gtway.h"

void  *do_stwc_listen(void *arg);
void   do_stwc_interface(UBUF *ubp);

/******************************************/
/***** for sink_node serial interface *****/
/******************************************/
void *do_stwc_listen(void *arg)
{
    char    *stwc_dev = (char *)arg;
    UBUF    *ubp;
    int	    sd;

    sd = open(stwc_dev, O_RDWR);

    if (sd < 0) {
        L01("Stopwatch device(%s) open error!\n", stwc_dev);
        exit(1);
    }

    ubp = ubp_alloc(sd);

    while(1) {
        D02("do_stwc_listen(0) : device %s(%d) available.\n", stwc_dev, sd);
        do_stwc_interface(ubp);
        ubp_flush(ubp);
    }
}

typedef struct time_stemp {	/* from stop watch device */
    BYTE    stat[2];
    BYTE    sec[4];
    BYTE    usec[4];
} TSTMP;

typedef struct upstw_dta {
    BYTE    addr[2];		/* node_id, dummy	*/
    BYTE    snsr;		/* sensor_id, dummy 	*/
    BYTE    min;		/* minute		*/
    BYTE    sec;		/* sec			*/
    BYTE    msec[2];		/* mili sec		*/
} STW_DTA;

#define	SNSR_STWC	0xB0

void do_stwc_interface(UBUF *ubp)
{
    TSTMP   *tstmp;
    PKBUC   *pkbuc;
    STW_DTA *stwcp;

    int     htyp, hlen, haddr, hsnsr, dvstat, dvusec, dvmsec, i;
    time_t  dvsec;
    BYTE    dvbuf[40];
    struct tm     *tm;

    D11("do_stwc_interface(0) : start(%d)\n", ubp->ufd);

    tstmp = (TSTMP *)dvbuf;

    htyp = UP_NORM_DTA;
    hlen = sizeof(STW_DTA);
    haddr = 5;		/* node_id, dummy ? */
    hsnsr = SNSR_STWC;

    for (i = 0; ; i++) {

	if (read(ubp->ufd, (BYTE *)tstmp, 40) <= sizeof(TSTMP))
	    break;

        dvstat = tstmp->stat[0] | (tstmp->stat[1] >> 8);
        dvsec  = get_bin_p2_n(tstmp->sec);
        dvusec = get_bin_p2_n(tstmp->usec);
        dvmsec = dvusec / 1000;

        tm = localtime(&dvsec);

        D26( "GMT   TIME %04d-%02d-%02d %02d:%02d:%02d\n", tm->tm_year + 1900, 
              tm->tm_mon +1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);

        pkbuc = upcdt_packet_alloc(hlen, UP_NORM_DTA); 
	stwcp = (STW_DTA *)pkbuc->b_data;

        stwcp->addr[0] = haddr & 0x0ff; 
        stwcp->addr[1] = haddr >> 8;
        stwcp->snsr    = hsnsr;
        stwcp->min     = tm->tm_min;
        stwcp->sec     = tm->tm_sec;
        stwcp->msec[0] = dvmsec & 0x0ff;
        stwcp->msec[1] = dvmsec >> 8;

        q_insert((PKBH *)pkbuc, htyp);

        D24("[%d-sec: %ld, usec: %ld] --> count : %d\n\n", dvstat, dvsec, dvmsec, i);
        i++;

    }
}
