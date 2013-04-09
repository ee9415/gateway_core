 /*----------------------------------------*
                   c_util.h
              coded by H.B. Lee
                   Feb/2007
	     Revisioned May/2009
 *-----------------------------------------*/

int    set_n2_esc_p(unsigned int num, BYTE *buf, int len); /* local int to esc'd pdu */
int    set_n2_bin_p(unsigned int num, BYTE *buf, int len); /* local int to binry pdu */
int    set_b2_esc_p(BYTE *str, BYTE *buf, int len);        /* local buf to esc'd pdu */
unsigned int   get_bin_p2_n(BYTE *buf);		  /* pdu to local int	    */
void   set_system_time(long int sec);
char  *get_pdu_name(BYTE pdu_code);
