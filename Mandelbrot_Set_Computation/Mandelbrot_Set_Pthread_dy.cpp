#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>

using namespace std;
Window          win;                            /* initialization for a window */
unsigned
int             width, height,                  /* window size */
                x, y,                           /* window position */
                border_width,                   /*border width in pixels */
                display_width, display_height,  /* size of screen */
                screen,                        /* which screen */
                NUM_thread,
                MAX_ITERATION;
float           lengthsq;
float           threshold;
char            *window_name = "Mandelbrot Set", *display_name = NULL;
GC              gc;
unsigned
long            valuemask = 0;
XGCValues       values;
Display         *display;
XSizeHints      size_hints;
Pixmap          bitmap;
XPoint          points[800];
FILE            *fp, *fopen ();
char            str[100];
pthread_mutex_t	mutex_draw, mutex_task;
struct timeval startTime;
struct timeval endTime;
int chunk;
int taskNum;

void* Mandelbrot_calc(void *t);

typedef struct complextype
        {
        float real, imag;
        } Compl;

typedef struct _thread_data{
    int thread_id;
    int *data;
}thread_data;

int main(int argc, char* argv[]){

    
    XSetWindowAttributes attr[1];
    
    /* connect to Xserver */

    if (  (display = XOpenDisplay (display_name)) == NULL ) {
        fprintf (stderr, "drawon: cannot connect to X server %s\n",
                            XDisplayName (display_name) );
    exit (-1);
    }
    
    /* get screen size */

    screen = DefaultScreen (display);
    display_width = DisplayWidth (display, screen);
    display_height = DisplayHeight (display, screen);

    /* set window position */

    x = 0;
    y = 0;

    /* create opaque window */
    NUM_thread = atoi(argv[1]);
	width = atoi(argv[2]);
	height = atoi(argv[3]);
    MAX_ITERATION = atoi(argv[4]);
    threshold = atoi(argv[5]);
    chunk = atoi(argv[6]);
    taskNum = width/chunk;

    border_width = 4;
    win = XCreateSimpleWindow (display, RootWindow (display, screen),
                            x, y, width, height, border_width,
                            BlackPixel (display, screen), WhitePixel (display, screen));

    size_hints.flags = USPosition|USSize;
    size_hints.x = x;
    size_hints.y = y;
    size_hints.width = width;
    size_hints.height = height;
    size_hints.min_width = 300;
    size_hints.min_height = 300;
    
    XSetNormalHints (display, win, &size_hints);
    XStoreName(display, win, window_name);

    /* create graphics context */

    gc = XCreateGC (display, win, valuemask, &values);

    XSetBackground (display, gc, WhitePixel (display, screen));
    XSetForeground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel(display, screen);

    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow (display, win);
    XSync(display, 0);
        
    /* Calculate and draw points */

    int* wholeArray = new int[height * width];

    printf("Name: Yaqian Chen\nStudent ID: 117010032\nAssignment 2, Mandelbrot Set, Pthread Implementation dynamic\n");
    gettimeofday(&startTime, NULL);
	pthread_mutex_init(&mutex_draw, NULL);	
    pthread_mutex_init(&mutex_task, NULL);	
    pthread_t thread[NUM_thread];
    thread_data input_data[NUM_thread];
    for (int i=0;i<NUM_thread;i++){
        input_data[i].data = wholeArray;
        input_data[i].thread_id = i;
        taskNum -= 1;
        int rc = pthread_create(&thread[i],NULL,Mandelbrot_calc,&input_data[i]);
        if (rc){
            fprintf(stderr,"error:pthread_create,rc: %d\n",rc);
            return EXIT_FAILURE;
        }
    }

    for (int i=0;i<NUM_thread;++i){
        pthread_join(thread[i],NULL);
    }
    gettimeofday(&endTime, NULL);
    XFlush (display);
    printf("Execution Time is: %f seconds.\n", (double)((endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec)*0.000001));
    sleep(30);
}

void* Mandelbrot_calc(void* arg){
    int i, j, k;
    Compl   z, c;
    float   temp;
    thread_data* input_data = (thread_data *)arg;
    int thread_id = input_data->thread_id;
    int* wholeArray = input_data->data;
    int startPosition = thread_id * chunk;
    while(1){
        for(int i= startPosition; i < startPosition + chunk ; i++){
                for(j=0; j < height; j++) {
                z.real = z.imag = 0.0;
                c.real = ((float) j - (height/2))/(height/4);               /* scale factors for 800 x 800 window */
                c.imag = ((float) i - (width/2))/(width/4);
                k = 0;
                do  {                                             /* iterate for pixel color */

                    temp = z.real*z.real - z.imag*z.imag + c.real;
                    z.imag = 2.0*z.real*z.imag + c.imag;
                    z.real = temp;
                    lengthsq = z.real*z.real+z.imag*z.imag;
                    k++;
                } while (lengthsq < threshold && k < MAX_ITERATION);
                pthread_mutex_lock(&mutex_draw);
                if (k == MAX_ITERATION) {
                    XDrawPoint (display, win, gc, j, i);
                    }
                pthread_mutex_unlock(&mutex_draw);
                }
        }
    if (taskNum > 0){
        pthread_mutex_lock(&mutex_task);
        startPosition = (width/chunk - taskNum) * chunk;
        taskNum -= 1;
        pthread_mutex_unlock(&mutex_task);
    } 
    else pthread_exit(NULL);
    }
}
