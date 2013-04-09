 /*----------------------------------------*
                g_gtway.h
            coded by H.B. Lee
                Feb/2007
	    Revisioned May/2009
 *-----------------------------------------*
 *       routines for server interface     *
 *-----------------------------------------*/


extern int  Node_conn_cnt,		/* activity count whith node    */
	    Srvr_conn_cnt,		/* activity count whith server  */
	    Wdog_node_evt,		/* cheked stat of node by wdog  */
	    Wdog_srvr_evt,		/* cheked stat of srvr by wdog  */

	    Cdsk_pppd_err,		/* # of not ready pppd for cdsk */
	    Cdsk_sock_err,		/* # of socket()  error in cdsk */
	    Cdsk_conn_err,		/* # of connect() error in cdks */
	    Cdsk_read_err,		/* # of read()    error in cdsk */
	    Cdsk_wrte_err;		/* # of write()   error in cdsk */

//used by Pppd_ctrl_req; 		/* request code for pppd admin  */
#define	PPPD_CTRL_BTDON		1	/* boot & deamnon on          	*/
#define	PPPD_CTRL_DMNON		2	/* deamon on                  	*/
#define	PPPD_CTRL_DMNOFF 	3	/* deamon only off              */
#define	PPPD_CTRL_PWROFF 	4	/* deamon & power off           */
#define	PPPD_CTRL_COOL		5	/* deamon & power off & cooling */

// caused by do_srvr_interface()
#define	PPPD_WRUP_NORM		1	/* normal termination		*/
#define	PPPD_WRUP_OPEN		2	/* socket()  error		*/
#define	PPPD_WRUP_CONN		3	/* connect() error		*/
#define	PPPD_WRUP_DATA		4	/* comm.() error		*/

extern pthread_mutex_t     Mutx_timer;
extern pthread_cond_t      Cond_timer;
extern pthread_condattr_t  Attr_timer;

extern int  (*srvr_ubp_read)(UBUF *ubp),
            (*srvr_ubp_write)(UBUF *ubp, BYTE *buf, int size);
extern int  Srvr_iot_msec;

void   wakeup_timer(), wakeup_node(), wakeup_srvr(), wakeup_pppd();

int    cdsk_ubp_read(UBUF *ubp), 
       cdsk_ubp_write(UBUF *ubp, BYTE *buf, int size),
       cdma_ubp_read(UBUF *ubp), 
       cdma_ubp_write(UBUF *ubp, BYTE *buf, int size);


void  *do_node_listen(void *arg),
      *do_cdma_listen(void *arg),
      *do_cdsk_listen(void *arg),
      *do_pppd_control(void *arg),
      *do_stwc_listen(void *arg),
      *do_gtwy_hw_wdog(void *arg),
      *do_gtwy_timer(void *arg),
      *do_wdog_timer(void *arg),
      *do_gtwy_stat(void *arg);
      
