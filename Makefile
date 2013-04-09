#
# Makefile
#

# for PC environment
#CC=gcc
CC=arm-linux-gcc
LIB1=-lc
LIB2=-lpthread

# for EM environment
#CC=arm-linux-gcc
#LIB1="-lpthread"
#LIB2="-lrt"

all: Eboot Egtwy 

Egtwy:  g_main.o g_node.o \
	g_srvr.o g_srvr_cdma.o g_srvr_cdsk.o g_srvr_pppd.o \
	g_stwc.o g_timer.o g_config.o \
	g_queue.o g_ubuf.o g_io.o g_util.o g_watchdog.o 

	${CC} -d -o $@ g_main.o g_node.o \
		g_srvr.o g_srvr_cdma.o g_srvr_cdsk.o g_srvr_pppd.o\
		g_stwc.o g_timer.o g_config.o \
		g_queue.o g_ubuf.o g_io.o g_util.o g_watchdog.o ${LIB1} ${LIB2}

Eboot : g_boot.o g_config.o
	${CC} -o $@ g_boot.o g_config.o ${LIB1}

clean:
	rm -rf *.o Egtwy Eboot

g_main.o:\
	g_macros.h g_config.h g_ubuf.h g_gtway.h

g_node.o:\
	g_macros.h g_config.h g_ubuf.h g_pack.h g_queue.h g_io.h g_util.h \
	g_gtway.h

g_srvr:\
	g_macros.h g_config.h g_ubuf.h g_pack.h g_queue.h g_io.h g_util.h \
	g_gtway.h

g_srvr_cdma.o:\
	g_macros.h g_config.h g_ubuf.h g_pack.h g_io.h g_util.h g_gtway.h

g_srvr_cdsk.o:\
	g_macros.h g_config.h g_ubuf.h g_pack.h g_io.h g_util.h g_gtway.h

g_srvr_pppd.o:\
	g_macros.h g_config.h g_ubuf.h g_pack.h g_io.h g_util.h g_gtway.h

g_stwc.o:\
	g_macros.h g_config.h g_ubuf.h g_pack.h g_queue.h g_io.h g_util.h \
	g_gtway.h

g_timer.o:\
	g_macros.h g_config.h g_ubuf.h g_pack.h g_queue.h g_io.h g_util.h \
	g_gtway.h

g_watchdog.o:\
	g_macros.h g_config.h g_ubuf.h g_pack.h g_queue.h g_io.h g_util.h \
	g_gtway.h

g_config.o:\
	g_macros.h g_config.h g_pack.h

g_queue.o:\
	g_macros.h g_config.h g_pack.h g_queue.h g_util.h

g_ubuf.o:\
	g_macros.h g_config.h g_ubuf.h

g_io.o:\
	g_macros.h g_config.h

g_util.o:\
	g_macros.h g_pack.h g_util.h

.c.o:
	$(CC) -c $*.c

