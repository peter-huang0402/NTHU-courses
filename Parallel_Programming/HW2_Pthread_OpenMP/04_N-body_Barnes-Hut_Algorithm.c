#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>
#define  NOT_NUMBER -987654321

# define xWindowEnable 1
# define debug 0
# define valifyCalculation 0

#define THREAD_ENABLE 1

# if xWindowEnable
#include <X11/Xlib.h>
#include <X11/Xutil.h>
  /// add_xWindow
# endif // xWindowEnable

int numOfNode =0;
int numOfNodeEnable =0;
int enableBuildTreeDetails = 0;


struct Body {
    double x;
    double y;
    double vx;
    double vy;
    double mass;
};

struct QuadrantNode{
  double maxX;
  double minX;
  double maxY;
  double minY;
  double centerX;
  double centerY;
  double d;
  double mass;
  struct Body *body;
  struct QuadrantNode *Q1;
  struct QuadrantNode *Q2;
  struct QuadrantNode *Q3;
  struct QuadrantNode *Q4;
  #if THREAD_ENABLE
  pthread_mutex_t lock_mutex;
  #endif
};

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
struct Body *nBody;
FILE *logFp;
double  *newXV;
double  *newYV;
struct QuadrantNode* node;

double maxX= 0.0;
double minX= 0.0;
double maxY=0.0;
double minY= 0.0;

bool isRunning = true;
pthread_t *threads;
int numThreadAction = 1;
pthread_mutex_t lock_mutex;
pthread_mutex_t globalVariable_lock_mutex;
pthread_cond_t thread_cv;
pthread_cond_t main_cv;

struct timeval tv, tv2;
unsigned long long start_utime, end_utime;

struct timeval io_tv, io_tv2;
unsigned long long io_start_utime, io_end_utime;

struct timeval buildTree_tv, buildTree_tv2;
unsigned long long buildTree_start_utime, buildTree_end_utime;
unsigned long long buildTree_utime;




char *stringConcat(char *str1, char *str2) {

    int length=strlen(str1)+strlen(str2)+1;

    char *result = (char*)malloc(sizeof(char) * length);

    // 複製第一個字串至新的陣列空間
    strcpy(result, str1);
    // 串接第二個字串至新的陣列空間
    strcat(result, str2);

    return result;
}

int mainAA(int argc, char *argv[])
{
    int a = strcmp("enable","enable");
    printf("a=%d",a);

    char *str2 = ".\\";
    char str3[] = "abc" ;

    char *result = stringConcat(&str2[0],&str3[0]);
    printf("result=%s##",result);

    getchar();
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

void showNBody(struct Body* body , int size){
    int i=0;
    for (i=0;i<size;i++){
       fprintf(logFp,"i=%d, body.x=%3.15lf, body.y=%3.15lf, body.vx=%lf, body.vy=%3.15lf, \n",i,body[i].x,body[i].y,body[i].vx , body[i].vy );
    }
    fflush(stderr);
}

/**/
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
/***/   ///add_xWindow
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



    //fprintf(logFp,"   x=%3.15lf, y=%3.15lf, xCoordinate=%3.15lf, yCoordinate=%3.15lf, xIntCoor=%d, yIntCoor=%d \n",x,y,xCoordinate , yCoordinate , xIntCoor , yIntCoor );


    # if xWindowEnable
	///draw point

    /**/
	XSetForeground(display,gc,WhitePixel(display,screen));
	XDrawPoint (display, window, gc, xIntCoor, yIntCoor);
	/**/
	/// add_xWindow
	# endif


}


void showArrayInfo(char info[],double *buf, int bufsize){
    int i=0;
    for(i=0;i<bufsize;i++){
        //printf("[%s] buf[%d]=%lf,\n",info, i,buf[i]);
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



enum Quadrant {Q1,Q2,Q3,Q4};

struct QuadrantNode* createTreeNode(struct Body *body, double maxX, double minX, double maxY, double minY){

    struct QuadrantNode* node = NULL;

    if (node != NULL){
       free(node);
       node = NULL;
    }else{
       numOfNode++;
       node = malloc( sizeof(struct QuadrantNode));
       if (numOfNodeEnable) fprintf(logFp,"numOfNode=%d ,",numOfNode);
       fflush(stderr);
    }

    if (node == NULL){
        fprintf(logFp,"#### Error-- assign pointer error (%3.15lf,%3.15lf) ####\n",body->x,body->y);
        fflush(stderr);
    }

    node->maxX = maxX;
    node->minX = minX;
    node->maxY = maxY;
    node->minY = minY;
    node->d =  (maxX - minX);


    node->centerX = body->x;
    node->centerY = body->y;
    node->mass = body->mass;
    node->body = body;

    //printf("node->body->x=%lf, node->body->vy=%lf \n",node->body->x,node->body->vy);

    node->Q1 = NULL;
    node->Q2 = NULL;
    node->Q3 = NULL;
    node->Q4 = NULL;


    #if THREAD_ENABLE
    pthread_mutex_init(&node->lock_mutex, NULL);
    # endif
    return node;
}

enum Quadrant judgeQuadrant(double x, double y, double maxX, double minX, double maxY, double minY){
    double midX;
    midX = minX + ((maxX - minX)/2);

    double midY;
    midY = minY + ((maxY - minY)/2);

    if (x > midX ){
       if (y > midY){
           return Q1;
       }else{
          return Q4;
       }
    }else{
       if (y >midY){
          return Q2;
       }else{
           return Q3;
       }
    }
}


void addNodeMass(struct Body *body, struct QuadrantNode* node){

    double totalMass;
    totalMass = node->mass + body->mass;

    node->centerX = ((node->centerX * node->mass) + (body->x * body->mass)) / totalMass;
    node->centerY = ((node->centerY * node->mass) + (body->y * body->mass)) / totalMass;

    node->mass = totalMass;

}



void insertNode(struct Body *body, struct QuadrantNode* node){

      double midX;
      double midY;


      bool running = true;

     enum Quadrant quadrant;

     #if THREAD_ENABLE
     pthread_mutex_lock(&node->lock_mutex);
     #endif
     while (running ){

               midX = node->minX + ((node->maxX - node->minX) /2);
               midY = node->minY + ((node->maxY - node->minY) /2);

              if (enableBuildTreeDetails){
                   fprintf(logFp,"========================================= \n");
                   fprintf(logFp,"node (%3.15lf,%3.15lf), midX=%3.15lf, midY=%3.15lf \n",node->centerX  , node->centerY  ,midX,midY);
                   fflush(stderr);
              }

              if(node->maxX == node->minX){
                 fprintf(logFp,"## Error Error Error Error node->maxX(%3.15f) == node->minX(3.15F) ##\n",node->maxX,node->minX);
                 fflush(stderr);
                 //return;
              }

              if (body->x == node->centerX && body->y == node->centerY){
                 fprintf(logFp,"## Error Error Error Error node(x,y) == body(x,y) (%3.15lf,%3.15lf) ##\n",body->x,body->y);
                 fflush(stderr);
                 running = false;
              }

              if (!running){
                 #if THREAD_ENABLE
                     pthread_mutex_unlock(&node->lock_mutex);
                 # endif
                 break;
              }



              if ( node->body != NULL){
                  quadrant = judgeQuadrant(node->body->x ,node->body->y,node->maxX,node->minX,node->maxY,node->minY );

                  switch(quadrant){
                      case Q1: node->Q1 = createTreeNode(node->body,node->maxX,midX, node->maxY, midY );
                          break;
                      case Q2: node->Q2 = createTreeNode(node->body,midX, node->minX, node->maxY, midY );
                          break;
                      case Q3: node->Q3 = createTreeNode(node->body,midX, node->minX ,midY, node->minY);
                          break;
                      case Q4: node->Q4 = createTreeNode(node->body,node->maxX,midX, midY, node->minY );
                          break;
                  }
                  node->body = NULL;
              }

              addNodeMass(body,node);


              quadrant = judgeQuadrant(body->x ,body->y,node->maxX,node->minX,node->maxY,node->minY );

              if (enableBuildTreeDetails){
                   fprintf(logFp,"body (%3.12lf,%3.12lf) \n",body->x  , body->y);
                   fflush(stderr);
              }


              if (quadrant == Q1){
                 if (node->Q1 ==NULL){
                     node->Q1 = createTreeNode( body,node->maxX,midX, node->maxY, midY );
                     #if THREAD_ENABLE
                     pthread_mutex_unlock(&node->lock_mutex);
                     # endif
                     running = false;
                 }else{
                     #if THREAD_ENABLE
                     pthread_mutex_unlock(&node->lock_mutex);
                     pthread_mutex_lock(&node->Q1->lock_mutex);
                      # endif
                     node = node->Q1 ;
                 }
              }else if (quadrant ==Q2){
                 if (node->Q2 ==NULL){
                     node->Q2 = createTreeNode( body,midX, node->minX, node->maxY, midY );
                     #if THREAD_ENABLE
                      pthread_mutex_unlock(&node->lock_mutex);
                       # endif
                     running = false;
                 }else{
                     #if THREAD_ENABLE
                      pthread_mutex_unlock(&node->lock_mutex);
                      pthread_mutex_lock(&node->Q2->lock_mutex);
                       # endif
                      node = node->Q2 ;
                 }
              }else if (quadrant ==Q3){
                 if (node->Q3 ==NULL){
                     node->Q3 =  createTreeNode( body,midX, node->minX ,midY, node->minY);
                     #if THREAD_ENABLE
                     pthread_mutex_unlock(&node->lock_mutex);
                      # endif
                     running = false;
                 }else{
                     #if THREAD_ENABLE
                      pthread_mutex_unlock(&node->lock_mutex);
                      pthread_mutex_lock(&node->Q3->lock_mutex);
                       # endif
                      node = node->Q3 ;
                 }
              }else{
                 if (node->Q4 ==NULL){
                     node->Q4 = createTreeNode( body,node->maxX,midX, midY, node->minY );
                     #if THREAD_ENABLE
                     pthread_mutex_unlock(&node->lock_mutex);
                      # endif
                     running = false;
                 }else{
                     #if THREAD_ENABLE
                      pthread_mutex_unlock(&node->lock_mutex);
                      pthread_mutex_lock(&node->Q4->lock_mutex);
                       # endif
                      node = node->Q4 ;
                 }
              }
     }


}




void computeForce(struct Body *nbody, struct QuadrantNode* node , double* xForce,double* yForce){

    double dtX;
    dtX = ( node->centerX - nbody->x );

    double dtY;
    dtY = ( node->centerY - nbody->y);

    double r;
    r = sqrt((dtX * dtX) + (dtY* dtY) );

    /*
    if ((node->d/r) < theta ){
        printf("(node->d/r)=%lf < theta=%lf", (node->d/r) , theta );
    }

    if (node->body){
        printf("node body is not null");
    }else{
        printf("node body is null");
    }

    if (node->body == NULL){
        printf("node body is null");
    }

     if (node->body != nbody){
        printf("ode->body != nbody");
     }else{
         printf("ode->body == nbody");
     }*/

     ///peter_RRR
     if ( ( ((node->d /r) < theta) ||  (node->body != NULL)) && (node->body != nbody) ){

         if ( r == 0.0 ) return ;

         /*
         if ((node->d/r) < theta ){
            printf("\n(%3.12lf,%3.12lf) => (node->d/r)=%lf < theta=%lf, ", node->centerX  , node->centerY, (node->d/r) , theta );
            if (node->body == NULL){
                printf("node body is null \n");
            }else{
                printf("\n");
            }
         }
         */


         double force;
         force = (GRAVITY_CONS * nbody->mass * node->mass) / (r*r);
         *xForce = *xForce + (force * (dtX / r));
         *yForce = *yForce +  ( force * (dtY / r));

          /*
          fprintf(logFp,"(%3.12lf,%3.12lf), dtX=%3.15lf, dtY=%3.15lf, r=%3.15lf, force=%3.15lf, xForceValue=%3.15lf, yForceValue=%3.15lf#\n",
                   node->centerX  , node->centerY  ,dtX,dtY,r,force, *xForce, *yForce);
          fflush(stderr);
          */

    }else{

        if (node->Q1 !=NULL){
             //fprintf(logFp,"node->Q1, ");
            computeForce(nbody, node->Q1, xForce, yForce);
        }

        if (node->Q2 !=NULL){
            //fprintf(logFp,"node->Q2, ");
            computeForce(nbody, node->Q2, xForce, yForce);
        }

        if (node->Q3 !=NULL){
            //fprintf(logFp,"node->Q3, ");
            computeForce(nbody, node->Q3, xForce, yForce);
        }

        if (node->Q4 !=NULL){
           // fprintf(logFp,"node->Q4, ");
            computeForce(nbody, node->Q4, xForce, yForce);
        }

    }
}

void freeQuadrantNode(struct QuadrantNode *quadrantNode){
    if (quadrantNode != NULL ){

        if (quadrantNode->Q1 !=NULL){
            freeQuadrantNode(quadrantNode->Q1);
        }

        if (quadrantNode->Q2 !=NULL){
            freeQuadrantNode(quadrantNode->Q2);
        }

        if (quadrantNode->Q3 !=NULL){
            freeQuadrantNode(quadrantNode->Q3);
        }

        if (quadrantNode->Q4 !=NULL){
            freeQuadrantNode(quadrantNode->Q4);
        }

         #if THREAD_ENABLE
          pthread_mutex_destroy(&quadrantNode->lock_mutex);
         #endif // THREAD_ENABLE

         free(quadrantNode);
         quadrantNode= NULL;
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


    /**/
    getArguments(argc, &argv[0]);
    char *relativePath = ".\/";
    char *path = stringConcat( &relativePath[0],&file[0]);
    fprintf(logFp,"path=%s\n",path);
    freopen(path,"r",stdin);
    //freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody_Barnes-Hut\\test_data\\test1.txt","r",stdin);
    /**/




    /**
    ///test2
    numThread =1;
    mass = 10000;
    steps = 1000000;
    steps = 100000;
    intervalTime =0.01;
    theta = 0.0;

    xMin =-1.0;
    yMin =-1.0;
    xAxisLength =3.0;
    xWindowLength = 600;
    freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody_Barnes-Hut\\test_data\\test1.txt","r",stdin);
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
    freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody_Barnes-Hut\\test_data\\test2.txt","r",stdin);
     /**/

      /**
    ///test3
    numThread =1;
    mass = 1;
    steps = 200;
    intervalTime =1;
    theta = 0.2;
    theta = 0.0;
    xMin =-0.5;
    yMin =-0.5;
    xAxisLength =1.0;
    xWindowLength = 500;
    freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody_Barnes-Hut\\test_data\\test3.txt","r",stdin);
     /**/

    /**
     ///test4
    numThread =1;
    mass = 1;
    steps = 300;
    intervalTime =1;
    theta = 0.5;
    theta = 0.0;
    xMin =-1;
    yMin =-1;
    xAxisLength =2.5;
    xWindowLength = 500;
    freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody_Barnes-Hut\\test_data\\test4.txt","r",stdin);
     /**/

    /**
    ///test big data
    numThread =1;
    mass = 1;
    steps = 1000;
    intervalTime =0.01;
    theta = 0.0;
    xMin =-1;
    yMin =-1;
    xAxisLength =2.5;
    xWindowLength = 500;
    freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody_Barnes-Hut\\test_data\\test-big.txt","r",stdin);
     /**/



    int row = 0;
    scanf("%d",&row);
    num_Nbody = row;

    fprintf(logFp,"row=%d \n",row);
    fflush(stderr);

    #if valifyCalculation
    double  *originalVx= (double*) malloc (num_Nbody * sizeof(double));
    double  *originalVy= (double*) malloc (num_Nbody * sizeof(double));
    double newTotalVx = 0.0;
    double newTotalVy = 0.0;
     # endif

    nBody = (struct Body*) malloc (num_Nbody * sizeof(struct Body));

    int i=0,j=0;
    double x =0.0;
    double y =0.0;
    double vx=0.0;
    double vy=0.0;


     initMaxMinXY();

    while( (--row) >=0 ){
       scanf("%lf %lf %lf %lf",&x ,&y, &vx , &vy );
       //fprintf(logFp,"i=%d, x=%3.15lf, y=%3.15lf, vx=%3.15lf, vy=%3.15lf, \n",i,x,y,vx , vy );
       //scanf("%e %e %e %e",&x ,&y, &vx , &vy );
       //fprintf(logFp,"i=%d, x=%e, y=%e, vx=%e, vy=%e, \n",i,x,y,vx , vy );
       nBody[i].x=x;
       nBody[i].y=y;
       nBody[i].vx=vx;
       nBody[i].vy=vy;
       nBody[i].mass= mass;

       # if valifyCalculation
       originalVx[i]=vx;
       originalVy[i]=vy;
       # endif

       if (x > maxX){ maxX = x; }
       if (y > maxY){ maxY = y; }
       if (x < minX){ minX = x; }
       if (y < minY){ minY = y; }

       i++;
    }

    //double distX = maxX- minX;
    //double distY = maxY- minY;
    fprintf(logFp,"maxX=%lf, minX=%lf, maxY=%lf, minY=%lf \n", maxX, minX,maxY,minY);
    fflush(stderr);

    //showNBody(nBody, num_Nbody);

     # if valifyCalculation
    double originalTotalVx =0.0;
    double originalTotalVy =0.0;
    aggregateVxVy(originalVx, originalVy ,num_Nbody, &originalTotalVx , &originalTotalVy );
   # endif


     if (isEnable){
         # if xWindowEnable
         /**/
        initGraph(xWindowLength, xWindowLength);
        /**/  ///add_xWindow
        #endif
     }


     //fprintf(logFp,"num_Nbody=%d\n",num_Nbody);

     if (isEnable){
        for (i=0;i<num_Nbody;i++){
            //fprintf(logFp,"i=%d, ",i);
            draw(nBody[i].x,nBody[i].y);
        }
     }




     double xForce = 0.0;
     double yForce = 0.0;
     newXV= (double*) malloc (num_Nbody * sizeof(double));
     newYV= (double*) malloc (num_Nbody * sizeof(double));
     double tempX =0.0;
     double tempY =0.0;

     int k=0;


     for (i=0;i<steps;i++){

         printf("### Step%d ####\n",i);
         fprintf(logFp,"### Step%d #### ",i);
         fprintf(logFp," maxX=%3.15lf, minX=%3.15lf, maxY=%3.15lf, minY=%3.15lf \n", maxX, minX,maxY,minY);
         fflush(stderr);

         for (k=0;k<num_Nbody;k++){

              /*
              if (i == 0 && k==81919){
                   enableBuildTreeDetails =1;
              }else{
                   enableBuildTreeDetails=0;
              }*/

             if ( k==0){
                 node = createTreeNode( &nBody[k],maxX,minX,maxY,minY);
             }else{
                  numOfNode=0;
                  if (numOfNodeEnable){
                      fprintf(logFp,"insert %dth node ",k);
                      fflush(stderr);
                  }

                  insertNode(&nBody[k], node);
                  if (numOfNodeEnable){
                      fprintf(logFp,"numOfNode=%d ,",numOfNode);
                      fflush(stderr);
                  }
             }
         }

         //fprintf(logFp,"1\n");
         //fflush(stderr);

        for (j=0;j<num_Nbody;j++){

            xForce = 0.0;
            yForce = 0.0;
            computeForce( &nBody[j],node, &xForce,&yForce);

            newXV[j] = nBody[j].vx +  ( (xForce) * intervalTime / mass);
            newYV[j] = nBody[j].vy +  ( (yForce) * intervalTime / mass);

            //fprintf(logFp,"[Force result] j=%d,(%3.15lf,%3.15lf), xForce=%3.15lf, yForce=%3.15lf, newXV=%3.15lf, newYV=%3.15lf#\n",j,nBody[j].x,nBody[j].y ,xForce, yForce, newXV[j] , newYV[j]);
            //fflush(stderr);
        }

        //fprintf(logFp,"2\n");
        //fflush(stderr);

        freeQuadrantNode(node);

        //fprintf(logFp,"3\n");
        //fflush(stderr);

         if (isEnable){
             # if xWindowEnable
             /**/
             clearArea(xWindowLength, xWindowLength);
             /**/ ///add_xWindow
             #endif
         }


         initMaxMinXY();

        for (j=0;j<num_Nbody;j++){
            tempX= nBody[j].x;
            tempY=nBody[j].y;
            nBody[j].vx =newXV[j];
            nBody[j].vy =newYV[j];
            nBody[j].x = nBody[j].x + (nBody[j].vx * intervalTime);
            nBody[j].y = nBody[j].y + (nBody[j].vy * intervalTime);


            if (nBody[j].x > maxX){ maxX = nBody[j].x; }
            if (nBody[j].y > maxY){ maxY = nBody[j].y; }
            if (nBody[j].x < minX){ minX = nBody[j].x; }
            if (nBody[j].y < minY){ minY = nBody[j].y; }
            //fprintf(logFp,"draw j=%d ,(%3.12f,%3.12f) ==> new location (x=%3.15lf, y=%3.15lf), new speed (vx=%3.15lf, vy=%3.15lf) \n",j,tempX,tempY, nBody[j].x,  nBody[j].y,nBody[j].vx ,nBody[j].vy  );
            //fflush(stderr);

            if (isEnable){
                draw(nBody[j].x, nBody[j].y);
            }
        }

        if (isEnable){
             # if xWindowEnable
             /**/
            XFlush(display);
            /***////add_xWindow
            #endif

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
    #endif
    free(newXV);
    free(newYV);
    free(nBody);
    fprintf(logFp,"Finished !!!\n");
    fclose(logFp);
    return 0;
}



int main(int argc, char *argv[])
{


    clock_t endTime;
    endTime =clock();
    gettimeofday(&tv, NULL);
    //Sleep(4000);
    //sequentialNbody(argc, argv);
    pthreadNbody(argc, argv);


    gettimeofday(&tv2, NULL);
    endTime =clock() - endTime ;

    start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
	end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

    //printf("Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );

	fprintf(logFp,"Total time, Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );

	fprintf(logFp,"Compute time,   Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",(end_utime - start_utime - buildTree_utime)/1000, (end_utime - start_utime - buildTree_utime)%1000, (end_utime - start_utime -buildTree_utime)/1000000, (end_utime - start_utime-buildTree_utime)%1000000  );
	fprintf(logFp,"build time ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n", (buildTree_utime)/1000, (buildTree_utime)%1000, (buildTree_utime)/1000000, (buildTree_utime)%1000000  );

	io_start_utime = io_tv.tv_sec * 1000000 + io_tv.tv_usec;
	io_end_utime = io_tv2.tv_sec * 1000000 + io_tv2.tv_usec;
	fprintf(logFp,"i/o time ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n", (io_end_utime - io_start_utime)/1000, (io_end_utime - io_start_utime)%1000, (io_end_utime - io_start_utime)/1000000, (io_end_utime - io_start_utime)%1000000  );



	fflush(stderr);
    fclose(logFp);

}

enum ThreadAction { BUILD_TREE_ACTION , CALCUL_FORCE_ACTION, UPDATE_DATA_ACTION , NOTTHING};
enum ThreadAction do_action = NOTTHING ;

void buildTreeAction(struct Task* task){
     int i=task->start;
     int count = task->count;
     int end = task->start + count;
     for ( ; i< end ;i++){
         if (i==0) continue; /// tree_root_node
         insertNode(&nBody[i], node);
          # if debug
         fprintf(logFp,"thread id=%d, insertNode:nBody[%d]  \n",task->id,i  );
         fflush(stderr);
            #endif
     }
}


void calculationForceAction(struct Task* task){

     int i=task->start;
     int count = task->count;
     int end = task->start + count;

     double xForce =0.0 , yForce=0.0;

     for ( ; i< end ;i++){
        xForce =0.0;
        yForce=0.0;
        computeForce( &nBody[i],node, &xForce,&yForce);


        newXV[i] = nBody[i].vx +  ( (xForce) * intervalTime / mass);
        newYV[i] = nBody[i].vy +  ( (yForce) * intervalTime / mass);

          # if debug
        fprintf(logFp,"\n [Force result] thread.id=%d, start=%d, count=%d, i=%d, xForce=%3.15lf, yForce=%3.15lf,  newXV=%3.15lf, newYV=%3.15lf \n", task->id, task->start,
               i,task->count,xForce,yForce,  newXV[i] , newYV[i]);
        fflush(stderr);
          # endif

     }

}

void updateLoactionSpeedAction(struct Task* task){
     int j=task->start;
     int count = task->count;
     int end = task->start + count;
     double tempX =0.0;
     double tempY =0.0;


     for ( ; j< end ;j++){
         tempX= nBody[j].x;
         tempY=nBody[j].y;
         nBody[j].vx =newXV[j];
         nBody[j].vy =newYV[j];
         nBody[j].x = nBody[j].x + (nBody[j].vx * intervalTime);
         nBody[j].y = nBody[j].y + (nBody[j].vy * intervalTime);

         pthread_mutex_lock(&globalVariable_lock_mutex);
         if (nBody[j].x > maxX){ maxX = nBody[j].x; }
         if (nBody[j].y > maxY){ maxY = nBody[j].y; }
         if (nBody[j].x < minX){ minX = nBody[j].x; }
         if (nBody[j].y < minY){ minY = nBody[j].y; }
         pthread_mutex_unlock(&globalVariable_lock_mutex);

            # if debug
         fprintf(logFp,"\n thread id=%d, draw j=%d ,(%3.15f,%3.15f) ==> new location (x=%3.15lf, y=%3.15lf), new speed (vx=%3.15lf, vy=%3.15lf) \n",task->id,j,tempX,tempY, nBody[j].x,  nBody[j].y,nBody[j].vx ,nBody[j].vy  );
         fflush(stderr);
            #endif
    }
}

void callAllThreadWait(){
   pthread_mutex_lock(&lock_mutex);
   if ( --numThreadAction == 0) {
         #if debug
       fprintf(logFp,"callAllThreadWait == 0, signify Main running. \n");
       fflush(stderr);
       #endif
       pthread_cond_signal(&main_cv);
   }
   pthread_cond_wait(&thread_cv, &lock_mutex);
   pthread_mutex_unlock(&lock_mutex);
}

void callMainWait(){
    pthread_mutex_lock(&lock_mutex);
    if ( numThreadAction > 0) {
         #if debug
        fprintf(logFp,"callMainWait, numThreadAction(%d) > 0 \n",numThreadAction);
        fflush(stderr);
         #endif
        pthread_cond_wait(&main_cv, &lock_mutex);
    }
    pthread_mutex_unlock(&lock_mutex);
}

void checkIFAllWait(){
    bool isWait = true;
    while (isWait){
       pthread_mutex_lock(&lock_mutex);
       if (numThreadAction <= 0){
           isWait= false;
           pthread_mutex_unlock(&lock_mutex);
       }else{
           fprintf(logFp,"Waiting ");
           fflush(stderr);
           isWait= true;
           pthread_mutex_unlock(&lock_mutex);
           usleep(1);
       }
    }
}

void callAllThreadDoAction(){
    //checkIFAllWait();
    pthread_mutex_lock(&lock_mutex);
    numThreadAction = numThread;
    pthread_cond_broadcast(&thread_cv);
    pthread_mutex_unlock(&lock_mutex);
}



void setDoAction(enum ThreadAction threadAction){
     pthread_mutex_lock(&lock_mutex);
     do_action = threadAction;
     pthread_mutex_unlock(&lock_mutex);
}


void threadAction(void *t){
     struct Task* task;
     task = (struct Task*) t;


     while(isRunning){

           callAllThreadWait();

           switch(do_action){

            case BUILD_TREE_ACTION:
                # if debug
                fprintf(logFp,"thread id=%d #buildTreeAction#.\n", task->id);
                fflush(stderr);
                #endif
                buildTreeAction(task);
                break;
			case CALCUL_FORCE_ACTION:
			    # if debug
                fprintf(logFp,"thread id=%d #calculationForceAction#.\n", task->id);
                fflush(stderr);
                #endif
				calculationForceAction(task);
				break;
			case UPDATE_DATA_ACTION:
			    # if debug
				fprintf(logFp,"thread id=%d #updateLoactionSpeedAction#.\n", task->id);
				fflush(stderr);
				#endif
				updateLoactionSpeedAction(task);
				break;
		    default:
                break;
		  }
     }


}

void initMaxMinXY(){
    maxX= -0.00000001;
    minX= 100000;
    maxY=-0.00000001;
    minY= 100000;
}


int pthreadNbody(int argc, char *argv[])
{

    logFp = freopen("./log.txt","wr", stderr);


    /**/
    getArguments(argc, &argv[0]);
    char *relativePath = ".\/";
    char *path = stringConcat( &relativePath[0],&file[0]);
    fprintf(logFp,"path=%s\n",path);
    freopen(path,"r",stdin);

    //freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody_Barnes-Hut\\test_data\\test1.txt","r",stdin);
    /**/




    /**
    ///test2
    numThread =1;
    mass = 10000;
    steps = 1000000;
    steps = 100000;
    intervalTime =0.01;
    theta = 0.0;

    xMin =-1.0;
    yMin =-1.0;
    xAxisLength =3.0;
    xWindowLength = 600;
    freopen("D:\\c\\codeblock\\c\\parallel_programming\\hw2\\nbody_Barnes-Hut\\test_data\\test1.txt","r",stdin);
    /**/



    int row = 0;
    scanf("%d",&row);
    num_Nbody = row;

    fprintf(logFp,"row=%d \n",row);
    fflush(stderr);

     # if valifyCalculation
    double  *originalVx= (double*) malloc (num_Nbody * sizeof(double));
    double  *originalVy= (double*) malloc (num_Nbody * sizeof(double));
    #endif

    nBody = (struct Body*) malloc (num_Nbody * sizeof(struct Body));

    int i=0,j=0;
    double x =0.0;
    double y =0.0;
    double vx=0.0;
    double vy=0.0;

    initMaxMinXY();


     gettimeofday(&io_tv, NULL);

    while( (--row) >=0 ){
       scanf("%lf %lf %lf %lf",&x ,&y, &vx , &vy );
       //fprintf(logFp,"i=%d, x=%3.15lf, y=%3.15lf, vx=%3.15lf, vy=%3.15lf, \n",i,x,y,vx , vy );
       //fflush(stderr);
       //scanf("%e %e %e %e",&x ,&y, &vx , &vy );
       //fprintf(logFp,"i=%d, x=%e, y=%e, vx=%e, vy=%e, \n",i,x,y,vx , vy );
       nBody[i].x=x;
       nBody[i].y=y;
       nBody[i].vx=vx;
       nBody[i].vy=vy;
       nBody[i].mass= mass;

        # if valifyCalculation
       originalVx[i]=vx;
       originalVy[i]=vy;
       #endif

       if (x > maxX){ maxX = x; }
       if (y > maxY){ maxY = y; }
       if (x < minX){ minX = x; }
       if (y < minY){ minY = y; }

       i++;
    }

    gettimeofday(&io_tv2, NULL);



    //showNBody(nBody, num_Nbody);

    /**/
    # if valifyCalculation
    double originalTotalVx =0.0;
    double originalTotalVy =0.0;
    double newTotalVx = 0.0;
    double newTotalVy = 0.0;
    aggregateVxVy(originalVx, originalVy ,num_Nbody, &originalTotalVx , &originalTotalVy );
     # endif
    /**/


     if (isEnable){
         # if xWindowEnable
         /**/
        initGraph(xWindowLength, xWindowLength);
        /**/  ///add_xWindow
        #endif
     }


     //fprintf(logFp,"num_Nbody=%d\n",num_Nbody);

     if (isEnable){
        for (i=0;i<num_Nbody;i++){
            //fprintf(logFp,"i=%d, ",i);
            draw(nBody[i].x,nBody[i].y);
        }
     }

     numThreadAction = numThread;
     pthread_mutex_init(&lock_mutex, NULL);
     pthread_mutex_init(&globalVariable_lock_mutex, NULL);

     pthread_cond_init (&thread_cv, NULL);
     pthread_cond_init (&main_cv, NULL);

     setDoAction(NOTTHING);

     threads = (pthread_t*) malloc( (numThread) * sizeof( pthread_t) );
     pthread_attr_t pattr;
     pthread_attr_init(&pattr);
     pthread_attr_setdetachstate(&pattr,PTHREAD_CREATE_JOINABLE);
     struct Task tasks[numThread];

     int data = num_Nbody/numThread;

     if ((num_Nbody % numThread) !=0 ){
        data = data + 1;
     }


     for (i=0;i<numThread;i++){
        tasks[i].id = i;
        tasks[i].start= i * data;
        if ( num_Nbody < ((i * data) + data) ){
            if (num_Nbody - (i * data) >= 0){
                tasks[i].count = num_Nbody - (i * data);
            }else{
                tasks[i].count = 0;
            }
        }else{
            tasks[i].count=data;
        }

        fprintf(logFp,"tasks[%d] start=%d, count=%d,\n",i,tasks[i].start, tasks[i].count );
     }

     for (j=0;j<numThread;j++){
          pthread_create(&threads[j],&pattr,threadAction,&tasks[j]);
      }




     double xForce = 0.0;
     double yForce = 0.0;
     newXV= (double*) malloc (num_Nbody * sizeof(double));
     newYV= (double*) malloc (num_Nbody * sizeof(double));
     double tempX =0.0;
     double tempY =0.0;




     for (i=0;i<steps;i++){

         printf("### Step%d ###, ",i);
         fprintf(logFp,"### Step%d ###, ",i);
         //printf("### Step%d, maxX=%3.15lf, minX=%3.15lf, maxY=%3.15lf, minY=%3.15lf \n",i, maxX, minX,maxY,minY);
         //fprintf(logFp,"### Step%d maxX=%3.15lf, minX=%3.15lf, maxY=%3.15lf, minY=%3.15lf \n",i, maxX, minX,maxY,minY);
         fflush(stderr);

         node = createTreeNode( &nBody[0],maxX,minX,maxY,minY); // create tree node.


         callMainWait();
         # if debug
         fprintf(logFp,"[Main] Ready to build quadrant tree. \n");
         fflush(stderr);
         #endif

         setDoAction(BUILD_TREE_ACTION);
         gettimeofday(&buildTree_tv, NULL);
         callAllThreadDoAction();


         callMainWait();
         gettimeofday(&buildTree_tv2, NULL);
         buildTree_start_utime = buildTree_tv.tv_sec * 1000000 + buildTree_tv.tv_usec;
         buildTree_end_utime = buildTree_tv2.tv_sec * 1000000 + buildTree_tv2.tv_usec;

         buildTree_utime = buildTree_utime + (buildTree_end_utime - buildTree_start_utime  );

          #if debug
         printf("[Main] finish built tree action: time = %llu.%03llu\n", (end_utime - start_utime)/1000, (end_utime - start_utime)%1000);
         fprintf(logFp,"[Main] finish built tree action: time = %llu.%03llu\n", (end_utime - start_utime)/1000, (end_utime - start_utime)%1000);
         fflush(stderr);
          # endif


         setDoAction(CALCUL_FORCE_ACTION);
         //gettimeofday(&tv, NULL);
         callAllThreadDoAction();
         callMainWait();
        // gettimeofday(&tv2, NULL);
         //start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
         //end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

          #if debug
         printf("[Main] finish calculation action: time = %llu.%03llu\n", (end_utime - start_utime)/1000, (end_utime - start_utime)%1000);
         fprintf(logFp,"[Main] finish calculation action: time = %llu.%03llu\n", (end_utime - start_utime)/1000, (end_utime - start_utime)%1000);
         fflush(stderr);
          # endif

           #if debug
         showArrayInfo( "newXV",newXV,num_Nbody);
         showArrayInfo( "newYV",newYV,num_Nbody);
          #endif




           if (isEnable){
              # if xWindowEnable
             clearArea(xWindowLength, xWindowLength);
              # endif
          }

         freeQuadrantNode(node);
         initMaxMinXY();

         setDoAction(UPDATE_DATA_ACTION);
         //gettimeofday(&tv, NULL);
         callAllThreadDoAction();
         callMainWait();
         //gettimeofday(&tv2, NULL);
         //start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
         //end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

          #if debug
         printf("[Main] finish update data action: time = %llu.%03llu\n", (end_utime - start_utime)/1000, (end_utime - start_utime)%1000);
         fprintf(logFp,"[Main] finish update data action: time = %llu.%03llu\n", (end_utime - start_utime)/1000, (end_utime - start_utime)%1000);
         fflush(stderr);
          # endif


         # if debug
         showNBody(nBody, num_Nbody);
         #endif

         /** show on xWindow**/
         if (isEnable){
            for (j=0;j<num_Nbody;j++){
                draw(nBody[j].x, nBody[j].y);
            }
         }



        if (isEnable){
             # if xWindowEnable
             /**/
            XFlush(display);
            /***////add_xWindow
            #endif

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
    #endif

    free(newXV);
    free(newYV);
    free(nBody);

    setDoAction(NOTTHING);
    pthread_mutex_lock(&lock_mutex);
    isRunning = false;
    pthread_mutex_unlock(&lock_mutex);
    callAllThreadDoAction();

    # if debug
    printf("Before Thread Join!!!!\n");
    fprintf(logFp,"Before Thread Join!!!!\n");
    fflush(stderr);
    #endif

    for (i = 0; i < (numThread); i++)
    {
        pthread_join(threads[i], NULL);
    }


    printf("Finish!!!!\n");
    fprintf(logFp,"Finish!!!!\n");
    fflush(stderr);
    //fclose(logFp);

    pthread_attr_destroy(&pattr);
    pthread_mutex_destroy(&globalVariable_lock_mutex);
    pthread_mutex_destroy(&lock_mutex);
    pthread_cond_destroy(&main_cv);
    pthread_cond_destroy(&thread_cv);

    //pthread_exit (NULL);
    return 0;
}

