 /*-----------------------------------------*
            c_ubuf.h
        coded by H.B. Lee
            Feb/2007
 *-----------------------------------------*/

/*------------ raw i/o buffer ------------*/
typedef struct rd_buf {
    int phase; /* phase */
#define HEAD_END    1
#define NORM_END    2
#define IMAG_END    3

    int flag;
#define UBUF_NOT    0
#define UBUF_USE    1
#define UBUF_NEW    2

    int ufd;    /* serial io point     */
    BYTE    *sp;    /* current start point */
    BYTE    *cp;    /* current read point  */
    BYTE    *mp;    /* midlle point           */
    BYTE    *ep;    /* end point           */
    struct rd_buf   *rw_ubp;  /* for CDMA interface */
    int rn;
    int     bsz;
    BYTE    *buf;   
    BYTE    gab[4]; /* for unlimited big cdma message */
} UBUF;

extern  void    ubp_init();
extern  void    ubp_free(UBUF *ubp);
extern  void    ubp_flush(UBUF *ubp);
extern  UBUF   *ubp_alloc(int ufd);

