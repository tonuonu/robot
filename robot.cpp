#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include <cvblob.h>
#include "libcam.h"


pthread_mutex_t count_mutex     = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var   = PTHREAD_COND_INITIALIZER;

void *camthread(void * arg);
void *parserthread(void * arg);

int  count = 0;
#define COUNT_DONE  10
#define COUNT_HALT1  3
#define COUNT_HALT2  6

main() {
   pthread_t thread1, thread2;

   pthread_create( &thread1, NULL, &camthread, NULL);
   pthread_create( &thread2, NULL, &parserthread, NULL);

   pthread_join( thread1, NULL);
   pthread_join( thread2, NULL);

   printf("Final count: %d\n",count);

   exit(0);
}

// Write numbers 1-3 and 8-10 as permitted by parserthread()

void *camthread(void * arg) {
   for(;;)
   {
      // Lock mutex and then wait for signal to relase mutex
      pthread_mutex_lock( &count_mutex );

      // Wait while parserthread() operates on count
      // mutex unlocked if condition varialbe in parserthread() signaled.
      pthread_cond_wait( &condition_var, &count_mutex );
      count++;
      printf("Counter value camthread: %d\n",count);

      pthread_mutex_unlock( &count_mutex );

      if(count >= COUNT_DONE) return(NULL);
    }
}

// Write numbers 4-7

void *parserthread(void * arg)
{
    for(;;)
    {
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
    }

}
