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
#include "image.h"
#include "globals.h"

using namespace cvb;

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

void *camthread(void * arg);
void *commthread(void * arg);

int  count = 0;
int  debug = 0;
long int centerx = 0;
long int centery = 0;
#define COUNT_DONE  10
#define COUNT_HALT1  3
#define COUNT_HALT2  6

main(int argc, char *argv[]) {
    config_t cfg, *cf;
    pthread_t thread1, thread2;
    const config_setting_t *retries;

    if(argc==2) {
        debug=1;
    }
    cf = &cfg;
    config_init(cf);
    if (!config_read_file(cf, "robot.conf")) {
        fprintf(stderr, "%d - %s\n",
            config_error_line(cf),
            config_error_text(cf));
        config_destroy(cf);
        return(EXIT_FAILURE);
    }
    
    config_lookup_int(cf, "center.x",&centerx);
    config_lookup_int(cf, "center.y",&centery);
    retries=config_lookup(cf, "colors.ball");
    count = config_setting_length(retries);
    if(count!=6) {
        printf("Error! Got %d arguments instead of 6\n", count);
	exit(1);
    }
    for (int n = 0; n < count; n++) {
        ball[n]=config_setting_get_int_elem(retries, n);
    }
    retries=config_lookup(cf, "colors.mygate");
    count = config_setting_length(retries);
    if(count!=6) {
        printf("Error! Got %d arguments instead of 6\n", count);
	exit(1);
    }
    for (int n = 0; n < count; n++) {
        mygate[n]=config_setting_get_int_elem(retries, n);
    }
    retries=config_lookup(cf, "colors.gate");
    count = config_setting_length(retries);
    if(count!=6) {
        printf("Error! Got %d arguments instead of 6\n", count);
	exit(1);
    }
    for (int n = 0; n < count; n++) {
        gate[n]=config_setting_get_int_elem(retries, n);
    }
   
    config_destroy(cf);

    pthread_create( &thread1, NULL, &camthread, NULL);
 //   pthread_create( &thread2, NULL, &commthread, NULL);

    pthread_join( thread1, NULL);
    pthread_join( thread2, NULL);

    printf("Final count: %d\n",count);

    exit(0);
}


