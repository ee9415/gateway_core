/*-----------------------------------------*
g_io.c
coded by H.B. Lee
Feb/2007
Revisioned May/2009
*-----------------------------------------*/

#include <stdio.h>
#include <sys/time.h>
#include <pthread.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_gtway.h"

#define PRINT_SIZE 16*2
BYTE bin2hex[]= {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
'A', 'B', 'C', 'D', 'E', 'F'};

/*":  ;  <  =  >  ?  @" */
BYTE hex2bin[]= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0, 0,
10,11,12,13,14,15 };
/*  " A  B  C  D  E  F" */

/******************************************/
/**** raw level io routine for logging ****/
/******************************************/

bb_read(int fd, BYTE *buf, int size)     /* binary, block              */
{
    int n;
    ND_D21("[__%s__]\n", __FUNCTION__);
    n=read(fd, buf, size);
    ND_D22("< read>(%2d,%2d)", fd, n);
    ND_MDH(buf, (n<PRINT_SIZE)?n:PRINT_SIZE);    
    return(n);
}

bt_read_cn(int fd, BYTE *buf, int size)  /* binary, timer, cancel null */
{
    int n, i, j;
    struct timeval tv;
    fd_set rdset;

    SV_D21("[__%s__]\n", __FUNCTION__);
    if (Srvr_iot_msec > 0) {
        FD_ZERO(&rdset); 
        FD_SET(fd, &rdset);

        tv.tv_sec  = Srvr_iot_msec/1000;
        tv.tv_usec = (Srvr_iot_msec - tv.tv_sec*1000)*1000;

        n = select(fd+1, &rdset, NULL, NULL, &tv);
        if (n <= 0) {
            if(n == 0)
                SV_D01("bt_read_cn() : timeout for %d.\n", fd);
            SV_D01("bt_read_cn() : error for %d.\n", n);
            return(-1);  /* timeout ...... */
        }
    }

    n = read(fd, buf, size);

    SV_D22("< read>(%2d,%2d)", fd, n);
    SV_MDH(buf, (n<PRINT_SIZE)?n:PRINT_SIZE);
    for (i = j = 0; i < n; i++) {
        //if (buf[i]){
        //D21("__ i = %d __\n", i);
        buf[j++] = buf[i];
        //}
    }

    buf[j] = 0; 
    n = j;
    return(n);
}

bt_read(int fd, BYTE *buf, int size)     /* binary, timer		*/
{
    int n, i, ret;

    struct timeval tv;
    fd_set rdset;

    D21("[__%s__]\n", __FUNCTION__);
    if (Srvr_iot_msec > 0) {
        FD_ZERO(&rdset); FD_SET(fd, &rdset);

        tv.tv_sec  = Srvr_iot_msec/1000;
        tv.tv_usec = (Srvr_iot_msec - tv.tv_sec*1000)*1000;

        n = select(fd +1, &rdset, NULL, NULL, &tv);
        if (n <= 0) {
            L01("bt_read() : timeout for %d.\n", fd);
            return(-1);  /* timeout ...... */
        }
    }
    ret= read(fd, buf, size);
    SV_D22("< read>(%2d,%2d)", fd, ret);
    SV_MDH(buf, (n<PRINT_SIZE)?n:PRINT_SIZE);
    return(ret);
}

ab_read(int fd, BYTE *buf, int size) 	/* ascii,  block		*/
{ 
    int n, tn, cn;
    BYTE tbuf[MAX_NODE_PSIZE*2], *tbp, hb, lb;

    D21("[__%s__]\n", __FUNCTION__);
    if (size > MAX_NODE_PSIZE)
        cn = MAX_NODE_PSIZE*2;
    else
        cn = size * 2;

    for (tn = 0, tbp = tbuf; tn < cn; ) {
        if ((n = read(fd, tbp, cn - tn)) <= 0)
            return(-1);

        tn += n, tbp += n;

        if ((tn & 0x01) == 0) /* even */
            break;
    }
    for (n = cn = 0; cn < tn; cn += 2, n++) {
        hb = tbuf[cn] - '0', lb = tbuf[cn + 1] - '0';
        buf[n] = (hex2bin[hb] << 4) | hex2bin[lb];
    }
    return(n);
}

at_read(int fd, BYTE *buf, int size)	/* ascii, timer			*/
{
    int  n, tn, cn;
    BYTE tbuf[MAX_NODE_PSIZE*2], *tbp, hb, lb;

    struct timeval tv;
    fd_set rdset;

    D21("[__%s__]\n", __FUNCTION__);
    if (size > MAX_NODE_PSIZE)
        cn = MAX_NODE_PSIZE*2;
    else
        cn = size * 2;

    for (tn = 0, tbp = tbuf; tn < cn; ) {
        if (Srvr_iot_msec > 0) {
            FD_ZERO(&rdset); FD_SET(fd, &rdset);
            tv.tv_sec  = Srvr_iot_msec/1000;
            tv.tv_usec = (Srvr_iot_msec - tv.tv_sec*1000)*1000;

            n = select(fd +1, &rdset, NULL, NULL, &tv);
            if (n <= 0) {
                L01("at_read() : timeout for %d.\n", fd);
                return(-1);  /* timeout ...... */
            }
        }

        if ((n = read(fd, tbp, cn - tn)) <= 0)
            return(-1);

        tn += n, tbp += n;

        if ((tn & 0x01) == 0) /* even */
            break;
    }

    for (n = cn = 0; cn < tn; cn += 2, n++) {
        hb = tbuf[cn] - '0', lb = tbuf[cn + 1] - '0';
        buf[n] = (hex2bin[hb] << 4) | hex2bin[lb];
    }
    return(n);
}

bb_write(int fd, BYTE *buf, int size) 	/* binary, block 		*/
{ 
    ND_D21("[__%s__]\n", __FUNCTION__);
    ND_D22("<write>(%2d,%2d)", fd, size);
    ND_MDH(buf, (size<PRINT_SIZE)?size:PRINT_SIZE);
    return(write(fd, buf, size));
}

bt_write(int fd, BYTE *buf, int size) 	/* binary, timeout, block	*/
{ 
    struct timeval tv;
    int    rsiz, n;
    fd_set wrset;

    SV_D21("[__%s__]\n", __FUNCTION__);
    if (Srvr_iot_msec > 0) {
        for (rsiz = size; rsiz; rsiz -= n, buf += n) {
            FD_ZERO(&wrset); FD_SET(fd, &wrset);
            tv.tv_sec  = Srvr_iot_msec/1000;
            tv.tv_usec = (Srvr_iot_msec - tv.tv_sec*1000)*1000;

            n=select(fd +1, NULL, &wrset, NULL, &tv);

            if (n <= 0) {
                L01("bt_write() : timeout for %d.\n", fd);
                return(-1);  /* timeout ...... */
            }

            SV_D22("<write>(%2d,%2d)", fd, rsiz);
            SV_MDH(buf, (rsiz<PRINT_SIZE)?rsiz:PRINT_SIZE);
            if ((n = write(fd, buf, rsiz)) <= 0)
                return(-1);
        }
        return(size);
    }
    else 
        return(write(fd, buf, size));
}

ab_write(int fd, BYTE *buf, int size)	/* ascii, block			*/
{
    int i, tn, ln, n;
    BYTE tbuf[MAX_NODE_PSIZE], hb, lb;

    D21("[__%s__]\n", __FUNCTION__);
    for (i = tn = 0; i < size; i++) {
        hb = buf[i] >> 4, lb = buf[i] & 0x0f;
        tbuf[tn] = bin2hex[hb], tbuf[tn+1] = bin2hex[lb];
        tn += 2;
        if (tn >= MAX_NODE_PSIZE) {
            if (write(fd, tbuf, tn) != tn) 
                return(-1);
            tn = 0;     
        }
    }
    if (tn > 0 && write(fd, tbuf, tn) != tn)
        return(-1);

    return(size);
}

ascii_dump(BYTE *buf, int size)
{
    int i;

    for (i = 0; i < size; i++) {
        if (buf [i] < ' ' || buf[i] & 0x80) 
            D20(".");
        else 
            D21("%c", buf[i]);
    }
    D20("\n");
}

hexa_dump(BYTE *buf, int size)
{
int i, hb, lb;

D20("\n");
for (i = 0; i < size; i++) {
hb = buf[i] >> 4; lb= buf[i] & 0x0f;
D22("%c%c ", bin2hex[hb], bin2hex[lb]);
if (((i+1) % 16) == 0)
D20("\n");
}
D20("\n");
}

hexa_dump_1(BYTE *buf, int size)
{
    int i, hb, lb;

    D10("\n");
    for (i = 0; i < size; i++) {
        hb = buf[i] >> 4; lb= buf[i] & 0x0f;
        D12("%c%c ", bin2hex[hb], bin2hex[lb]);
        if (((i+1) % 16) == 0)
            D10("\n");
    }
    D10("\n");
}

