
/** 
gcc River_and_Frog_Game_XLib.c -o River_and_Frog_Game_XLib.out -lX11  
**/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#define  ROW	7
int windowLength =650;
int windowWidth = 550;
int boardLength =0;
int boardWidth =0;
int divideLength = 0;
int divideWidth =0;
int isRunning = 1;
pthread_mutex_t count_mutex;

typedef struct {
    int x, y;
    int length,width;
} Frog;

typedef struct{
   int x, y;
   int length,width,speed , colorIndex;
} Wood;


typedef struct{
  Wood *woods;
  int count;
  int fromLeftToRight;
} Woods;

struct RowThread{
   int threadId;
  int rowIndex;
};


Woods  row_1_7_Woods[ROW];

Frog frog;
Display *display;
Window window;
XEvent event;
int screen;
GC  gc;
GC brownGc[7];
GC seaGc[7];
GC frogGc;
GC grassGc;
Colormap screenColormap;
XColor screenDefColor, exactDefColor;
///http://www.december.com/html/spec/color1.html
/// green #385E0F  ligt green #A2CD5A  ///blue #0147FA  #104E8B #4973AB



void initRowWood(){
    int i=0;
    int count = 3;
    int speed =1;
    int fromLeftToRight = 1;

    for (i=0;i< ROW ;i++){
        speed =1;
        count = 4;
        fromLeftToRight = 1;

        if (i ==6){
           speed = 2;
           count =4;
        }else if ( i== 5 ){
            speed = 3;
            count =3;
            fromLeftToRight = 0;
        }else if (i ==4){
            speed = 2;
            count =5;
        }else if (i==3){
            speed = 3;
            count =3;
            fromLeftToRight = 0;
        }else if (i ==2){
            speed =1;
            count =4;
        }else if (i ==1){
            speed =2;
            count =5;
            fromLeftToRight = 0;
        }


        row_1_7_Woods[i].fromLeftToRight = fromLeftToRight;
        row_1_7_Woods[i].count = count;
        row_1_7_Woods[i].woods = (Wood*) malloc( row_1_7_Woods[i].count* sizeof(Wood) );
        generateWoods( row_1_7_Woods[i].woods,row_1_7_Woods[i].count ,i+1,speed);
    }
}


int judgeWinOrLose(){
     //fprintf(stderr, "judgeWinOrLose#");
     fflush(stderr);
     int i=0;
     int j=0;

     //fprintf(stderr, "frog.y=%d, frog.length/2=%d, boardLength=%d\n",frog.y , (frog.length/2),boardLength  );
     //fflush(stderr);

     if ((frog.y + (frog.length/2)) < boardLength) {
        //fprintf(stderr, "in the destination...#");
        //fflush(stderr);
        return 1;  /// in destination
     }

     if ((frog.y + (frog.length/2)) >=  (boardLength * (ROW+1) )){
         //fprintf(stderr, "in the grass...%d >= %d #",(frog.y + (frog.length/2)),(boardLength * (ROW+1)) );
         //fflush(stderr);
         return 2; ///maybe in the grass!!
     }

     //fprintf(stderr, "1111#");
     //fflush(stderr);
     for (j=0; j<ROW;j++){

        for (i=0; i<row_1_7_Woods[j].count ; i++)
        {
            if ( row_1_7_Woods[j].woods[i].x <= frog.x && (row_1_7_Woods[j].woods[i].x+row_1_7_Woods[j].woods[i].width) >= frog.x &&
            row_1_7_Woods[j].woods[i].y <= (frog.y + (frog.length/2)) && ((row_1_7_Woods[j].woods[i].y+row_1_7_Woods[j].woods[i].length) >= (frog.y + (frog.length/2))) )
            {
                //fprintf(stderr, "in wood strips#");
                //fflush(stderr);
                return 2;  /// in board.

            }
        }
     }

    fprintf(stderr, "in river#");
    fflush(stderr);
     return 0; /// in river
}


void showMessage(int win){
    if (win == 1){
         isRunning = 0;
         XDrawString(display, window,gc, 50, 50, "You win the game!!", strlen("You win the game!!"));
    }else if (win ==0){
        isRunning = 0;
        XDrawString(display, window,gc, 50, 50 , "You lose the game!!", strlen("You lose the game!!"));
    }else if (win ==2){
        return;
    }

    XSync(display, False);
    sleep(1);
    //XDestroyWindow(display, window);
    return;
}

void judgeFrogBoundary(){
    if (frog.y <=0)  frog.y =0;
    if (frog.y >= (windowLength- frog.length)) frog.y=  windowLength - frog.length;
    if (frog.x <=0)  frog.x =0;
    if (frog.x>= (windowWidth- frog.width)) frog.x=  windowWidth - frog.width;
}



void initGrass(){

    XColor screenColor, exactColor;
    grassGc = XCreateGC(display, window, 0, 0);
    XColor grassCol;
    char grass[] = "#A2CD5A";
    XParseColor(display, screenColormap, grass, &grassCol);
    XAllocColor(display, screenColormap, &grassCol);
    XSetForeground(display, grassGc, grassCol.pixel);
}

void initfrog(){
    XColor screenColor, exactColor;
    frogGc = XCreateGC(display, window, 0, 0);
    XColor frogCol;
    char frog[] = "#385E0F";
    XParseColor(display, screenColormap, frog, &frogCol);
    XAllocColor(display, screenColormap, &frogCol);
    XSetForeground(display, frogGc, frogCol.pixel);
}

void initSea(){
    XColor screenColor, exactColor;
    seaGc[0] = XCreateGC(display, window, 0, 0);
    XColor seaCol;
    char sea[] = "#0147FA";
    XParseColor(display, screenColormap, sea, &seaCol);
    XAllocColor(display, screenColormap, &seaCol);
    XSetForeground(display, seaGc[0], seaCol.pixel);


     seaGc[1] = XCreateGC(display, window, 0, 0);
    char sea1[] = "#22316C";
    XParseColor(display, screenColormap, sea1, &seaCol);
    XAllocColor(display, screenColormap, &seaCol);
    XSetForeground(display, seaGc[1], seaCol.pixel);

     seaGc[2] = XCreateGC(display, window, 0, 0);
    char sea2[] = "#3232CC";
    XParseColor(display, screenColormap, sea2, &seaCol);
    XAllocColor(display, screenColormap, &seaCol);
    XSetForeground(display, seaGc[2], seaCol.pixel);

     seaGc[3] = XCreateGC(display, window, 0, 0);
    char sea3[] = "#00009C";
    XParseColor(display, screenColormap, sea3, &seaCol);
    XAllocColor(display, screenColormap, &seaCol);
    XSetForeground(display, seaGc[3], seaCol.pixel);

     seaGc[4] = XCreateGC(display, window, 0, 0);
    char sea4[] = "#0000FF";
    XParseColor(display, screenColormap, sea4, &seaCol);
    XAllocColor(display, screenColormap, &seaCol);
    XSetForeground(display, seaGc[4], seaCol.pixel);

     seaGc[5] = XCreateGC(display, window, 0, 0);
    char sea5[] = "#436EEE";
    XParseColor(display, screenColormap, sea5, &seaCol);
    XAllocColor(display, screenColormap, &seaCol);
    XSetForeground(display, seaGc[5], seaCol.pixel);

     seaGc[6] = XCreateGC(display, window, 0, 0);
    char sea6[] = "#7EB6FF";
    XParseColor(display, screenColormap, sea6, &seaCol);
    XAllocColor(display, screenColormap, &seaCol);
    XSetForeground(display, seaGc[6], seaCol.pixel);


}

void initWood(){
    XColor screenColor, exactColor;
    brownGc[0] = XCreateGC(display, window, 0, 0);
    XColor brownCol;
    char brown[] = "#5E2612";
    XParseColor(display, screenColormap, brown, &brownCol);
    XAllocColor(display, screenColormap, &brownCol);
    XSetForeground(display, brownGc[0], brownCol.pixel);

    char brown1[] = "#B13E0F";
    brownGc[1] = XCreateGC(display, window, 0, 0);
    XParseColor(display, screenColormap, brown1, &brownCol);
    XAllocColor(display, screenColormap, &brownCol);
    XSetForeground(display, brownGc[1], brownCol.pixel);


    char brown2[] = "#8B5742";
    brownGc[2] = XCreateGC(display, window, 0, 0);
    XParseColor( display, screenColormap, brown2 , &brownCol);
    XAllocColor(display, screenColormap, &brownCol);
    XSetForeground(display, brownGc[2], brownCol.pixel);


    char brown3[] = "#D19275";
    brownGc[3] = XCreateGC(display, window, 0, 0);
    XParseColor(display, screenColormap, brown3, &brownCol);
    XAllocColor(display, screenColormap, &brownCol);
    XSetForeground(display, brownGc[3], brownCol.pixel);


    char brown4[] = "#E47833";
    brownGc[4] = XCreateGC(display, window, 0, 0);
    XParseColor(display, screenColormap, brown4 ,&brownCol);
    XAllocColor(display, screenColormap, &brownCol);
    XSetForeground(display, brownGc[4], brownCol.pixel);

     char brown5[] = "#AA6600";
    brownGc[5] = XCreateGC(display, window, 0, 0);
    XParseColor(display, screenColormap, brown5 ,&brownCol);
    XAllocColor(display, screenColormap, &brownCol);
    XSetForeground(display, brownGc[5], brownCol.pixel);

     char brown6[] = "#B3432B";
    brownGc[6] = XCreateGC(display, window, 0, 0);
    XParseColor(display, screenColormap, brown6 ,&brownCol);
    XAllocColor(display, screenColormap, &brownCol);
    XSetForeground(display, brownGc[6], brownCol.pixel);

    initRowWood();

}



void generateWoods(Wood woods[], int count,int row  , int speed){

     printf("generateWoods");
    int i=0;


    int width = 0;
    if (count >=5){
        width = boardWidth * 2;
    }else if (count ==4){
        width =boardWidth *  3 ;
    }else if (count == 3){
         width =boardWidth *  4;
    }else{
         width = boardWidth * ((rand() % 3) +1);
    }

    int interval =  ((windowWidth - (width * count)) );


    for(i=0;i<count;i++){

        woods[i].colorIndex = (rand() % 7) ;
        woods[i].width=width;
        woods[i].length=(boardLength /3) * 2 ;
        woods[i].x= 0;
        woods[i].y=boardLength * row + ((boardLength - woods[i].length) /2);
        woods[i].speed = speed;


        if ( i> 0){
             woods[i].x = woods[i-1].x - woods[i].width - (interval/count)  ;
              //printf("woods[%d]=> x=%d \n",i, woods[i].x);
        }else{
            //woods[i].x =  windowLength - woods[i].width -  ((rand() % 5) +1) ;
            woods[i].x =  windowLength - woods[i].width -  5 ;
        }


        if (row ==1 && ( i ==0 || i ==1) ){
            printf("wood[%d]=%d , width=%d ##" ,i,woods[i].x,woods[i].width);
        }else{
             printf("AAAA");
        }


  }


}



int seaColorIndex = 0;
int drawNumber = 1;
int drawForgLength = 0;
int drawFrogY =0;

void drawWoodStripNoThread(){
    //XClearWindow( display, window );
    int i=0;
    int y = boardLength * 7;
    drawNumber++;
    if ((drawNumber  % 20 ) ==0 ){
        seaColorIndex = (rand() % 7);
    }


    Bool exposureEvent = False;
    XClearArea(display,window, 0,boardLength * (ROW+1), 0   , 0, exposureEvent );

    XClearArea(display,window, 0,0 , windowWidth   , boardLength, exposureEvent );



    int j=0;

    for (j=0; j< ROW ; j++ ){

         XClearArea(display,window, 0,  boardLength * (j+1) , 0 , boardLength * (j+1), exposureEvent );

        for (i=0; i<row_1_7_Woods[j].count ; i++){

            if (row_1_7_Woods[j].fromLeftToRight ==1 ){

                row_1_7_Woods[j].woods[i].x = row_1_7_Woods[j].woods[i].x+ row_1_7_Woods[j].woods[i].speed;

                if ( row_1_7_Woods[j].woods[i].x <= frog.x && (row_1_7_Woods[j].woods[i].x+row_1_7_Woods[j].woods[i].width) >= frog.x &&
                row_1_7_Woods[j].woods[i].y <= (frog.y + (frog.length/2)) && ((row_1_7_Woods[j].woods[i].y+row_1_7_Woods[j].woods[i].length) >= (frog.y + (frog.length/2))) ){
                    frog.x = frog.x+  row_1_7_Woods[j].woods[i].speed;
                    judgeFrogBoundary();
                }

                if (row_1_7_Woods[j].woods[i].x > windowWidth ) row_1_7_Woods[j].woods[i].x = 0 - row_1_7_Woods[j].woods[i].width;

            }else{

                row_1_7_Woods[j].woods[i].x = row_1_7_Woods[j].woods[i].x- row_1_7_Woods[j].woods[i].speed;

                if ( row_1_7_Woods[j].woods[i].x <= frog.x && (row_1_7_Woods[j].woods[i].x+row_1_7_Woods[j].woods[i].width) >= frog.x &&
                row_1_7_Woods[j].woods[i].y <= (frog.y + (frog.length/2)) && ((row_1_7_Woods[j].woods[i].y+row_1_7_Woods[j].woods[i].length) >= (frog.y + (frog.length/2))) ){
                    frog.x = frog.x-  row_1_7_Woods[j].woods[i].speed;
                    judgeFrogBoundary();



                }

                if (row_1_7_Woods[j].woods[i].x + row_1_7_Woods[j].woods[i].width < 0 ) row_1_7_Woods[j].woods[i].x = windowWidth;


            }
        }
    }


    usleep(10000);
    XFillRectangle(display, window, grassGc, 0, 0, windowWidth,boardLength);
    XFillRectangle(display, window, grassGc, 0, boardLength * (ROW+1), windowWidth,boardLength);
    for (j=0; j< ROW ; j++ ){
        XFillRectangle(display, window, seaGc[seaColorIndex], 0, boardLength * (j+1), windowWidth,boardLength);
        for (i=0; i<row_1_7_Woods[j].count; i++){
            XFillRectangle(display, window, brownGc[ row_1_7_Woods[j].woods[i].colorIndex ], row_1_7_Woods[j].woods[i].x, row_1_7_Woods[j].woods[i].y, row_1_7_Woods[j].woods[i].width, row_1_7_Woods[j].woods[i].length);
        }
    }

    drawForgLength = (int)(frog.length /2 );
    drawFrogY = frog.y + (int)(drawForgLength /2);

     XFillRectangle(display, window, frogGc, frog.x, drawFrogY ,  frog.width, drawForgLength );
     //XSync(display, 1);
     XFlush(display);

      int win =2;
      win = judgeWinOrLose();
      if (win != 2){
         showMessage(win);
      }


}


void *drawAction(void *data){


    //struct RowThread rowThread = * (struct RowThread*) data;
    //int row = rowThread.rowIndex;

    Bool exposureEvent = False;

    while(isRunning)
    {

        pthread_mutex_lock(&count_mutex);
        if (isRunning)
        {
            printf("drawWoodStrip");

            int i=0;


            int y = boardLength * 7;
            drawNumber++;
            if ((drawNumber  % 20 ) ==0 )
            {
                seaColorIndex = (rand() % 7);
            }

            XClearArea(display,window, 0,boardLength * (ROW+1), 0   , 0, exposureEvent );
            XClearArea(display,window, 0,0 , windowWidth   , boardLength, exposureEvent );



            int j=0;

            for (j=0; j< ROW ; j++ )
            {

                XClearArea(display,window, 0,  boardLength * (j+1) , 0 , boardLength * (j+1), exposureEvent );

                for (i=0; i<row_1_7_Woods[j].count ; i++)
                {

                    if (row_1_7_Woods[j].fromLeftToRight ==1 )
                    {

                        row_1_7_Woods[j].woods[i].x = row_1_7_Woods[j].woods[i].x+ row_1_7_Woods[j].woods[i].speed;

                        if ( row_1_7_Woods[j].woods[i].x <= frog.x && (row_1_7_Woods[j].woods[i].x+row_1_7_Woods[j].woods[i].width) >= frog.x &&
                                row_1_7_Woods[j].woods[i].y <= (frog.y + (frog.length/2)) && ((row_1_7_Woods[j].woods[i].y+row_1_7_Woods[j].woods[i].length) >= (frog.y + (frog.length/2))) )
                        {
                            frog.x = frog.x+  row_1_7_Woods[j].woods[i].speed;
                            judgeFrogBoundary();
                        }

                        if (row_1_7_Woods[j].woods[i].x > windowWidth ) row_1_7_Woods[j].woods[i].x = 0 - row_1_7_Woods[j].woods[i].width;

                    }
                    else
                    {

                        row_1_7_Woods[j].woods[i].x = row_1_7_Woods[j].woods[i].x- row_1_7_Woods[j].woods[i].speed;

                        if ( row_1_7_Woods[j].woods[i].x <= frog.x && (row_1_7_Woods[j].woods[i].x+row_1_7_Woods[j].woods[i].width) >= frog.x &&
                                row_1_7_Woods[j].woods[i].y <= (frog.y + (frog.length/2)) && ((row_1_7_Woods[j].woods[i].y+row_1_7_Woods[j].woods[i].length) >= (frog.y + (frog.length/2))) )
                        {
                            frog.x = frog.x-  row_1_7_Woods[j].woods[i].speed;
                            judgeFrogBoundary();



                        }

                        if (row_1_7_Woods[j].woods[i].x + row_1_7_Woods[j].woods[i].width < 0 ) row_1_7_Woods[j].woods[i].x = windowWidth;


                    }
                }
            }


            usleep(10000);
            XFillRectangle(display, window, grassGc, 0, 0, windowWidth,boardLength);
            XFillRectangle(display, window, grassGc, 0, boardLength * (ROW+1), windowWidth,boardLength);
            for (j=0; j< ROW ; j++ )
            {
                XFillRectangle(display, window, seaGc[seaColorIndex], 0, boardLength * (j+1), windowWidth,boardLength);
                for (i=0; i<row_1_7_Woods[j].count; i++)
                {
                    XFillRectangle(display, window, brownGc[ row_1_7_Woods[j].woods[i].colorIndex ], row_1_7_Woods[j].woods[i].x, row_1_7_Woods[j].woods[i].y, row_1_7_Woods[j].woods[i].width, row_1_7_Woods[j].woods[i].length);
                }
            }

            drawForgLength = (int)(frog.length /2 );
            drawFrogY = frog.y + (int)(drawForgLength /2);

            XFillRectangle(display, window, frogGc, frog.x, drawFrogY ,  frog.width, drawForgLength );
            //XSync(display, 1);
            XFlush(display);

            int win =2;
            win = judgeWinOrLose();
            if (win != 2)
            {
                showMessage(win);
            }

        }
        pthread_mutex_unlock(&count_mutex);
        usleep(500);
    }



}

void *eventAction(void *data){


     int win = 2;
    while (isRunning)
    {

        pthread_mutex_lock(&count_mutex);
        if (isRunning)
        {


            //XNextEvent(display, &event);
            if (XCheckWindowEvent(display, window, ButtonPressMask | ButtonReleaseMask | ExposureMask | ButtonMotionMask | PointerMotionMask | KeyPressMask, &event))
            {

                /// draw or redraw the window
                if (event.type == Expose)
                {
                    printf("##Expose##");
                }



                if (event.type == KeyPress)
                {
                    Window the_win = event.xkey.window;
                    KeySym keySymbol = XKeycodeToKeysym(display, event.xkey.keycode, 0);
                    if (keySymbol == XK_q)
                    {
                        isRunning = 0;
                        //break;
                    }
                    else if (keySymbol == XK_w)
                    {
                        fprintf(stderr, "press w\n");
                        frog.y= frog.y -boardLength;

                        judgeFrogBoundary();

                    }
                    else if (keySymbol == XK_s)
                    {
                        fprintf(stderr, "press s\n");
                        frog.y=frog.y +boardLength;

                        judgeFrogBoundary();

                    }
                    else if (keySymbol == XK_a)
                    {
                        fprintf(stderr, "press a\n");
                        frog.x= frog.x - boardWidth;

                        judgeFrogBoundary();

                    }
                    else if (keySymbol == XK_d)
                    {
                        fprintf(stderr, "press d\n");
                        frog.x= frog.x +boardWidth;

                        judgeFrogBoundary();
                    }
                }

            }
        }

        //drawWoodStrip();
        pthread_mutex_unlock(&count_mutex);
        usleep(500);
    }


}



int main() {

  srand(time(NULL));

  char *msg = "Hello, World!";


  /* open connection with the server */
  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }

  screen = DefaultScreen(display);
  screenColormap = DefaultColormap(display, screen);

  /* create window */
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 50, 0, windowWidth, windowLength,
       1, BlackPixel(display, screen), WhitePixel(display, screen));


   gc = XCreateGC(display, window, 0, 0);



   divideWidth = 9;
   divideWidth = 15;
   divideLength = 9;
   boardLength = (windowLength /divideLength);
   boardWidth = (windowWidth /divideWidth);

   frog.x= boardWidth *4 ;
    frog.x= boardWidth *8 ;
   frog.y= boardLength *8 ;
   frog.length = boardLength;
   frog.width =boardWidth;


   initWood();
   initfrog();
   initSea();
   initGrass();

   printf("4444\n");
  /* select kind of events we are interested in */
  XSelectInput(display, window, ExposureMask | KeyPressMask);

  printf("5555\n");
  /* map (show) the window */
  XMapWindow(display, window);

printf("6666\n");



   long t1=1, t2=2;
   pthread_t threads[2];
   pthread_attr_t attr;
   pthread_attr_init(&attr);

   struct RowThread rowThread1;
   struct RowThread rowThread2;




   rowThread1.threadId= 1;
   rowThread1.rowIndex= 1;

   rowThread2.threadId= 2;
   rowThread2.rowIndex= 2;

   pthread_mutex_init(&count_mutex, NULL);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   pthread_create(&threads[0], &attr, drawAction, (void *) &rowThread1);
   pthread_create(&threads[1], &attr, eventAction, (void *) &rowThread2);

   //pthread_create(&threads[1], &attr, drawWoodStrip, (void *)&rowThread7);

   int i=0;
   for (i = 0; i < 2; i++)
    {
        pthread_join(threads[i], NULL);
    }

    pthread_attr_destroy(&attr);
    pthread_mutex_destroy(&count_mutex);
    pthread_exit (NULL);
  /* close connection to server */
  XCloseDisplay(display);

  return 0;
}



int noTreadMain() {

  srand(time(NULL));

  char *msg = "Hello, World!";


  /* open connection with the server */
  display = XOpenDisplay(NULL);
  if (display == NULL) {
    fprintf(stderr, "Cannot open display\n");
    exit(1);
  }

  screen = DefaultScreen(display);
  screenColormap = DefaultColormap(display, screen);

  /* create window */
  window = XCreateSimpleWindow(display, RootWindow(display, screen), 50, 0, windowWidth, windowLength,
       1, BlackPixel(display, screen), WhitePixel(display, screen));


   gc = XCreateGC(display, window, 0, 0);



   divideWidth = 9;
   divideWidth = 15;
   divideLength = 9;
   boardLength = (windowLength /divideLength);
   boardWidth = (windowWidth /divideWidth);

   frog.x= boardWidth *4 ;
    frog.x= boardWidth *8 ;
   frog.y= boardLength *8 ;
   frog.length = boardLength;
   frog.width =boardWidth;


   initWood();
   initfrog();
   initSea();
   initGrass();

   printf("4444\n");
  /* select kind of events we are interested in */
  XSelectInput(display, window, ExposureMask | KeyPressMask);

  printf("5555\n");
  /* map (show) the window */
  XMapWindow(display, window);

printf("6666\n");



   long t1=1, t2=2;
   pthread_t threads[2];
   pthread_attr_t attr;
   pthread_attr_init(&attr);

   struct RowThread rowThread1;
   struct RowThread rowThread2;




   rowThread1.threadId= 1;
   rowThread1.rowIndex= 1;

   rowThread2.threadId= 2;
   rowThread2.rowIndex= 2;

   pthread_mutex_init(&count_mutex, NULL);
   pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
   pthread_create(&threads[0], &attr, drawAction, (void *) &rowThread1);

   //pthread_create(&threads[1], &attr, drawWoodStrip, (void *)&rowThread7);




  /* event loop */

  int win = 2;
  while (isRunning) {

    //XNextEvent(display, &event);
    if (XCheckWindowEvent(display, window, ButtonPressMask | ButtonReleaseMask | ExposureMask | ButtonMotionMask | PointerMotionMask | KeyPressMask, &event)) {

    /// draw or redraw the window
    if (event.type == Expose) {

       printf("##Expose##");
    }



    if (event.type == KeyPress){
        Window the_win = event.xkey.window;
        KeySym keySymbol = XKeycodeToKeysym(display, event.xkey.keycode, 0);
        if (keySymbol == XK_q) {
                isRunning = 0;
                //break;
        }else if (keySymbol == XK_w){
             fprintf(stderr, "press w\n");
             frog.y= frog.y -boardLength;

              /*
                win = judgeWinOrLose();
                if (win != 2){
                   showMessage(win);
                }*/

             judgeFrogBoundary();

        }else if (keySymbol == XK_s){
             fprintf(stderr, "press s\n");
             frog.y=frog.y +boardLength;


             judgeFrogBoundary();

        }else if (keySymbol == XK_a){
             fprintf(stderr, "press a\n");
             frog.x= frog.x - boardWidth;


             judgeFrogBoundary();

        }else if (keySymbol == XK_d){
             fprintf(stderr, "press d\n");
            frog.x= frog.x +boardWidth;

             judgeFrogBoundary();
        }
    }


    }

    drawWoodStripNoThread();

  }

  /* close connection to server */
  XCloseDisplay(display);

  return 0;
}


