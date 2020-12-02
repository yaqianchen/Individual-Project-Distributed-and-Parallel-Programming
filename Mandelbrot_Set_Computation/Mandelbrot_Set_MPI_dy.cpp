#include "mpi.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <queue>

using namespace std;

typedef struct complextype
        {
        float real, imag;
        } Compl;

int main(int argc, char* argv[]){
    int rank, size;
    int *reps, *result;
    Window win;   
    GC gc;
    Display *display;
    /* Mandlebrot variables */
    int i, j, k;
    Compl   z, c;
    float   lengthsq, temp;
    float   threshold;
    int MAX_Iteration;

    unsigned int    width, height,                  /* window size */
                    x, y,                           /* window position */
                    border_width,                   /*border width in pixels */
                    display_width, display_height,  /* size of screen */
                    screen;                         /* which screen */

    char            *window_name = "Mandelbrot Set", *display_name = NULL;
    unsigned
    long            valuemask = 0;
    XGCValues       values;
    XSizeHints      size_hints;
    Pixmap          bitmap;
    XPoint          points[800];
    FILE            *fp, *fopen ();
    char            str[100];
    struct timeval startTime;
    struct timeval endTime;
    MPI_Init(&argc, &argv);
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
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

    /* set window size */
    width = atoi(argv[1]);
    height = atoi(argv[2]);
    threshold = atoi(argv[4]);
    MAX_Iteration = atoi(argv[3]); 

    int chunk  = atoi(argv[5]);
    int taskNum = width/chunk;
    int startPosition = 0;
    /* set window position */

    x = 0;
    y = 0;

    /* create opaque window */

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

    if(rank==0){
        XSetBackground (display, gc, WhitePixel (display, screen));
        XSetForeground (display, gc, BlackPixel (display, screen));
        XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

        attr[0].backing_store = Always;
        attr[0].backing_planes = 1;
        attr[0].backing_pixel = BlackPixel(display, screen);

        XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

        XMapWindow (display, win);
        XSync(display, 0);
        printf("Name: Yaqian Chen\nStudent ID: 117010032\nAssignment 2, Mandelbrot Set, MPI Implementation dynamic\n");
        int startPosition = 0;
        gettimeofday(&startTime, NULL);
    }
    int* wholeArray = new int[height * width];
    while (1){
        if (rank != 0){
            int count = 1;
            int sepNum = chunk * height + 1;
            int* sepArray = new int[sepNum];
            MPI_Send(&rank, 1, MPI_INT, 0, 0, comm);
            MPI_Recv(&startPosition, 1, MPI_INT, 0, 1, comm, MPI_STATUS_IGNORE);
            if (startPosition < 0) break;
            sepArray[0] = startPosition;
            for(int i= startPosition; i < startPosition + chunk; i++){
                for(int j=0; j < height; j++) {
                    z.real = z.imag = 0.0;
                    c.real = ((float) j - height/2)/(height/4);               /* scale factors for 800 x 800 window */
                    c.imag = ((float) i - width/2)/(width/4);
                    k = 0;
                    do  {         /* iterate for pixel color */
                    temp = z.real*z.real - z.imag*z.imag + c.real;
                    z.imag = 2.0*z.real*z.imag + c.imag;
                    z.real = temp;
                    lengthsq = z.real*z.real+z.imag*z.imag;
                    k++;
                    } while (lengthsq < threshold && k < MAX_Iteration);
                    sepArray[count++] = k;
                }
            }
            MPI_Send(sepArray, sepNum, MPI_INT, 0, 2, comm);
        }
        if (rank == 0){
            int rank_recv;
            int sepNum = chunk * height + 1;
            int stopSign = -1;
            int* sepArray = new int[sepNum];
            MPI_Recv(&rank_recv, 1, MPI_INT, MPI_ANY_SOURCE, 0, comm, MPI_STATUS_IGNORE);
            if (startPosition == -1){
                for (int i = 1; i < size; i++){
                    MPI_Send(&startPosition, 1, MPI_INT, i, 1, comm);
                }
                break;
            }
            MPI_Send(&startPosition, 1, MPI_INT, rank_recv, 1, comm);
            MPI_Recv(sepArray, sepNum, MPI_INT, rank_recv, 2, comm, MPI_STATUS_IGNORE);
            int start = sepArray[0]*height;
            for (int i = 0; i < sepNum - 1; i++){
                wholeArray[start + i] = sepArray[1 + i];
            }
            startPosition += chunk;
            taskNum -= 1;
            if (taskNum < 0) startPosition = -1;
        }
    }
    if (rank==0){
        gettimeofday(&endTime, NULL);
        int count = 0;
        for (int i = 0; i < width; i++){
            for (int j = 0; j < height; j++){
                    if (wholeArray[count++] == MAX_Iteration) XDrawPoint (display, win, gc, j, i);
            }
        }
        XFlush (display);   
        printf("Execution Time is: %f seconds.\n", (double)((endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec)*0.000001));
        sleep (30);
    }
    MPI_Finalize();	
    return 0;
}
