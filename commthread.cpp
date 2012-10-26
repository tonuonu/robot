/*
 *  Copyright (c) 2012, Tõnu Samuel
 *  All rights reserved.
 *
 *  This is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This software is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with TYROS.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>  /* UNIX standard function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include <libconfig.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <cvblob.h>
#include "libcam.h"

int fd=-1;

#define PORT "/dev/ttyUSB0"

int
open_port(void){
    struct termios tio;
    memset(&tio,0,sizeof(tio));
    tio.c_iflag=0;
    tio.c_oflag=0;
    tio.c_cflag=CS8|CREAD|CLOCAL;           // 8n1, see termios.h for more information
    tio.c_lflag=0;
    tio.c_cc[VMIN]=1;
    tio.c_cc[VTIME]=5;
    fd = open(PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd == -1) {
        printf("open_port: Unable to open %s, errno is %d\n",PORT,errno);
        sleep(1);
    } else {
        printf("open_port: opened %s\n",PORT);
        fcntl(fd, F_SETFL, 0);
        cfsetospeed(&tio,B115200);            // 115200 baud
        cfsetispeed(&tio,B115200);            // 115200 baud
    }
   //     fcntl(fd,F_SETFL,FNDELAY); // Make read() call nonblocking
	return (fd);
}


void *commthread(void * arg) {
    for(;;) {

    if(fd < 0) {
        //printf("/dev/ttyUSB0 is not yet open. Trying to fix this...\n");
        open_port();
    } 
    int len=0;
    if(fd < 0)  {
        //printf("/dev/ttyUSB0 is still not open?! Now I give up!\n");
    } else {
	char buf[256];
	int nbytes=read(fd,buf,sizeof(buf));
	char *p=strchr(buf,27);
	int offset=p-buf;
        if(offset<200 && p[1]=='[' && p[4]==';' && p[6]=='H') {
            char *colonptr=strchr(&p[7],':');
	    if(colonptr>0) {
#if 0
	        *colonptr=0;
	        //printf("%s\n",&p[7]);
		const char **kw=keywords;
		int kwnum=0;
		while(*kw) {
		    if(strcmp(*kw,&p[7])==0) {
			    printf("->Matched '%s'\n",*kw);
#endif
             }
        }
    }
#if 0
       pthread_mutex_lock( &count_mutex );

       if( count < COUNT_HALT1 || count > COUNT_HALT2 )
       {
          // Condition of if statement has been met. 
          // Signal to free waiting thread by freeing the mutex.
          // Note: camthread() is now permitted to modify "count".
          pthread_cond_signal( &condition_var );
       }
       else
       {
          count++;
          printf("Counter value parserthread: %d\n",count);
       }

       pthread_mutex_unlock( &count_mutex );

       if(count >= COUNT_DONE) return(NULL);
#endif
    }
}

