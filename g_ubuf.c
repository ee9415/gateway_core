 /*-----------------------------------------*
                 g_ubuf.c
              coded by H.B. Lee
                  Feb/2007
             Revisioned Mya/2009
 *-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"

/*******************************************/
/******** io buffer space management *******/
/*******************************************/

static  UBUF	Ubuf[MAX_UBUF_NUMBR];

void ubp_init() 
{
    int i;

    for (i = 0; i < MAX_UBUF_NUMBR; i++) {
        //Ubuf[i].buf = malloc(100);
        Ubuf[i].buf = (BYTE *)malloc(MAX_NODE_PSIZE*2 +1);
        Ubuf[i].bsz = MAX_NODE_PSIZE*2;
        ubp_free(&Ubuf[i]);
    }
}

void ubp_free(UBUF *ubp) 
{
    ubp->sp = ubp->cp = ubp->buf;
    ubp->mp = ubp->sp + ubp->bsz/2;
    ubp->ep = ubp->sp + ubp->bsz;  /* for unlimited big cdma packet */
    ubp->rn = ubp->phase = ubp->flag = 0;
}

UBUF *ubp_alloc(int ufd) 
{
    int i;

    for (i = 0; i < MAX_UBUF_NUMBR; i++) {
        if (!Ubuf[i].flag) {
            ubp_flush(&Ubuf[i]);
            Ubuf[i].flag = UBUF_USE;
            Ubuf[i].ufd = ufd;
            return(&Ubuf[i]);
        }
    }
    return(NULL);
}

void ubp_flush(UBUF *ubp) 
{
    ubp->sp = ubp->cp = ubp->buf; 
    ubp->mp = ubp->sp + ubp->bsz/2;
    ubp->ep = ubp->sp + ubp->bsz;  /* for unlimited big cdma packet */
    ubp->rn = ubp->phase = ubp->sp[0] = 0;
}
