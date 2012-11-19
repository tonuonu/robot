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

// YUYV byte order
#define Y1 0
#define U  1
#define Y2 2
#define V  3

using namespace cvb;

IplImage*imgYUV;

CvRect box;
bool drawing_box = false;
bool setroi = false;
extern long int centerx,centery;

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
                CvScalar pixel=cvGet2D(imgYUV,y,x);
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

void detectblob(const char*txt,IplImage*needle,IplImage*labelImg,IplImage*imgBGR,CvTracks*tracks) {
    CvBlobs blobs;
    int result=cvLabel(needle, labelImg, blobs);
    // result=cvLabel(imgMyGate, labelImg, blobs);
    cvFilterByArea(blobs, 5, 1000);
    CvLabel label=cvLargestBlob(blobs);
    if(label!=0) {
        // Delete all blobs except the largest
        cvFilterByLabel(blobs, label);
        // if(blobs.begin()->second->maxy-blobs.begin()->second->miny < 50) { // Cut off too high objects
        if(debug) {
            CvFont font;
            double hScale=1.0;
            double vScale=1.0;
            int    lineWidth=1;
            cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);
            cvPutText (imgBGR,txt,cvPoint(blobs.begin()->second->centroid.x,blobs.begin()->second->centroid.y), &font, cvScalar(255,255,0));
        }
        printf("%s at %.1f %.1f\n",txt,blobs.begin()->second->centroid.x,blobs.begin()->second->centroid.y);
        if(strcasecmp("ball",txt)==0) {
	    int gox=blobs.begin()->second->centroid.x - centerx ;
	    int goy=centery - blobs.begin()->second->centroid.y ;



            // Lock mutex and then wait for signal to relase mutex
            pthread_mutex_lock( &count_mutex );
            printf("cammutex %d %d\n",gox,goy); 
#if 0
            // Wait while parserthread() operates on count
            // mutex unlocked if condition varialbe in parserthread() signaled.
            pthread_cond_wait( &condition_var, &count_mutex );
#endif
            pthread_mutex_unlock( &count_mutex );


        }
    }
    if(debug) {
        cvRenderBlobs(labelImg, blobs, imgBGR, imgBGR,CV_BLOB_RENDER_BOUNDING_BOX);
        cvUpdateTracks(blobs, *tracks, 200., 5);
        cvRenderTracks(*tracks, imgBGR, imgBGR, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);
    }
    cvReleaseBlobs(blobs);
}
/*
  Color ranges for these items 
*/
int ball[6]={0,0,0,0,0,0};
int gate[6]={0,0,0,0,0,0};
int mygate[6]={0,0,0,0,0,0};

void *camthread(void * arg) {
    CvTracks tracks_ball;
    CvTracks tracks_gate;
    CvTracks tracks_mygate;
    IplImage* iply,*iplu,*iplv;
    IplImage* imgBall,*imgGate,*imgMyGate;
    IplImage* imgBGR;
    int input=0;
    Camera cam("/dev/video0", IMAGE_WIDTH, IMAGE_HEIGHT,15);
    cam.setInput(input);

    iply      = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    iplu      = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    iplv      = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    imgYUV    = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 3);
    imgBall   = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    imgGate   = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    imgMyGate = cvCreateImage(cvSize(IMAGE_WIDTH,IMAGE_HEIGHT), 8, 1);
    if(debug) {
#ifdef DEBUG
        cvNamedWindow( "result Y", 0 );
        cvNamedWindow( "result U", 0 );
        cvNamedWindow( "result V", 0 );
#endif
        cvNamedWindow( "result BGR", 0 );
        cvNamedWindow( "ball", 0 );
        cvNamedWindow( "gate", 0 );
        cvNamedWindow( "mygate", 0 );
        cvStartWindowThread(); 
        imgBGR= cvCreateImage(cvGetSize(iply), 8, 3);
        cvSetMouseCallback("result BGR",my_mouse_callback,(void*) imgBGR);
    }
    for(;;) {
        IplImage *labelImg;
        int i,j;
        int image_size = IMAGE_HEIGHT*IMAGE_WIDTH;
        unsigned char* ptr = cam.Update();
        /* 
         * For each input pixel we do have 2 bytes of data. But we read them in 
         * groups of four because of YUYV format.
         * i represent absolute number of pixel, j is byte.
         */
        for(i=0,j=0;j<(IMAGE_WIDTH*IMAGE_HEIGHT*2) ; i+=2,j+=4) {
            /* Y channel */
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
        cvMerge(iply,iplu ,iplv , NULL, imgYUV);
        cvCircle(imgYUV, cvPoint((int)centerx,(int)centery), 80, cvScalarAll(0), 80);
	//cvSmooth(imgYUV,imgYUV,CV_MEDIAN,5,5);
        labelImg=cvCreateImage(cvGetSize(imgYUV), IPL_DEPTH_LABEL, 1);
        if(debug) {
            cvCvtColor(imgYUV,imgBGR,CV_YUV2BGR);
        }
        cvInRangeS(imgYUV, cvScalar(ball[0]  ,ball[1]  ,ball[2]  ), 
                           cvScalar(ball[3]  ,ball[4]  ,ball[5]  ), imgBall  );
        cvInRangeS(imgYUV, cvScalar(gate[0]  ,gate[1]  ,gate[2]  ), 
                           cvScalar(gate[3]  ,gate[4]  ,gate[5]  ), imgGate  );
        cvInRangeS(imgYUV, cvScalar(mygate[0],mygate[1],mygate[2]), 
                           cvScalar(mygate[3],mygate[4],mygate[5]), imgMyGate);

        detectblob("ball",imgBall,labelImg,imgBGR,&tracks_ball) ;
        detectblob("gate",imgGate,labelImg,imgBGR,&tracks_gate) ;
        detectblob("mygate",imgMyGate,labelImg,imgBGR,&tracks_mygate) ;
        if(debug) {
            //printf("center x %ld y %ld\n",centerx,centery);
            /* if( drawing_box ) {
            draw_box( iply, box );
            } */
#ifdef DEBUG
            cvShowImage( "result Y", iply );
            cvShowImage( "result U", iplu );
            cvShowImage( "result V", iplv );
#endif
            cvShowImage( "result BGR", imgBGR );
            cvShowImage( "ball", imgBall);
            cvShowImage( "gate", imgGate);
            cvShowImage( "mygate", imgMyGate);
        }
        cvReleaseImage(&labelImg);
    } // for
#ifdef DEBUG
    cvReleaseImage(&iply);
    cvReleaseImage(&iplu);
    cvReleaseImage(&iplv);
    cvReleaseTracks(tracks_ball);
    cvReleaseTracks(tracks_gate);
    cvReleaseTracks(tracks_mygate);
#endif
}

