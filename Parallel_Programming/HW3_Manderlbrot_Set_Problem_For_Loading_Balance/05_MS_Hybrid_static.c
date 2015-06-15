#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdbool.h>
#include <omp.h>
#include <mpi.h>

#define  NOT_NUMBER -987654321
#define  MASTER		0
#define xWindowEnable 1
#define dynamicPoints 1

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


double *openmpTimeRecording;
int  *openmpPointsRecording;

int mpiPointsRecording = 0;

int myrank, numprocs,len, numOfXpointsForMPI;
int totalTasks =0;
char hostname[MPI_MAX_PROCESSOR_NAME];
MPI_Status status;
MPI_Request request;
int tag1 =1, tag2=2 , tagStop=3;

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
    fprintf(logFp,"enable=%s \n",enable);

    if (strcmp(enable , "enable") ==0 ){
        isEnable = true;
    }
    fflush(stderr);

}


char *stringConcat(char *str1, char *str2) {

    int length=strlen(str1)+strlen(str2)+1;

    char *result = (char*)malloc(sizeof(char) * length);

    // 複製第一個字串至新的陣列空間
    strcpy(result, str1);
    // 串接第二個字串至新的陣列空間
    strcat(result, str2);

    return result;
}

void showArrayInfo(char info[],int *buf, int bufsize){
    int i=0;
    for(i=0;i<bufsize;i++){
        //printf("[%s] hostname=%s, myrank=%d, buf[%d]=%d,\n",info,hostname,myrank,i,buf[i]);
        fprintf( logFp, "[%s] hostname=%s, myrank=%d, buf[%d]=%d,\n",info, hostname,myrank,i,buf[i]);
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

void  calculateMandelbrot(double minX, double maxX, double minY, double maxY,int xPoints, int yPoints){

    Compl z, c;
	int repeats;
	float temp, lengthsq;

    double deltaX = (maxX - minX ) / xPoints;
    double deltaY = (maxY - minY) / yPoints;
    int i,j=0;
    double x,y;



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
			 if (isEnable){
                # if xWindowEnable
                draw(i, j,repeats) ;
                # endif
			 }
        }
    }
}


int sequentialMandelbrot(int argc, char *argv[]){

    logFp = freopen("./log.txt","wr", stderr);

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

    printf("Finish!!!!\n");
    fprintf(logFp,"Finish!!!!\n");
    fflush(stderr);

}

typedef struct{
    int startX;
    int endX;
}Task;


void staticMasterAction(){

     Task *tasks = malloc((numprocs-1) * sizeof(Task) );

     int i,j=0;

     for(i=0;i<(numprocs-1); i++ ){
        tasks[i].startX= i* numOfXpointsForMPI;

        //fprintf(logFp, "i=%d, numOfXpointsForMPI=%d, tasks[i].startX=%d \n" ,i,numOfXpointsForMPI,tasks[i].startX);
        //fflush(stderr);

        if ( ((i* numOfXpointsForMPI)+numOfXpointsForMPI) > numXPoints ){
            tasks[i].endX= numXPoints;
            //fprintf(logFp, "i=%d, numXPoints=%d, (i* numOfXpointsForMPI)=%d \n" ,i,numXPoints,(i* numOfXpointsForMPI));
            //fflush(stderr);
        }else{
            tasks[i].endX= (i* numOfXpointsForMPI)+numOfXpointsForMPI;
             //fprintf(logFp, "i=%d, numXPoints=%d, (i* numOfXpointsForMPI)=%d,  (i* numOfXpointsForMPI)+numOfXpointsForMPI=%d \n" ,i,numXPoints,(i* numOfXpointsForMPI),(i* numOfXpointsForMPI)+numOfXpointsForMPI);
            //fflush(stderr);
        }
        ///fprintf(logFp, "tasks[%d] startX=%d, endX=%d \n" ,i,tasks[i].startX,tasks[i].endX);
        ///fflush(stderr);
     }

     int taskInfo[2];
     for(i=0;i<(numprocs-1); i++ ){
        taskInfo[0]= tasks[i].startX;
        taskInfo[1]= tasks[i].endX;

        ///fprintf(logFp, "i=%d, taskInfo[0]=%d, taskInfo[1]=%d \n" ,i,taskInfo[0],taskInfo[1]);
        ///fflush(stderr);

        ///MPI_Send( &taskInfo[0], 2, MPI_INT, (i+1), tag2, MPI_COMM_WORLD);
        MPI_Isend(&taskInfo[0], 2, MPI_INT, (i+1), tag2, MPI_COMM_WORLD,&request);
     }

     int *colors = malloc(numXPoints * numYPoints * sizeof(int) );
     initNumber(colors, (numXPoints * numYPoints)  );
     /// showArrayInfo("init colors",colors ,(numXPoints * numYPoints) );


     int receiveSize=0;
     char idString[10];
     char *rankIDString="rank=";
     int index=0;
     int receiveRank = 0;
     int k=0;
     int realDataSize=0;

     for(i=0;i<(numprocs-1); i++ ){

         ///MPI_Recv(&receiveRank , 1 , MPI_INT,MPI_ANY_SOURCE ,tag1, MPI_COMM_WORLD,&status); /// for variable data size
         ///fprintf(logFp,"###[Master Receives colors]### i=%d, #slave=%d, receiveRank=%d, startX=%d, endX=%d\n",i,(numprocs-1),receiveRank, tasks[receiveRank-1].startX,tasks[receiveRank -1].endX );
         ///fflush(stderr);
         ///receiveSize = tasks[receiveRank -1].endX - tasks[receiveRank-1].startX ; /// for variable data size

         receiveSize = numOfXpointsForMPI;
         int *results = malloc( receiveSize * numYPoints * sizeof(int)  );
         initNumber( results, receiveSize*numYPoints);

         MPI_Recv(results , receiveSize*numYPoints, MPI_INT,MPI_ANY_SOURCE , tag2,MPI_COMM_WORLD,&status);
         receiveRank = status.MPI_SOURCE;   /// for fixed data size

         ///fprintf(logFp,"receiveRank=%d, receiveSize=%d\n",receiveRank,receiveSize );
         ///fflush(stderr);

         sprintf(idString,"%d",receiveRank);
         rankIDString = stringConcat(rankIDString, idString);

         ///showArrayInfo(rankIDString,results,receiveSize*numYPoints);
         ///fprintf( logFp, "====================================\n");
         ///fflush(stderr);

         index =  tasks[receiveRank -1].startX *numYPoints;

         ///fprintf(logFp,"receiveRank=%d, receiveSize=%d, index=%d\n",receiveRank,receiveSize,index );
         ///fflush(stderr);
         realDataSize =  tasks[receiveRank -1].endX - tasks[receiveRank-1].startX ;  /// for fixed data size
         memcpy(colors+ index , results, (realDataSize*numYPoints*sizeof(int)) ) ;

         free(results);
         results=NULL;

         ///showArrayInfo(rankIDString , colors,(numXPoints * numYPoints) );




         if (isEnable){
             # if xWindowEnable
             k=tasks[receiveRank-1].startX;

             for ( ;k<tasks[receiveRank-1].endX;k++ ){
                 index = k *numYPoints;
                 for (j=0;j<numYPoints;j++){
                     draw(k, j,colors[index+j]) ;
                 }
             }
             XFlush(display);
             # endif
         }

     }

      if (isEnable){
         # if xWindowEnable
         sleep(1);
          # endif
     }


}


void  staticMpiCalculateMandelbrot(double minX, double maxX, double minY, double maxY,int xPointsStart, int xPointsEnd, int *results){

    Compl z, c;
	int repeats;
	float temp, lengthsq;

    double deltaX = (maxX - minX ) / numXPoints;
    double deltaY = (maxY - minY) / numYPoints;
    int i,j=0;
    double x,y;


    #pragma omp parallel for private(x,y,i,j,z,c,temp,lengthsq,repeats)schedule(static)
    for (i=xPointsStart;i<xPointsEnd;i++){
        double st   = omp_get_wtime();
        x= minX + i*deltaX;
        ///#pragma omp parallel for private(j) schedule(static, 1)
        for (j=0;j<numYPoints;j++){
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
			results[((i-xPointsStart)*numYPoints) +j] = repeats;
        }
        double et = omp_get_wtime();
        mpiPointsRecording = mpiPointsRecording+ numYPoints;

        openmpTimeRecording[omp_get_thread_num()]= openmpTimeRecording[omp_get_thread_num()] + (et-st);
        openmpPointsRecording[omp_get_thread_num()]= openmpPointsRecording[omp_get_thread_num()] + numYPoints;

    }
}




void staticSlaveAction(){

    char idString[10];
    char *rankIDString="rank=";
    sprintf(idString,"%d",myrank);
    rankIDString = stringConcat(rankIDString, idString);


    int taskInfo[2];
    MPI_Recv(&taskInfo[0],  2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);



    ///int matrixSize = (taskInfo[1] - taskInfo[0]) * numYPoints;  /// for variable data size
    int matrixSize = numOfXpointsForMPI * numYPoints;



    ///fprintf(logFp,"[Slave] Name=%s, rank=%d, task.startX=%d, endX=%d, matrixSize=%d \n",hostname,myrank,taskInfo[0], taskInfo[1],matrixSize);
    ///fflush(stderr);


    //printf(logFp,"Name=%s, rank=%d, matrixSize=%d \n",hostname,myrank,matrixSize);
    //fflush(stderr);


    int *colors =  malloc( matrixSize * sizeof(int) );
    initNumber( colors, matrixSize);

    ///showArrayInfo( rankIDString ,colors, matrixSize);


    //(double minX, double maxX, double minY, double maxY,int xPointsStart, int xPointsEnd, int yPointsm, int *results){
    staticMpiCalculateMandelbrot(minX, maxX, minY, maxY, taskInfo[0], taskInfo[1],colors);
    ///showArrayInfo( rankIDString ,colors, matrixSize);

    ///MPI_Send( &myrank, 1 , MPI_INT, MASTER, tag1 , MPI_COMM_WORLD);   /// for variable data size
    ///MPI_Send(colors, matrixSize , MPI_INT, MASTER, tag2 , MPI_COMM_WORLD); /// switch to Isend
    MPI_Isend(colors, matrixSize , MPI_INT, MASTER, tag2, MPI_COMM_WORLD,&request);
    ///fprintf(logFp,"[Slave] rank=%d sent data (size=%d) back to Master=%d\n", myrank, matrixSize, status.MPI_SOURCE);
    ///fflush(stderr);

}

int staticMpiMandelbrot(int argc, char *argv[]){
    logFp = freopen("./log.txt","wr", stderr);
    getArguments(argc, &argv[0]);
    omp_set_num_threads(numThread);

    openmpTimeRecording = malloc(numThread * sizeof(double));
    openmpPointsRecording = malloc(numThread * sizeof(int));

    initDouble(openmpTimeRecording,numThread, 0.0);
    initInt(openmpPointsRecording,numThread, 0);


    int width = numXPoints;
	int height = numYPoints;


    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Get_processor_name(hostname, &len);


    numOfXpointsForMPI = numXPoints / (numprocs-1);

    if ( (numXPoints % (numprocs-1)) !=0 ){
        numOfXpointsForMPI = numOfXpointsForMPI+1;
    }



    if (myrank == MASTER){
       fprintf( logFp, "numOfXpointsForMPI=%d \n",numOfXpointsForMPI);
       fflush(stderr);

        if (isEnable){
       # if xWindowEnable
        initGraph(width,height);
       # endif
        }

        staticMasterAction();
    }else{
        staticSlaveAction();
    }

    //printf("myrank=%d, Finish!!!!\n",myrank);

    ///MPI_Finalize();
    //fprintf(logFp,"myrank=%d, Finish!!!!\n",myrank);
    //fflush(stderr);

}


void getTaskIndex(int rankIndex, int taskIndex, Task *tasks ){
     tasks[rankIndex].startX= taskIndex * numOfXpointsForMPI;

      ///fprintf(logFp, "[Master] taskIndex=%d, rank=%d, numOfXpointsForMPI=%d, tasks[%d].startX=%d \n" ,taskIndex,rankIndex, numOfXpointsForMPI,rankIndex, tasks[rankIndex].startX);
      ///fflush(stderr);

      if ( ((taskIndex* numOfXpointsForMPI)+numOfXpointsForMPI) > numXPoints ){
            tasks[rankIndex].endX= numXPoints;
            ///fprintf(logFp, "[Master] taskIndex=%d, rank=%d, numXPoints=%d, (taskIndex* numOfXpointsForMPI)=%d \n" ,taskIndex,rankIndex,numXPoints, (taskIndex * numOfXpointsForMPI) );
            ///fflush(stderr);
      }else{
            tasks[rankIndex].endX= (taskIndex* numOfXpointsForMPI)+numOfXpointsForMPI;
            ///fprintf(logFp, "[Master] taskIndex=%d, rank=%d, numXPoints=%d, (taskIndex* numOfXpointsForMPI)=%d,  (taskIndex* numOfXpointsForMPI)+numOfXpointsForMPI=%d \n" ,taskIndex,rankIndex,numXPoints,(taskIndex* numOfXpointsForMPI),(taskIndex* numOfXpointsForMPI)+numOfXpointsForMPI);
            ///fflush(stderr);
      }

     ///fprintf(logFp, "[Master] tasks[%d] startX=%d, endX=%d \n" ,rankIndex,tasks[rankIndex].startX,tasks[rankIndex].endX);
     ///fflush(stderr);

}

void dynamicMasterAction(){

     Task *tasks = malloc((numprocs-1) * sizeof(Task) );
     int sentTasks=0;
     int completedTasks =0;

     int i,j=0;

     for(i=0;i<(numprocs-1); i++ ){
        getTaskIndex(i,i,tasks);
     }

     int taskInfo[2];
     for(i=0;i<(numprocs-1); i++ ){
        taskInfo[0]= tasks[i].startX;
        taskInfo[1]= tasks[i].endX;

        ///fprintf(logFp, "[Master] i=%d, taskInfo[0]=%d, taskInfo[1]=%d \n" ,i,taskInfo[0],taskInfo[1]);
        ///fflush(stderr);

        ///MPI_Send( &taskInfo[0], 2, MPI_INT, (i+1), 0, MPI_COMM_WORLD);
        MPI_Isend( &taskInfo[0], 2, MPI_INT, (i+1), 0, MPI_COMM_WORLD,&request);
        sentTasks++;
        completedTasks++;
     }

     int *colors = malloc(numXPoints * numYPoints * sizeof(int) );
     initNumber(colors, (numXPoints * numYPoints)  );
     /// showArrayInfo("init colors",colors ,(numXPoints * numYPoints) );


     int receiveSize=0;
     char idString[10];
     char *rankIDString="rank=";
     int index=0;
     int receiveRank = 0;
     int k=0, kEnd=0;
     int realDataSize=0;

     do{

          receiveSize = numOfXpointsForMPI ;
          int *results = malloc( receiveSize * numYPoints * sizeof(int)  );
          initNumber( results, receiveSize*numYPoints);

          MPI_Recv(results , receiveSize*numYPoints, MPI_INT,MPI_ANY_SOURCE , tag1,MPI_COMM_WORLD,&status);
          receiveRank = status.MPI_SOURCE;
          completedTasks--;

          ///fprintf(logFp,"receiveRank=%d, receiveSize=%d\n",receiveRank,receiveSize );
          ///fflush(stderr);

          sprintf(idString,"%d",receiveRank);
          rankIDString = stringConcat(rankIDString, idString);

          index =  tasks[receiveRank -1].startX *numYPoints;

          ///fprintf(logFp,"[Master] receiveRank=%d, receiveSize=%d, index=%d, tasks[%d].startX=%d, endX=%d\n",receiveRank,receiveSize,index, (receiveRank -1), tasks[receiveRank -1].startX,  tasks[receiveRank -1].endX );
          ///fflush(stderr);

          realDataSize =  tasks[receiveRank -1].endX - tasks[receiveRank-1].startX ;
          memcpy(colors+ index , results, (realDataSize*numYPoints*sizeof(int)) ) ;

          free(results);
          results=NULL;

          ///showArrayInfo(rankIDString , colors,(numXPoints * numYPoints) );


          /// pre-record it, before we update it soon.
          k=tasks[receiveRank-1].startX;
          kEnd= tasks[receiveRank-1].endX;

          ///fprintf(logFp,"[Master] receiveRank=%d, k=%d, kEnd=%d\n",receiveRank,k,kEnd);
          ///fflush(stderr);


          if ( sentTasks < totalTasks){

             getTaskIndex( (receiveRank-1) ,sentTasks,tasks);
             taskInfo[0]= tasks[(receiveRank-1)].startX;
             taskInfo[1]= tasks[(receiveRank-1)].endX;

             ///fprintf(logFp,"[Master] receiveRank=%d, next tasks, startX=%d, endX=%d\n",receiveRank,taskInfo[0],taskInfo[1]);
             ///fflush(stderr);
             ///MPI_Send( &taskInfo[0], 2, MPI_INT, receiveRank, tag1, MPI_COMM_WORLD);
             MPI_Isend( &taskInfo[0], 2, MPI_INT, receiveRank, tag1, MPI_COMM_WORLD,&request);

             sentTasks++;
             completedTasks++;
          }else{
             taskInfo[0] = NOT_NUMBER;
             taskInfo[1] = NOT_NUMBER;
             ///fprintf(logFp,"[Master] receiveRank=%d, send Stop-tag\n",receiveRank);
             ///fflush(stderr);
             ///MPI_Send(&taskInfo[0], 2 , MPI_INT, receiveRank , tagStop, MPI_COMM_WORLD);
             MPI_Isend( &taskInfo[0], 2, MPI_INT, receiveRank, tagStop, MPI_COMM_WORLD,&request);
          }

          ///fprintf(logFp,"[Master] sentTasks=%d, completedTasks=%d\n",receiveRank,sentTasks,completedTasks);
          ///fflush(stderr);


          if (isEnable){
             # if xWindowEnable
             for ( ;k<kEnd ;k++ ){
                 index = k *numYPoints;
                 for (j=0;j<numYPoints;j++){
                     draw(k, j,colors[index+j]) ;
                 }
             }
             XFlush(display);
             # endif
         }
     }while(completedTasks >0);


     //showArrayInfo("## final colors ##" , colors,(numXPoints * numYPoints) );

     ///sleep(1);
      if (isEnable){
         # if xWindowEnable
         sleep(1);
          # endif
     }


}


void  dynamicMpiCalculateMandelbrot(double minX, double maxX, double minY, double maxY,int xPointsStart, int xPointsEnd, int *results){
      Compl z, c;
	int repeats;
	float temp, lengthsq;

    double deltaX = (maxX - minX ) / numXPoints;
    double deltaY = (maxY - minY) / numYPoints;
    int i,j=0;
    double x,y;


    #pragma omp parallel for private(x,y,i,j,z,c,temp,lengthsq,repeats)schedule(dynamic,1)
    for (i=xPointsStart;i<xPointsEnd;i++){
        double st   = omp_get_wtime();
        x= minX + i*deltaX;
        ///#pragma omp parallel for private(j) schedule(dynamic, 1)
        for (j=0;j<numYPoints;j++){
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
			results[((i-xPointsStart)*numYPoints) +j] = repeats;
        }
        double et = omp_get_wtime();
        mpiPointsRecording = mpiPointsRecording+ numYPoints;

        openmpTimeRecording[omp_get_thread_num()]= openmpTimeRecording[omp_get_thread_num()] + (et-st);
        openmpPointsRecording[omp_get_thread_num()]= openmpPointsRecording[omp_get_thread_num()] + numYPoints;


        /*
        #pragma omp critical
         {
            fprintf(logFp,"rank=%d, thread id=%d,  mpiPointsRecording=%d\n",myrank,omp_get_thread_num(),mpiPointsRecording  );
	        fflush(stderr);
         }*/

    }
}

void dynamicSlaveAction(){

    char idString[10];
    char *rankIDString="rank=";
    sprintf(idString,"%d",myrank);
    rankIDString = stringConcat(rankIDString, idString);
    int taskInfo[2];
    bool isRunning = true;

    do{

        taskInfo[0]=NOT_NUMBER;
        taskInfo[1]=NOT_NUMBER;

        MPI_Recv(&taskInfo[0],  2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if(status.MPI_TAG == tagStop){
             isRunning = false;
             ///fprintf(logFp,"[Slave] rank=%d get tag-stop from Master.\n", myrank);
             ///fflush(stderr);
             continue;
        }

        int matrixSize = numOfXpointsForMPI * numYPoints;

        ///fprintf(logFp,"[Slave] Name=%s, rank=%d, task.startX=%d, endX=%d, matrixSize=%d \n",hostname,myrank,taskInfo[0], taskInfo[1],matrixSize);
        ///fflush(stderr);

        int *colors =  malloc( matrixSize * sizeof(int) );
        initNumber( colors, matrixSize);

        ///showArrayInfo( rankIDString ,colors, matrixSize);

        //(double minX, double maxX, double minY, double maxY,int xPointsStart, int xPointsEnd, int yPointsm, int *results){
        dynamicMpiCalculateMandelbrot(minX, maxX, minY, maxY, taskInfo[0], taskInfo[1],colors);

        /*
        if ( taskInfo[1] == numXPoints){
            fprintf(logFp,"[Slave] rank=%d show last records. matrixSize=%d, task.startX=%d, endX=%d \n", myrank, matrixSize,  taskInfo[0],  taskInfo[1]);
            fflush(stderr);
            showArrayInfo( rankIDString ,colors, matrixSize);
        }*/

        ///MPI_Send(colors, matrixSize , MPI_INT, MASTER, tag1 , MPI_COMM_WORLD);
        MPI_Isend( colors, matrixSize , MPI_INT, MASTER, tag1 , MPI_COMM_WORLD,&request);
        ///fprintf(logFp,"[Slave] rank=%d sent data (size=%d) back to Master=%d\n", myrank, matrixSize, status.MPI_SOURCE);
        ///fflush(stderr);

        free(colors);
        colors=NULL;



    }while(isRunning);



}


void dynamicMpiMandelbrot(int argc, char *argv[]){
    logFp = freopen("./log.txt","wr", stderr);

    getArguments(argc, &argv[0]);
    omp_set_num_threads(numThread);

    openmpTimeRecording = malloc(numThread * sizeof(double));
    openmpPointsRecording = malloc(numThread * sizeof(int));

    initDouble(openmpTimeRecording,numThread, 0.0);
    initInt(openmpPointsRecording,numThread, 0);

    int width = numXPoints;
	int height = numYPoints;


    MPI_Init(NULL, NULL);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Get_processor_name(hostname, &len);


    /// numOfXpointsForMPI = numXPoints / (numprocs-1);
    numOfXpointsForMPI = dynamicPoints;

    /// key for hybrid
    numOfXpointsForMPI = numThread;

    totalTasks = numXPoints / numOfXpointsForMPI;

    if ( (numXPoints % numOfXpointsForMPI) !=0 ){
        totalTasks = totalTasks+1;
    }



    if (myrank == MASTER){
       fprintf( logFp, "numOfXpointsForMPI=%d, totalTasks=%d \n",numOfXpointsForMPI,totalTasks);
       fflush(stderr);

        if (isEnable){
       # if xWindowEnable
        initGraph(width,height);
       # endif
        }

        dynamicMasterAction();
    }else{
        dynamicSlaveAction();
    }


    ///MPI_Finalize();
    ///fprintf(logFp,"myrank=%d, Finish!!!!\n",myrank);
    ///fflush(stderr);

}

void recordTimePoints(unsigned long long start_utime, unsigned long long end_utime){

    if (false){

        fprintf(logFp,"test, \n");
        fflush(stderr);
        /// for simulating critial section or atomic actions for each mpi processor
        int rank = 0;
        MPI_Barrier(MPI_COMM_WORLD);
        while (rank < numprocs)
        {
            if (myrank == rank)
            {
                fprintf(logFp,"###[mpi] rank id=%d, mpi total points=%d, Gettimeofday time = %llu.%03llu milisecond;  %llu.%06llu sec \n",myrank,mpiPointsRecording, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );
                fflush(stderr);

            }
            rank ++;
            MPI_Barrier(MPI_COMM_WORLD);
        }
    }


  MPI_File logfile;
  MPI_File_open(MPI_COMM_WORLD, "mpiLog.txt", MPI_MODE_WRONLY | MPI_MODE_CREATE,
                   MPI_INFO_NULL, &logfile);
  char mylogbuffer[1024];

  sprintf(mylogbuffer,"###[mpi] rank id=%d, mpi total points=%d, Gettimeofday time = %llu.%03llu milisecond;  %llu.%06llu sec \n",myrank,mpiPointsRecording, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );
  MPI_File_write_ordered(logfile, mylogbuffer, strlen(mylogbuffer), MPI_CHAR, MPI_STATUS_IGNORE);

  int i=0;
  for(i=0;i<numThread;i++){
      sprintf(mylogbuffer,"rank id=%d, ###thread id=%d,  timeRecording[%d]=%lf sec, pointsRecording[%d]=%d, \n",myrank,i, i,openmpTimeRecording[i],omp_get_thread_num(), openmpPointsRecording[i]   );
      MPI_File_write_ordered(logfile, mylogbuffer, strlen(mylogbuffer), MPI_CHAR, MPI_STATUS_IGNORE);
  }


  MPI_File_close(&logfile);

}

int main(int argc, char *argv[]){


    struct timeval tv, tv2;

    clock_t endTime;
    unsigned long long start_utime, end_utime;
    endTime =clock();
    gettimeofday(&tv, NULL);

    /// main functions
    staticMpiMandelbrot(argc,argv);
    ///dynamicMpiMandelbrot(argc,argv);
    ///sequentialMandelbrot(argc,argv);


    gettimeofday(&tv2, NULL);
    endTime =clock() - endTime ;

    start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
	end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

    recordTimePoints(start_utime, end_utime);

    free(openmpTimeRecording);
    free(openmpPointsRecording);
    openmpTimeRecording=NULL;
    openmpPointsRecording=NULL;

	printf("rank=%d, Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%06llu sec \n",myrank,((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );
	fprintf(logFp,"rank=%d, Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%06llu sec \n",myrank,((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );
	fflush(stderr);

	MPI_Finalize();


    fclose(logFp);

    return 0;
}
