 /*----------------------------------------*
              g_srvr_cdma.c
            coded by H.B. Lee
                Feb/2007
	    Revisioned May/2009
 *-----------------------------------------*
 *       routines for server interface     *
 *-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/termios.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_pack.h"
#include "g_io.h"
#include "g_util.h"
#include "g_gtway.h"

void *do_cdma_listen(void *arg);

int   cdma_packet_head_peek(UBUF *ubp, int *len);
int   cdma_write_req_to_rcv2(UBUF *ubp);  /* REQ_TO_RCV2 : g/w --> server */
int   cdma_write_dat_of_gtwy(UBUF *ubp);  /* DAT_OF_GTWY : g/w --> server */

int   cdma_conn_circuit(UBUF *ubp);
int   cdma_login_tcpip(UBUF *ubp);
int   cdma_conn_packet(UBUF *ubp);
int   cdma_conn_tcpip(UBUF *ubp);
int   cdma_disc_tcpip(UBUF *ubp);
int   cdma_exit_tcpip(UBUF *ubp);
int   cdma_send_atcmd(UBUF *ubp, char *atcmd, int atc_ack, int tcp_ack);

int   cdma_ubp_read(UBUF *ubp);
int   cdma_ubp_write(UBUF *ubp, BYTE *buf, int size);

int   cdma_time_sync(UBUF *ubp);
int   cdma_send_atcmd2(UBUF *ubp, char *atcmd, int atc_ack, int tcp_ack);

void  cdma_set_time(UBUF *ubp);

#define CDMA_CMD_ECHO   0

#define CDMA_ATC_OKOK   1
#define CDMA_ATC_ERRR   2
#define CDMA_ATC_CONN   3

/* echo cancel */
#define CDMA_ATC_ATE0   4

#define CDMA_ATC_TEST   5
#define CDMA_ATC_RING   6
#define CDMA_ATC_NOCO   7
#define CDMA_ATC_NODI   8 /* No Dialtone */
#define CDMA_ATC_BUSY   9
#define CDMA_ATC_NOAN   10

#define CDMA_ATC_UNKN   88

#define CDMA_TCP_OPEN   11
#define CDMA_TCP_CLSD   12
#define CDMA_TCP_EXIT   13
#define CDMA_TCP_SNDN   14
#define CDMA_TCP_DATA   15
#define CDMA_TCP_NOCR   16
/* stopwatch */
#define CDMA_STP_WTCH   17

#define CDMA_TCP_UNKN   99


/*******************************************/
/*********** for cdma interface ************/
/*******************************************/
void *do_cdma_listen(void *arg)
{
    char *cdma_dev = (char *)arg;
    struct termios	newtio;
    UBUF *ubp;
    int sd, i, echo_cnt, first = 1, ret;

    if ((sd = open(cdma_dev, O_RDWR | O_NOCTTY)) < 0) {
        SV_D01("do_cdma_listen(2) : dev %s not available.\n", cdma_dev);
        L01("do_cdma_listen(2) : dev %s not available.\n", cdma_dev);
        pthread_exit((void *)-1);
    }

    memset(&newtio, 0, sizeof(newtio));
    /* 
    newtio.c_cflag = CS8 | B115200 | CLOCAL | CREAD;
    newtio.c_iflag = IGNBRK | IGNPAR;
    */
    newtio.c_cflag = CS8 | B115200 | CLOCAL | CREAD | CRTSCTS;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;
    newtio.c_lflag = 0;
    /*
    newtio.c_lflag&= (~ICANON);
    */
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;
    tcflush(sd, TCIFLUSH);
    tcflush(sd, TCOFLUSH);
    tcsetattr(sd, TCSANOW, &newtio);

    ubp = ubp_alloc(sd); Mfree(ubp->buf);
    ubp->buf = (BYTE *)Malloc(MAX_FIRM_PSIZE +1);
    ubp->bsz = MAX_FIRM_PSIZE;
    ubp_flush(ubp);

    ubp->rw_ubp = ubp_alloc(sd); Mfree(ubp->rw_ubp->buf);	/* raw I/O in case hexa format... */
    ubp->rw_ubp->buf = (BYTE *)Malloc(MAX_FIRM_PSIZE*2 +1);
    ubp->rw_ubp->bsz = MAX_FIRM_PSIZE*2;
    ubp_flush(ubp->rw_ubp);

    SV_D22("do_cdma_listen(3) : dev %s(%d) available.\n", cdma_dev, sd);

    for( echo_cnt = 0 ; echo_cnt < 10 ; echo_cnt++ ) {
        if(cdma_echo_reset(ubp) > 0)
            break;
        sleep(3);
        if(echo_cnt > 3) {
            cdma_hw_reset(ubp);
            break;
        }
    }
    SV_D20("do_cdma_listen(5) : cdma_echo_reset Ok.\n");

    //cdma_set_time(ubp);
    while(1) {
        SV_D21("[Test] : %d\n", __LINE__);
    	if (!first) 
    	    wait_srvr();
    	else {
    	    if (++first > 5) {
        		SV_D00("do_cdma_listen(5) : kickoff srvr fail...\n");
        		L00("do_cdma_listen(5) : kickoff srvr fail...\n");
    		    exit(2);
    	    }
    	    sleep(3);
    	}
        SV_D21("[Test] : %d\n", __LINE__);
        SV_D20("do_cdma_listen(6) : wakeup...\n");
        tcflush(sd, TCIFLUSH);
        tcflush(sd, TCOFLUSH);
        SV_D21("[Tes]t : %d\n", __LINE__);
        cdma_dev_clear(ubp);
        /******************************************************/	
        SV_D20("do_cdma_interface(1) : conn_circuit...\n");
            if (cdma_conn_circuit(ubp) < 0){
                continue;
            }
        SV_D10("do_cdma_interface(1) : conn_circuit Ok.\n\n");

        SV_D20("do_cdma_interface(2) : logn_tcpip...\n");
            if (cdma_login_tcpip(ubp) < 0){
                continue;
            }
        SV_D10("do_cdma_interface(2) : logn_tcpip Ok.\n\n");

        SV_D20("do_cdma_interface(3) : conn_packet...\n");
            if (cdma_conn_packet(ubp) < 0) {
                cdma_disc_circuit(ubp);
                continue;
            }
        SV_D10("do_cdma_interface(3) : conn_packet Ok.\n\n");

        SV_D20("do_cdma_interface(4) : conn_tcpip...\n");
            if (cdma_conn_tcpip(ubp) < 0) {
                cdma_disc_tcpip(ubp);
                cdma_disc_circuit(ubp);
                continue;
            }
        SV_D10("do_cdma_interface(4) : conn_tcpip Ok.\n\n");

        /******************************************************/
        SV_D21("[Test] : %d\n", __LINE__);
        if (do_srvr_interface(ubp, first) < 0) {
            cdma_disc_tcpip(ubp);            
            ret = cdma_disc_circuit(ubp);
            sleep(1); 
            ubp_flush(ubp);
            if(ret < 0)
                cdma_hw_reset(ubp);


            SV_D20("do_cdma_listen(8-2) : cdma_echo_reset Ok.\n");
            //cdma_set_time(ubp);
        }		
        else {
            tcflush(sd, TCIFLUSH);
            tcflush(sd, TCOFLUSH);
            SV_D21("[Test] : %d\n", __LINE__);
            SV_D20("do_cdma_listen(6) : disc_tcpip...\n");
            cdma_disc_tcpip(ubp);
            SV_D10("do_cdma_listen(6) : disc_tcpip Ok.\n");

            SV_D20("do_cdma_listen(7) : disc_circuit...\n");
            cdma_disc_circuit(ubp);
            SV_D10("do_cdma_listen(7) : disc_circuit Ok.\n");
    	    if (first) {
    	        D21("[Test] : %d\n", __LINE__);
        		wakeup_node();
        		first = 0;
    	    }
        }
        SV_D21("[Test] : %d\n", __LINE__);
        ubp_flush(ubp);
        ubp_flush(ubp->rw_ubp);
        SV_D21("[Test] : %d\n", __LINE__);

    }
}

/*------------------------------
    cdma_echo_reset()
 ------------------------------*/
cdma_echo_reset(UBUF *ubp)
{
    static  char    at_ech[] = "ATE0\r";
    int ack, flag, n, i;
    char    buf[128];

    SV_D20("cdma_echo_reset(0) : entry.\n");

    ubp_flush(ubp);

    for (i = 0; i < Cdma_rst_nrty; i++) {
        flag = fcntl(ubp->ufd, F_GETFL, NULL);  
        fcntl(ubp->ufd, F_SETFL, flag | O_NONBLOCK);

        while((n = read(ubp->ufd, buf, sizeof(buf))) > 0)
            ; /* waste away of CDMA buff */

        fcntl(ubp->ufd, F_SETFL, flag);
        ubp_flush(ubp);

				at_ech[0] = 'A';
				at_ech[1] = 'T';
				at_ech[2] = 'E';
				at_ech[3] = '0';
				at_ech[4] = 0x0A;
        if ((ack = cdma_send_atcmd2(ubp, at_ech, CDMA_ATC_ATE0, CDMA_ATC_OKOK)) > 0) 
            break;
        SV_D21("cdma_echo_reset(1.%d) : second.\n", i);

        if ((ack = cdma_send_atcmd2(ubp, at_ech, CDMA_ATC_OKOK, 0)) > 0) 
            break;
        SV_D21("cdma_echo_reset(1.%d) : third.\n", i);

        if ((ack = cdma_send_atcmd2(ubp, at_ech, CDMA_ATC_OKOK, 0)) > 0) 
            break;

        SV_D21("cdma_echo_reset(1.%d) : fail.\n", i);
        sleep(Cdma_rst_nsec);
    }

    if (i >= Cdma_rst_nrty)
        return(-1);

    D20("cdma_echo_reset(2) : Ok.\n");
    return(ack);
}

/*------------------------------
    cdma_HW_reset()
 ------------------------------*/
cdma_hw_reset(UBUF *ubp)
{
    static char at_hw_reset[] = "AT$BWMODE=RESET\r";
    int ack, flag, n, i;
    char    buf[128];

    SV_D20("cdma_hw_reset(0) : entry.\n");

    ubp_flush(ubp);

    //for (i = 0; i < Cdma_rst_nrty; i++) {
        flag = fcntl(ubp->ufd, F_GETFL, NULL);
        fcntl(ubp->ufd, F_SETFL, flag | O_NONBLOCK);

        while((n = read(ubp->ufd, buf, sizeof(buf))) > 0)
            ; /* waste away of CDMA buff */

        fcntl(ubp->ufd, F_SETFL, flag);
        ubp_flush(ubp);

        if ((ack = cdma_send_atcmd2(ubp, at_hw_reset, CDMA_ATC_OKOK, 0)) > 0) 
            ;
        //break;
        SV_D21("cdma_hw_reset(1.%d) : fail.\n", i);
        sleep(Cdma_rst_nsec);
    //}

    //if (i >= Cdma_rst_nrty)
        //return(-1);

    SV_D20("cdma_hw_reset(2) : Ok.\n");
    sleep(10);
    cdma_echo_reset(ubp);
    return(ack);
}



/*------------------------------
    cdma_dev_clear()
 ------------------------------*/
cdma_dev_clear(UBUF *ubp)
{
    int flag;
    char buf[512];

    SV_D20("cdma_devc_clear(0) : entry.\n");

    flag = fcntl(ubp->ufd, F_GETFL, NULL);
    fcntl(ubp->ufd, F_SETFL, flag | O_NONBLOCK);

    while(read(ubp->ufd, buf, sizeof(buf)) > 0) {
        ; /* waste away of CDMA buff */
    }
    fcntl(ubp->ufd, F_SETFL, flag);

    SV_D20("cdma_dev_clear(1) : Ok.\n");
}

/*------------------------------
    cdma_conn_circuit()
 ------------------------------*/
cdma_conn_circuit(UBUF *ubp)
{
    static  char at_cir[] = "AT+CRM=251\r";
    int ack;

    SV_D20("cdma_conn_circuit(0) : entry.\n");

    if ((ack = cdma_send_atcmd2(ubp, at_cir, CDMA_ATC_OKOK, 0)) <= 0) {
        SV_D20("cdma_conn_circuit(1) : fail.\n\n");
        return(-1);
    }
    SV_D20("cdma_conn_circuit(2) : Ok.\n");
    return(ack);
}

/*------------------------------
    cdma_login_tcpip()
 ------------------------------*/
cdma_login_tcpip(UBUF *ubp)
{
    static  char    at_tid[48] = "AT$TCPUID=";
    static  char    at_pwd[] = "AT$TCPPASSWDNULL\r";
    int ack;

    SV_D20("cdma_login_tcpip(0) : entry.\n");
    sprintf(at_tid+10, "%s\r", Cdma_tcpc_uid);
    if ((ack = cdma_send_atcmd2(ubp, at_tid, CDMA_ATC_OKOK, 0)) <= 0) {
        SV_D20("cdma_login_tcpip(1) : tcp id fail.\n\n");
        return(-1);
    }
    SV_D20("cdma_login_tcpip(2) : tcp id Ok.\n");
    if ((ack = cdma_send_atcmd2(ubp, at_pwd, CDMA_ATC_OKOK, 0)) <= 0) {
        SV_D20("cdma_login_tcpip(3) : tcp passwd fail.\n\n");
        return(-1);
    }
    SV_D20("cdma_login_tcpip(4) : tcp passwd Ok.\n");
    return(ack);
}

/*------------------------------
    cdma_conn_packet()
 ------------------------------*/
cdma_conn_packet(UBUF *ubp)
{
    static  char    at_con[16] = "ATD";
    int ack, i;
    int tmp_Srvr_iot_msec;

    tmp_Srvr_iot_msec = Srvr_iot_msec;
    Srvr_iot_msec = 10000;
    SV_D20("cdma_conn_packet(0) : entry.\n");
    sprintf(at_con+3, "%d\r", Cdma_pack_uid);

    for (i = 0; i < Cdma_atd_nrty; i++) {
        if ((ack = cdma_send_atcmd2(ubp, at_con, CDMA_ATC_CONN, 0)) > 0) 
            break;
        SV_D21("cdma_conn_packet(1,%dth) : fail.\n\n", i);
        sleep(Cdma_atd_nsec);
    }
    Srvr_iot_msec = tmp_Srvr_iot_msec;
    if (i >= Cdma_atd_nrty)
        return(-1);
    SV_D20("cdma_conn_packet(2) : Ok.\n");
    return(ack);
}

/*------------------------------
    cdma_conn_tcpip()
 ------------------------------*/
cdma_conn_tcpip(UBUF *ubp)
{
    static  char    at_tcp[48] = "AT$TCPOPEN=";
    int ack, i;

    SV_D20("cdma_conn_tcpip(0) : entry.\n");
    sprintf(at_tcp +11, "%s,%d\r", Cdma_srv_addr, Cdma_srv_port);
    //sprintf(at_tcp +11, "%s,%d\r", "203.255.217.189", "9188");

    for(i=0;i < 3; i++) {
        if ((ack = cdma_send_atcmd(ubp, at_tcp, CDMA_ATC_OKOK, CDMA_TCP_OPEN)) > 0) {
            break;
        }
        SV_D10("cdma_conn_tcpip(1) : fail.\n\n");
        sleep(3);
    }
    if (i >= 3)
        return(-1);
    SV_D20("cdma_conn_tcpip(2) : Ok.\n");
    return(ack);
}

/*------------------------------
    cdma_disc_tcpip()
 ------------------------------*/
cdma_disc_tcpip(UBUF *ubp)
{
    static  char at_dis[] = "AT$TCPCLOSE\r";
    int ack;

    SV_D20("cdma_disc_tcpip(0) : entry.\n");

    if ((ack = cdma_send_atcmd(ubp, at_dis, CDMA_ATC_OKOK, CDMA_TCP_CLSD)) <= 0) {
        SV_D10("cdma_disc_tcpip(1) : fail.\n\n");
        return(-1);
    }
    SV_D20("cdma_disc_tcpip(2) : Ok.\n");
    return(ack);
}

/*------------------------------
    cdma_exit_tcpip()
 ------------------------------*/
cdma_disc_circuit(UBUF *ubp)
{
    static  char    at_ext[] = "AT$TCPEXIT\r";
    static  char    at_key[] = "AT$KEYE\r";
    int ack;

    SV_D20("cdma_disc_circuit(0) : entry.\n");

    if ((ack = cdma_send_atcmd(ubp, at_ext, CDMA_ATC_OKOK, CDMA_TCP_NOCR)) < 0) {
        SV_D10("cdma_disc_circuit(1) : fail.\n\n");
        return(-1);
    }

    if ((ack = cdma_send_atcmd(ubp, at_key, CDMA_ATC_OKOK, 0)) < 0) {
        SV_D10("cdma_disc_circuit(2) : fail.\n\n");
        return(-1);
    }
    SV_D20("cdma_disc_circuit(3) : Ok.\n");
    return(ack);
}

/*------------------------------
    cdma_send_atcmd()
 ------------------------------*/
cdma_send_atcmd(UBUF *ubp, char *atcmd, int atc_ack, int tcp_ack)
{
    int len, n;

    SV_D21("cdma_send_atcmd(1) : writing at command %s\n", atcmd);
    len = strlen(atcmd);
    if ((n = cdma_write(ubp->ufd, atcmd, len)) != len) {
        SV_D12("cdma_send_atcmd(2) : at send fail(=%d) of %s\n", n, atcmd);
        return(-1);
    }
#if CDMA_CMD_ECHO==1
    cdma_echo_read(ubp);	/* discard echoed command */
#endif

    if ((atc_ack && (n = cdma_ubp_read(ubp)) != atc_ack)) {
        SV_D12("cdma_send_atcmd(3) : at response fail(=%d) of %s\n", n, atcmd);
        return(-1);
    }

    if (tcp_ack && (n = cdma_ubp_read(ubp)) != tcp_ack) {
        SV_D12("cdma_send_atcmd(4) : tcp response fail(=%d) of %s\n", n, atcmd);
        return(-1);
    }
    return(n);
}

/*------------------------------
    cdma_ubp_read()
 ------------------------------*/
cdma_ubp_read(UBUF *ubp)
{
    UBUF    *rwubp;
    char    *ep, *cp;
    int     cn, c2, tn, n, i, j;
    extern  BYTE    hex2bin[];

    rwubp = ubp->rw_ubp;

    if (!ubp->rn)
        ubp_flush(ubp);
    else if (ubp->sp >= ubp->mp) {
        memcpy(ubp->buf, ubp->sp, ubp->rn);
        ubp->sp = ubp->buf;
        ubp->cp = ubp->sp + ubp->rn;
    }

    if (rwubp->rn > 0) {
        do {
            if (rwubp->sp[0] != '\r' && rwubp->sp[0] != '\n') 
            break;
            rwubp->rn--; rwubp->sp++;
        } while(rwubp->rn > 0);

        if (rwubp->sp > rwubp->mp -1) { /* mode data to front */
            for (j = 0; j <= rwubp->rn; j++)
                rwubp->buf[j] = rwubp->sp[j];
            rwubp->sp = rwubp->buf;
            rwubp->cp = rwubp->sp + rwubp->rn;
        }
    }
    else
        ubp_flush(rwubp);

    tn = rwubp->ep - rwubp->cp;

    if (tn > rwubp->bsz / 2) {
        tn = rwubp->bsz / 2;
    }

    while ((!rwubp->rn || !(ep = strchr(rwubp->sp, '\n'))) && tn > 0) {
        if ((n = cdma_read(rwubp->ufd, rwubp->cp, tn)) <= 0) {
            return(n);
        }
        rwubp->cp += n;
        rwubp->rn +=n;
        tn -= n; 
        rwubp->cp[0] = 0;

        while(rwubp->rn > 0) {
            if (rwubp->sp[0] != '\r' && rwubp->sp[0] != '\n') 
            break;
            rwubp->rn--; rwubp->sp++;
        }
    }


    if(!tn && !ep) {
        if(*(rwubp->cp -1) != '\r') {
            *rwubp->cp = '\r';
            rwubp->cp++;
        }
        ep = (char *)(rwubp->cp -1);
    }

    if (!ep) {
        return(-1);
    }

    cp = (char *)rwubp->sp;
    rwubp->sp = (BYTE *)(ep + 1);
    rwubp->rn = rwubp->cp - rwubp->sp;

    while(ep[-1] == '\r') 
        ep--;

    cn = ep - cp;

    if (!strncmp(cp, "$TCP", 4)) {
        if (!strncmp(cp +4, "OPEN", 4)) 
            return(CDMA_TCP_OPEN);
        else if (!strncmp(cp +4, "CLOSED", 6)) 
            return(CDMA_TCP_CLSD);
        else if (!strncmp(cp +4, "SENDDONE", 8)) 
            return(CDMA_TCP_SNDN);
        else if (!strncmp(cp +4, "READDATA", 8))  {
            cp += 13; cn -= 13;	/* discard '$TCPREADDATA=' */
            c2 = (cn/2)*2;     /* make even */
            for (i = j = 0; i < c2; i += 2, j++) {
                ubp->cp[j] = (hex2bin[cp[i]-'0'] << 4) | hex2bin[cp[i+1]-'0']; 
            }
            ubp->cp += j; ubp->rn += j;
            if(c2<cn){
                rwubp->sp--;
                rwubp->rn++;
                *rwubp->sp=cp[i];
            }
            SV_D21("* rn=%d\n",ubp->rn);
            return(CDMA_TCP_DATA);
        }
        return(CDMA_TCP_UNKN);
    }
    else if (!strncmp(cp, "OK", 2))
        return(CDMA_ATC_OKOK);
    else if (!strncmp(cp, "ERROR", 5))
        return(CDMA_ATC_ERRR);
    else if (!strncmp(cp, "NO CARRIER", 10))
        return(CDMA_TCP_NOCR);
    else if (!strncmp(cp, "CONNECT", 7))
        return(CDMA_ATC_CONN);
    else if (!strncmp(cp, "ATE0", 4))
        return(CDMA_ATC_ATE0);
    else if (!strncmp(cp, "NO DI", 5))
        return(CDMA_ATC_NODI);

    c2 = (cn/2)*2;     /* make even */

    for (i = j = 0; i < c2; i += 2, j++) {
        ubp->cp[j] = (hex2bin[cp[i]-'0'] << 4) | hex2bin[cp[i+1]-'0']; 
    }

    ubp->cp += j; ubp->rn += j;
    if(c2<cn){
        rwubp->sp--;
        rwubp->rn++;
        *rwubp->sp=cp[i];
    }
    return(CDMA_ATC_UNKN);
}

/*------------------------------
    cdma_echo_reset()
 ------------------------------*/
cdma_echo_read(UBUF *ubp)	/* waste away echoed data */
{
    UBUF    *rwubp;
    char    *ep, *cp;
    int tn, n = 0, tag = 0, tech;

    rwubp = ubp->rw_ubp;

    SV_D20("cdma_echo_read(0) : entry\n");

    while(rwubp->rn > 0) {
        if (rwubp->sp[0] != '\r' && rwubp->sp[0] != '\n') 
            break;
        rwubp->rn--; rwubp->sp++;
    }

    tech = rwubp->rn;

    while (!rwubp->rn || !(ep = strchr(rwubp->sp, '\n'))) {

        ubp_flush(rwubp);
        tech += n;

        tn = rwubp->bsz / 2;

        SV_D22("cdma_echo_read(1) : %d, %d\n", rwubp->rn, tn);

        if ((n = cdma_read(rwubp->ufd, rwubp->cp, tn)) <= 0)
            return(n);
        rwubp->cp += n; rwubp->rn +=n; rwubp->cp[0] = 0;

        if (!tag) {	/* in case first */
            while(rwubp->rn > 0) {
                if (rwubp->sp[0] != '\r' && rwubp->sp[0] != '\n') 
                break;
                rwubp->rn--; rwubp->sp++;
            }
        }
        if (rwubp->rn > 0)
            tag++;

        SV_D23("cdma_echo_read(2) : %d(%d) %d\n", rwubp->rn, n, tn);
    }

    cp = (char *)rwubp->sp; tech += (ep - (char *)rwubp->sp + 1);
    rwubp->sp = (BYTE *)(ep +1); rwubp->rn = rwubp->cp - rwubp->sp;

    return(CDMA_ATC_UNKN);
}

/*------------------------------
    cdma_ubp_write()
 ------------------------------*/
cdma_ubp_write(UBUF *ubp, BYTE *buf, int size)
{
    char tbf[MAX_CDMA_PSIZE]; /* at command + \r\r */

    char *ep, *cp;
    int cn, tn, n, i, j;
    extern BYTE bin2hex[];

    SV_D20("cdma_ubp_write(1) : entry.\n");

    strcpy(tbf, "AT$TCPWRITE="); 
    for (i = 0; i < size; ) {
        cp = tbf + 12;
        if ((tn = size - i) > (MAX_CDMA_PSIZE-14)/2)
            tn = (MAX_CDMA_PSIZE-15)/2;
            
        for (j = 0; j < tn; j++, i++) {
            *cp = bin2hex[buf[i] >> 4];   cp++;
            *cp = bin2hex[buf[i] & 0x0f]; cp++;
        }
        cp[0] = '\r';
        SV_D21("cdma_ubp_write(2) : send : %d bytes.\n", 12+tn*2+1);

        if (cdma_write(ubp->ufd, tbf, 12 + tn*2 + 1) <= 0)
            return(-1);

        SV_D21("cdma_ubp_write(3) : send : Ok.\n", 12+tn*2+1);
#if CDMA_CMD_ECHO==1
        cdma_echo_read(ubp);	/* discard echoed data */
#endif
        SV_D20("cdma_ubp_write(4) : recv : OK.\n");

        if (cdma_ubp_read(ubp) != CDMA_ATC_OKOK)
            return(-1);

        SV_D20("cdma_ubp_write(5) : recv : 'OK' Ok.\n");
        SV_D20("cdma_ubp_write(6) : recv : SNDN.\n");

        if (cdma_ubp_read(ubp) != CDMA_TCP_SNDN)
            return(-1);
        SV_D20("cdma_ubp_write(7) : recv : 'SNDN' Ok.\n");
    }
    SV_D20("cdma_ubp_write(8) : exit.\n");
    return(size);
}

/*------------------------------
    cdma_img_write()
 ------------------------------*/
cdma_img_write(UBUF *ubp, BYTE *buf, int size)
{
    char    tbf[MAX_CDMA_PSIZE]; /* at command + \r\r */

    char    *ep, *cp;
    int cn, tn, n, i, j, pn, code, resend;
    extern  BYTE    bin2hex[];

    SV_D20("cdma_img_write(1) : entry.\n");
    strcpy(tbf, "AT$TCPWRITE="); 

    for (i = pn = 0; i < size; pn++) {
        cp = tbf + 12;
        if ((tn = size - i) > (MAX_CDMA_PSIZE-14)/2)
            tn = (MAX_CDMA_PSIZE-15)/2;

        for (j = 0; j < tn; j++, i++) {
            *cp = bin2hex[buf[i] >> 4];   cp++;
            *cp = bin2hex[buf[i] & 0x0f]; cp++;
        }
        cp[0] = '\r';

        //for(resend = 0; resend < 3; resend++) {
        resend = 0;
        do {
            SV_D21("cdma_img_write(2) : send : %d bytes.\n", 12+tn*2+1);
            if (write/* cdma_write */(ubp->ufd, tbf, 12 + tn*2 + 1) <= 0)
                return(-1);
            SV_D21("cdma_img_write(3) : send : %d bytes Ok .\n", 12+tn*2+1);
#if CDMA_CMD_ECHO==1
            cdma_echo_read(ubp);	/* discard echoed data */
#endif
            while ((code = cdma_ubp_read(ubp)) != CDMA_ATC_OKOK) {
                SV_D22("___ 0-1code = %d__ resend : %d\n",code, resend);
                if(code == CDMA_ATC_ERRR){
                    resend++;
                    if(resend > 10)
                        return(-1);
                    break;
                }
                if (code != CDMA_TCP_DATA) { 
                    D11("___ 0code = %d__\n",code);
                    return(-1);
                }

                if (srvr_enque_mid_dncmd(ubp) < 0) {
                    D11("___ 1code = %d__\n",code);
                    return(-1);
                }
            }
            if( code != CDMA_ATC_ERRR)
            resend = 0;
            SV_D22("___ 0-3code = %d__ resend : %d\n",code, resend);
        } while((resend > 0) && (resend  < 10));
        SV_D20("cdma_img_write(6) : recv : OK Ok.\n");
        while ((code = cdma_ubp_read(ubp)) != CDMA_TCP_SNDN) {
            SV_D11("___ 0-2code = %d__\n",code);
            if (code != CDMA_TCP_DATA) { 
                SV_D11("___ 3code = %d__\n",code);
                return(-1);
            }

            if (srvr_enque_mid_dncmd(ubp) < 0) {
                SV_D11("___ 4code = %d__\n",code);
                return(-1);
            }
        }
        SV_D20("cdma_img_write(9) : recv : SNDN Ok.\n");
    }
    SV_D20("cdma_img_write(A) : exit.\n");
    return(size);
}

/*------------------------------
    cdma_send_atcmd()
 ------------------------------*/
cdma_send_atcmd2(UBUF *ubp, char *atcmd, int atc_ack, int tcp_ack)
{
    int len, n, tl/*time lenth */;
    char time[32];

    SV_D21("cdma_send_atcmd(1) : writing at command %s\n", atcmd);

    len = strlen(atcmd);

    if ((n = cdma_write(ubp->ufd, atcmd, len)) != len) {
        SV_D12("cdma_send_atcmd(2) : at send fail(=%d) of %s\n", n, atcmd);
        return(-1);
    }
#if CDMA_CMD_ECHO==1
    cdma_echo_read(ubp);	/* discard echoed command */
#endif

    if ((atc_ack && ((n = cdma_ubp_read(ubp)) != atc_ack))) {
        SV_D12("cdma_send_atcmd2(3) : at response fail(=%d) of %s\n", n, atcmd);
        return(-1);
    }
    SV_D22("cdma_send_atcmd2(3) : at response (=%d) of %s\n", n, atcmd);
    tl=ubp->rn;
    memcpy(time, ubp->sp, ubp->rn);

    ubp_flush(ubp);

    if (tcp_ack && ((n = cdma_ubp_read(ubp)) != tcp_ack)) {
        SV_D22("cdma_send_atcmd2(4) : tcp response fail(=%d) of %s\n", n, atcmd);
        return(-1);
    }
    SV_D22("cdma_send_atcmd2(4) : tcp response (=%d) of %s\n", n, atcmd)
    ubp_flush(ubp);
    memcpy(ubp->sp, time, tl);
    ubp->rn = tl;
    ubp->cp = ubp->sp+tl;
    return(n);
}


/*------------------------------
    time_sync()
AT$BWMODE=SIMPLE

AT$PHONENUM=01063880474,01063880474
AT$PHONENUM=01063880454,01063880454
AT$PHONENUM=01063880271,01063880271
AT$PHONENUM=01063880276,01063880276
AT$PHONENUM=01063880293,01063880293
AT$PHONENUM=01063880297,01063880297

AT$PHONENUM?

AT$BWMODE=RESET

AT$KEY=01025238199s

AT$BWMODEM=12?

AT$BWMODEM?
 ------------------------------*/
cdma_time_sync(UBUF *ubp)
{
    static char at_bwmodem[] = "AT$BWMODEM=4?\r";
                                 /* 41542442574D4F44454D3D343F0D0A */
    int     ack, flag, n, i;
    char    buf[128];

    SV_D20("cdma_time_sync(0) : entry.\n");
    ubp_flush(ubp);
    for (i = 0; i < Cdma_rst_nrty; i++) {
        flag = fcntl(ubp->ufd, F_GETFL, NULL);
        fcntl(ubp->ufd, F_SETFL, flag | O_NONBLOCK);
        while((n = read(ubp->ufd, buf, sizeof(buf))) > 0)
            ; /* waste away of CDMA buff */
        fcntl(ubp->ufd, F_SETFL, flag);
        ubp_flush(ubp);
        if ((ack = cdma_send_atcmd2(ubp, at_bwmodem, CDMA_STP_WTCH, CDMA_ATC_OKOK)) > 0) 
            break;
        SV_D21("cdma_time_sync(1. %d th) : fail.\n", i);
        sleep(Cdma_rst_nsec);
    }
    if (i >= Cdma_rst_nrty)
        return(-1);
    SV_D20("cdma_time_sync(2) : Ok.\n");
    return(ack);
}

#define TRUE    1
#define FALSE   0
void cdma_set_time(UBUF *ubp)
{
#define TIMEZONE (3600 * 0)

    struct  timeval     tp;
    struct  timeval     timecurrent;
    struct  tm          tm_src;
    //int   first=1;
    int first=TRUE;
    int old_sec = 0, cnt=0;

    do {
        gettimeofday( &timecurrent, NULL ); 
        if(cdma_time_sync(ubp) > 0) {
            tm_src.tm_year =  ((ubp->sp[0])   - 0x30) * 1000;
            tm_src.tm_year += ((ubp->sp[0+1]) - 0x30) * 100;
            tm_src.tm_year += ((ubp->sp[0+2]) - 0x30) * 10;
            tm_src.tm_year += ((ubp->sp[0+3]) - 0x30);
            SV_D21("...tm_src.tm_year = %d....\n", tm_src.tm_year);

            tm_src.tm_mon  =  ((ubp->sp[4])   - 0x30) * 10;
            tm_src.tm_mon  += ((ubp->sp[4+1]) - 0x30);
            SV_D21("...tm_src.tm_mon = %d....\n", tm_src.tm_mon);

            tm_src.tm_mday =  ((ubp->sp[6])   - 0x30) * 10;
            tm_src.tm_mday += ((ubp->sp[6+1]) - 0x30);
            SV_D21("...tm_src.tm_mday = %d....\n", tm_src.tm_mday);

            tm_src.tm_hour =  ((ubp->sp[8])   - 0x30) * 10;
            tm_src.tm_hour += ((ubp->sp[8+1]) - 0x30);
            SV_D21("...tm_src.tm_hour = %d....\n", tm_src.tm_hour);

            tm_src.tm_min  =  ((ubp->sp[10])   - 0x30) * 10;
            tm_src.tm_min  += ((ubp->sp[10+1]) - 0x30);
            SV_D21("...tm_src.tm_min = %d....\n", tm_src.tm_min);

            tm_src.tm_sec  =  ((ubp->sp[12])   - 0x30) * 10;
            tm_src.tm_sec  += ((ubp->sp[12+1]) - 0x30);
            SV_D21("...tm_src.tm_sec = %d....\n", tm_src.tm_sec);

            tm_src.tm_year -= 1900;
            tm_src.tm_mon  -= 1;

            if(first) {
                tp.tv_sec = mktime(&tm_src);
                //tp.tv_usec = 0;
                //settimeofday ( &tp, NULL );
                //first = 0;
                first = FALSE;
                old_sec = tm_src.tm_sec;
            }

            if( old_sec != tm_src.tm_sec)
            {
                //tp.tv_sec = mktime(&tm_src);
                tp.tv_sec++;
                tp.tv_usec = 0;
                settimeofday ( &tp, NULL );
                SV_D21("....cnt = %d .....\n", cnt);
                break;
            }
            //old_sec = tm_src.tm_sec;
        }
    } while(cnt++ < 1000);
    ubp_flush(ubp);
}
