#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <omp.h>
//#include <windows.h>

#define  NOT_NUMBER -987654321
#define xWindowEnable 1

# if xWindowEnable
#include <X11/Xlib.h>
# endif

FILE *logFp;
int numThread;
double minX;
double maxX;
double minY;
double maxY;
int numXPoints;
int numYPoints;
bool isEnable = false;

double *timeRecording;
int  *pointsRecording;



typedef struct complextype
{
	float real, imag;
} Compl;

 # if xWindowEnable
GC gc;
Display *display;
Window window;      ///initialization for a window
int screen;         ///which screen

void initGraph(int width,int height)
{
	/// open connection with the server
	display = XOpenDisplay(NULL);
	if(display == NULL) {
		fprintf(stderr, "cannot open display\n");
		exit(1);
	}

	screen = DefaultScreen(display);

	///set window position
	int x = 0;
	int y = 0;

	/// border width in pixels
	int border_width = 0;

	/// create window
	window = XCreateSimpleWindow(display, RootWindow(display, screen), x, y, width, height, border_width, BlackPixel(display, screen), WhitePixel(display, screen));

	/// create graph
	XGCValues values;
	long valuemask = 0;

	gc = XCreateGC(display, window, valuemask, &values);
	//XSetBackground (display, gc, WhitePixel (display, screen));
	XSetForeground (display, gc, BlackPixel (display, screen));
	XSetBackground(display, gc, 0X0000FF00);
	XSetLineAttributes (display, gc, 1, LineSolid, CapRound, JoinRound);

	/// map(show) the window
	XMapWindow(display, window);
	XSync(display, 0);

	/// draw rectangle
	//XSetForeground(display,gc,BlackPixel(display,screen));
	//XFillRectangle(display,window,gc,0,0,width,height);
	//XFlush(display);
}

void draw(int x,int y,int repeats)
{
    ///change coordinate to the location of window
    /*
    double xCoordinate = (x - xMin) * ((1.0/xAxisLength) *  xWindowLength ) ;
    int xIntCoor = (int) xCoordinate;
    double yCoordinate = (y - yMin) * ((1.0/xAxisLength) * xWindowLength ) ;
    int yIntCoor = (int) yCoordinate;
    //fprintf(logFp,"   x=%lf, y=%lf, xCoordinate=%lf, yCoordinate=%lf, xIntCoor=%d, yIntCoor=%d \n",x,y,xCoordinate , yCoordinate , xIntCoor , yIntCoor );
    */

	///draw point
	if (isEnable){
        # if xWindowEnable
        XSetForeground (display, gc,  1024 * 1024 * (repeats % 256));
        XDrawPoint (display, window, gc, x, y);
        # endif
	}
}

 # endif // xWindowEnable


void showArrayInfo(char info[],int *buf, int bufsize){
    int i=0;
    for(i=0;i<bufsize;i++){
        //printf( "[%s] buf[%d]=%d,\n",info, i,buf[i]);
        fprintf( logFp, "[%s] buf[%d]=%d,\n",info, i,buf[i]);
        fflush(stderr);
    }
}

void initNumber(int *buf, int bufsize){
    int i=0;
    for(i=0;i<bufsize;i++){
        buf[i]=NOT_NUMBER;
    }
}

void initDouble(double *buf, int bufsize, double initValue){
    int i=0;
    for(i=0;i<bufsize;i++){
        buf[i]=initValue;
    }
}

void initInt(int *buf, int bufsize, int initValue){
     int i=0;
    for(i=0;i<bufsize;i++){
        buf[i]=initValue;
    }
}



void getArguments(int argc, char *argv[]){

    numThread =atoi(argv[1]);
    fprintf(logFp,"numThread=%d, ",numThread);

    sscanf(argv[2],"%lf",&minX);
    fprintf(logFp,"minX=%lf, ",minX);

    sscanf(argv[3],"%lf",&maxX);
    fprintf(logFp,"maxX=%lf, ",maxX);

    sscanf(argv[4],"%lf",&minY);
    fprintf(logFp,"minY=%f, ",minY);

    sscanf(argv[5],"%lf",&maxY);
    fprintf(logFp,"maxY=%f, ",maxY);

    numXPoints =atoi(argv[6]);
    fprintf(logFp,"numXPoints=%d, ",numXPoints);

    numYPoints =atoi(argv[7]);
    fprintf(logFp,"numYPoints=%d, ",numYPoints);

    char *enable = argv[8];
    fprintf(logFp,"enable=%s, ",enable);

    if (strcmp(enable , "enable") ==0 ){
        isEnable = true;
    }
    fflush(stderr);

}

/*
int mainTTT(int argc, char *argv[]){

    int *buf  = malloc( 10*3*sizeof(int));

    initNumber(buf,10*3);
    showArrayInfo("buf",buf,10*3);

    int *small = malloc( 3*2*sizeof(int));

    int i=0;
    for(i=0;i<6;i++){
        small[i]=i;
    }

    memcpy(buf+ (20) , small, 6*sizeof(int));

    free(small);
    small=NULL;

    showArrayInfo("result",buf,10*3);
    return 0;

}*/

int mainTest(int argc, char *argv[]){

    int i=0;
    int j=0;
    int x=0;

     omp_set_num_threads(3);

    double recordingTime[3];
    recordingTime[0]=10.0;
    recordingTime[1]=20.0;
    recordingTime[2]=30.0;

    #pragma omp parallel for private(i,j,x) schedule(static)
       for (i=0;i<10;i++){
        double st   = omp_get_wtime();
        //Sleep(100+(i*500));
            x = i*2;
            printf("### treadID=%d, x=%d, i=%d\n",omp_get_thread_num(),x,i);
            ///#pragma omp parallel for private(j) schedule(dynamic, 1)
            for (j=0;j<4;j++){
                ///printf("2 treadID=%d, x=%d, i=%d, j=%d\n",omp_get_thread_num(),x,i,j);
                //Sleep(1000+(j*500));
                ///printf("3 treadID=%d, x=%d, i=%d, j=%d, i+j=%d \n",omp_get_thread_num(),x,i,j,(i+j));
            }
         double et = omp_get_wtime();
         recordingTime[omp_get_thread_num()]= recordingTime[omp_get_thread_num()] + (et-st);
         printf("### treadID=%d, time=%f, (et-st)=%f\n",omp_get_thread_num(),recordingTime[omp_get_thread_num()],(et-st));
       }





}

int main(int argc, char *argv[]){


    struct timeval tv, tv2;
    clock_t endTime;
    unsigned long long start_utime, end_utime;
    endTime =clock();
    gettimeofday(&tv, NULL);


    staticOpenmpMandelbrot(argc,argv);
    //sequentialMandelbrot(argc,argv);


    gettimeofday(&tv2, NULL);
    endTime =clock() - endTime ;

    start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
	end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;


	printf("Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%06llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );
	fprintf(logFp,"Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%06llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );
	fflush(stderr);

    fclose(logFp);

}



void  staticOpenmpCalculateMandelbrot(double minX, double maxX, double minY, double maxY,int xPoints, int yPoints){

    Compl z, c;
	int repeats;
	float temp, lengthsq;

    double deltaX = (maxX - minX ) / xPoints;
    double deltaY = (maxY - minY) / yPoints;
    int i,j=0;
    double x,y;

    int *results = malloc( xPoints * yPoints * sizeof(int));


    ///#pragma omp parallel for private(x,y,i,j,z,c,temp,lengthsq,repeats)schedule(static)
    ///#pragma omp parallel for private(x,y,i,j,z,c,temp,lengthsq,repeats)schedule(dynamic,1)
    #pragma omp parallel for private(x,y,i,j,z,c,temp,lengthsq,repeats)schedule(static)
    for (i=0;i<xPoints;i++){
        double st   = omp_get_wtime();
        x= minX + i*deltaX;

         ///#pragma omp parallel for private(j) schedule(dynamic, 1)
        for (j=0;j<yPoints;j++){
             y= minY + j* deltaY;
             z.real = 0.0;
			 z.imag = 0.0;

			 c.real= x;
             c.imag = y;

			 //c.real = ((float)i - 400.0)/200.0; /* Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2 */
			 //c.imag = ((float)j - 400.0)/200.0; /* So needs to scale the window */
			 repeats = 0;
			 lengthsq = 0.0;
			 while(repeats < 100000 && lengthsq < 4.0) { /* Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4 */
			 	temp = z.real*z.real - z.imag*z.imag + c.real;
				z.imag = 2*z.real*z.imag + c.imag;
				z.real = temp;
				lengthsq = z.real*z.real + z.imag*z.imag;
				repeats++;
			 }
			 results[i* yPoints + j]= repeats;
        }
        double et = omp_get_wtime();
        timeRecording[omp_get_thread_num()]= timeRecording[omp_get_thread_num()] + (et-st);
        pointsRecording[omp_get_thread_num()]= pointsRecording[omp_get_thread_num()] + yPoints;

        /* /// for testing
         #pragma omp critical
         {
            fprintf(logFp,"thread id=%d,  timeRecording[%d]=%lf sec, pointsRecording[%d]=%d, \n",omp_get_thread_num(), omp_get_thread_num(),timeRecording[omp_get_thread_num()],omp_get_thread_num(), pointsRecording[omp_get_thread_num()]   );
	        fflush(stderr);
         }
        */

        /*
        if (isEnable){
            # if xWindowEnable
            #pragma omp critical
             draw(i, j,results[i* yPoints + j]) ;
            # endif
        }*/
    }

    ///showArrayInfo( "colors", results, (xPoints * yPoints));


    if (isEnable){
        # if xWindowEnable
        for (i=0;i<xPoints;i++){
             for (j=0;j<yPoints;j++){
                 draw(i, j,results[i* yPoints + j]) ;
                 //fprintf(logFp,"results[%d]=%d ,",(i* yPoints + j), results[(i* yPoints + j)]);
                 //fflush(stderr);
             }
        }
        # endif
     }


}

void staticOpenmpMandelbrot(int argc, char *argv[]){
    logFp = freopen("./log.txt","wr", stderr);

    printf("1\n");
    fprintf(logFp,"1\n");
     fflush(stderr);

    getArguments(argc, &argv[0]);
    /*
    numThread =1;
    minX= -2;
    maxX= 2;
    minY= -2;
    maxY= 2;
    numXPoints=150;
    numYPoints=150;

    /*
    ///test1
    numThread =10;
    minX= 0;
    maxX= 2;
    minY= -2;
    maxY= 2;
    numXPoints=800;
    numYPoints=400;

    /*
    ///test2
    numThread =10;
    minX= -2;
    maxX= 2;
    minY= -2;
    maxY= 2;
    numXPoints=400;
    numYPoints=400;
    */

    omp_set_num_threads(numThread);

    timeRecording = malloc(numThread * sizeof(double));
    pointsRecording = malloc(numThread * sizeof(int));

    initDouble(timeRecording,numThread, 0.0);
    initInt(pointsRecording,numThread, 0);



    int width = numXPoints;
	int height = numYPoints;

    if (isEnable){
        # if xWindowEnable
        initGraph(width,height);
        # endif
    }

    staticOpenmpCalculateMandelbrot(minX, maxX, minY, maxY, numXPoints, numYPoints);

    if (isEnable){
        # if xWindowEnable
        XFlush(display);
        sleep(1);
        # endif
    }

    fprintf(logFp,"\n");
    int i=0;
    for(i=0;i<numThread;i++){
        fprintf(logFp,"###thread id=%d,  timeRecording[%d]=%lf sec, pointsRecording[%d]=%d, \n",i, i,timeRecording[i],omp_get_thread_num(), pointsRecording[i]   );
        fflush(stderr);
    }

    free(timeRecording);
    free(pointsRecording);
    timeRecording=NULL;
    pointsRecording=NULL;

    printf("Finish!!!!\n");
    fprintf(logFp,"Finish!!!!\n");
    fflush(stderr);

}


void  calculateMandelbrot(double minX, double maxX, double minY, double maxY,int xPoints, int yPoints){

    Compl z, c;
	int repeats;
	float temp, lengthsq;

    double deltaX = (maxX - minX ) / xPoints;
    double deltaY = (maxY - minY) / yPoints;
    int i,j=0;
    double x,y;

    int *results = malloc( xPoints * yPoints * sizeof(int));
    initNumber(results, ( xPoints * yPoints ));

    for (i=0;i<xPoints;i++){
        x= minX + i*deltaX;

        for (j=0;j<yPoints;j++){
             y= minY + j* deltaY;
             z.real = 0.0;
			 z.imag = 0.0;

			 c.real= x;
             c.imag = y;

			 //c.real = ((float)i - 400.0)/200.0; /* Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2 */
			 //c.imag = ((float)j - 400.0)/200.0; /* So needs to scale the window */
			 repeats = 0;
			 lengthsq = 0.0;
			 while(repeats < 100000 && lengthsq < 4.0) { /* Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4 */
			 	temp = z.real*z.real - z.imag*z.imag + c.real;
				z.imag = 2*z.real*z.imag + c.imag;
				z.real = temp;
				lengthsq = z.real*z.real + z.imag*z.imag;
				repeats++;
			 }

			  results[i* yPoints + j]= repeats;

             /*
			 if (isEnable){
                # if xWindowEnable
                draw(i, j,repeats) ;
                # endif
			 }*/
        }
    }

    # if xWindowEnable
     for (i=0;i<xPoints;i++){
         for (j=0;j<yPoints;j++){
            if (isEnable){
                draw(i, j,results[i*yPoints+j]) ;
			 }
         }
     }
    # endif

    showArrayInfo( "colors", results, (xPoints * yPoints));



}

void sequentialMandelbrot(int argc, char *argv[]){

    logFp = freopen("./log.txt","wr", stderr);

    printf("1\n");
    fprintf(logFp,"1\n");
    fflush(stderr);

    getArguments(argc, &argv[0]);

    /*
    ///test1
    numThread =10;
    minX= 0;
    maxX= 2;
    minY= -2;
    maxY= 2;
    numXPoints=800;
    numYPoints=400;

    ///test2
    numThread =10;
    minX= -2;
    maxX= 2;
    minY= -2;
    maxY= 2;
    numXPoints=400;
    numYPoints=400;
    */

    int width = numXPoints;
	int height = numYPoints;

	# if xWindowEnable
    initGraph(width,height);
    # endif

    calculateMandelbrot(minX, maxX, minY, maxY, numXPoints, numYPoints);

     # if xWindowEnable
	XFlush(display);
	sleep(1);
    # endif


    printf("Finish!!!!\n");
    fprintf(logFp,"Finish!!!!\n");
    fflush(stderr);

}


void defaultFixedMandelbrot(int argc, char *argv[])
{

    int width = 800;
	int height = 800;

	# if xWindowEnable
    initGraph(width,height);
    #endif

	/* draw points */
	Compl z, c;
	int repeats;
	float temp, lengthsq;
	int i, j;
	for(i=0; i<width; i++) {
		for(j=0; j<height; j++) {
			z.real = 0.0;
			z.imag = 0.0;
			c.real = ((float)i - 400.0)/200.0; /// Theorem : If c belongs to M(Mandelbrot set), then |c| <= 2
			c.imag = ((float)j - 400.0)/200.0; /// So needs to scale the window
			repeats = 0;
			lengthsq = 0.0;

			while(repeats < 100000 && lengthsq < 4.0) { /// Theorem : If c belongs to M, then |Zn| <= 2. So Zn^2 <= 4
				temp = z.real*z.real - z.imag*z.imag + c.real;
				z.imag = 2*z.real*z.imag + c.imag;
				z.real = temp;
				lengthsq = z.real*z.real + z.imag*z.imag;
				repeats++;
			}

            # if xWindowEnable
            draw(i, j,repeats) ;
            # endif
		}
	}

    # if xWindowEnable
	XFlush(display);
	sleep(1);
    # endif

}
