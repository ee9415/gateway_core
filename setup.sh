#
# setup script(setup.sh)
#

DBG_MSG_LEVEL=1
SV_DBG_MSG_LEVEL=1
ND_DBG_MSG_LEVEL=1
LOG_MSG_LEVEL=1
LOG_FILE_NAME=/tmp/mb3/gtwy.log
#LOG_FILE_NAME=/gtwy.log

MY_GATEWAY_ID=231

NODE_UART_USE=4      #  
NODE_UART_DEV=/dev/ttyS2
NODE_SFMI_SEC=10     # suferframe interval
NODE_IGTR_SEC=3      # image transmission time
NODE_IOT_MSEC=12     # node uart i/o timeout 

CDMA_UART_USE=0
CDMA_UART_DEV=/dev/ttyS0 
CDMA_ATD_NRTY=3   
CDMA_ATD_NSEC=3      # max waiting time for ATD command
CDMA_RST_NRTY=3      # max number of retry for CDMA reset
CDMA_RST_NSEC=3      # max waiting time for CDMA reset
CDMA_TCPC_UID=sktelecom 
CDMA_PACK_UID=1501        

CDMA_SRV_ADDR=203.255.217.204
CDMA_SRV_PORT=2879
CDMA_IOT_MSEC=10000   # cdma i/o  timeout

CDSK_PPPD_USE=0
CDSK_PPPD_DEV=/dev/usb_power  
CDSK_PPPD_TRT=3      # max comm. time(sec) with servr
CDSK_PPPD_PWT=20     # ppp device(HDDPA) pwrup time(sec
CDSK_PPPD_DMT=10     # ppp deamon bootup time(sec)
CDSK_PPPD_CLT=60     # ppp device cooling time(sec) 

CDSK_SRV_ADDR=203.255.217.200
CDSK_SRV_PORT=2874
CDSK_IOT_MSEC=2000   # cdsk i/o  timeout

SRVR_CONN_FRQ=3     # sever conn frq. based on sfmi
SRVR_GWST_FRQ=6      # gateway send status info. frq.
SRVR_GWTM_FRQ=9    # gateway time request frq. careful in PC !!!

GTWY_STAT_FRQ=3      # staticstics put frq.

GTWY_STWC_USE=0
GTWY_STWC_DEV=/dev/gpio

GTWY_SWDG_NDF=90     # node failure allowance
GTWY_SWDG_SVF=30     # srvr failure allowance

GTWY_HWDG_USE=0
GTWY_HWDG_SEC=20
GTWY_HWDG_DEV=/dev/misc/S3C2440_watchdog

TZ=GMT-09:00:00

GATEWAY_IPADDR=203.255.217.223
GATEWAY_GWADDR=203.255.217.254
ETHX=eth0
ESSID='gems_01_lab'
MODEM_DEV=telit

export MODEM_DEV
export GATEWAY_IPADDR
export GATEWAY_GWADDR
export ETHX
export ESSID

export DBG_MSG_LEVEL
export SV_DBG_MSG_LEVEL
export ND_DBG_MSG_LEVEL
export LOG_MSG_LEVEL
export LOG_FILE_NAME

export MY_GATEWAY_ID

export NODE_UART_USE
export NODE_UART_DEV
export NODE_SFMI_SEC
export NODE_IGTR_SEC
export NODE_IOT_MSEC

export CDMA_UART_USE
export CDMA_UART_DEV
export CDMA_ATD_NRTY
export CDMA_ATD_NSEC
export CDMA_RST_NRTY
export CDMA_RST_NSEC
export CDMA_TCPC_UID
export CDMA_PACK_UID
export CDMA_SRV_ADDR
export CDMA_SRV_PORT
export CDMA_IOT_MSEC

export CDSK_PPPD_USE
export CDSK_PPPD_DEV
export CDSK_SRV_ADDR
export CDSK_SRV_PORT
export CDSK_IOT_MSEC

export SRVR_CONN_FRQ
export SRVR_GWST_FRQ
export SRVR_GWTM_FRQ

export GTWY_STAT_FRQ

export GTWY_STWC_USE
export GTWY_STWC_DEV

export GTWY_SWDG_NDF
export GTWY_SWDG_SVF

export GTWY_HWDG_USE
export GTWY_HWDG_SEC
export GTWY_HWDG_DEV
export TZ=GMT-09:00:00
stty -F $NODE_UART_DEV speed 460800
#stty -F $NODE_UART_DEV speed 115200
