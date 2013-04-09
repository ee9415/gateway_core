/*-----------------------------------------*
                g_queue.c
            coded by H.B. Lee
                Feb/2007
             Revisioned May/2009
 *-----------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/time.h>
#include <pthread.h>
 
#include "g_macros.h"
#include "g_config.h"
#include "g_pack.h"
#include "g_queue.h"
#include "g_util.h"

int Ugnt_flag = 0, Full_flag = 0;

QUEUE Queue[N_OF_QUEUE] = 
{   /*           flag          type        maxnpk maxrty order ...  */  
    { Q_CMDTA|Q_UPWRD|Q_URGNT, UP_UGNT_DTA, 0,      0,    1,     
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_CMDTA|Q_UPWRD,         UP_NORM_CMD, 0,      0,    2,    
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_CMDTA|Q_UPWRD,         UP_NORM_DTA, 0,      0,    3,    
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_IMAGE|Q_UPWRD|Q_URGNT, UP_UGNT_IG2, 0,      0,    4,    
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_IMAGE|Q_UPWRD|Q_URGNT, UP_UGNT_IG1, 0,      0,    5,    
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_BGDTA|Q_UPWRD|Q_URGNT, UP_UGNT_BD2, 0,      0,    6,    
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_BGDTA|Q_UPWRD|Q_URGNT|Q_IMAGE, UP_UGNT_BD1, 0,      0,    7,    
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_IMAGE|Q_UPWRD,         UP_NORM_IG2, 0,      0,    8,    
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_IMAGE|Q_UPWRD,         UP_NORM_IG1, 0,      0,    9,   
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_BGDTA|Q_UPWRD,         UP_NORM_BD2, 0,      0,   10,   
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_BGDTA|Q_UPWRD|Q_IMAGE, UP_NORM_BD1, 0,      0,   11,   
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_CMDTA|Q_UPWRD,         UP_TNOR_DTA, 0,      0,   12,   
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_IMAGE|Q_UPWRD,         UP_TNOR_IG1, 0,      0,   13,   
                        PTHREAD_MUTEX_INITIALIZER },


    { Q_FIRMW|Q_DNWRD,         DN_NORM_FMW, 0,      0,   -1, 
                        PTHREAD_MUTEX_INITIALIZER },	/* must be the 1st */

    { Q_CMDTA|Q_DNWRD|Q_URGNT, DN_UGNT_CMD, 0,      0,   -2,     
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_CMDTA|Q_DNWRD,         DN_NORM_CMD, 0,      0,   -3,    
                        PTHREAD_MUTEX_INITIALIZER },

    { Q_CMDTA|Q_DNWRD,         DN_UNKN_CMD, 0,      0,   -4,    
                        PTHREAD_MUTEX_INITIALIZER },
};

que_init()
{
    QUEUE *q;

    if (q = q_by_htype(UP_UGNT_DTA))
        q->maxnp = Up_ugnt_dat_q, q->maxrt = Up_ugnt_dat_r;

    if (q = q_by_htype(UP_NORM_CMD))
        q->maxnp = Up_norm_cmd_q, q->maxrt = Up_norm_cmd_r;

    if (q = q_by_htype(UP_NORM_DTA))
        q->maxnp = Up_norm_dat_q, q->maxrt = Up_norm_dat_r;

    if (q = q_by_htype(UP_UGNT_IG2))
        q->maxnp = Up_ugnt_img_q, q->maxrt = Up_ugnt_img_r;

    if (q = q_by_htype(UP_UGNT_IG1))
        q->maxnp = Up_ugnt_img_q, q->maxrt = Up_ugnt_img_r;

    if (q = q_by_htype(UP_UGNT_BD2))
        q->maxnp = Up_ugnt_bdt_q, q->maxrt = Up_ugnt_bdt_r;

    if (q = q_by_htype(UP_UGNT_BD1))
        q->maxnp = Up_ugnt_bdt_q, q->maxrt = Up_ugnt_bdt_r;

    if (q = q_by_htype(UP_NORM_IG2))
        q->maxnp = Up_norm_img_q, q->maxrt = Up_norm_img_r;

    if (q = q_by_htype(UP_NORM_IG1))
        q->maxnp = Up_norm_img_q, q->maxrt = Up_norm_img_r;

    if (q = q_by_htype(UP_NORM_BD2))
        q->maxnp = Up_norm_bdt_q, q->maxrt = Up_norm_bdt_r;

    if (q = q_by_htype(UP_NORM_BD1))
        q->maxnp = Up_norm_bdt_q, q->maxrt = Up_norm_bdt_r;
        
    if (q = q_by_htype(UP_TNOR_DTA))
        q->maxnp = Up_tnor_dat_q, q->maxrt = Up_tnor_dat_r;
        
    if (q = q_by_htype(UP_TNOR_IG1))
        q->maxnp = Up_tnor_ig1_q, q->maxrt = Up_tnor_ig1_r;

    if (q = q_by_htype(DN_NORM_FMW))
        q->maxnp = 1,		  q->maxrt = Dn_norm_fmw_r;

    if (q = q_by_htype(DN_UGNT_CMD))
        q->maxnp = Dn_ugnt_cmd_q, q->maxrt = Dn_ugnt_cmd_r;

    if (q = q_by_htype(DN_NORM_CMD))
        q->maxnp = Dn_norm_cmd_q, q->maxrt = Dn_norm_cmd_r;

    if (q = q_by_htype(DN_UNKN_CMD))
        q->maxnp = Dn_unkn_cmd_q, q->maxrt = Dn_unkn_cmd_r;
}

/*******************************************/
/*-----  get queue by type or priority ----*/
/*******************************************/
QUEUE *q_by_htype(int htyp)
{
    QUEUE   *qp = Queue;
    int      i;

    for (i = 0; i < N_OF_QUEUE; qp++, i++)
        if (qp->type == htyp)
            return(qp);
    return(NULL);
}

QUEUE *q_by_order(int ordr)
{
    QUEUE   *qp = Queue;
    int      i;

    for (i = 0; i < N_OF_QUEUE; qp++, i++) {
        if (qp->order == ordr)
            return(qp);
    }
    return(NULL);
}

/*******************************************/
/******* up/downwward  packet queueing *****/
/*******************************************/
void q_insert(PKBH *pkbh, int htyp)
{
    QUEUE   *q = q_by_htype(htyp);
    PKBH    *pkbt;


    pkbh->hnext = NULL;

    if (q->flag & Q_DNWRD) {
	if (!(q->flag & Q_FIRMW))
            dncmd_packet_insert((PKBDC *)pkbh, q);    /* merge if possible */
	else
            dnfmw_packet_insert((PKBDF *)pkbh, q);
        return;
    }

    pthread_mutex_lock(&q->lock);

    if (!q->f)
        q->f = q->r = pkbh;
    else
        q->r->hnext = pkbh, q->r = pkbh;

    q->npack++;
    q->msize += pkbh->hmlen;
    q->trcvd++;
    
    if (q->npack > q->maxnp) {
        pkbt = q->f;
        q->f = q->f->hnext;
        q->npack--, q->msize -= pkbt->hmlen; q->taway++;
        Mfree(pkbt);
    }
    else if ((q->npack+5) == q->maxnp && q->flag & Q_UPWRD)
        Full_flag++;

    pthread_mutex_unlock(&q->lock);

    if (q->flag & Q_URGNT)
        Ugnt_flag++;
}

void q_insert_bak(PKBH *pkbh, QUEUE *q)
{
    pthread_mutex_lock(&q->lock);
    if (q->npack < q->maxnp) { /* insert in front */
        if (!q->f)
            q->f = q->r = pkbh;
        else
            pkbh->hnext = q->f, q->f = pkbh;
        q->npack++, q->msize += pkbh->hmlen;
    }
    else
    q->taway++;
    pthread_mutex_unlock(&q->lock);
}

PKBH *q_peek(QUEUE *q)
{
    return(q->f);
}

PKBH *q_delete(QUEUE *q)
{
    PKBH *pkbh;

    pthread_mutex_lock(&q->lock);

    if (pkbh = q->f) {
        if (!(q->f = pkbh->hnext))
            q->r = NULL;
        q->npack--;
        q->msize -= pkbh->hmlen;
    }
    pthread_mutex_unlock(&q->lock);
    return(pkbh);
}

/*---------------------------------------------* 
   merge downwards commands packet if possible   
 *---------------------------------------------*/
void dncmd_packet_insert(PKBDC *pkbdc, QUEUE *q)
{
    pthread_mutex_lock(&q->lock);

    q->msize += pkbdc->h_mlen;

    if (!q->f) { 
        q->f = q->r = (PKBH *)pkbdc;
        q->trcvd++; q->npack++;
    }
    else {
        if (q->r->hrlen + pkbdc->h_rlen < MAX_REAL_FSIZE && 
           (q->r->hmlen + pkbdc->h_mlen < MAX_NODE_PSIZE)) { 
            memcpy(((PKBDC *)(q->r))->b_data + q->r->hmlen, pkbdc->b_data, pkbdc->h_mlen);
            q->r->hmlen += pkbdc->h_mlen;
            q->r->hrlen += pkbdc->h_rlen;
            ((PKBDC *)(q->r))->b_data[q->r->hmlen] = E_CHR;
            Mfree(pkbdc);
        }
        else { 
            q->trcvd++;
            if (q->npack >= q->maxnp) {
                q->taway++;
                q->msize -= pkbdc->h_mlen;
                Mfree(pkbdc);
            }
            else {
                q->r->hnext = (PKBH *)pkbdc, q->r = (PKBH *)pkbdc;
                q->npack++; 
            }
        }
    }
    pthread_mutex_unlock(&q->lock);
}

/*---------------------------------------------* 
   does not allow overlapped firmware download   
 *---------------------------------------------*/
void dnfmw_packet_insert(PKBDF *pkbdf, QUEUE *q)
{
    if (q->f != NULL) {
	D00("firmware  overlapped, will be discarded...\n");
	return;
    }

    pthread_mutex_lock(&q->lock);

    q->msize += pkbdf->h_mlen;
    q->f = q->r = (PKBH *)pkbdf;
    q->trcvd++; 
    q->npack++;

    pthread_mutex_unlock(&q->lock);
}

  
/*******************************************/
/***** up/downwward  packet allocation *****/
/*******************************************/
PKBUC *upcdt_packet_alloc(int csize, BYTE ctype)
{
    PKBUC   *pkubc;
    int      ctime;

    if (!(pkubc = (PKBUC *)Malloc(sizeof(PKBUC) + csize + 1)))
        return(NULL);

    pkubc->h_next = NULL;
    pkubc->h_mlen = csize;
    pkubc->h_nretry = 0;

    pkubc->b_gwid[0] = My_gateway_id & 0xff;
    pkubc->b_gwid[1] = My_gateway_id >> 8;
    pkubc->b_type = ctype;
    pkubc->b_leng[0] = csize; /* include itself */

    ctime = time(NULL);
    set_n2_bin_p(ctime, pkubc->b_time, 4);

    return (pkubc);
}

PKBUI *upbin_packet_alloc(int bsize, BYTE itype)
{
    PKBUI *pkbui;
    int    btime;

    if (!(pkbui = (PKBUI *)Malloc(sizeof(PKBUI) + bsize + 1)))
        return(NULL);

    btime = time(NULL);

    pkbui->h_next = NULL;
    pkbui->h_mlen = bsize;
    pkbui->h_nretry = 0;

    pkbui->b_gwid[0] = My_gateway_id & 0xff;
    pkbui->b_gwid[1] = My_gateway_id >> 8;
    pkbui->b_type = itype;

    set_n2_bin_p(bsize, pkbui->b_leng, 4);
    set_n2_bin_p(btime, pkbui->b_time, 4);

    return (pkbui);
}

PKBDC *dncmd_packet_alloc(int csize)
{
    PKBDC *pkbdc;

    if (!(pkbdc = (PKBDC *)Malloc(sizeof(PKBDC) + MAX_NODE_PSIZE +1)))
    	return(NULL);       /* for easy merging */

    pkbdc->h_next = NULL,   pkbdc->h_mlen = csize;  /* head+escaped body */
    pkbdc->h_nretry = 0;
    pkbdc->b_sch1 = S_CHR1, pkbdc->b_sch2 = S_CHR2;

    return (pkbdc);
}

int dncmd_packet_rlength(PKBDC *pkbdc)
{ 				/* real length withot escape sequence */
    int rtlen = 0;
    BYTE *cp, *ep;

    for (cp = pkbdc->b_data; cp < pkbdc->b_data + pkbdc->h_mlen; cp = ep) {
        ep = cp + cp[1] +1 +1 +2; /* pid(1), len(1), adr(2) */	
        for (cp +=4; cp < ep; cp++) {
             if (*cp == C_CHR)
                cp++;
             rtlen++;
        }
    }

    return(rtlen);
}

PKBDF *dnfmw_packet_alloc(int fsize)
{
    PKBDF *pkbdf;
                                      
    if (!(pkbdf = (PKBDF *)Malloc(sizeof(PKBDF) + fsize +1)))
        return(NULL); /* for easy merging */

    pkbdf->h_next = NULL;
    pkbdf->h_mlen = fsize; /* head+escaped body */
    pkbdf->h_nretry = 0;

    return (pkbdf);
}

static pthread_mutex_t Mutex_mem = PTHREAD_MUTEX_INITIALIZER;

void *mmalloc(size_t  size)
{
    char *mp;

    pthread_mutex_lock(&Mutex_mem);
    mp = (char *)malloc(size);
    pthread_mutex_unlock(&Mutex_mem);

    if (!mp) {
        printf("!!!!!!!!!!!!!!!!!!\n");
    }
    return(mp);
}

void mfree(void *mp)
{
    pthread_mutex_lock(&Mutex_mem);
    free(mp);
    pthread_mutex_unlock(&Mutex_mem);
}
