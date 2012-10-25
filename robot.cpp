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

#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <cvblob.h>
#include "libcam.h"
#define IMAGE_WIDTH 640
#define IMAGE_HEIGHT 480
// YUYV byte order
#define Y1 0
#define U  1
#define Y2 2
#define V  3

pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

void *camthread(void * arg);
void *parserthread(void * arg);

int  count = 0;
#define COUNT_DONE  10
#define COUNT_HALT1  3
#define COUNT_HALT2  6


CvRect box;
bool drawing_box = false;
bool setroi = false;

void draw_box( IplImage* img, CvRect rect ) {
    cvRectangle (img,
	cvPoint(box.x,box.y),
	cvPoint(box.x+box.width,box.y+box.height),
        cvScalar(0xff,0x00,0x00)/* red */);
}

void my_mouse_callback(int event, int x, int y, int flags, void* param) {
    IplImage* image = (IplImage*) param;
    switch( event ) {
        case CV_EVENT_MOUSEMOVE: {
            if( drawing_box ) {
                box.width = x-box.x;
                box.height = y-box.y;
            }
        }
        break;
        case CV_EVENT_LBUTTONDOWN: {
            drawing_box = true;
            setroi= false;
            box = cvRect(x, y, 0, 0);
	    if(x<=IMAGE_WIDTH && y<=IMAGE_HEIGHT) {
	        CvScalar pixel=cvGet2D(image,y,x);
	        printf("coord x:%3d y:%3d color Y:%3.0f :U%3.0f V:%3.0f\n",x,y,pixel.val[0],pixel.val[1],pixel.val[2]);
            }
        }
        break;
        case CV_EVENT_LBUTTONUP: {
            drawing_box = false;
            setroi = true;
            if(box.width<0) {
                box.x+=box.width;
                box.width *=-1;
            }
            if(box.height<0) {
                box.y+=box.height;
                box.height*=-1;
            }
            draw_box(image, box);
	    
        }
        break;
    }
}

int fd=-1;

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
	fd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
	if (fd == -1) {
	    printf("open_port: Unable to open /dev/ttyUSB0\n");
            sleep(1);
	} else {
	    printf("open_port: opened /dev/ttyUSB0\n");
	    fcntl(fd, F_SETFL, 0);
            cfsetospeed(&tio,B115200);            // 115200 baud
            cfsetispeed(&tio,B115200);            // 115200 baud
	}
   //     fcntl(fd,F_SETFL,FNDELAY); // Make read() call nonblocking
	return (fd);
}

main() {
   pthread_t thread1, thread2;

   pthread_create( &thread1, NULL, &camthread, NULL);
   pthread_create( &thread2, NULL, &parserthread, NULL);

   pthread_join( thread1, NULL);
   pthread_join( thread2, NULL);

   printf("Final count: %d\n",count);

   exit(0);
}

void *camthread(void * arg) {
    int input=0;
    Camera cam("/dev/video0", IMAGE_WIDTH, IMAGE_HEIGHT);
	
    cam.setInput(input);

    IplImage* iply,*iplu,*iplv;
    iply= cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    iplu= cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    iplv= cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    cvZero(iply);
    cvZero(iplu);
    cvZero(iplv);
    IplImage* imgOrange = cvCreateImage(cvGetSize(iply), 8, 1);
    IplImage* imgBlue   = cvCreateImage(cvGetSize(iply), 8, 1);
    IplImage* imgYellow = cvCreateImage(cvGetSize(iply), 8, 1);
#ifdef DEBUG
    cvNamedWindow( "result Y", 0 );
    cvNamedWindow( "result U", 0 );
    cvNamedWindow( "result V", 0 );
    cvNamedWindow( "result YUV", 0 );
    cvNamedWindow( "result BGR", 0 );
    cvNamedWindow( "orange", 0 );
    cvNamedWindow( "blue", 0 );
    cvNamedWindow( "yellow", 0 );
    cvStartWindowThread(); 
    IplImage*imgYUV= cvCreateImage(cvGetSize(iply), 8, 3);
    IplImage*imgBGR= cvCreateImage(cvGetSize(iply), 8, 3);
    cvSetMouseCallback("result YUV",my_mouse_callback,(void*) imgYUV);
#endif
    for(;;) {
	unsigned char* ptr = cam.Update();
	int i,j;
	
	int image_size = IMAGE_HEIGHT*IMAGE_WIDTH;

	/* 
         * For each input pixel we do have 2 bytes of data. But we read them in 
         * groups of four because of YUYV format.
         * i represent absolute number of pixel, j is byte.
         */
       	for(i=0,j=0;j<(IMAGE_WIDTH*IMAGE_HEIGHT*2) ; i+=2,j+=4) {

	        iply->imageData[i  ] = ptr[j+Y1];
                iply->imageData[i+1] = ptr[j+Y2];
		/* U channel */
	        iplu->imageData[i  ] = ptr[j+U];
                iplu->imageData[i+1] = ptr[j+U];
                       /* V channel */
	        iplv->imageData[i  ] = ptr[j+V];
                iplv->imageData[i+1] = ptr[j+V];
	}
	//double minVal,maxVal;
#ifdef DEBUG 
	cvZero(imgYUV);
	cvMerge(iply,iplu ,iplv , NULL, imgYUV);
	cvCvtColor(imgYUV,imgBGR,CV_YUV2BGR);

        cvInRangeS(imgYUV, cvScalar( 55,  65, 152), cvScalar(203, 109, 199), imgOrange);
        cvInRangeS(imgYUV, cvScalar( 0, 100,  115), cvScalar(40, 133, 128), imgBlue  );
        cvInRangeS(imgYUV, cvScalar(101,  83, 123), cvScalar(155, 114, 142), imgYellow);
        IplImage *labelImg=cvCreateImage(cvGetSize(imgYUV), IPL_DEPTH_LABEL, 1);
#endif
#if 0
        unsigned int result;



// Orange
	result=cvLabel(imgOrange, labelImg, blobs);
        cvFilterByArea(blobs, 15, 1000000);
        CvLabel label=cvLargestBlob(blobs);
        if(label!=0) {
		// Delete all blobs except the largest
          	cvFilterByLabel(blobs, label);
		if(blobs.begin()->second->maxy - blobs.begin()->second->miny < 50) { // Cut off too high objects
	        	printf("largest orange blob at %.1f %.1f\n",blobs.begin()->second->centroid.x,blobs.begin()->second->centroid.y);
				//blobs.begin()->second->label="orange";
		}
        }
        cvRenderBlobs(labelImg, blobs, imgYUV, imgYUV,CV_BLOB_RENDER_BOUNDING_BOX);
        cvUpdateTracks(blobs, tracks_o, 200., 5);
        cvRenderTracks(tracks_o, imgYUV, imgYUV, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);

// Blue
//		result=cvLabel(imgBlue, labelImg, blobs);

//assert(0);
                cvFilterByArea(blobs, 15, 1000000);
                label=cvLargestBlob(blobs);
                if(label!=0) {
			// Delete all blobs except the largest
			cvFilterByLabel(blobs, label);
			if(blobs.begin()->second->maxy - blobs.begin()->second->miny < 50) { // Cut off too high objects
		            printf("largest blue blob at %.1f %.1f\n",blobs.begin()->second->centroid.x,blobs.begin()->second->centroid.y);
			    //blobs.begin()->second->label="orange";
			}
                 }
                cvRenderBlobs(labelImg, blobs, imgYUV, imgYUV,CV_BLOB_RENDER_BOUNDING_BOX);
                cvUpdateTracks(blobs, tracks_b, 200., 5);
                cvRenderTracks(tracks_b, imgYUV, imgYUV, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);




		for (CvBlobs::const_iterator it=blobs.begin(); it!=blobs.end(); ++it) {
			//printf("res %d minx %d,miny %d centroid %.1fx%.1f\n",result,it->second->minx,it->second->miny,it->second->centroid.x,it->second->centroid.y);
        	}

		/*cvMinMaxLoc( const CvArr* A, double* minVal, double* maxVal,
                  CvPoint* minLoc, CvPoint* maxLoc, const CvArr* mask=0 ); */
		//find_circles(iply);

#ifdef DEBUG
                double hScale=0.7;
                double vScale=0.7;
                int    lineWidth=1;
                CvFont font;
                cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX, hScale,vScale,0,lineWidth,0);
#endif
                for (i=0; circles_st[i].x_in_picture != -1; i++) {
#ifdef DEBUG
                    //char buf[256];
#endif
                    tyros_camera::Object o;

                    o.vector.x=circles_st[i].x_in_picture;
                    o.vector.y=circles_st[i].y_in_picture;
                    o.vector.z=i;
                    o.certainity=i;
                    o.type="Ball";
                    msg.object.push_back(o);
#ifdef DEBUG
#if 0
                    printf("Circle[%d] x:%d y:%d r:%d\n", i, circles_st[i].x_in_picture, circles_st[i].y_in_picture, circles_st[i].r_in_picture);
                    printf("Circle[%d] real: x:%lf y:%lf r:%lf\n", i, circles_st[i].x_from_robot, circles_st[i].y_from_robot, circles_st[i].r_from_robot);
//                  cvCircle(iply[devnum], cvPoint(cvRound(circles[i].x_in_picture), cvRound(circles[i].y_in_picture)), 3, CV_RGB(0,255,0), -1, 8, 0);
                    cvCircle(iply, cvPoint(cvRound(circles_st[i].x_in_picture), cvRound(circles_st[i].y_in_picture)), cvRound(circles_st[i].r_in_picture), CV_RGB(0,255,0), 3, 8, 0);

                    sprintf(buf, "%d x:%d y:%d r:%d", i, circles_st[i].x_in_picture, circles_st[i].y_in_picture, circles_st[i].r_in_picture);
                    cvPutText (iply,buf,cvPoint(circles_st[i].x_in_picture-110,circles_st[i].y_in_picture+circles_st[i].r_in_picture*2), &font, cvScalarAll(255));
                    sprintf(buf, "%d x: %.1lf y: %.1lf r: %.1lf", i, circles_st[i].x_from_robot, circles_st[i].y_from_robot, circles_st[i].r_from_robot);
                    cvPutText (iply,buf,cvPoint(circles_st[i].x_in_picture-110,circles_st[i].y_in_picture+circles_st[i].r_in_picture*2+20), &font, cvScalarAll(255));
#endif 
#endif
                }

#endif
#ifdef DEBUG
/*	        if( drawing_box ) {
			draw_box( iply, box );
		}
 */               cvShowImage( "result Y", iply );
                cvShowImage( "result U", iplu );
                cvShowImage( "result V", iplv );
                cvShowImage( "result YUV", imgYUV );
                cvShowImage( "result BGR", imgBGR );
                cvShowImage( "orange", imgOrange);
                cvShowImage( "blue", imgBlue);
                cvShowImage( "yellow", imgYellow);
	//	cvWaitKey(10);
		cvReleaseImage(&labelImg);
#endif
	}
#ifdef DEBUG
        cvReleaseImage(&iply);
        cvReleaseImage(&iplu);
        cvReleaseImage(&iplv);
#endif
#if 0
      // Lock mutex and then wait for signal to relase mutex
      pthread_mutex_lock( &count_mutex );

      // Wait while parserthread() operates on count
      // mutex unlocked if condition varialbe in parserthread() signaled.
      pthread_cond_wait( &condition_var, &count_mutex );
      count++;
      printf("Counter value camthread: %d\n",count);

      pthread_mutex_unlock( &count_mutex );

      if(count >= COUNT_DONE) return(NULL);
#endif
}

// Write numbers 4-7

void *parserthread(void * arg) {
    for(;;) {

    if(0 > fd) {
        //printf("/dev/ttyUSB0 is not yet open. Trying to fix this...\n");
        open_port();
    } 
    int len=0;
    if(0 > fd)  {
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

