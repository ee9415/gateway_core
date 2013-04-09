 /*----------------------------------------*
                   g_util.c
              coded by H.B. Lee
                   May/2009
 *-----------------------------------------*/

#include <stdio.h>
#include <sys/time.h>

#include "g_macros.h"
#include "g_pack.h"
#include "g_util.h"
#include "g_config.h"


/*-------------------------------------------------*
    convert local integer to escaped pdu format
 *-------------------------------------------------*/
int set_n2_esc_p(unsigned int num, BYTE *buf, int len)
{
    int	    i, li;
    BYTE    lc;

    for (i = li = 0; i < len; i++, num >>= 8) {
	lc = num & 0xff; 
        if (lc == S_CHR1 || lc == C_CHR) {
            buf[li++] = C_CHR;
            buf[li++] = lc ^ C_MASK;
        }
        else
            buf[li++] = lc;
    }            
    return(li);
}

/*-------------------------------------------------*
    convert local integer to binary pdu format
 *-------------------------------------------------*/
int set_n2_bin_p(unsigned int num, BYTE *buf, int len)
{
    int	    i;

    for (i = 0; i < len; i++, num >>= 8)
        buf[i] = num & 0xff;

    return(i);
}

/*-------------------------------------------------*
    convert local byte stream to escaped pdu format
 *-------------------------------------------------*/
int set_b2_esc_p(BYTE *str, BYTE *buf, int len)
{
    int	    i, li;
    BYTE    lc;

    for (i = li = 0; i < len; i++) {
	lc = str[i]; 
        if (lc == S_CHR1 || lc == C_CHR) {
            buf[li++] = C_CHR;
            buf[li++] = lc ^ C_MASK;
        }
        else
            buf[li++] = lc;
    }            
    return(li);
}

/*-------------------------------------------------*
        convert normal pdu format to integer 
 *-------------------------------------------------*/
unsigned int get_bin_p2_n(BYTE *buf)
{
    unsigned int     num;
    int     	     i;

    for (i = 2, num = buf[3]; i >= 0; i--) {
        num = (num << 8) | (buf)[i];
        //D22(" num = %d(%d)\n", num, i);
    }

    return(num);
}

void set_system_time(long int sec)
{
    struct timeval  tv = { 0, 0 };

    if (sec > 0) {
        tv.tv_sec = sec;
        settimeofday(&tv, NULL );
        D21("TIME Set OK...[%d]\n", sec);
    }
    else {
        D21("TIME Set Error...[%d]\n", sec);
    }
}



char *get_pdu_name(BYTE pdu_code)
{
    static char  Undefined_pdu[] = "??_????_???";
    static char *pdu_name[] = { 
     "UP_NORM_CMD", "DN_NORM_CMD", "UP_UGNT_DTA", "UP_NORM_DTA", /* 0x00-0x03 */
     "UP_UGNT_IG1", "UP_NORM_IG1", "UP_UGNT_IG2", "UP_NORM_IG2", /* 0x04-0x07 */
     Undefined_pdu, Undefined_pdu, "UP_NORM_BD1", "UP_UGNT_BD1", /* 0x08-0x0b */
     "DN_UNKN_CMD", Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x0c-0x0f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x10-0x13 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x14-0x17 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x18-0x1b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x1c-0x1f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x20-0x23 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x24-0x27 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x28-0x2b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x2c-0x2f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, "UP_TNOR_DTA", /* 0x30-0x33 */
     Undefined_pdu, "UP_TNOR_IG1", Undefined_pdu, Undefined_pdu, /* 0x34-0x37 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x38-0x3b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x3c-0x3f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x40-0x43 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x44-0x47 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x48-0x4b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x4c-0x4f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x50-0x53 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x54-0x57 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x58-0x5b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x5c-0x5f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x60-0x63 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x64-0x67 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x68-0x6b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x6c-0x6f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x70-0x73 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x74-0x77 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x78-0x7b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x7c-0x7f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x80-0x83 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x84-0x87 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x88-0x8b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x8c-0x8f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x90-0x93 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x94-0x97 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x98-0x9b */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0x9c-0x9f */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xa0-0xa3 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xa4-0xa7 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xa8-0xab */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xac-0xaf */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xb0-0xb3 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xb4-0xb7 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xb8-0xbb */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xbc-0xbf */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xc0-0xc3 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xc4-0xc7 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xc8-0xcb */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xcc-0xcf */

     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xd0-0xd3 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xd4-0xd7 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xd8-0xdb */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xdc-0xdf */

     "DN_GTWY_FMW", Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xe0-0xe3 */
     Undefined_pdu, Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xe4-0xe7 */
     "END_OF_OTA1", "END_OF_OTA2", "REQ_TO_SOTA", "RES_TO_SOTA", /* 0xe8-0xeb */
     "NCK_TO_SNDF", "ACK_TO_SNDF", "DN_NORM_FMW", Undefined_pdu, /* 0xec-0xef */

     Undefined_pdu, "GTWAY_START", "REQ_TO_RCV1", "REQ_TO_STAT", /* 0xf0-0xf3 */
     "DTA_OF_GTWY", Undefined_pdu, Undefined_pdu, Undefined_pdu, /* 0xf4-0xf7 */
     "REQ_TO_CNTD", "END_OF_SND1", Undefined_pdu, Undefined_pdu, /* 0xf8-0xfb */
     "REQ_TO_TIME", "SET_GW_TIME", Undefined_pdu, Undefined_pdu, /* 0xfc-0xff */
    };
    return(pdu_name[pdu_code]);
}
