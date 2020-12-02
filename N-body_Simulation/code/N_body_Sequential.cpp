#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <stdio.h>
#include <string.h>

using namespace std;

const double G = 6.67259;
const int MAX_x = 500;
const int MIN_x  = 300;
const int MAX_y = 500;
const int MIN_y  = 300;
const int MAX_w = 100;
const int MIN_w  = 50;
double delta_t  = 0.005;
int body_num;
int iteration_num;
double deltaX, deltaY;
double dist = 0;
double Force;
Window win;/* initialization for a window */
GC gc;
Display *display;

typedef struct Body{
    double x,y;
    double vx,vy;
    double w;
} Body;


// parameters for sequential version 
unsigned int    width=800, height=800,          /* window size */
                x=0, y=0,                       /* window position */
                border_width,                   /*border width in pixels */
                display_width, display_height,  /* size of screen */
                screen;                         /* which screen */

double loss = 0.8;
double epsilon = pow(10,-20);

char            *window_name = "N Body", *display_name = NULL;
unsigned long   valuemask = 0;
XSetWindowAttributes attr[1]; // setup the window property
XGCValues       values;
XSizeHints      size_hints;
Pixmap          bitmap;
XPoint          points[800];
FILE            *fp, *fopen ();
char            str[100];


int init_window(unsigned int x, unsigned int y, unsigned int width, unsigned int height){
    /* connect to Xserver */
    if (  (display = XOpenDisplay (display_name)) == NULL ) {
        fprintf (stderr, "drawon: cannot connect to X server %s\n",
                            XDisplayName (display_name) );
    exit (-1);
    }
    // XInitThreads();
    /* get screen size */
    screen = DefaultScreen (display);
    display_width = DisplayWidth (display, screen);
    display_height = DisplayHeight (display, screen);


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

    XSetBackground (display, gc, WhitePixel (display, screen));
    XSetForeground (display, gc, BlackPixel (display, screen));
    XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

    attr[0].backing_store = Always;
    attr[0].backing_planes = 1;
    attr[0].backing_pixel = BlackPixel(display, screen);

    XChangeWindowAttributes(display, win, CWBackingStore | CWBackingPlanes | CWBackingPixel, attr);

    XMapWindow (display, win);
    XSync(display, 0);
    return 0;
}

void check_boundary(int i, struct Body* Nbody){
    double new_x_pos = Nbody[i].x + Nbody[i].vx * delta_t;
    double new_y_pos = Nbody[i].y + Nbody[i].vy * delta_t;
    if (new_x_pos>=width || new_x_pos<0)Nbody[i].vx = -Nbody[i].vx * loss;
    if (new_y_pos>=height || new_y_pos<0)Nbody[i].vy = -Nbody[i].vy * loss;
}

void collision(int i, int j, struct Body* Nbody){
    Nbody[i].vx = (Nbody[i].w - Nbody[j].w)/(Nbody[i].w + Nbody[j].w) * Nbody[i].vx + (2*Nbody[j].w)/(Nbody[i].w + Nbody[j].w)*Nbody[j].vx;
    Nbody[i].vy = (Nbody[i].w - Nbody[j].w)/(Nbody[i].w + Nbody[j].w) * Nbody[i].vy + (2*Nbody[j].w)/(Nbody[i].w + Nbody[j].w)*Nbody[j].vy;
    Nbody[j].vx = (Nbody[j].w - Nbody[i].w)/(Nbody[j].w + Nbody[i].w) * Nbody[j].vx + (2*Nbody[i].w)/(Nbody[j].w + Nbody[i].w)*Nbody[i].vx;
    Nbody[j].vy = (Nbody[j].w - Nbody[i].w)/(Nbody[j].w + Nbody[i].w) * Nbody[j].vy + (2*Nbody[i].w)/(Nbody[j].w + Nbody[i].w)*Nbody[i].vy;
}

int main (int argc,char *argv[]) {
    struct timeval startTime;
    struct timeval endTime;
    double time_period;
    body_num = atoi(argv[1]);
    iteration_num = atoi(argv[2]);
    init_window(x,y,width,height);
    printf("Student Name: Yaqian Chen\n");
    printf("Student ID: 117010032\n");
    printf("Assignment 3, NBody Simulation, Sequential Version\n");
    int screen = DefaultScreen(display); 
    int depth = DefaultDepth(display,screen); // the number of bits per pixel
    int pixmap = XCreatePixmap(display,win,width,height,depth);
    //initialize the weight and start position 
    struct Body Nbody[body_num];
    srand(time(NULL));
    
    for(int i=0;i<body_num;i++){
        Nbody[i].x = rand() % (MAX_x-MIN_x+1)+MIN_x;
        Nbody[i].y = rand() % (MAX_y-MIN_y+1)+MIN_y;
        Nbody[i].vx = 0;
        Nbody[i].vy = 0;
        Nbody[i].w = rand() % (MAX_w-MIN_w+1)+MIN_w;
    }
    // cout << "Nbody[i].y "<< Nbody[1].x<< "Nbody[i].y "<< Nbody[1].x << endl;
    //iteration
    gettimeofday(&startTime, NULL);
    for(int k=0;k<iteration_num;k++){

        XSetForeground(display,gc,0); //sets the foreground in the specified GC
		XFillRectangle(display,pixmap,gc,x,y,width,height);
        
        for(int i=0;i<body_num;i++)
        {
            for(int j=0;j<body_num;j++)
            {
                if (j==i) continue;
                deltaX = Nbody[j].x - Nbody[i].x;
                deltaY = Nbody[j].y - Nbody[i].y;
                dist = sqrt(pow(deltaX,2)+pow(deltaY,2));
                if(dist <= epsilon){
                    collision(i,j,Nbody);
                }
                else{
                    Force = G * Nbody[j].w * Nbody[i].w / (pow(dist,2)+epsilon);
                    Nbody[i].vx = Nbody[i].vx + delta_t * Force * (deltaX /(dist+epsilon)) / Nbody[i].w;
                    Nbody[i].vy = Nbody[i].vy + delta_t * Force * (deltaY /(dist+epsilon)) / Nbody[i].w;
                }
            }
        }
        for(int i=0;i<body_num;i++)
        {        
            // printf("body i: %d, pos x: %f, pos y: %f, v_x: %f, v_y: %f\n",i, Nbody[i].x, Nbody[i].x, Nbody[i].vx, Nbody[i].vy);
            check_boundary(i, Nbody);
            Nbody[i].x = Nbody[i].x + Nbody[i].vx * delta_t; //here we assume that for each iteration, time should be 0.01
            Nbody[i].y = Nbody[i].y + Nbody[i].vy * delta_t;
        }
        XSetForeground(display, gc, WhitePixel(display,screen));
        for(int i=0;i<body_num;i++){      		
            XDrawPoint(display, pixmap, gc, Nbody[i].y, Nbody[i].x);			
        }
        XCopyArea(display,pixmap,win,gc,0,0,width,height,0,0);
    }
        gettimeofday(&endTime, NULL);
        time_period = (double)((endTime.tv_sec - startTime.tv_sec) + (endTime.tv_usec - startTime.tv_usec)*0.000001);
        printf("Time: %f seconds; Number of body: %d; Max_iter: %d\n", time_period, body_num, iteration_num);
        XFreePixmap(display,pixmap);
		XCloseDisplay(display);

        return 0;
}