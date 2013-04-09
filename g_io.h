  /*-----------------------------------------*
             c_io.h
        coded by H.B. Lee
        Feb/2007
 *-----------------------------------------*/

#define node_read   bb_read
#define node_write  bb_write

#define cdma_read   bt_read_cn
#define cdma_write  bt_write

#define cdsk_read   bt_read
#define cdsk_write  bt_write

int bb_read(int fd, BYTE *buf, int size);   /* binary, block            */
int bt_read_cn(int fd, BYTE *buf, int size);/* binary, timer, cancel nul*/
int bt_read(int fd, BYTE *buf, int size);   /* binary, timer		*/
int ab_read(int fd, BYTE *buf, int size);   /* ascii,  block		*/
int at_read(int fd, BYTE *buf, int size);   /* ascii,  timer		*/
int bb_write(int fd, BYTE *buf, int size);  /* binary, block 		*/
int bt_write(int fd, BYTE *buf, int size);  /* binary, timeout, block	*/
int ab_write(int fd, BYTE *buf, int size);  /* ascii,  block		*/
