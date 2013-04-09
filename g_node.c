/*-----------------------------------------*
	             g_node.c
		coded by H.B. Lee
  		    Feb/2007
	       Revisioned May/2009
 *-----------------------------------------*
 *         routines for node interface     *
 *-----------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>
#include <sys/fcntl.h>
#include <sys/termios.h>
#include <pthread.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_pack.h"
#include "g_queue.h"
#include "g_io.h"
#include "g_util.h"
#include "g_gtway.h"

int 	Node_conn_cnt;
time_t 	Node_conn_tim;

void   *do_node_listen(void *arg);
void   *do_node_interface(void *arg);

int     node_packet_type_peek(UBUF *ubp);
int     node_cmdat_packet_read(UBUF *ubp, BYTE *buf, int *cln, int *adr);
int     node_binry_packet_peek(UBUF *ubp, int *adr);	/* image & big data   */
int     node_binry_packet_read(UBUF *ubp, BYTE *buf, int clen);

int     node_dwn_commnds_write(UBUF *ubp); /* cmds of svr : sink_node <-- g/w */
int     node_dwn_firmwre_write(UBUF *ubp); /* frmw of svr : sink_node <-- g/w */

int     node_end_of_snd1_write(UBUF *ubp); /* END_OF_SND1 : sink_node <-- g/w */
					   /* REQ_TO_SOTA : sink node <-- g/w */
int     node_req_to_sota_write(UBUF *ubp, PKBDF *pkbdf);
int 	node_end_of_ota1_write(UBUF *ubp); /* END_OF_OTA1 : sink_node <-- g/w */

int     node_ubp_read(UBUF *ubp, int dbg);

void    wait_node();

pthread_mutex_t     Mutx_node = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      Cond_node = PTHREAD_COND_INITIALIZER;
pthread_condattr_t  Attr_node;

#ifndef B460800
#define B460800 0
#endif
/******************************************/
/***** for sink_node serial interface *****/
/******************************************/
void *do_node_listen(void *arg)
{
    char  *node_dev = (char *)arg, cmd[16];
    struct termios	newtio;
    UBUF  *ubp;
    int    nud, i, flag, uart_speed;

    wait_node();

    ND_D10("DEBUG:NODE:do_node_listen(0) : wakeup & begin...\n");

    switch(Node_uart_use) {
    case 0 : uart_speed = 0;       break;
    case 1 : uart_speed = B115200; break;
    default: uart_speed = B460800; break;
    }

    while (1) {
    	if ((nud = open(node_dev, O_RDWR | O_NOCTTY)) < 0) {
            L01("DEBUG:NODE:do_node_listen(0) : device %s not available.\n", node_dev);
            ND_D01("DEBUG:NODE:do_node_listen(0) : device %s not available.\n", node_dev);
	    sleep(Node_sfmi_sec*2);
            continue;
    	}

        memset(&newtio, 0, sizeof(newtio));
        newtio.c_cflag = CS8 | uart_speed | CLOCAL | CREAD;
        newtio.c_iflag = IGNBRK | IGNPAR;
        newtio.c_oflag = 0;
        newtio.c_lflag = 0;
        newtio.c_cc[VTIME] = 0;
        newtio.c_cc[VMIN] = 1;

        tcflush(nud, TCIFLUSH);
        tcflush(nud, TCOFLUSH);
        tcsetattr(nud, TCSANOW, &newtio);

	flag = fcntl(nud, F_GETFL, NULL);
	fcntl(nud, F_SETFL, flag | O_NONBLOCK);
	while(read(nud, cmd, sizeof(cmd)) > 0)
	    ;
	fcntl(nud, F_SETFL, flag);

        ubp = ubp_alloc(nud);

        ND_D12("DEBUG:NODE:do_node_listen(1) : device %s(%d) available.\n", node_dev, nud);
        do_node_interface(ubp);
        ND_D12("DEBUG:NODE:do_node_listen(1) : device %s(%d) close.\n", node_dev, nud);
        close(nud);
        ubp_free(ubp);
    }
}

void *do_node_interface(void *arg)
{
    PKBUC *pkbuc;
    PKBUI *pkbui;
    UBUF *ubp = (UBUF *)arg;
    BYTE  buf[MAX_NODE_PSIZE+1];
    int   htyp, hlen, hadr, clen, ilen, tn, n, eflag = 0, node_conn = 0;

    ND_D11("DEBUG:NOIN:do_node_interface(0) : start(%d)\n", ubp->ufd);

    while (!eflag) {

        if ((htyp = node_packet_type_peek(ubp)) < 0) {
            ND_D00("DEBUG:NOIN:do_node_interface(1) : type peek error...\n");
            break; /* htyp is the data entity id of the first data entity */
	    }

	    D12("\nDEBUG:NOIN:node-->gtwy : %s[%.2x]\n", get_pdu_name(htyp), htyp);

        if (htyp != UP_NORM_IG1 && htyp != UP_UGNT_IG1 &&
        			htyp != UP_NORM_IG2 && htyp != UP_UGNT_IG2 && 
        			htyp != UP_TNOR_IG1 && htyp != UP_NORM_BD1 && htyp != UP_UGNT_BD1) {
            hlen = node_cmdat_packet_read(ubp, buf, &clen, &hadr);

            /* hlen excludes o<7e 42 ... 7e> */
            /* hlen is the length of the all original data entites */
            /* clen is the real read size including 7e 42 ... 7e */
        }
        else {
	        hlen = node_binry_packet_peek(ubp, &hadr);
            /* hlen excludes any head info. : 7e 42 id(1) ln(4) ad(2) ... 7e */
            /* hlen is the length of the image data without head info */
        }

    	if (hlen <= 0) {
            ND_D01("DEBUG:NOIN:do_node_interface(3) : packet read error : htyp=%.2x\n", htyp);
    	    break;
    	}

        switch(htyp) {

        case UP_TNOR_DTA:
        case UP_UGNT_DTA: 
        case UP_NORM_DTA:
        case UP_NORM_CMD: /* = 4 : id(1) ln(1) ad(2) */
            if (hlen > MAX_REAL_FSIZE) {
                    ND_D03("DEBUG:NOIN:do_node_interface(5) : fail : CMDTA_PACKT(%.2x):%.8x,%d\n", htyp, hlen, hlen);
                    L03("DEBUG:NOIN:do_node_interface(5) : fail : CMDTA_PACKT(%.2x):%.8x,%d\n", htyp, hlen, hlen);
        		eflag++;
        		break;
    	    }
    	    
            if (!(pkbuc = upcdt_packet_alloc(hlen, htyp))) {
                    ND_D00("DEBUG:NOIN:do_node_interface(5) : fail : memory alloc\n");
		        eflag++;
                break;
            }
	        else {
                memcpy(pkbuc->b_data, buf, pkbuc->h_mlen);
                q_insert((PKBH *)pkbuc, htyp);
                    ND_D21("DEBUG:NOIN:do_node_interface(5) : recv : CMDTA_PACKT(%.2x)\n", htyp);
	        } 		/* fall through !!! */

        case REQ_TO_RCV1: 
            if ((n = node_dwn_commnds_write(ubp)) < 0) {
                    ND_D00("DEBUG:NOIN:do_node_interface(6) : fail : dwn commnds(%.2x)\n");
                eflag++;
    	 	    break;
    	    }
    	    if (!n && node_end_of_snd1_write(ubp) < 0) { /* END_OF_SND1 */
                    ND_D01("DEBUG:NOIN:do_node_interface(6) : fail : END_OF_SND1(%.2x)\n", END_OF_SND1);
                eflag++;
    	 	    break;
    	    }
    	    Node_conn_cnt++;
                ND_D21("DEBUG:NOIN:Node_conn_cnt=%d\n", Node_conn_cnt);
    	    time(&Node_conn_tim);
    	    wakeup_timr();
	        break;

        case RES_TO_SOTA: // response to start ota(REQ_TO_SOTA)
            if (node_dwn_firmwre_write(ubp) < 0)
    	        eflag++;
                break;
        case UP_TNOR_IG1:
        case UP_NORM_IG1: case UP_NORM_IG2: case UP_NORM_BD1: case UP_NORM_BD2:
        case UP_UGNT_IG1: case UP_UGNT_IG2: case UP_UGNT_BD1: case UP_UGNT_BD2:
            if (hlen >= MAX_BINRY_SIZE) {
                    ND_D03("DEBUG:NOIN:do_node_interface(7) : fail : IMAGE_PACKT(%.2x):%.8x,%d\n", htyp, hlen, hlen);
        		eflag++;
        		break;
    	    }					/* excludes sch1 & sch2    */
            clen = hlen + NDEU_HEADLN_IMG - 2; 	/* = 7 : id(1) ln(4) ad(2) */	
            if (!(pkbui = upbin_packet_alloc(clen, htyp))) {
                    ND_D00("DEBUG:NOIN:do_node_interface(7) : fail : memory alloc\n");
        		eflag++;
        		break;
    	    }
            if (node_binry_packet_read(ubp, pkbui->b_data, clen) < 0) {
                    ND_D03("DEBUG:NOIN:do_node_interface(8) : fail : IMAGE_PACKT(%.2x):%.8x,%d\n", htyp, hlen, hlen);
		        eflag++;
                break; 
            }
            q_insert((PKBH *)pkbui, htyp);

    	    if (Ugnt_flag) {
        		wakeup_srvr();
        		Ugnt_flag = 0;
    	    }

            break;

        default :
                ND_D02("DEBUG:NOIN:do_node_interface(9) : recv : ??? Unknown type(%d, %d)\n", htyp, ubp->ufd);
            eflag++;
        }
    }

    L01("DEBUG:NOIN:do_node_interface(0) : stop by error(%d)\n", ubp->ufd);
    ND_D01("DEBUG:NOIN:do_node_interface(0) : stop by error(%d)\n", ubp->ufd);
}

/******************************************/
/*****      synchronize preamble      *****/
/***** return packet Data Entity Type *****/
/******************************************/
int node_packet_type_peek(UBUF *ubp)	
{ /* see only type */
    int tn, i;
    int new_entry = 1;

    while(1) {

        ND_D21("DEBUG:NOPE:node_packet_type_peek(1) : rn=%d\n", ubp->rn); 
        MDH(ubp->sp, ubp->rn);

        if (new_entry)  {
            while(ubp->rn > 0 && *ubp->sp != S_CHR1) {    /* discard garbage */
                ubp->sp++, ubp->rn--;
            }
        }

        while(ubp->rn > 1 && ubp->sp[1] == S_CHR1)  /* skip preamble */
            ubp->sp++, ubp->rn--;
            
        if (ubp->rn > 1) {
            if (ubp->sp[0] == S_CHR1 && ubp->sp[1] == S_CHR2)
                break;
            else {
                ubp->sp++, ubp->rn--;	/* skip current S_CHR1 */
                continue;
            }
        }

        if (!ubp->rn)
            ubp_flush(ubp), new_entry = 1;
        else
            new_entry = 0;  /* continuous read */

        if ((tn = node_ubp_read(ubp, 1)) <= 0) 
            return(tn); /* fatal error : unknown */
    }

    while (ubp->rn < NDEU_DTAOFF_CMD +1) { /* for case escape +1    */
        if ((tn = node_ubp_read(ubp, 2)) <= 0) 
            return(tn); /* fatal error : unknown */

        ND_D21("DEBUG:NOPE:node_packet_type_peek(7) : rn=%d, sp=", ubp->rn); 
        ND_MDH(ubp->sp, ubp->rn);

    }

    if (ubp->sp[NDEU_DIDOFF_ALL] != E_CHR)
        return(ubp->sp[NDEU_DIDOFF_ALL]);  /* return (Data Entity Type) */
    else
        return((ubp->sp[NDEU_DIDOFF_ALL +1]) ^ C_MASK); /* return (Data Entity Type) */
}

/******************************************/
/*********** only for image packet ********/
/*** return size & addr of image packet ***/
/******************************************/
int node_binry_packet_peek(UBUF *ubp, int *adr)
{ /* return size of image */
    int len, tn, i;
    int new_entry = 1;
    NUPKI *nupki;

    ND_D21("DEBUG:NOBI:node_binry_packet_peek(1) : rn=%d, sp=\n", ubp->rn); 
    //ND_MDH(ubp->sp, ubp->rn);

    /* = 9 */
    while (ubp->rn < NDEU_DTAOFF_IMG) { /* try image packet first */
        if ((tn = node_ubp_read(ubp, 2)) <= 0) 
        return(tn); /* fatal error : unknown */
        ND_D21("DEBUG:NOBI:node_binry_packet_peek(2) : rn=%d, sp=\n", ubp->rn); 
        //ND_MDH(ubp->sp, ubp->rn);
    }

    ND_D21("DEBUG:NOBI:node_binry_packet_peek(3) : rn=%d, sp=\n", ubp->rn); 
    //ND_MDH(ubp->sp, ubp->rn);

    ubp->phase = HEAD_END;

    nupki = (NUPKI *)ubp->sp;
    /* = 3 */
    len  = get_bin_p2_n(nupki->leng); 

/*
    len  = (nupki->leng[3] << 24); 
    len |= (nupki->leng[2] << 16) & 0x00ff0000;
    len |= (nupki->leng[1] <<  8) & 0x0000ff00;
    len |= (nupki->leng[0]);
*/

    /* = 7 */
    *adr  = (nupki->addr[1] <<  8) & 0x0000ff00; 
    *adr |= (nupki->addr[0]);

    ND_D24("DEBUG:NOBI:node_binry_packet_peek(4) : %.2x%.2x%.2x%.2x\n",ubp->sp[0],ubp->sp[1],ubp->sp[2],ubp->sp[3]);

    return(len); /* return (Data Entity Type) */
}

/*******************************************/
/** for up/dnward command & numeric data ***/
/*******************************************/
int node_cmdat_packet_read(UBUF *ubp, BYTE *buf, int *cln, int *adr)
{ /*return length including 7e,42, 7e */
    int tn;
    int i = NDEU_DTAOFF_CMD, j, k;
    NUPKC *nupkc;
    /* = 6 */

    ND_D21("DEBUG:CMRE:node_cmdat_packet_read(1) : rn=%d, sp=", ubp->rn); 
    ND_MDH(ubp->sp, ubp->rn);

    while(1) {
        for ( ; i < ubp->rn; i++) {
            if (ubp->sp[i] == E_CHR)
            break;
        }

        if (i < ubp->rn)
            break; /* Ok. one packet */

        if ((tn = node_ubp_read(ubp, 3)) <= 0) 
            return(tn); /* fatal error : unknown */

    }

    i++;
    *cln  = i;

    for (j = 2, k = 0; j < i -1; j++, k++) { /* discard 7e 42 ... 7e */
         if (k >= MAX_NODE_PSIZE) {
            ND_D01("DEBUG:CMRE:buffer error....[%d]!\n", k);
            return -1;
          }
         if (ubp->sp[j] != C_CHR)
            buf[k] = ubp->sp[j];
         else
            buf[k] = ubp->sp[++j] ^ C_MASK;
    }
    ubp->sp += i; ubp->rn -= i;
    ubp->phase = NORM_END;

    nupkc = (NUPKC *)(buf-2);

    /* = 2 : id(1) ln(1) */
    *adr  = nupkc->addr[1] << 8;
    *adr |= nupkc->addr[0];

    ND_D22("DEBUG:CMRE:node_cmdat_packet_read(8) : i=%d,k=%d, buf=", i, k); 
    ND_MDH(buf, k);

    return(k); /* return not yet packet */
}

/*******************************************/
/******** for up/dnward image data *********/
/*******************************************/
int node_binry_packet_read(UBUF *ubp, BYTE *buf, int clen)
{ /* clen : id(1)+ln(4)+a(2)+data(n) = 1+4+2+hlen */
    int tn, cn;
    BYTE *bp;

    ND_D21("DEBUG:BIRE:node_binry_packet_read(1) : rn=%d, sp=", ubp->rn); 
    ND_MDH(ubp->sp, ubp->rn);

    if (ubp->rn > 0) { /* header part */
                                                  /* = 9 *             = 2 */ 
        memcpy(buf, ubp->sp + NDEU_DIDOFF_ALL, NDEU_HEADLN_IMG - NDEU_DIDOFF_ALL);
        ubp->sp += NDEU_HEADLN_IMG;
        ubp->rn -= NDEU_HEADLN_IMG; 
        bp = buf + (NDEU_HEADLN_IMG - NDEU_DIDOFF_ALL);
        tn = NDEU_HEADLN_IMG - NDEU_DIDOFF_ALL; 
    }
    else {
        bp = buf;
        tn = 0;
    }
    for ( ; tn <= clen; tn += cn) { /* waste away last 7e */
        cn = clen -tn +1;
        if (cn > MAX_NODE_PSIZE)
            cn =  MAX_NODE_PSIZE;
        while (ubp->rn < cn) {
            if (node_ubp_read(ubp, 5) <= 0)
            	return(-1);
        }
        memcpy(bp, ubp->sp, cn);
        ubp->sp += cn, ubp->rn -= cn;
        bp += cn;
    }
    return(clen);
}


/*******************************************/
/**** for sink_node interface cmd & fmw ****/
/*******************************************/
int node_dwn_commnds_write(UBUF *ubp)	/*  sink_node <-- g/w */
{
    /*  only one command ??? */
    QUEUE    *q;
    PKBDC    *pkbdc;
    PKBDF    *pkbdf;
    int      n, i, odr, eflag = 0;

    for (odr = -1, i = 0; (!eflag && (odr >= -MAX_DORDER)); odr--) {
        if (!(q = q_by_order(odr)))
            continue;

        if (q->flag & Q_FIRMW) { // maybe the first queue
            if (!(pkbdf = (PKBDF *)q_peek(q)))
                continue;
            node_req_to_sota_write(ubp, pkbdf);
                return(1);
        }

        if (!(pkbdc = (PKBDC *)q_delete(q)))
            continue;

        if (node_write(ubp->ufd, &pkbdc->b_sch1, pkbdc->h_mlen +3) < 0) {
            if (++(pkbdc->h_nretry) > q->maxrt) {
                q->taway++;
                Mfree(pkbdc);
            }
            else {
                q->tfail++;
                q_insert_bak((PKBH *)pkbdc, q);
            }
            ND_D01("DEBUG:DWWR:node_dwn_comnds_write: send : DNCMD PACKT(%.2x) fail.\n", pkbdc->b_data[0]);
            return(-1);
        }
        else {
            ND_D12("DEBUG:DWWRnode<--gtwy : %s[%.2x]\n", get_pdu_name(pkbdc->b_data[0]), pkbdc->b_data[0]);
            q->tsend++;
            Mfree(pkbdc);
            return(1); 		// send only one frame
        }
        i++;
    }
    return(i);
}

int node_dwn_firmwre_write(UBUF *ubp)	/*  sink_node <-- g/w */
{
    QUEUE   *q;
    PKBDF   *pkbdf;
    NDPKF   *ndpkf;
    BYTE    buf[MAX_NODE_PSIZE], *bp;
    //BYTE    si, fi, lng[4], seq[2];
    int     i, tx_ok, eflag, tlen, rlen, didx, fidx, flen, fseq;
    int     htyp, hlen, clen, hadr;

    q = q_by_htype(DN_NORM_FMW);

    pkbdf = (PKBDF *)q_delete(q);
    tlen = rlen = pkbdf->h_mlen;
    ndpkf = (NDPKF *)buf;
    
    for (eflag = tx_ok = fseq = didx = 0; !tx_ok && !eflag; didx += flen ) {
    ndpkf->sch1 = 0x7E; 	    /* 0 */
    ndpkf->sch2 = 0x42; 	    /* 1 */
    ndpkf->deid = REQ_TO_SNDF;      /* 2 */ // not esc character */



        if (rlen > 0) {
            if (rlen > 120)  
                flen = 120;
            else
                flen = rlen;

    	    fidx  = set_n2_esc_p(flen, ndpkf->data, 4);

            ndpkf->data[fidx++] = 0xff;	// node addr
            ndpkf->data[fidx++] = 0xff;

	    fidx += set_n2_esc_p(fseq, ndpkf->data + fidx, 2);

	    fidx += set_b2_esc_p(pkbdf->buffer_body.data + didx, ndpkf->data + fidx, flen);
            
            ndpkf->data[fidx] = E_CHR; 			// tail

            node_write(ubp->ufd, &ndpkf->sch1, 3 + fidx + 1 );

            ND_D12("DEBUG:FWWR:node<--gtwy : %s[%.2x]\n", get_pdu_name(REQ_TO_SNDF), REQ_TO_SNDF);
        }
	else {	// rlen == 0
            node_end_of_ota1_write(ubp);
        ND_D21("DEBUG:FWWR:do_node_interface(2) : send : END_OF_OTA1(%.2x)\n", END_OF_OTA1);
	}

        if ((htyp = node_packet_type_peek(ubp)) < 0 || 
		(hlen = node_cmdat_packet_read(ubp, buf, &clen, &hadr)) <= 0)
            break;

        ND_D12("DEBUG:FWWR:node-->gtwy : %s[%.2x]\n", get_pdu_name(htyp), htyp);

        switch (htyp) {
        case ACK_TO_SNDF: 
            fseq++;
            rlen -= flen;
            break;

        case END_OF_OTA2:
            q->tsend++;
            Mfree(pkbdf);
            tx_ok = 1;
            break;

        case NCK_TO_SNDF:
            if (++(pkbdf->h_nretry) > q->maxrt) {
                q->taway++;
                Mfree(pkbdf);
            }
            else {
                q->tfail++;
                q_insert_bak((PKBH *)pkbdf, q);
            }
            eflag = 1;
            break;

        default :
            if (++(pkbdf->h_nretry) > q->maxrt) {
                q->taway++;
                Mfree(pkbdf);
            }
            else {
                q->tfail++;
                q_insert_bak((PKBH *)pkbdf, q);
            }
            eflag = 1;
        }
    }
    if (tx_ok)
	return(1);
    else
	return(-1);
}

int node_end_of_snd1_write(UBUF *ubp)	/*  sink_node <-- g/w */
{
    static BYTE end_of_snd1[] = {S_CHR1, S_CHR2, END_OF_SND1, 0, 0,0, E_CHR};

    if (node_write(ubp->ufd, end_of_snd1, sizeof(end_of_snd1)) < 0) {
        ubp_flush(ubp);
        return(-1);
    }
    ND_D12("DEBUG:SND1:node<--gtwy : %s[%.2x]\n\n", get_pdu_name(END_OF_SND1), END_OF_SND1);
    return(sizeof(end_of_snd1));

}

int node_req_to_sota_write(UBUF *ubp, PKBDF *pkbdf)
{
    int	    flen;

    static BYTE req_to_sota[16] = {S_CHR1, S_CHR2, REQ_TO_SOTA, 0x04, 0xff, 0xff, };  
    flen = set_n2_esc_p(pkbdf->h_mlen, req_to_sota + 6, 4) + 6; 
    req_to_sota[flen++] = E_CHR;

    if ((node_write(ubp->ufd, req_to_sota, flen)) < 0) {
    	ubp_flush(ubp);
        return(-1);
    }
    ND_D12("DEBUG:SOTA:node<--gtwy : %s[%.2x]\n", get_pdu_name(REQ_TO_SOTA), REQ_TO_SOTA);
    return(flen);
}

int node_end_of_ota1_write(UBUF *ubp) /*  sink_node <-- g/w */
{
    static BYTE end_of_ota1[] = {S_CHR1, S_CHR2, END_OF_OTA1, 0, 0,0, E_CHR};

    if (node_write(ubp->ufd, end_of_ota1, sizeof(end_of_ota1)) < 0) {
        ubp_flush(ubp);
        return(-1);
    }
    ND_D12("DEBUG:OTA1:node<--gtwy : %s[%.2x]\n", get_pdu_name(END_OF_OTA1), END_OF_OTA1);
    return(sizeof(end_of_ota1));
}

/*******************************************/
/********* block & time expireation ********/
/*******************************************/
int  node_ubp_read(UBUF *ubp, int dbg)
{
    int tn, i, j;
    BYTE *sp;

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
        tn = ubp->bsz / 2;

    if (!tn) {
        D24("???:%d-%d:%d,%d\n", dbg, ubp->rn, ubp->sp - ubp->buf, ubp->cp - ubp->buf);
        ubp_flush(ubp);
        return(-1);
    }
    if ((tn = node_read(ubp->ufd, ubp->cp, tn)) <= 0) {
        ubp_flush(ubp);
        return(-1);
    }
    ubp->cp += tn, ubp->rn += tn, *ubp->cp = 0;
    return(tn);
}

void wait_node()
{
        pthread_mutex_lock(&Mutx_node);
        pthread_cond_wait(&Cond_node, &Mutx_node);
        pthread_mutex_unlock(&Mutx_node);
}

void wakeup_node()
{
    pthread_mutex_lock(&Mutx_node);
    pthread_cond_signal(&Cond_node);
    pthread_mutex_unlock(&Mutx_node);
}
