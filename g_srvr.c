 /*----------------------------------------*
                   g_srvr.c
              coded by H.B. Lee
                   Feb/2007
         Revisioned May/2009
 *-----------------------------------------*
 *       routines for server interface     *
 *-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <sys/signal.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_pack.h"
#include "g_queue.h"
#include "g_io.h"
#include "g_util.h"
#include "g_gtway.h"

pthread_mutex_t     Mutx_srvr = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      Cond_srvr = PTHREAD_COND_INITIALIZER;
pthread_condattr_t  Attr_srvr;

int	Srvr_conn_cnt;
time_t	Srvr_conn_tim;

int (*srvr_ubp_read)(UBUF *ubp);
int (*srvr_ubp_write)(UBUF *ubp, BYTE *buf, int size);
int Srvr_iot_msec;

int     srvr_dnfmw_packet_read(UBUF *ubp, BYTE *buf, int hlen);

int     srvr_write_nego_packet(UBUF *ubp, BYTE *pdu);
int     srvr_write_dta_of_gtwy(UBUF *ubp);

void    srvr_enque_dncmd(int htyp, int hlen, int clen, BYTE *buf);
void    srvr_enque_dnfmw(int htyp, int hlen, int clen, BYTE *buf);
int     srvr_enfuse_gtwy(int htyp, int hlen, int clen, BYTE *buf);

int     srvr_packet_head_peek(UBUF *ubp, int *len);
int     srvr_dncmd_packet_read(UBUF *ubp, BYTE *buf, int hlen, char htyp);

// signal
void sig_pipe();
void sig_int();


BYTE    gtway_start[] = { MY_GATEWAY_ID & 0xff, MY_GATEWAY_ID >> 8, GTWAY_START,  0,  0,0,0,0};
BYTE    req_to_rcv2[] = { MY_GATEWAY_ID & 0xff, MY_GATEWAY_ID >> 8, REQ_TO_RCV2,  0,  0,0,0,0};
BYTE    end_of_snd3[] = { MY_GATEWAY_ID & 0xff, MY_GATEWAY_ID >> 8, END_OF_SND3,  0,  0,0,0,0};
BYTE    dta_of_gtwy[] = { MY_GATEWAY_ID & 0xff, MY_GATEWAY_ID >> 8, DTA_OF_GTWY,  5,  0,0,0,0, 0,0,0,0,0};
BYTE    req_to_time[] = { MY_GATEWAY_ID & 0xff, MY_GATEWAY_ID >> 8, REQ_TO_TIME,  0,  0,0,0,0};
BYTE    req_to_cntd[] = { MY_GATEWAY_ID & 0xff, MY_GATEWAY_ID >> 8, REQ_TO_CNTD,  0,  0,0,0,0};
void svr_init()
{
    SUPKC   *pk;
    pk = (SUPKC *)gtway_start;
    pk->gwid[0] = My_gateway_id & 0xff; pk->gwid[1] = My_gateway_id >> 8;

    pk = (SUPKC *)req_to_rcv2;
    pk->gwid[0] = My_gateway_id & 0xff; pk->gwid[1] = My_gateway_id >> 8;

    pk = (SUPKC *)end_of_snd3;
    pk->gwid[0] = My_gateway_id & 0xff; pk->gwid[1] = My_gateway_id >> 8;

    pk = (SUPKC *)dta_of_gtwy;
    pk->gwid[0] = My_gateway_id & 0xff; pk->gwid[1] = My_gateway_id >> 8;

    pk = (SUPKC *)req_to_time;
    pk->gwid[0] = My_gateway_id & 0xff; pk->gwid[1] = My_gateway_id >> 8;

    pk = (SUPKC *)req_to_cntd;
    pk->gwid[0] = My_gateway_id & 0xff; pk->gwid[1] = My_gateway_id >> 8;

    pthread_cond_init(&Cond_srvr, &Attr_srvr);

    if (Cdma_uart_use > 0) {
    	srvr_ubp_read  = cdma_ubp_read;
    	srvr_ubp_write = cdma_ubp_write;
    	Srvr_iot_msec   = Cdma_iot_msec;

    }
    else {
    	srvr_ubp_read  = cdsk_ubp_read;
    	srvr_ubp_write = cdsk_ubp_write;
    	Srvr_iot_msec   = Cdsk_iot_msec;
    }
}

int do_srvr_interface(UBUF *ubp, int first)
{
    int    htyp, hlen, clen, ilen, eflag, bflag, odr, nego_seq, end;
    long int tim;
    time_t time, stime = 0;
    BYTE   buf[MAX_NODE_PSIZE+1], *ip;
    BYTE  *fmw;

    QUEUE *q;
    PKBH  *pkbh;
    PKBUC *pkbuc;
    PKBUI *pkbui;

    SV_D20("do_srvr_interface(1)...\n");

    sigset(SIGINT, sig_int);
    sigset(SIGPIPE, sig_pipe);

    if (first) {
        if (srvr_write_nego_packet(ubp, gtway_start) < 0) { /* GTWAY_START */
            ubp_flush(ubp);
            return(-1);    
        }
    }

    for (nego_seq = eflag = bflag = 0; !eflag && nego_seq < 3; nego_seq++) {
        switch(nego_seq) {
            case 0:    
                if (srvr_write_nego_packet(ubp, req_to_rcv2) < 0) /* REQ_TO_RCV2 */
                    eflag++;
                break;

            case 1:
                if (!Srvr_gwtm_frq || (Srvr_conn_cnt % Srvr_gwtm_frq && !first))
                    continue;    // skip time request phase

                if (srvr_write_nego_packet(ubp, req_to_time) < 0){/* REQ_TO_TIME */
                    eflag++;
                    break;        // proc the response at out of switch
                }
                if (srvr_write_nego_packet(ubp, req_to_cntd) < 0) /* REQ_TO_CNTD */
                    eflag++;
                break;        // proc the response at out of switch

            case 2:
                if (Srvr_gwst_frq && (Srvr_conn_cnt % Srvr_gwst_frq) == 0) {
                    if (srvr_write_dta_of_gtwy(ubp) < 0) {
                        eflag++;
                        break;
                    }
                }

            for (odr = 1; !eflag && odr <= MAX_UORDER; odr++) { /* upwards */
                if (!(q = q_by_order(odr)))
                    continue;
                while(!eflag && (pkbh = q_delete(q))) {
                    if (!(q->flag & Q_IMAGE)) {
                        pkbuc = (PKBUC *)pkbh;

                        SV_D23("\n---> %.2x,%.2x,%.2x\n", pkbuc->b_type, pkbuc->h_mlen, pkbuc->h_mlen+SRVU_HEADLN_CMD);

                        if ((*srvr_ubp_write)(ubp, pkbuc->b_gwid, pkbuc->h_mlen + SRVU_HEADLN_CMD) > 0) {
                            q->tsend++;
                            q->tfail = 0;

                            SV_D12("gtwy-->srvr : %s[%.2x]\n", get_pdu_name(pkbuc->b_type), pkbuc->b_type);

                            Mfree(pkbuc);

                        }
                        else {
                            if (++(pkbuc->h_nretry) > q->maxrt) {
                                q->taway++;

                                L01("do_srvr_interface(9): send: CMDTA_PACKT(%.2x) away.\n", pkbuc->b_type);
                                SV_D01("do_srvr_interface(9): send: CMDTA_PACKT(%.2x) away.\n", pkbuc->b_type);
                                Mfree(pkbuc);
                            }
                            else {
                                q->tfail++;
                                q_insert_bak((PKBH *)pkbuc, q);

                                L01("do_srvr_interface(9): send: CMDTA_PACKT(%.2x) fail.\n", pkbuc->b_type);
                                SV_D01("do_srvr_interface(9): send: CMDTA_PACKT(%.2x) fail.\n", pkbuc->b_type);
                            }
                            eflag++;
                        }
                    }
                    else {
                        pkbui = (PKBUI *)pkbh;

                        SV_D23("\n---> %.2x,%.8x,%.8x\n", pkbui->b_type, pkbui->h_mlen, pkbui->h_mlen+SRVU_HEADLN_IMG);

                        if ((*srvr_ubp_write)(ubp, pkbui->b_gwid, pkbui->h_mlen + SRVU_HEADLN_IMG) > 0) {
                            q->tsend++;
                            q->tfail = 0;

                            SV_D12("gtwy-->srvr : %s[%.2x]\n", get_pdu_name(pkbui->b_type), pkbui->b_type);
                            Mfree(pkbui);

                            if (srvr_write_nego_packet(ubp, req_to_cntd) < 0)
                                eflag++;

                        }
                        else {
                            L03("do_srvr_interface(B): send: IMAGE_PACKT(%.2x):%.8x,%d: fail.\n", pkbui->b_type, pkbui->h_mlen-NDEU_HEADLN_IMG-2, pkbui->h_mlen-NDEU_HEADLN_IMG-2);
                            SV_D03("do_srvr_interface(B): send: IMAGE_PACKT(%.2x):%.8x,%d: fail.\n", pkbui->b_type, pkbui->h_mlen-NDEU_HEADLN_IMG-2, pkbui->h_mlen-NDEU_HEADLN_IMG-2);
                            if (++(pkbui->h_nretry) > q->maxrt) {
                                q->taway++;

                                L01("do_srvr_interface(B): send: CMDTA_PACKT(%.2x) away.\n", pkbui->b_type);
                                SV_D01("do_srvr_interface(B): send: CMDTA_PACKT(%.2x) away.\n", pkbui->b_type);
                                Mfree(pkbui);

                            }
                            else {
                                q->tfail++;
                                q_insert_bak((PKBH *)pkbui, q);

                                L01("do_srvr_interface(B_2): send: CMDTA_PACKT(%.2x) fail.\n", pkbui->b_type);
                                SV_D01("do_srvr_interface(B_2): send: CMDTA_PACKT(%.2x) fail.\n", pkbui->b_type);
                            }
                            eflag++;
                        }
                    }
                } // end while
            } // end for
        } // end switch

        for (end = 0; nego_seq < 2 && !end && !eflag; ) {
            if ((htyp = srvr_packet_head_peek(ubp, &hlen)) < 0) {
                eflag++;
                break; /* hlen :  ln(1)  --> sizeof binary body */
            }

            SV_D12("gtwy<--srvr : %s[%.2x]\n", get_pdu_name(htyp), htyp);

            if (htyp != DN_NORM_FMW && htyp != DN_GTWY_FMW) {
                if ((clen = srvr_dncmd_packet_read(ubp, buf, hlen, htyp)) <= 0) {
                    eflag++;
                    break; /* clen = id(1)+length(1)+address(2)+data(n)+escs */
                }
            }
            else {             // length == 4 byte
                fmw = (BYTE *)Malloc(MAX_FIRMW_SIZE);
                if ((clen = srvr_dnfmw_packet_read(ubp, fmw, hlen)) <= 0) {
                    Mfree(fmw);     /* clen = did(1)+data(n) = 1 + hlen */
                    eflag++;
                    break; 
                }
            }

            switch(htyp) {
                case DN_NORM_CMD:
                    srvr_enque_dncmd(htyp, hlen, clen, buf);
                    continue;    

                case DN_NORM_FMW:
                    srvr_enque_dnfmw(htyp, hlen, clen, fmw);
                    Mfree(fmw);
                    continue;

                case DN_GTWY_FMW:
                    if (srvr_enfuse_gtwy(htyp, hlen, clen, fmw) < 0)
                        eflag++;
                    else
                        bflag++;
                    Mfree(fmw);
                    continue;

                case SET_GW_TIME:
                    SV_D20("time = ");
                    SV_MDH(buf + SRVD_ADROFF_CMD, 4);
                    tim = get_bin_p2_n(buf + SRVD_ADROFF_CMD);
                    /* should be SRVD_DTAOFF_CMD */
                    SV_D21("fd Set OK...[%d]\n", tim);
                    set_system_time(tim);
                    continue;

                case REQ_GW_STAT:
                    if (srvr_write_dta_of_gtwy(ubp) < 0)
                    eflag++;
                    end++; 
                    break;

                case END_OF_SND2: 
                    end++; 
                    break;

                default : 
                    L01("do_srvr_interface(9) : recv: unknown type(%.2x)\n", htyp);
                    SV_D01("do_srvr_interface(9) : recv: unknown type(%.2x)\n", htyp);
                    end++; 
                    eflag++;
            } // end switch
        } // end for
    } // end for

        Srvr_conn_cnt++;
        SV_D01("Srvr_conn_cnt = %d\n", Srvr_conn_cnt);
    if (!eflag) {
    	srvr_write_nego_packet(ubp, end_of_snd3); /* END_OF_SND3 */
    	if (bflag){
            if(Cdma_uart_use) {
                cdma_disc_tcpip(ubp);            
                cdma_disc_circuit(ubp);
            }
            else {
                close(ubp->ufd);
                if(Cdsk_pppd_use) {
                    wakeup_pppd(PPPD_CTRL_PWROFF);
                }
            }
            sleep(3);
            SV_D00("do_srvr_interface(A) : Restart Egtwy..\n");
            srvr_reboot_gtwy();              /* no rturn */
        }
            
    }

    ubp_flush(ubp);

    if (!eflag)
        return(1);
    else
        return(-1);
}

/*--------------------------------------------------------*
    peeking subroutine for downward commands & firmware
            return packet Data Entity Id 
            & length of first data entry  
 *--------------------------------------------------------*/
int srvr_packet_head_peek(UBUF *ubp, int *len)
{
    int tn;
    
    while (ubp->rn < SRVD_HEADLN_CMD) {
        if ((tn = srvr_ubp_read(ubp)) <= 0)
        return(tn);
    }

    if(ubp->sp[SRVD_DIDOFF_ALL] != DN_NORM_FMW && 
       ubp->sp[SRVD_DIDOFF_ALL] != DN_GTWY_FMW)
        *len = ubp->sp[SRVD_LENOFF_ALL];
    else
    {
        while (ubp->rn < SRVD_HEADLN_FMW) {
            if ((tn = srvr_ubp_read(ubp)) <= 0)
            return(tn);
        }
        *len = get_bin_p2_n(ubp->sp+SRVD_LENOFF_ALL);
    }
    return(ubp->sp[SRVD_DIDOFF_ALL]);
}

/*--------------------------------------------------------*
    reading subroutines for downward commands & firmware
 *--------------------------------------------------------*/
int srvr_dncmd_packet_read(UBUF *ubp, BYTE *buf, int hlen, char htyp)
{
    int i, tn, clen;    /* hlen = data */

    if(htyp == DN_NORM_CMD)    {/* start from did, skip gwno */
        clen = set_b2_esc_p(ubp->sp + SRVD_DIDOFF_ALL, buf, SRVD_HEADLN_CMD - SRVD_DIDOFF_ALL);
    }
    else {
        memcpy(buf, ubp->sp + SRVD_DIDOFF_ALL, SRVD_HEADLN_CMD - SRVD_DIDOFF_ALL);
        clen = SRVD_HEADLN_CMD - SRVD_DIDOFF_ALL;
    }
    SV_D21("Buf[%d]=", __LINE__);
    SV_MDH(buf, clen);
    ubp->sp += SRVD_HEADLN_CMD;
    ubp->rn -= SRVD_HEADLN_CMD;

    while (ubp->rn < hlen) {
        if ((tn = srvr_ubp_read(ubp)) <= 0) 
            return(tn); /* fatal error : unknown */
    }

    if(htyp == DN_NORM_CMD)    {
        clen += set_b2_esc_p(ubp->sp, buf + clen, hlen);
    }
    else {
        memcpy(buf + clen, ubp->sp, hlen);
        clen += hlen;
    }
    SV_D21("Buf[%d]=", __LINE__);
    SV_MDH(buf, clen);
    ubp->sp += hlen; 
    ubp->rn -= hlen;

    return(clen);  /* clen : SVRD_HEAD_LN + len + ecs */
}

int srvr_dnfmw_packet_read(UBUF *ubp, BYTE *buf, int hlen)
{
    int i, tn, clen; /* clen = head(did)+hlen = 1+data */

    buf[0] = ubp->sp[SRVD_DIDOFF_ALL]; clen = 1;

    ubp->sp += SRVD_HEADLN_FMW;    /* skip length & addr */
    ubp->rn -= SRVD_HEADLN_FMW;

    for (  ; clen < hlen;  ) {
        if ((tn = srvr_ubp_read(ubp)) <= 0) 
            return(tn); /* fatal error : unknown */

        for (i = 0; i < ubp->rn; i++) { 
            if (clen > hlen)
                break;
            buf[clen++] = ubp->sp[i];
        }
        ubp->sp += i;
        ubp->rn -= i;
    }

    SV_D21("srvr_firmware_packet_read:\n ubp->rn = %d, ubp->sp=", ubp->rn);
    SV_MDH(ubp->sp, ubp->rn);
    SV_D22("[%s]: LINE(%d)\n", __FUNCTION__, __LINE__);
    return(clen); /* clen == hlen */
}

/*--------------------------------------------------------*
    queueing subroutines for downward commands & firmware
 *--------------------------------------------------------*/
void srvr_enque_dncmd(int htyp, int hlen, int clen, BYTE *buf)
{
    PKBDC   *pkbdc;

    pkbdc = dncmd_packet_alloc(clen); /* head+esced body */
    pkbdc->h_rlen = hlen + SRVD_HEADLN_CMD; /*real binary len */
    memcpy(pkbdc->b_data, buf, clen);
    pkbdc->b_data[clen] = E_CHR;

    q_insert((PKBH *)pkbdc, htyp);
}

void srvr_enque_dnfmw(int htyp, int hlen, int clen, BYTE *buf)
{
    PKBDF   *pkbdf;

    pkbdf = dnfmw_packet_alloc(clen); /* head(did)+binary body */
    pkbdf->h_rlen = clen;           /* total binary len      */
    memcpy(&pkbdf->buffer_body.deid, buf, clen);

    q_insert((PKBH *)pkbdf, htyp);
}

#define	H2D(c) (c > '9' ? c - 'A' + 10 : c - '0')
int srvr_enfuse_gtwy(int htyp, int hlen, int clen, BYTE *buf)
{
    char   old[48], *bp;
    time_t ct;
    int    exd, n;
    FILE   *fp;

    SV_D22("[%s]: LINE(%d)\n", __FUNCTION__, __LINE__);

    time(&ct);
    bp = ctime(&ct);
    bp[strlen(bp) -1] = 0;
    sprintf(old, "Egtwy.old_%s", bp);
    rename("Egtwy", old);

    SV_D22("[%s]: LINE(%d)\n", __FUNCTION__, __LINE__);
    exd = open("Egtwy", O_WRONLY | O_CREAT, 0755);
    n = write(exd, buf + 1, hlen);
    close(exd);

    D24("[%s]: LINE(%d), n=%d, hlen=%d\n", __FUNCTION__, __LINE__, n, hlen);

    if (n != hlen) {
    	unlink("Egtwy");
    	rename(old, "Egtwy");
        SV_D22("[%s]: LINE(%d)\n", __FUNCTION__, __LINE__);
    	return(-1);
    }
    SV_D22("[%s]: LINE(%d)\n", __FUNCTION__, __LINE__);
    return(1);
}

/*--------------------------------------------------------*
    sending subroutines for short PDUs
 *--------------------------------------------------------*/
int srvr_write_nego_packet(UBUF *ubp, BYTE *pdu) 
{
    SUPKC  *supkc = (SUPKC *)pdu;
    time_t  stime;

    stime = time(NULL);
    SV_D23("[%s:%d] time : %s\n", __FUNCTION__, __LINE__, ctime(&stime)); 
    set_n2_bin_p(stime, supkc->time, 4);

    if ((*srvr_ubp_write)(ubp, (BYTE *)supkc, SRVU_HEADLN_CMD + supkc->leng[0]) < 0) {
        D02("gtwy-->srvr : %s[%.2x] fail...\n", get_pdu_name(supkc->deid), supkc->deid);
        return(-1);
    }

    SV_D22("gtwy-->srvr : %s[%.2x]\n", get_pdu_name(supkc->deid), supkc->deid);

    return(SRVU_HEADLN_CMD + supkc->leng[0]);
}

int srvr_write_dta_of_gtwy(UBUF *ubp) {    /*  g/w --> server    */    
    typedef struct qstat {
	BYTE	voltg;    /* battery voltage */
	BYTE    q_type[4];
	BYTE    q_maxnp[4];
	BYTE    q_npack[4];
	BYTE    q_msize[4];
	BYTE    q_trcvd[4];
	BYTE    q_tsend[4];
	BYTE    q_tfail[4];
	BYTE    q_taway[4]; 
    } GWSTAT;

    GWSTAT  *gwstat;
    SUPKC   *supkc; /* packet buffer upstream cmdat */
    QUEUE   *q;
    char     buf[512];
    int      i;
    time_t   stime;

    supkc = (SUPKC *)buf;
    gwstat = (GWSTAT *)supkc->data;
    
    supkc->gwid[0]  = My_gateway_id & 0xff;
    supkc->gwid[1]  = My_gateway_id >> 8;
    supkc->deid     = DTA_OF_GTWY;
    supkc->leng[0]  = sizeof(GWSTAT);

    time(&stime);
    set_n2_bin_p(stime, supkc->time, 4);

    gwstat->voltg = 0;    /* dummy */

    for (i = 0, q = Queue;  i < N_OF_QUEUE; i++, q++) {
	set_n2_bin_p(q->type,  gwstat->q_type,  4);
	set_n2_bin_p(q->maxnp, gwstat->q_maxnp, 4);
	set_n2_bin_p(q->npack, gwstat->q_npack, 4);
	set_n2_bin_p(q->msize, gwstat->q_msize, 4);
	set_n2_bin_p(q->trcvd, gwstat->q_trcvd, 4);
	set_n2_bin_p(q->tsend, gwstat->q_tsend, 4);
	set_n2_bin_p(q->tfail, gwstat->q_tfail, 4);
	set_n2_bin_p(q->taway, gwstat->q_taway, 4);

        if ((*srvr_ubp_write)(ubp, (BYTE *)supkc, SRVU_HEADLN_CMD+supkc->leng[0]) < 0) {
            L02("gtwy-->srvr : %s[%.2x] fail...\n", get_pdu_name(DTA_OF_GTWY), DTA_OF_GTWY);
            return -1;
    }
        SV_D12("gtwy-->srvr : %s[%.2x]\n", get_pdu_name(DTA_OF_GTWY), DTA_OF_GTWY);
    }    
    return 1;
}

srvr_enque_mid_dncmd(UBUF *ubp) /* dn cmd while up image is being transmitted*/ 
{
    int htyp, hlen, clen;
    time_t tim;
    BYTE buf[MAX_NODE_PSIZE];
    QUEUE *q;
    PKBH *pkbh;

    SV_D20("enque_mid_dncmd(0) : entry \n");

    if ((htyp = srvr_packet_head_peek(ubp, &hlen)) <= 0)
        return(-1);
    if ((clen = srvr_dncmd_packet_read(ubp, buf, hlen, htyp)) <= 0)
        return(-1);

    srvr_enque_dncmd(htyp, hlen, clen, buf);

    q = q_by_htype(UP_NORM_IG1);
    while(pkbh = q_delete(q)) {
        Mfree(pkbh); q->taway++;
    }

    q = q_by_htype(UP_UGNT_IG1);
    while(pkbh = q_delete(q)) {
        Mfree(pkbh); q->taway++;
    }

    q = q_by_htype(UP_NORM_IG2);
    while(pkbh = q_delete(q)) {
        Mfree(pkbh); q->taway++;
    }

    q = q_by_htype(UP_UGNT_IG2);
    while(pkbh = q_delete(q)) {
        Mfree(pkbh); q->taway++;
    }

    q = q_by_htype(UP_TNOR_IG1);
    while(pkbh = q_delete(q)) {
        Mfree(pkbh); q->taway++;
    }
    
    time(&tim);
    SV_D22("enque_mid_dncmd(1) : recv : DNCMD_PACKT(%.2x)\n", htyp, ctime(&tim));
}

srvr_reboot_gtwy()
{
    SV_D02("[%s] : Line(%d), exit(7)\n", __FUNCTION__, __LINE__);
    exit(7);
}

void wait_srvr()
{
        pthread_mutex_lock(&Mutx_srvr);
        pthread_cond_wait(&Cond_srvr, &Mutx_srvr);
        pthread_mutex_unlock(&Mutx_srvr);
}

void wakeup_srvr()
{
    pthread_mutex_lock(&Mutx_srvr);
    pthread_cond_signal(&Cond_srvr);
    pthread_mutex_unlock(&Mutx_srvr);
}


void sig_pipe()
{
    SV_D00("SIGNAL PIPE ERROR....!\n");
}

void sig_int()
{
    SV_D00("SIGNAL INT ERROR ....!\n");
}



