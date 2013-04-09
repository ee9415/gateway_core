/*-----------------------------------------*
             g_queue.h
         coded by H.B. Lee
              Feb/2007
         Revisoned May/2009
 *-----------------------------------------*/

/*------------------------------*/
typedef struct pack_buffer_head {
    struct pack_buffer_head *hnext;
    int hnretry;
    int hrlen; /* real length of dncmds        */
    int hmlen; /* pure data part without head  */
} PKBH; /* packet buffer header */

/*------------------------------*/
typedef struct cmdat_pack_buff {
    PKBH    buffer_head;
#define h_next   buffer_head.hnext
#define h_nretry buffer_head.hnretry
#define h_rlen   buffer_head.hrlen
#define h_mlen   buffer_head.hmlen

    SUPKC   buffer_body;
#define b_gwid buffer_body.gwid
#define b_type buffer_body.deid
#define b_leng buffer_body.leng
#define b_time buffer_body.time
#define b_data buffer_body.data
} PKBUC; /* packet buffer for upward command/data */

typedef struct image_pack_buff {
    PKBH    buffer_head;
    SUPKI   buffer_body;
} PKBUI; /* packet buffer for upward image */

/*------------------------------*/

typedef struct dncmd_pack_buff {
    PKBH    buffer_head;
    NDPKC   buffer_body;
#define b_sch1  buffer_body.sch1
#define b_sch2  buffer_body.sch2
} PKBDC; /* packet buffer for downward command */

typedef struct dnfmw_pack_buff {
    PKBH    buffer_head;
    NDPKF   buffer_body;
} PKBDF;    /* packet buffer for downward fmw  */

/*------------------------------*/
typedef struct queue {
    int flag;
#define Q_CMDTA 0x01 /* command & data  */
#define Q_IMAGE 0x02 /* Image           */
#define Q_BGDTA 0x04 /* Big data        */
#define Q_FIRMW 0x08 /* Firmware        */

#define Q_UPWRD 0x10 /* Upstream data   */
#define Q_DNWRD 0x20 /* Downstream data */

#define Q_URGNT 0x40 /* Emgergency data */

    int type;
    int maxnp; /* max n of packet  */
    int maxrt; /* max n of retry */
    int order; /* priority */
    pthread_mutex_t lock; /* mutex lock */
    PKBH    *f, *r; /* pointer to PKH */
    int npack;
    int msize;
    int trcvd; /* total number of pack */
    int tsend; /* total number of successful send */
    int tfail; /* total number of unsuccessful send */
    int taway; /* total number of discarded */
    int tsecs; /* total secs of image transmission */
} QUEUE;

/*------------------------------*/
#define N_OF_QUEUE 	17
#define MAX_UORDER      13  /*  1 ==>  2 ==> ... MAX_UORDER  */ 
#define MAX_DORDER       4  /* -1 ==> -2 ==> ...-MAX_DORDER  */

extern  int     Ugnt_flag, Full_flag;

extern  QUEUE   Queue[N_OF_QUEUE]; 
extern  QUEUE  *q_by_htype(int htyp), *q_by_order(int ordr);

extern  PKBH   *q_peek(QUEUE *q), *q_delete(QUEUE *q);
extern  void    q_insert(PKBH *pkbh, int htyp), q_insert_bak(PKBH *pkbh, QUEUE *q);

extern  void     dncmd_packet_insert(PKBDC *pkh, QUEUE *q);
extern  void     dnfmw_packet_insert(PKBDF *pkbfd, QUEUE *q);

extern  PKBUC   *upcdt_packet_alloc(int csize, BYTE htype);
extern  PKBUI   *upbin_packet_alloc(int isize, BYTE itype);

extern  PKBDC   *dncmd_packet_alloc(int csize);
extern  PKBDF   *dnfmw_packet_alloc(int fsize);

