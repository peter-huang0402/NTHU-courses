#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
//#include <windows.h>
#define  NOT_NUMBER -987654321

#define xWindowEnable 1
#define debug 0
# define valifyCalculation 0

# if xWindowEnable
#include <X11/Xlib.h>
#include <X11/Xutil.h>
  /// add_xWindow
# endif // xWindowEnable

typedef struct {
    double x;
    double y;
    double vx;
    double vy;
}Body;

struct Task{
    int id, start, count;
};

const double GRAVITY_CONS=0.0000000000667259;
int num_Nbody = 0;
int numThread = 1;
double mass = 1.0;
int steps = 1;
double intervalTime = 1.0;
char *file;
double theta = 0.0;
char *enable;
bool isEnable = false;
double xMin = 0.0;
double yMin = 0.0;
double xAxisLength = 0.0;
int xWindowLength = 500;
Body *nBody;
FILE *logFp;
double  *newXV;
double  *newYV;



char *stringConcat(char *str1, char *str2) {

    int length=strlen(str1)+strlen(str2)+1;

    char *result = (char*)malloc(sizeof(char) * length);

    // 複製第一個字串至新的陣列空間
    strcpy(result, str1);
    // 串接第二個字串至新的陣列空間
    strcat(result, str2);

    return result;
}



void getArguments(int argc, char *argv[]){
    numThread =atoi(argv[1]);
    fprintf(logFp,"numThread=%d, ",numThread);

    mass = atof(argv[2]);
    fprintf(logFp,"mass=%f, ",mass);

    steps =atoi(argv[3]);
    fprintf(logFp,"steps=%d, ",steps);

    intervalTime  =atof(argv[4]);
    fprintf(logFp,"intervalTime=%f, ",intervalTime);

    file = argv[5];
    fprintf(logFp,"file=%s, ",file);

    theta  =atof(argv[6]);
    fprintf(logFp,"theta=%f, ",theta);

    enable = argv[7];
    fprintf(logFp,"enable=%s, ",enable);

    if (strcmp(enable , "enable") ==0 ){
        isEnable = true;
        fprintf( logFp,"enable Xwindow\n");

        xMin  =atof(argv[8]);
        fprintf(logFp,"xMin=%f, ",xMin);

        yMin  =atof(argv[9]);
        fprintf(logFp,"yMin=%f, ",yMin);

        xAxisLength  =atof(argv[10]);
        fprintf(logFp,"xAxisLength=%f, ",xAxisLength);

        xWindowLength =atoi(argv[11]);
        fprintf(logFp,"xWindowLength=%d, \n",xWindowLength);
    }
    fflush(stderr);

}

void showNBody(Body *body , int size){
    int i=0;
    for (i=0;i<size;i++){
       fprintf(logFp,"i=%d, body.x=%lf, body.y=%lf, body.vx=%lf, body.vy=%lf, \n",i,body[i].x,body[i].y,body[i].vx , body[i].vy );
    }
    fflush(stderr);
}

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
	XSetForeground(display,gc,BlackPixel(display,screen));
	XFillRectangle(display,window,gc,0,0,width,height);
	XFlush(display);
}


void clearArea(int width,int height){
  /// draw rectangle
	XSetForeground(display,gc,BlackPixel(display,screen));
	XFillRectangle(display,window,gc,0,0,width,height);
}
# endif

void draw(double x,double y)
{
    ///change coordinate to the location of window

    if (x < xMin || x > (xMin + xAxisLength)){
        //fprintf(logFp,"(x=%lf < xMin=%lf) or (x=%lf > xMin+xAxisLength=%lf) \n",x,xMin,x,(xMin + xAxisLength) );
        //fflush(stderr);
        return;
    }else if (y < yMin || y> (yMin + xAxisLength)){
        //fprintf(logFp,"(y=%lf < yMin=%lf) or (y=%lf > yMin+yAxisLength=%lf) \n",y,yMin,y,(yMin + xAxisLength) );
        //fflush(stderr);
        return ;
    }


    double xCoordinate = (x - xMin) * ((1.0/xAxisLength) *  xWindowLength ) ;
    int xIntCoor = (int) xCoordinate;

    double yCoordinate = (y - yMin) * ((1.0/xAxisLength) * xWindowLength ) ;
    int yIntCoor = (int) yCoordinate;



    //fprintf(logFp,"   x=%lf, y=%lf, xCoordinate=%lf, yCoordinate=%lf, xIntCoor=%d, yIntCoor=%d \n",x,y,xCoordinate , yCoordinate , xIntCoor , yIntCoor );

	///draw point
	  # if xWindowEnable
	XSetForeground(display,gc,WhitePixel(display,screen));
	XDrawPoint (display, window, gc, xIntCoor, yIntCoor);
	# endif
}


void computeForce(int no,Body *nbody,double* xForce,double* yForce){

    //fprintf(logFp,"======computForce no=%d, (%3.12lf,%3.12lf)======\n",no, nbody[no].x,nbody[no].y);
    //fflush(stderr);

    int i=0;
    double dtX = 0.0;
    double dtY = 0.0;
    double r = 0.0;
    double force = 0.0;

    double newX = 0.0;
    double newY=0.0;
    double newXv = 0.0;
    double newYv = 0.0;

    double xForceValue = 0.0;
    double yForceValue = 0.0;

    for (i=0;i<num_Nbody;i++){
         if (no == i){
            continue;
         }
         dtX = nbody[i].x - nbody[no].x ;
         dtY = nbody[i].y -nbody[no].y  ;
         r = sqrt((dtX*dtX)+(dtY*dtY));

         if (r ==0){
            continue;
         }

         force = (GRAVITY_CONS*mass*mass) / (r*r);
         xForceValue = xForceValue + (force * (dtX / r));
         yForceValue   = yForceValue+ ( force * (dtY / r));

           # if debug
          fprintf(logFp,"\n i=%d, (%3.15lf,%3.15lf), dtX=%3.15lf, dtY=%3.15lf, r=%3.15lf, force=%3.15lf, xForceValue=%3.15lf, yForceValue=%3.15lf#\n",
                  i,nbody[i].x,nbody[i].y,dtX,dtY,r,force,xForceValue, yForceValue);
           fflush(stderr);
          # endif // debug
    }
    *xForce = xForceValue;
    *yForce = yForceValue;
}

void showArrayInfo(char info[],double *buf, int bufsize){
    int i=0;
    for(i=0;i<bufsize;i++){
        fprintf( logFp, "[%s] buf[%d]=%lf,\n",info, i,buf[i]);
        fflush(stderr);
    }
}

void initNumber(double *buf, int bufsize){
    int i=0;
    for(i=0;i<bufsize;i++){
        buf[i]=NOT_NUMBER;
    }
}




void aggregateVxVy(double *originalVx,double *originalVy, double size, double *totalVx, double *totalVy){

     int i=0;
     double tempVx = 0.0;
     double tempVy = 0.0;

     for (i=0;i<size;i++){
        tempVx = tempVx + originalVx[i];
        tempVy = tempVy + originalVy[i];
     }
     *totalVx = tempVx;
     *totalVx = tempVy;
}




int sequentialNbody(int argc, char *argv[])
{

    logFp = freopen("./log.txt","wr", stderr);


    /***/
    getArguments(argc, &argv[0]);
    char *relativePath = ".\/";
    char *path = stringConcat( &relativePath[0],&file[0]);
    fprintf(logFp,"path=%s\n",path);
    freopen(path,"r",stdin);
    //freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody\\test_data\\test1.txt","r",stdin);
    /**///add_Xwindow

    /**
    ///test1
    numThread =10;
    mass = 10000;
    steps = 1000000;
    steps = 100000;
    intervalTime =0.01;
    theta = 0.0;
    xMin =-1.0;
    yMin =-1.0;
    xAxisLength =3.0;
    xWindowLength = 600;
    freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody\\test_data\\test1.txt","r",stdin);
    /**/

     /**
    ///test2
    numThread =1;
    mass = 1;
    steps = 200;
    intervalTime =1;
    theta = 0.0;
    xMin =-0.3;
    yMin =-0.3;
    xAxisLength =0.6;
    xWindowLength = 600;
    freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody\\test_data\\test2.txt","r",stdin);
     /**/


    int row = 0;
    scanf("%d",&row);
    num_Nbody = row;

    printf("row=%d \n",row);
    fprintf(logFp,"row=%d \n",row);
    fflush(stderr);

    # if valifyCalculation
    double  *originalVx= (double*) malloc (num_Nbody * sizeof(double));
    double  *originalVy= (double*) malloc (num_Nbody * sizeof(double));
    double newTotalVx = 0.0;
    double newTotalVy = 0.0;
    # endif



    nBody = (Body*) malloc (num_Nbody * sizeof(Body));

    //printf("1\n",row);
    //fprintf(logFp,"1 \n",row);
    //fflush(stderr);

    int i=0,j=0;
    double x =0.0;
    double y =0.0;
    double vx=0.0;
    double vy=0.0;

    while( (--row) >=0 ){
       scanf("%lf %lf %lf %lf",&x ,&y, &vx , &vy );
       //fprintf(logFp,"i=%d, x=%3.15lf, y=%3.15lf, vx=%3.15lf, vy=%3.15lf, \n",i,x,y,vx , vy );
       //scanf("%e %e %e %e",&x ,&y, &vx , &vy );
       //fprintf(logFp,"i=%d, x=%e, y=%e, vx=%e, vy=%e, \n",i,x,y,vx , vy );
       nBody[i].x=x;
       nBody[i].y=y;
       nBody[i].vx=vx;
       nBody[i].vy=vy;

        # if valifyCalculation
          originalVx[i]=vx;
           originalVy[i]=vy;
       # endif
       i++;
    }



    # if valifyCalculation
    double originalTotalVx =0.0;
    double originalTotalVy =0.0;
    aggregateVxVy(originalVx, originalVy ,num_Nbody, &originalTotalVx , &originalTotalVy );
    # endif

     if (isEnable){
         # if xWindowEnable
        initGraph(xWindowLength, xWindowLength);
        /**/ //add_Xwindow
        # endif
     }


     //fprintf(logFp,"num_Nbody=%d\n",num_Nbody);

      if (isEnable){
        for (i=0;i<num_Nbody;i++){
            //fprintf(logFp,"i=%d, ",i);
            draw(nBody[i].x,nBody[i].y);
        }
      }

      omp_set_num_threads(numThread);


       //printf("2\n",row);
      //fprintf(logFp,"2 \n",row);
      //fflush(stderr);

     double xForce = 0.0;
     double yForce = 0.0;

     newXV= (double*) malloc (num_Nbody * sizeof(double));
     newYV= (double*) malloc (num_Nbody * sizeof(double));

     double tempX =0.0;
     double tempY =0.0;


     for (i=0;i<steps;i++){

          printf("### Step%d ####, ",i);
           //# if debug
          fprintf(logFp,"### Step%d ####, ",i);
          fflush(stderr);
           //# endif

        #pragma omp parallel for private(j)
        for (j=0;j<num_Nbody;j++){

             xForce = 0.0;
             yForce = 0.0;

            computeForce(j, nBody, &xForce,&yForce);

            newXV[j] = nBody[j].vx +  ( (xForce) * intervalTime / mass);
            newYV[j] = nBody[j].vy +  ( (yForce) * intervalTime / mass);
           // # if debug
            //fprintf(logFp,"\n [Force result] threadId=%d, j=%d,(%3.12lf,%3.12lf), xForce=%3.15lf, yForce=%3.15lf, newXV=%3.15lf, newYV=%3.15lf#\n", omp_get_thread_num(),j,nBody[j].x,nBody[j].y ,xForce, yForce, newXV[j] , newYV[j]);
            //fflush(stderr);
            //# endif
        }

         if (isEnable){
             # if xWindowEnable
             clearArea(xWindowLength, xWindowLength);
             /**///add_Xwindow
               # endif
         }

            #if debug
          fprintf(logFp,"## update data ##\n");
          fflush(stderr);
            #endif

            #if debug
         showArrayInfo( "newXV",newXV,num_Nbody);
         showArrayInfo( "newYV",newYV,num_Nbody);
          #endif

         #pragma omp parallel for private(j)
        for (j=0;j<num_Nbody;j++){
            tempX= nBody[j].x;
            tempY=nBody[j].y;
            nBody[j].vx =newXV[j];
            nBody[j].vy =newYV[j];
            nBody[j].x = nBody[j].x + (nBody[j].vx * intervalTime);
            nBody[j].y = nBody[j].y + (nBody[j].vy * intervalTime);

              // # if debug
            //fprintf(logFp,"\n threadID=%d, draw j=%d ,(%3.15f,%3.15f) ==> new location (x=%3.15lf, y=%3.15lf), new speed (vx=%3.15lf, vy=%3.15lf) \n", omp_get_thread_num(),j,tempX,tempY, nBody[j].x,  nBody[j].y,nBody[j].vx ,nBody[j].vy  );
            //fflush(stderr);
             //#endif
        }

         # if debug
         showNBody(nBody, num_Nbody);
         #endif

         if (isEnable){
            for (j=0;j<num_Nbody;j++){
                 draw(nBody[j].x, nBody[j].y);
            }
         }

        if (isEnable){
             # if xWindowEnable
            XFlush(display);
            /**///add_Xwindow
            # endif
        }


         # if valifyCalculation
         newTotalVx = 0.0;
         newTotalVy = 0.0;
         aggregateVxVy(newXV, newYV ,num_Nbody, &newTotalVx , &newTotalVy );
         fprintf(logFp,"## step=%d, originalTotalVx=%3.15lf, newTotalVx=%3.15lf, originalTotalVy=%3.15lf, newTotalVy=%3.15lf \n",i, originalTotalVx, newTotalVx, originalTotalVy, newTotalVy );
         fprintf(logFp,"## step=%d, compare (originalTotalVx -newVx )=%3.15lf,( originalTotalVy -newVy )=%3.15lf \n",i, (originalTotalVx -newTotalVx ), ( originalTotalVy -newTotalVy ));
         fflush(stderr);
         # endif // debug
     }




  # if valifyCalculation
    free(originalVx);
    free(originalVy);
  #endif // valifyCalculation
    free(newXV);
    free(newYV);
    free(nBody);
    printf("Finish!!!!\n");
    fprintf(logFp,"Finish!!!!\n");
    fflush(stderr);

}


int main(int argc, char *argv[])
{
    struct timeval tv, tv2;

    clock_t endTime;
    unsigned long long start_utime, end_utime;


    endTime =clock();
    gettimeofday(&tv, NULL);
    //Sleep(4000);
    sequentialNbody(argc, argv);
    gettimeofday(&tv2, NULL);
    endTime =clock() - endTime ;

    start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
	end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

    //printf("Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );

	fprintf(logFp,"Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );
	fflush(stderr);
    fclose(logFp);

    return 0;


}



void Test( int n )
{
  int i=0;
  for (i=0;i<n;i++){
     printf( "<Thread=%d> - ( n(%d) + i(%d))=%d \n", omp_get_thread_num(),n,i ,(n+i) );
  }
}

int mainTest(int argc, char* argv[])
{

   omp_set_num_threads(10);
    int i=0;

    #pragma omp parallel for  private(i)
    for (i=0;i<5;i++){
		Test( i );
	}
	system( "pause" );
}
