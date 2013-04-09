/*----------------------------------------*
              g_srvr_pppd.c
            coded by H.B. Lee
                 May/2009
 *-----------------------------------------*
 *       routines for ppp(HSDPA) control   *
 *-----------------------------------------*/

#include <stdio.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/errno.h>
#include <pthread.h>
#include <signal.h>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "g_macros.h"
#include "g_config.h"
#include "g_ubuf.h"
#include "g_pack.h"
#include "g_io.h"
#include "g_util.h"
#include "g_gtway.h"

pthread_mutex_t     Mutx_pppd = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t      Cond_pppd = PTHREAD_COND_INITIALIZER;
pthread_condattr_t  Attr_pppd;

void	wait_pppd();
void    wakeup_pppd();

int  	Pppd_ctrl_req;	/* CDSK_PPPD_DMNON,  CDSK_PPPD_DMNOFF */
			            /* CDSK_PPPD_PWROFF, CDSK_PPPD_ERROFF */
int     Pppd_onfrq_no;  /* used in do_gtwy_timer()            */

int  	Pppd_shut_way;
#define PPPD_SHUT_NO		1	/* never shut			*/
#define PPPD_SHUT_DMN		2	/* deamon only shut		*/
#define PPPD_SHUT_PWR		3	/* deamon & power shut		*/

int     Pppd_stat_dmn;
int     Pppd_stat_pwr;

#define	PPP_DMNON	0x00	/* for pppd_dmnctl() */
#define	PPP_DMNOFF	0x01

#define	PPP_PWRON	0x00	/* for pppd_pwrctl() */
#define	PPP_PWROFF	0x01

int 	pppd_dmnctl(int dmn);
int     pppd_pwrctl(char pwr, int slp);

void  pppd_frq_init()
{
    Pppd_onfrq_no = 0;
    if (Cdsk_pppd_use && Srvr_conn_frq > 2) {
	if (Srvr_conn_frq * Node_sfmi_sec > (Cdsk_pppd_pwt + Cdsk_pppd_dmt) * 2) {
	    Pppd_shut_way = PPPD_SHUT_PWR;
	    Pppd_onfrq_no = Srvr_conn_frq - 
		(Cdsk_pppd_dmt+Cdsk_pppd_pwt+Node_sfmi_sec/2)/Node_sfmi_sec -1;
        }
	else if (Srvr_conn_frq * Node_sfmi_sec > Cdsk_pppd_dmt * 2) {
	    Pppd_shut_way = PPPD_SHUT_DMN;
	    Pppd_onfrq_no = Srvr_conn_frq - 
			  (Cdsk_pppd_dmt+Node_sfmi_sec/2)/Node_sfmi_sec -1;
	}
    }
    if (Pppd_onfrq_no <= 0) { 
    	Pppd_onfrq_no = 0; 
    	Pppd_shut_way = PPPD_SHUT_NO; 
    }

}
void *do_pppd_control(void *arg)/* use Pppd_ctrl_req */
{

    pthread_cond_init(&Cond_pppd, &Attr_pppd);

    while(1) {

	wait_pppd();

	switch(Pppd_ctrl_req) {
	case PPPD_CTRL_BTDON:
	    pppd_dmnctl(PPP_DMNOFF);
	    pppd_pwrctl(PPP_PWROFF, 1);
	    /* fall through */
	case PPPD_CTRL_DMNON:
	    if (pppd_dmnctl(PPP_DMNON)>0) D20("do_pppd_control():success..\n");
            else 			  D10("do_pppd_control():failure..\n");
	    break;

	case PPPD_CTRL_DMNOFF:
	    pppd_dmnctl(PPP_DMNOFF);
	    break;

	case PPPD_CTRL_PWROFF:
	    pppd_dmnctl(PPP_DMNOFF);
	    pppd_pwrctl(PPP_PWROFF, 1);
	    break;

	case PPPD_CTRL_COOL:		/* deamon & power off	*/
	    pppd_dmnctl(PPP_DMNOFF);
	    pppd_pwrctl(PPP_PWROFF, 1);
	    sleep(Cdsk_pppd_clt);
	    break;
	}
    }
}

/*-----------------------------------------------------
      pppd_check() - check pppd deamon status 
 *-----------------------------------------------------*/
int pppd_check()
{
    int     fd, pid = 0, flag = 0, n;
    FILE   *fp;
    char    buf[256], *idp;
    
    char    pid_path[30]="/var/run/ppp0.pid";

    fp = popen("/bin/ps", "r");
    while(fgets(buf, sizeof(buf), fp)) {
       if (strstr(buf, "/sbin/pppd")) {
           flag = 1; // Process ID(PID) exist.
           idp = strtok(buf, " \t\r\n");
           pid = atoi(idp);                
	   break;
       }
    }
    fclose(fp);

    if ((fd = open(pid_path, O_RDONLY)) < 0 || 
				      (n = read(fd, buf, sizeof(buf))) <= 0) { 
        close(fd);
        if (flag) {
	    SV_D02("pppd_check : %s lost, buf pppd(%d) is running...\n", pid_path, pid);
	    L02("pppd_check : %s lost, buf pppd(%d) is running...\n", pid_path, pid);
	    kill(pid, SIGKILL);
	    pppd_pwrctl(PPP_PWROFF, 0);
	    pppd_pwrctl(PPP_PWRON,  0);
#if 0
	    if ((fp = fopen(pid_path, "w")))	// create pid file
		fprintf(fp, "%d", pid);
	    fclose(fp);
#endif
            return(-1);      // is running(abnormal)
        }
        else {
            return(-1);     // is not running(normal) 
        }
    }
    close(fd);

    if (!flag) { 	    // pppd is not running
	SV_D01("pppd_check : %s exist, buf pppd is not running...\n", pid_path);
	L01("pppd_check : %s exist, buf pppd is not running...\n", pid_path);
#if 0
	unlink(pid_path);
#endif
        return(-1);         // is not running(abnormal)
    }
    
    buf[n] = 0; 	    // NULL, '\0'
    if (atoi(buf) != pid) { // PID mismatch
	D02("pppd_check : pid(%d,%d) is mismatch...\n", pid, atoi(buf));
	L02("pppd_check : pid(%d,%d) is mismatch...\n", pid, atoi(buf));
#if 0
	unlink(pid_path);
	if ((fp = fopen(pid_path, "w")))	// create pid file
	    fprintf(fp, "%d", pid);
	fclose(fp);
#endif
        return(1);          // is running(abnormal)
    }

    return(1);              // is running(normal)
}

/*-----------------------------------------------------
   pppd_dmnctl() - pppd deamon control 
 *-----------------------------------------------------*/
int pppd_dmnctl(int dmn)   /* on : 0x00,  off : 0x01 */
{
    switch(dmn) {
    case PPP_DMNON : 
    	if (pppd_check() > 0) {
	    Pppd_stat_dmn = 1;
	    return(1); 	    	    // is running already
	}
	if (!Pppd_stat_pwr && pppd_pwrctl(PPP_PWRON, 1) < 0) {
	    Pppd_stat_dmn = 0;
	    return(-1);
	}

    	system("/sbin/pppd > /dev/null 2>&1 &"); 
	sleep(Cdsk_pppd_dmt);

    	if (pppd_check() > 0) {
	    Pppd_stat_dmn = 1;
	    return(1); 	     // is running already
	}
	return(-1);

    case PPP_DMNOFF :
        system("/gems/ppp-off > /dev/null 2>&1");
	Pppd_stat_dmn = 0;
        return(1);

    default :
        return(-1);
    }
}

int pppd_ison()
{
    return(Pppd_stat_dmn);
}

/*-----------------------------------------------------
    pppd_pwrctl() - HSDPA power control 
 *-----------------------------------------------------*/
int pppd_pwrctl(char pwr, int slp)   /* on : 0x00,  off : 0x01 */
{
    int  udev, i;		   // USB device for HSDPA control

    if ((udev = open(Cdsk_pppd_dev, O_WRONLY)) < 0) {
        SV_D01("pppd_pwrctl() : ppp device(%s) fail...\n", Cdsk_pppd_dev);
        L01("pppd_pwrctl() : ppp device(%s) fail...\n", Cdsk_pppd_dev);
        return(-1);
   }
   if (write(udev, &pwr, 1) <= 0) {
        SV_D02("pppd_pwrctl() : ppp device(%s) write(%.2x) fail...\n", Cdsk_pppd_dev, pwr);
        L02("pppd_pwrctl() : ppp device(%s) write(%.2x) fail...\n", Cdsk_pppd_dev, pwr);
        close(udev);
        return(-1);
   }
   close(udev);

   if (pwr == PPP_PWRON) {
	if (slp) sleep(Cdsk_pppd_pwt);
   	Pppd_stat_pwr = 1;
   }
   else
   	Pppd_stat_pwr = 0;

   return(1);
}

int pppd_wrapup(int sock_stat)
{
   switch(sock_stat) {
   case PPPD_WRUP_OPEN :
   case PPPD_WRUP_CONN :
   case PPPD_WRUP_DATA :
   case PPPD_WRUP_NORM : 
   	switch(Pppd_shut_way) {
   	case PPPD_SHUT_NO:
	    break;

   	case PPPD_SHUT_DMN:
	    wakeup_pppd(PPPD_CTRL_DMNOFF);
	    break;

   	case PPPD_SHUT_PWR:
	    wakeup_pppd(PPPD_CTRL_PWROFF);
	    break;
   	}
       	break;
   }
   return(1);
}

void wait_pppd()
{
        pthread_mutex_lock(&Mutx_pppd);
        pthread_cond_wait(&Cond_pppd, &Mutx_pppd);
        pthread_mutex_unlock(&Mutx_pppd); 
}

void wakeup_pppd(int code)
{
    SV_D20("wakeup_pppd..........\n");
    Pppd_ctrl_req = code;

    pthread_mutex_lock(&Mutx_pppd);
    pthread_cond_signal(&Cond_pppd);
    pthread_mutex_unlock(&Mutx_pppd);
}

