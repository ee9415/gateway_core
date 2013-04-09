/*-----------------------------------------*
               g_config.c
            coded by H.B. Lee
                Feb/2007
             Revisoned May/2009
 *-----------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_pack.h"

/*--------------Debugging level -----------*/
int   Dbg_msg_level = DBG_MSG_LEVEL;
int   SV_Dbg_msg_level = SV_DBG_MSG_LEVEL;
int   ND_Dbg_msg_level = ND_DBG_MSG_LEVEL;
int   Log_msg_level = LOG_MSG_LEVEL;
char *Log_file_name = LOG_FILE_NAME;
char *Log_file_nam2 = LOG_FILE_NAM2;
char *Execute_file  = EXECUTE_FILE;
FILE *Log_dev = NULL; /*fp of Log_file_name*/

/*******************************************/
/******* initialization paramaters  ********/
/*******************************************/
int   My_gateway_id = MY_GATEWAY_ID;

int   Node_uart_use = NODE_UART_USE;
char *Node_uart_dev = NODE_UART_DEV;
int   Node_sfmi_sec = NODE_SFMI_SEC;
int   Node_igtr_sec = NODE_IGTR_SEC;
int   Node_iot_msec = NODE_IOT_MSEC;

int   Cdma_uart_use = CDMA_UART_USE;
char *Cdma_uart_dev = CDMA_UART_DEV;
int   Cdma_atd_nrty = CDMA_ATD_NRTY;
int   Cdma_atd_nsec = CDMA_ATD_NSEC;
int   Cdma_rst_nrty = CDMA_RST_NRTY;
int   Cdma_rst_nsec = CDMA_RST_NSEC;
char *Cdma_tcpc_uid = CDMA_TCPC_UID;
int   Cdma_pack_uid = CDMA_PACK_UID;
char *Cdma_srv_addr = CDMA_SRV_ADDR; 
int   Cdma_srv_port = CDMA_SRV_PORT;
int   Cdma_iot_msec = CDMA_IOT_MSEC;

int   Cdsk_pppd_use = CDSK_PPPD_USE;
char *Cdsk_pppd_dev = CDSK_PPPD_DEV;
int   Cdsk_pppd_trt = CDSK_PPPD_TRT;
int   Cdsk_pppd_pwt = CDSK_PPPD_PWT;
int   Cdsk_pppd_dmt = CDSK_PPPD_DMT;
int   Cdsk_pppd_clt = CDSK_PPPD_CLT;

char *Cdsk_srv_addr = CDSK_SRV_ADDR;
int   Cdsk_srv_port = CDSK_SRV_PORT;
int   Cdsk_iot_msec = CDSK_IOT_MSEC;

int   Srvr_conn_frq = SRVR_CONN_FRQ;
int   Srvr_gwst_frq = SRVR_GWST_FRQ; 
int   Srvr_gwtm_frq = SRVR_GWTM_FRQ; 

int   Gtwy_stat_frq = GTWY_STAT_FRQ;

int   Gtwy_stwc_use = GTWY_STWC_USE;
char *Gtwy_stwc_dev = GTWY_STWC_DEV;

int   Gtwy_swdg_ndf = GTWY_SWDG_NDF;
int   Gtwy_swdg_svf = GTWY_SWDG_SVF; 

int   Gtwy_hwdg_use = GTWY_HWDG_USE;
int   Gtwy_hwdg_sec = GTWY_HWDG_SEC;
char *Gtwy_hwdg_dev = GTWY_HWDG_DEV;


//////////////////////////////////////////////
// queue
int   Up_ugnt_dat_q = UP_UGNT_DAT_Q;
int   Up_norm_cmd_q = UP_NORM_CMD_Q;
int   Up_norm_dat_q = UP_NORM_DAT_Q;
int   Up_ugnt_img_q = UP_UGNT_IMG_Q;
int   Up_norm_img_q = UP_NORM_IMG_Q;
int   Up_ugnt_bdt_q = UP_UGNT_BDT_Q;
int   Up_norm_bdt_q = UP_NORM_BDT_Q;
int   Up_tnor_dat_q = UP_TNOR_DAT_Q;
int   Up_tnor_ig1_q = UP_TNOR_IG1_Q;

int   Dn_ugnt_cmd_q = DN_UGNT_CMD_Q;
int   Dn_norm_cmd_q = DN_NORM_CMD_Q;
int   Dn_unkn_cmd_q = DN_UNKN_CMD_Q;
//////////////////////////////////////////////
// retry
int   Up_ugnt_dat_r = UP_UGNT_DAT_R;
int   Up_norm_cmd_r = UP_NORM_CMD_R;
int   Up_norm_dat_r = UP_NORM_DAT_R;
int   Up_ugnt_img_r = UP_UGNT_IMG_R;
int   Up_norm_img_r = UP_NORM_IMG_R;
int   Up_ugnt_bdt_r = UP_UGNT_BDT_R;
int   Up_norm_bdt_r = UP_NORM_BDT_R;
int   Up_tnor_dat_r = UP_TNOR_DAT_R;
int   Up_tnor_ig1_r = UP_TNOR_IG1_R;

int   Dn_norm_fmw_r = DN_NORM_FMW_R;
int   Dn_ugnt_cmd_r = DN_UGNT_CMD_R;
int   Dn_norm_cmd_r = DN_NORM_CMD_R;
int   Dn_unkn_cmd_r = DN_UNKN_CMD_R;

typedef struct  param {
    int flag;
#define ENV_INT 0x01
#define ENV_STR 0x02
    void    *valp;
    char    *varp;
} PARAM;

PARAM   Param[] = {
    { ENV_INT,  &Dbg_msg_level,   "DBG_MSG_LEVEL" },
    { ENV_INT,  &SV_Dbg_msg_level,   "SV_DBG_MSG_LEVEL" },
    { ENV_INT,  &ND_Dbg_msg_level,   "ND_DBG_MSG_LEVEL" },
    { ENV_INT,  &Log_msg_level,   "LOG_MSG_LEVEL" },
    { ENV_STR,  &Log_file_name,   "LOG_FILE_NAME" },
    { ENV_STR,  &Log_file_nam2,   "LOG_FILE_NAM2" },
    { ENV_STR,  &Execute_file,    "EXECUTE_FILE" },
    {   0,  NULL,	      ""	      },

    { ENV_INT,  &My_gateway_id,   "MY_GATEWAY_ID" },
    {   0,  NULL,	      ""	      },

    { ENV_INT,  &Node_uart_use,   "NODE_UART_USE" },
    { ENV_STR,  &Node_uart_dev,   "NODE_UART_DEV" },
    { ENV_INT,  &Node_sfmi_sec,   "NODE_SFMI_SEC" },
    { ENV_INT,  &Node_igtr_sec,   "NODE_IGTR_SEC" },
    { ENV_INT,  &Node_iot_msec,   "NODE_IOT_MSEC" },
    {   0,  NULL,         ""	      },

    { ENV_INT,  &Cdma_uart_use,   "CDMA_UART_USE" },
    { ENV_STR,  &Cdma_uart_dev,   "CDMA_UART_DEV" },
    { ENV_INT,  &Cdma_atd_nrty,   "CDMA_ATD_NRTY" },
    { ENV_INT,  &Cdma_atd_nsec,   "CDMA_ATD_NSEC" },
    { ENV_INT,  &Cdma_rst_nrty,   "CDMA_RST_NRTY" },
    { ENV_INT,  &Cdma_rst_nsec,   "CDMA_RST_NSEC" },
    { ENV_STR,  &Cdma_tcpc_uid,   "CDMA_TCPC_UID" },
    { ENV_INT,  &Cdma_pack_uid,   "CDMA_PACK_UID" },
    { ENV_STR,  &Cdma_srv_addr,   "CDMA_SRV_ADDR" }, 
    { ENV_INT,  &Cdma_srv_port,   "CDMA_SRV_PORT" },
    { ENV_INT,  &Cdma_iot_msec,   "CDMA_IOT_MSEC" },
    {   0,  NULL,         ""	      },

    { ENV_INT,  &Cdsk_pppd_use,   "CDSK_PPPD_USE" },
    { ENV_STR,  &Cdsk_pppd_dev,   "CDSK_PPPD_DEV" },
    { ENV_INT,  &Cdsk_pppd_trt,   "CDSK_PPPD_TRT" },
    { ENV_INT,  &Cdsk_pppd_pwt,   "CDSK_PPPD_PWT" },
    { ENV_INT,  &Cdsk_pppd_dmt,   "CDSK_PPPD_DMT" },
    { ENV_INT,  &Cdsk_pppd_clt,   "CDSK_PPPD_CLT" },
    {   0,  NULL,         ""	      },

    { ENV_STR,  &Cdsk_srv_addr,   "CDSK_SRV_ADDR" }, 
    { ENV_INT,  &Cdsk_srv_port,   "CDSK_SRV_PORT" },
    { ENV_INT,  &Cdsk_iot_msec,   "CDSK_IOT_MSEC" },

    { ENV_INT,  &Srvr_conn_frq,   "SRVR_CONN_FRQ" },
    { ENV_INT,  &Srvr_gwst_frq,   "SRVR_GWST_FRQ" },
    { ENV_INT,  &Srvr_gwtm_frq,   "SRVR_GWTM_FRQ" },
    {   0,  NULL,         ""	      },

    { ENV_INT,  &Gtwy_stat_frq,   "GTWY_STAT_FRQ" },
    {   0,  NULL,         ""	      },

    { ENV_INT,  &Gtwy_stwc_use,   "GTWY_STWC_USE" },
    { ENV_STR,  &Gtwy_stwc_dev,   "GTWY_STWC_DEV" },
    {   0,  NULL,         ""	      },

    { ENV_INT,  &Gtwy_swdg_ndf,   "GTWY_SWDG_NDF" },
    { ENV_INT,  &Gtwy_swdg_svf,   "GTWY_SWDG_SVF" },

    { ENV_INT,  &Gtwy_hwdg_use,   "GTWY_HWDG_USE" },
    { ENV_INT,  &Gtwy_hwdg_sec,   "GTWY_HWDG_SEC" },
    { ENV_STR,  &Gtwy_hwdg_dev,   "GTWY_HWDG_DEV" },

    {   0,  NULL,         ""	      },

    { ENV_INT,  &Up_ugnt_img_q,   "UP_UGNT_IMG_Q" }, 
    { ENV_INT,  &Up_ugnt_bdt_q,   "UP_UGNT_BDT_Q" }, 
    { ENV_INT,  &Up_ugnt_dat_q,   "UP_UGNT_DAT_Q" }, 
    { ENV_INT,  &Up_norm_cmd_q,   "UP_NORM_CMD_Q" }, 
    { ENV_INT,  &Up_norm_img_q,   "UP_NORM_IMG_Q" }, 
    { ENV_INT,  &Up_norm_bdt_q,   "UP_NORM_BDT_Q" }, 
    { ENV_INT,  &Up_norm_dat_q,   "UP_NORM_DAT_Q" },
    { ENV_INT,  &Up_tnor_dat_q,   "UP_TNOR_DAT_Q" },
    { ENV_INT,  &Up_tnor_ig1_q,   "UP_TNOR_IG1_Q" },
    { ENV_INT,  &Dn_ugnt_cmd_q,   "DN_UGNT_CMD_Q" }, 
    { ENV_INT,  &Dn_norm_cmd_q,   "DN_NORM_CMD_Q" }, 
    { ENV_INT,  &Dn_unkn_cmd_q,   "DN_UNKN_CMD_Q" }, 
    {   0,  NULL,         ""	      },

    { ENV_INT,  &Up_ugnt_img_r,   "UP_UGNT_IMG_R" },
    { ENV_INT,  &Up_ugnt_bdt_r,   "UP_UGNT_BDT_R" },
    { ENV_INT,  &Up_ugnt_dat_r,   "UP_UGNT_DAT_R" },
    { ENV_INT,  &Up_norm_cmd_r,   "UP_NORM_CMD_R" },
    { ENV_INT,  &Up_norm_img_r,   "UP_NORM_IMG_R" },
    { ENV_INT,  &Up_norm_bdt_r,   "UP_NORM_BDT_R" },
    { ENV_INT,  &Up_norm_dat_r,   "UP_NORM_DAT_R" },
    { ENV_INT,  &Up_tnor_dat_r,   "UP_TNOR_DAT_R" },
    { ENV_INT,  &Up_tnor_ig1_r,   "UP_TNOR_IG1_R" },
    { ENV_INT,  &Dn_ugnt_cmd_r,   "DN_UGNT_CMD_R" },
    { ENV_INT,  &Dn_norm_cmd_r,   "DN_NORM_CMD_R" },
    { ENV_INT,  &Dn_unkn_cmd_r,   "DN_UNKN_CMD_R" },
    {   0,  NULL,         ""	      },

    {	0,  NULL,	  NULL	      }
};

par_init(int hlp) 
{
    int     i, nl;
    char    *vp;
    PARAM   *pp;
    SUPKC   *pk;
   
    env_init();

    for (pp = Param, nl = 0; pp->varp; pp++, nl++) {
        if (*pp->varp == 0) {

            if (!hlp) continue;

            if (nl > 5) nl = 5;

            for (i = 0; i < nl; i++) D20("."); FFLUSH(stdout); D20("\r");

            for (i = 0; i < nl; i++) { D20(" "); FFLUSH(stdout); }

            D20("\r");

            for (i = 0; i < nl; i++) printf("."); printf("\n");

            nl = 0;
            continue;
        }

        if (vp = getenv(pp->varp)) {
            if (pp->flag == ENV_INT)
                *((int *)pp->valp) = atoi(vp);
            else {
                *(char **)(pp->valp) = (char *)malloc(strlen(vp) + 1);
                strcpy(*((char **)pp->valp), vp);
            }
        }

        if (!hlp) continue;

        if (pp->flag == ENV_INT)
            printf("%s=%d\n", pp->varp, *((int **)pp->valp));
        else
            printf("%s=%s\n", pp->varp, *((char **)pp->valp));
    }

    if (hlp)
        exit(0);
}

env_init()
{
    FILE *fp;
    char buf[128], *tp, *ep;

    if (!(fp = fopen("setup.sh", "r")))
	    return;

    while(fgets(buf, sizeof(buf), fp)) {
    	if (buf[0] == '#' || !(tp = strtok(buf, " \t\n")) || !strchr(tp, '='))
    	    continue;
    	ep = malloc(strlen(tp) +1);
    	strcpy(ep, tp);
    	putenv(ep);
    }
    fclose(fp);
}
