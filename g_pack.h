 /*-----------------------------------------*
            c_pack.h
        coded by H.B. Lee
            Feb/2007
 *-----------------------------------------*/

/////////////////////////////////////////////
// packet types
/////////////////////////////////////////////
#define UP_NORM_CMD 0x000
#define UP_UGNT_DTA 0x002
#define UP_NORM_DTA 0x003
#define UP_UGNT_IG1 0x004
#define UP_NORM_IG1 0x005
#define UP_UGNT_IG2 0x006 
#define UP_NORM_IG2 0x007
#define UP_NORM_BD1 0x00A   /* big message data			       */
#define UP_UGNT_BD1 0x00B
#define UP_NORM_BD2 0x00C 
#define UP_UGNT_BD2 0x00D
#define UP_TNOR_DTA 0x033   /* Time data			       */
#define UP_TNOR_IG1 0x035   /* Time data			       */ 

#define DN_NORM_CMD 0x001
#define DN_UGNT_CMD 0x008
#define DN_NORM_FMW 0x0ee   /* firmware(ota) data of node 	       */
#define DN_GTWY_FMW 0x0e0   /* firmware(ota) data of gateway	       */

#define DN_UNKN_CMD 0x009

/////////////////////////////////////////////
// node <--> gateway negotiation codes
/////////////////////////////////////////////
#define	GTWAY_START 0x0f1   /* gateway   --> server     */

#define REQ_TO_RCV1 0x0f2   /* sink_node --> gateway    */
#define REQ_TO_RCV2 0x0f2   /* gateway   --> server     */
#define REQ_TO_RCV3 0x0f2   /* gateway   <-- server     */

#define END_OF_SND1 0x0f9   /* sink_node <-- gateway    */
#define END_OF_SND2 0x0f9   /* gateway   --> server     */
#define END_OF_SND3 0x0f9   /* gateway   --> server     */

#define REQ_GW_STAT 0x0f3   /* gateway   <-- server     */

#define DTA_OF_GTWY 0x0f4   /* gateway   --> server     */

#define REQ_TO_TIME 0x0fc   /* gateway   --> server     */
#define SET_GW_TIME 0x0fd   /* gateway   <-- server     */
#define REQ_TO_CNTD 0x0f8   /* gateway   --> server     */

#define REQ_TO_SOTA 0x0ea   /* node <-- gateway, request to start ota  */
#define RES_TO_SOTA 0x0eb   /* node --> gateway, ack     to start ota  */
#define END_OF_OTA1 0x0e8   /* node <-- gateway, end of ota            */
#define END_OF_OTA2 0x0e9   /* node --> gateway, ack to end of ota     */

#define REQ_TO_SNDF 0x0ee   /* node <-- gateway, request to send fdata */
#define ACK_TO_SNDF 0x0ed   /* node --> gateway, ack to REQ_TO_SNDF    */
#define NCK_TO_SNDF 0x0ec   /* node --> gateway, nck to REQ_TO_SNDF    */

#define S_CHR1      0x07e
#define S_CHR2      0x042
#define E_CHR       0x07e
#define C_CHR       0x07d   /* for escape sequence      */
#define C_MASK      0x020

/*-------------- from sink_node -------------*/
typedef struct nodu_cmd_packet {
    BYTE    sch1;
    BYTE    sch2;
    BYTE    deid;
    BYTE    leng[1];
    BYTE    addr[2];
    BYTE    data[1];
} NUPKC; /* node upward command packet */
#define NDEU_DIDOFF_ALL 2 /*offsetof(NUPKC, deid)  * 7e+42+                     =  2 */
#define NDEU_LENOFF_ALL 3 /*offsetof(NUPKC, leng)  * 7e+42+did(1)+              =  3 */
#define NDEU_ADROFF_CMD 4 /*offsetof(NUPKC, addr)  * 7e+42+did(1)+len(1)+       =  4 */
#define NDEU_DTAOFF_CMD 6 /*offsetof(NUPKC, data)  * 7e+42+did(1)+len(1)+adr(2) =  6 */
#define NDEU_HEADLN_CMD NDEU_DTAOFF_CMD           /* head length                =  6 */
#define NDEU_TAILLN_CMD 1                         /* tail length                =  1 */

typedef struct nodu_img_packet {
    BYTE    sch1;
    BYTE    sch2;
    BYTE    deid;
    BYTE    leng[4];
    BYTE    addr[2];
    BYTE    data[1];
} NUPKI;    /* node upward packet of image  */
#define NDEU_ADROFF_IMG 7 /*offsetof(NUPKI, addr)  * 7e+42+did(1)+len(4)        =  7 */
#define NDEU_DTAOFF_IMG 9 /*offsetof(NUPKI, data)  * 7e+42+did(1)+len(4)+adr(2) =  9 */
#define NDEU_HEADLN_IMG NDEU_DTAOFF_IMG           /* head length                =  9 */
#define NDEU_TAILLN_IMG 1                         /* tail length                =  1 */

typedef struct  nodd_cmd_packet {
BYTE    sch1;
BYTE    sch2;
BYTE    data[1];
} NDPKC;    /* node downward packet of cmd */

typedef struct  nodd_fmw_packet {
BYTE    sch1;
BYTE    sch2;
BYTE	deid;
BYTE    data[1];
} NDPKF;    /* node downward packet of fmw */

/*---------------- to server --------------*/
typedef struct srvu_cmd_packet { /* upward */
BYTE    gwid[2];
BYTE    deid;
BYTE    leng[1];
BYTE    time[4];
BYTE    data[1];
} SUPKC;    /* server upward packet of cmdta */
#define SRVU_GWNOFF_ALL 0 /*offsetof(SUPKC, gwid)  *                            =  0 */
#define SRVU_DIDOFF_ALL 2 /*offsetof(SUPKC, deid)  * gid(2)+                    =  2 */
#define SRVU_LENOFF_ALL 3 /*offsetof(SUPKC, leng)  * gid(2)+did(1)+             =  3 */
#define SRVU_TIMOFF_CMD 4 /*offsetof(SUPKC, time)  * gid(2)+did(1)+len(1)+      =  4 */
#define SRVU_DTAOFF_CMD 8 /*offsetof(SUPKC, data)  * gid(2)+did(1)+len(1)+tim(4)=  8 */
#define SRVU_HEADLN_CMD SRVU_DTAOFF_CMD           /* head length                =  8 */

typedef struct srvu_img_packet {/* upward */
    BYTE    gwid[2];
    BYTE    deid;
    BYTE    leng[4];
    BYTE    time[4];
    BYTE    data[1];
} SUPKI;    /* server upward packet of image */
#define SRVU_TIMOFF_IMG 7 /*offsetof(SUPKI, time)  * gid(2)+did(1)+len(4)+      =  7 */
#define SRVU_DTAOFF_IMG 11 /*offsetof(SUPKI, data) * gid(2)+did(1)+len(4)+tim(4)=  11*/
#define	SRVU_HEADLN_IMG SRVU_DTAOFF_IMG           /* head length                =  11*/

	    /* server upward packet of gw data */
#define	SUPKG		SUPKI	
#define SRVU_TIMOFF_GWY	SRVU_TIMOFF_IMG
#define SRVU_DTAOFF_GWY	SRVU_DTAOFF_IMG
#define SRVU_HEADLN_GWY	SRVU_HEADLN_IMG

/*---------------from server --------------*/
typedef struct srvd_cmd_packet { /* dnward */
    BYTE    gwid[2];
    BYTE    deid;
    BYTE    leng[1];
    BYTE    addr[2];
    BYTE    data[1];
} SDPKC; /* server downward packet of command */
#define SRVD_GWNOFF_ALL 0 /*offsetof(SDPKC, gwid)  *                            =  0 */
#define SRVD_DIDOFF_ALL 2 /*offsetof(SDPKC, deid)  * gid(2)+                    =  2 */
#define SRVD_LENOFF_ALL 3 /*offsetof(SDPKC, leng)  * gid(2)+did(1)+             =  3 */
#define SRVD_ADROFF_CMD 4 /*offsetof(SDPKC, addr)  * gid(2)+did(1)+len(1)       =  4 */
#define SRVD_DTAOFF_CMD 6 /*offsetof(SDPKC, data)  * gid(2)+did(1)+len(1)+adr(2)=  6 */
#define SRVD_HEADLN_CMD SRVD_DTAOFF_CMD       /* head length                    =  6 */

/* OTA */
typedef struct srvd_fmw_packet { /* dnward */
    BYTE    gwid[2];
    BYTE    deid;
    BYTE    leng[4];
    BYTE    addr[2];
    BYTE    data[1];
} SDPKF; /* server downward packet of firmware */
#define SRVD_ADROFF_FMW 7 /*offsetof(SDPKF, addr)  * gid(2)+did(1)+len(4)       =  7 */
#define SRVD_DTAOFF_FMW 9 /*offsetof(SDPKF, data)  * gid(2)+did(1)+len(4)+adr(2)=  9 */   
#define SRVD_HEADLN_FMW SRVD_DTAOFF_FMW       /* head length                    =  9 */

