/*-----------------------------------------*
               g_config.h
           coded by H.B. Lee
                Feb/2007
           Revisoned May/2009
 *-----------------------------------------*/

#define DBG_MSG_LEVEL	2
#define SV_DBG_MSG_LEVEL    2
#define ND_DBG_MSG_LEVEL    2
#define LOG_MSG_LEVEL	2
#define LOG_FILE_NAME   "/tmp/mb3/gtwy.log"
#define LOG_FILE_NAM2   "/tmp/mb3/Eboot.log"
#define EXECUTE_FILE    "/tmp/mb3/Egtwy"

#define MY_GATEWAY_ID   0   /* gateway id */

#define NODE_UART_USE   4   /* 400K bps */
#define NODE_UART_DEV   "/dev/ttyS?" /* UART device for sink node  */
#define NODE_SFMI_SEC	10  /* superframe interval                 */
#define NODE_IGTR_SEC	3   /* image transmission time             */
#define NODE_IOT_MSEC   10  /* maximum read waiting time from node */

#define CDMA_UART_USE   0   /* whether if use CDMA modem	   */
#define CDMA_UART_DEV   "/dev/ttyS?" /* UART device for CDMA modem */
#define CDMA_ATD_NRTY   3   /* max number of retry for ATD command */
#define CDMA_ATD_NSEC   3   /* max waiting time for ATD command    */
#define CDMA_RST_NRTY   3   /* max number of retry for CDMA reset  */
#define CDMA_RST_NSEC   3   /* max waiting time for CDMA reset     */
#define CDMA_TCPC_UID   "sktelecom" /* TCP/IP uid for CDMA-1X svc  */
#define CDMA_PACK_UID   1501        /* Pcket  uid for CDMA-1X svc  */
#define CDMA_SRV_ADDR   "127.0.0.1" /* TCP/IP addr through CDMA-1X */
#define CDMA_SRV_PORT   2874        /* TCP/IP port through CDMA-1X */
#define CDMA_IOT_MSEC   3000 /* max waiting i/o for read from CDMA*/

#define CDSK_PPPD_USE   1                 /* socket with PPP(HSDPA)*/
#define CDSK_PPPD_DEV   "/dev/usb_power"  /* PPP(HSDPA)	device	   */
#define CDSK_PPPD_TRT   3 	/* max comm. time(sec) with servr  */
#define CDSK_PPPD_PWT   20	/* ppp device(HSDPA) pwrup time(sec*/
#define CDSK_PPPD_DMT   10	/* ppp deamon bootup time(sec)     */
#define CDSK_PPPD_CLT   60	/* ppp device cooling time(sec)    */

#define CDSK_SRV_ADDR   "127.0.0.1" /* TCP/IP addr for srvr socket */
#define CDSK_SRV_PORT   8888        /* TCP/IP port for srvr socket */
#define CDSK_IOT_MSEC   20000 /* maximum i/o waiting time from node*/

#define SRVR_CONN_FRQ   4   /* sever conn frq. based on super frame*/
#define	SRVR_GWST_FRQ	10  /* send gateway status info. frq.      */
#define	SRVR_GWTM_FRQ	10  /* server time request frq.		   */

#define GTWY_STAT_FRQ   30  /* staticstics put interval            */

#define GTWY_STWC_USE	0
#define GTWY_STWC_DEV   "/dev/gpio"

#define GTWY_SWDG_NDF	50  /* node failure allowance */
#define GTWY_SWDG_SVF	20  /* srvr failure allowance */

#define GTWY_HWDG_USE	1
#define GTWY_HWDG_SEC	25
#define GTWY_HWDG_DEV   "/dev/misc/S3C2440_watchdog"


//////////////////////////////////////////////
// Queue
#define UP_UGNT_DAT_Q   500  
#define UP_NORM_CMD_Q   500  
#define UP_UGNT_IMG_Q   50  
#define UP_NORM_IMG_Q   50
#define UP_UGNT_BDT_Q   10  
#define UP_NORM_BDT_Q   10
#define UP_NORM_DAT_Q   500
#define UP_TNOR_DAT_Q   500
#define UP_TNOR_IG1_Q   50
#define DN_UGNT_CMD_Q   30  
#define DN_NORM_CMD_Q   30  
#define DN_UNKN_CMD_Q   30  

//////////////////////////////////////////////
// retry
#define UP_UGNT_DAT_R   3
#define UP_NORM_CMD_R   3 
#define UP_UGNT_IMG_R   3
#define UP_NORM_IMG_R   3 
#define UP_UGNT_BDT_R   3
#define UP_NORM_BDT_R   3 
#define UP_NORM_DAT_R   3 
#define UP_TNOR_DAT_R   3
#define UP_TNOR_IG1_R   3
#define DN_NORM_FMW_R   3
#define DN_UGNT_CMD_R   3 
#define DN_NORM_CMD_R   3 
#define DN_UNKN_CMD_R   3 

/*---------------------------------------------------------------------*/
#define MAX_NODE_PSIZE  256 /* Packet Size */
#define MAX_FIRM_PSIZE  4096
#define MAX_REAL_FSIZE  200  /* Frame Size, real message without esc seq */
#define MAX_CDMA_PSIZE  512 /* Packet Size */
#define MAX_UBUF_NUMBR  8

#define MAX_BINRY_SIZE	(100*1024)
#define MAX_FIRMW_SIZE  (100*1024)


extern	 int	 Dbg_msg_level;
extern   int     SV_Dbg_msg_level;
extern   int     ND_Dbg_msg_level;
extern	 int	 Log_msg_level;
extern	 char	*Log_file_name;
extern	 char	*Log_file_nam2;
extern   char   *Execute_file;
extern	 FILE	*Log_dev;   /* fp of Log_file_name */

extern   int     My_gateway_id;

extern   int     Node_uart_use;
extern   char   *Node_uart_dev;
extern   int     Node_sfmi_sec;
extern   int     Node_igtr_sec;
extern   int     Node_iot_msec;

extern   int     Cdma_uart_use;
extern   char   *Cdma_uart_dev;
extern   int     Cdma_atd_nrty;
extern   int     Cdma_atd_nsec;
extern   int     Cdma_rst_nrty;
extern   int     Cdma_rst_nsec;
extern   char   *Cdma_tcpc_uid;
extern   int     Cdma_pack_uid;
extern   char   *Cdma_srv_addr;
extern   int     Cdma_srv_port;
extern   int     Cdma_iot_msec;

extern	 int	 Cdsk_pppd_use;
extern	 char   *Cdsk_pppd_dev;
extern	 int	 Cdsk_pppd_trt;
extern	 int	 Cdsk_pppd_pwt;
extern	 int	 Cdsk_pppd_dmt;
extern	 int	 Cdsk_pppd_clt;

extern   char   *Cdsk_srv_addr;
extern   int     Cdsk_srv_port;
extern   int     Cdsk_iot_msec;

extern   int     Srvr_conn_frq;
extern   int     Srvr_gwst_frq; 	
extern   int     Srvr_gwtm_frq; 

extern   int     Gtwy_stat_frq;

extern   int     Gtwy_stwc_use;
extern	 char	*Gtwy_stwc_dev;

extern   int     Gtwy_swdg_ndf;
extern   int     Gtwy_swdg_svf; 

extern   int     Gtwy_hwdg_use;
extern   int     Gtwy_hwdg_sec;
extern	 char	*Gtwy_hwdg_dev;

extern   char   *Stop_watc_dev;
extern   char   *Watch_dog_dev;

extern   int     Up_ugnt_dat_q; 
extern   int     Up_norm_cmd_q; 
extern   int     Up_norm_dat_q; 
extern   int     Up_ugnt_img_q; 
extern   int     Up_norm_img_q; 
extern   int     Up_ugnt_bdt_q; 
extern   int     Up_norm_bdt_q; 
extern   int     Up_tnor_dat_q; 
extern   int     Up_tnor_ig1_q; 
extern   int     Dn_norm_fmw_q;
extern   int     Dn_ugnt_cmd_q; 
extern   int     Dn_norm_cmd_q; 
extern   int     Dn_unkn_cmd_q;

extern   int     Up_ugnt_dat_r;
extern   int     Up_norm_cmd_r;
extern   int     Up_norm_dat_r;
extern   int     Up_ugnt_img_r;
extern   int     Up_norm_img_r;
extern   int     Up_ugnt_bdt_r;
extern   int     Up_norm_bdt_r;
extern   int     Up_tnor_dat_r;
extern   int     Up_tnor_ig1_r;
extern   int     Dn_norm_fmw_r;
extern   int     Dn_ugnt_cmd_r;
extern   int     Dn_norm_cmd_r;
extern   int     Dn_unkn_cmd_r;
