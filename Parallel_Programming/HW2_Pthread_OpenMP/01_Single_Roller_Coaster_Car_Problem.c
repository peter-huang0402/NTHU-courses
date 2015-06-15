#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#define  NOT_NUMBER -987654321
#define  QUEUESIZE 1000
#define windowRemark 0
#define linuxRemark 0
#define logEnable 1
#define ScreenEnable 1


# if windowRemark
#include <windows.h> /// windows
# endif

FILE *logFp;
int numberOfPassengers;  /// 2<= n <=10
int capacityOfCar;
int timeOfOneJourney; /// millisecond , is a integer
int numberOfJourney;
int waitingQueue[QUEUESIZE];
int isRunning = 1;
int isFull =0;
int count =0;
int queueResetCv_Count =0;
int queueResetTime = 0;

int finishUnboard=0;

int wakeUpPassenger = 0;

pthread_t *threads;

pthread_mutex_t full_lock_mutex;
pthread_mutex_t lock_mutex;
pthread_mutex_t count_mutex;

pthread_cond_t carIsReady_cv;
pthread_cond_t boardCar_cv;
pthread_cond_t unboardCar_cv;
pthread_cond_t fullLoading_cv;

pthread_cond_t queueReset_cv;
pthread_cond_t queueReset_Result_cv;


pthread_cond_t *passengers_board_cv;
pthread_cond_t *passengers_board_ready_cv;

pthread_cond_t passengers_unboard_cv;

/// for calculating average waiting time
unsigned long long waitingDuration;
int waitingPeople = 0.0;



struct ThreadInfo{
    int id;
};


void carThreadAction();
void passengerThreadAction();

void initArray(int array[],int count){
    int i=0;
    for (i=0;i<count;i++){
        array[i]=NOT_NUMBER;
    }
}


/// In windows Sleep => milliseconds
/// In linux sleep => seconds
/// In linux usleep => microseconds  , usleep(1000 * ms) => milliseconds


char *stringConcat(char *str1, char *str2) {

    int length=strlen(str1)+strlen(str2)+1;

    char *result = (char*)malloc(sizeof(char) * length);

    // 複製第一個字串至新的陣列空間
    strcpy(result, str1);
    // 串接第二個字串至新的陣列空間
    strcat(result, str2);

    return result;
}

void *passengerAction(void *t){

    pthread_cleanup_push(pthread_mutex_unlock,&lock_mutex);


    struct timeval tv, tv2;
    unsigned long long start_utime, end_utime;


    struct ThreadInfo* threadInfo;
     threadInfo = (struct ThreadInfo*) t;



    int milliseconds = 1;
    int passengerDone = 0;
    while (isRunning){
        # if ScreenEnable
        printf("PassengerID=%d wanders around the park\n",threadInfo->id);
        #endif
        milliseconds =(rand()%1000)+1; /// 1~1000
        usleep(1000 * milliseconds); /// linux
        //Sleep(milliseconds);  ///windows

        pthread_mutex_lock(&lock_mutex);



        waitingQueue[count] = threadInfo->id;
        count++;
        # if ScreenEnable
        printf("#PassengerID=%d return for a ride. waitingQueue[%d]=%d \n",threadInfo->id,count,threadInfo->id);
        #endif
        gettimeofday(&tv, NULL);
        //printf("passenger id=%d ,wait to passengers_board_cv. \n",threadInfo->id);
        pthread_cond_wait(&passengers_board_cv[threadInfo->id], &lock_mutex);
        gettimeofday(&tv2, NULL);
        waitingPeople = waitingPeople+1;
        start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
        end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;
        waitingDuration = waitingDuration +(end_utime - start_utime);
        # if ScreenEnable
        printf("PassengerID=%d boards the car.\n", threadInfo->id);
        #endif
        //printf("passenger id=%d ,signal car passengers_board_ready_cv. \n",threadInfo->id);
        pthread_cond_signal(&passengers_board_ready_cv[ threadInfo->id]  );



        //printf("passenger id=%d wait unboardCar_cv \n",threadInfo->id );
        pthread_cond_wait(&unboardCar_cv, &lock_mutex);
        # if ScreenEnable
        printf("PassengerID=%d get off the car.\n", threadInfo->id);
        #endif
        //printf("passenger id=%d wait unboardCar_cv \n",threadInfo->id );
        //pthread_cond_wait(&unboardCar_cv, &lock_mutex);

        finishUnboard--;

        if(finishUnboard ==0){
             //printf("passenger id=%d signal passengers_unboard_cv \n",threadInfo->id );
             pthread_cond_signal(&passengers_unboard_cv);
        }
        # if srceenEnable
        printf("#PassengerId=%d have taken %d-th rides. \n",threadInfo->id,++passengerDone );
        #endif
        pthread_mutex_unlock(&lock_mutex);


         /*  ///round robin
         while (isResetAgain ){
                pthread_cond_wait(&queueReset_cv, &lock_mutex);
                printf("[queueReset_cv] passenger id=%d ,waitingQueue[%d]=%d \n",threadInfo->id,(count - queueResetCv_Count), waitingQueue[count - queueResetCv_Count]);
                if (threadInfo->id == waitingQueue[count - queueResetCv_Count]){
                      isResetAgain = 0;
                      queueResetCv_Count--;
                      queueResetTime++;
                }
                pthread_cond_signal(&queueReset_Result_cv);
            }*/



    }


     pthread_cleanup_pop(0);


}

void showWaitingQueue(){
    int i=0;
    for (i=0;i<count;i++){
        printf("waitingQueue[%d]=%d ,",i,waitingQueue[i]);
    }
     printf("\n");
}

void resetQueue(){

    //printf("#Before update# ");
    //showWaitingQueue();


    int i=capacityOfCar;
    for (i=capacityOfCar;i<count;i++){
        if ( (i-capacityOfCar) <0){
              printf("####Error Error ==> (i-capacityOfCar) <0 ##### i=%d, capacity=%d waitingQueue[i]=%d, \n",i,capacityOfCar,waitingQueue[i]);
        }else{
            waitingQueue[i-capacityOfCar] = waitingQueue[i];
        }
    }
    count = count - capacityOfCar;

    if (count < 0){
        count =0;
    }
    //printf("#After update# ");
    //showWaitingQueue();
}

void intTostring(int i, char *s) {
   sprintf(s,"%d",i);
}

char* getQueueIDString(){

   int i=0;
   int id=0;

   char result[10];
   char *result2;
   char *result3="";

   for (i=0;i<capacityOfCar;i++){
       id = waitingQueue[i];
       intTostring(id,result);
       if (i == (capacityOfCar-1) ){
          result3 = stringConcat(result3, result);
       }else{
          result2 = stringConcat(result,", ");
          result3 = stringConcat(result3, result2);
       }
   }

   int length=strlen(result3);
   char *newResult = (char*)malloc(sizeof(char) * length);
   strcpy(newResult, result3);

   //printf("result3=%s# \n",result3 );

   return newResult;
}

char* getTimeString(){

   char cBuffer[100];
   time_t zaman;
   struct tm *ltime;
   //static struct timeval _t;
   //static struct timezone tz;

    struct timeval _t;
    struct timezone tz;

   time(&zaman);
   ltime = (struct tm *) localtime(&zaman);
   gettimeofday(&_t, &tz);

   //strftime(cBuffer,40,"%d.%m.%y %H:%M:%S",ltime);
   strftime(cBuffer,40,"%H:%M:%S",ltime);
   sprintf(cBuffer, "%s.%d", cBuffer,(int)_t.tv_usec);

   //printf(" %s \n",cBuffer);
   return cBuffer;
}

void *carAction(void *t){
    time_t tt;
    struct tm *tm;
    struct ThreadInfo* threadInfo;
    threadInfo = (struct ThreadInfo*) t;
     int carDone = 0;
     int i=0;
     int index=0;
     struct timeval tv, tv2;
     char *idString;
     char *deparetureTimeString;
     char *arrivalTimeString;
     while (isRunning){


         int isfull=0;
         while (!isfull){
             //printf("car < capacityOfCar \n");
            if (count >= capacityOfCar){
                pthread_mutex_lock(&lock_mutex);
                # if screenEnable
                printf("Car Count(%d) >= capacityOfCar(%d) \n",count,capacityOfCar);
                # endif
                isfull =1;

                  i=capacityOfCar;
                  index=0;
                  wakeUpPassenger =0;
                  while( i>0){
                     //printf("Car signify passengers_cv #%d \n",waitingQueue[index]);
                     pthread_cond_signal(&passengers_board_cv[ waitingQueue[index]]  );
                     //printf("Car index=%d,i=%d \n",index,i);
                     pthread_cond_wait(&passengers_board_ready_cv[ waitingQueue[index]],&lock_mutex);
                     //printf("Car get passengers_board_ready_cv from %d \n",waitingQueue[index]);
                     index++;
                     i--;
                  }
                 pthread_mutex_unlock(&lock_mutex);
            }else{
                // pthread_mutex_unlock(&lock_mutex);
                 //usleep(1);
            }
         }


          tt = time(NULL);
	      tm= localtime(&tt);
	    //printf("now: %d-%d-%d %d:%d:%d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	      idString=getQueueIDString();
	      deparetureTimeString = getTimeString();
          //gettimeofday(&tv, NULL);
          #if screenEnable
          printf("Car departures at %s  with ID=%s . (#person=%d) \n",deparetureTimeString, idString,capacityOfCar);
          #endif
          usleep(1000 * timeOfOneJourney); /// linux
          arrivalTimeString = getTimeString();
          //Sleep(timeOfOneJourney);  ///windows
          #if screenEnable
          printf("Car arrive at %s and passenger ID=%s get off car.\n",arrivalTimeString,idString);
          #endif

          pthread_mutex_lock(&lock_mutex);
          finishUnboard = capacityOfCar;
          //printf("car id=%d broadcast unboardCar_cv \n",threadInfo->id );
          pthread_cond_broadcast(&unboardCar_cv);
          resetQueue();

          /*
          int record = 0;
          int tempQueueResetCv_Count = queueResetCv_Count;

          while(queueResetCv_Count !=0 && ((tempQueueResetCv_Count- queueResetCv_Count) != capacityOfCar) ){
            printf("car id=%d signal queueReset_cv, how many time #%d, tempQueueResetCv_Count=%d, queueResetCv_Count=%d, capacity=%d  \n",threadInfo->id,record,tempQueueResetCv_Count,queueResetCv_Count,capacityOfCar );
            pthread_cond_signal(&queueReset_cv);
            record++;
            pthread_cond_wait(&queueReset_Result_cv, &lock_mutex);
          }*/

          //printf("car id=%d wait all passenger off cars \n",threadInfo->id );
          pthread_cond_wait(&passengers_unboard_cv, &lock_mutex);

          printf("###Car have taken the %d-th journey.\n",++carDone);
          numberOfJourney--;
          if (numberOfJourney ==0 ) isRunning =0;
          pthread_mutex_unlock(&lock_mutex);


    }


    if (!isRunning ){
                // collecting waiting time
                pthread_mutex_lock(&lock_mutex);
                  i=count;
                  index=0;
                  wakeUpPassenger =0;
                  while( i>0){
                     //printf("Car signify passengers_cv #%d \n",waitingQueue[index]);
                     pthread_cond_signal(&passengers_board_cv[ waitingQueue[index]]  );
                     //printf("Car index=%d,i=%d \n",index,i);
                     pthread_cond_wait(&passengers_board_ready_cv[ waitingQueue[index]],&lock_mutex);
                     //printf("Car get passengers_board_ready_cv from %d \n",waitingQueue[index]);
                     index++;
                     i--;
                  }
                 pthread_mutex_unlock(&lock_mutex);


          int i=0;
          for (i=1;i< (numberOfPassengers+1);i++){
            pthread_cancel(threads[i]);
            printf("cancel passenger thread=%d \n",i);
         }
    }
}


void getArguments(int argc, char *argv[]){
    numberOfPassengers =atoi(argv[1]);
    capacityOfCar = atoi(argv[2]);
     timeOfOneJourney =atoi(argv[3]);
      numberOfJourney  =atoi(argv[4]);

    #if logEnable
    fprintf(logFp,"numberOfPassengers=%d, ",numberOfPassengers);
    fprintf(logFp,"capacityOfCar=%d, ",capacityOfCar);
    fprintf(logFp,"timeOfOneJourney=%d, ",timeOfOneJourney);
    fprintf(logFp,"numberOfJourney=%d, ",numberOfJourney);
    #endif


}




///a.out n C T N
int main(int argc, char *argv[])
{


    /*
     struct timeval tv, tv2;
	unsigned long long start_utime, end_utime;
	gettimeofday(&tv, NULL);
	start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
	printf("1 millisecond  = %llu \n", (tv.tv_usec )/1000);

	usleep(1000 * 5);
		gettimeofday(&tv, NULL);
	printf("2 millisecond  = %llu \n", (tv.tv_usec )/1000);

	printf("ta time = %llu.%03llu\n", (start_utime )/1000, (start_utime)%1000);
    return -1;
    */
      #if logEnable
    logFp = freopen("./log.txt","wr", stderr);
    #endif
    initArray(waitingQueue,QUEUESIZE);
    srand(time(NULL));

    /*
     numberOfPassengers = 8;
    capacityOfCar = 5;
    timeOfOneJourney = 1000;
    numberOfJourney = 30;
    */

    getArguments(argc, &argv[0]);




    int i=0;

    pthread_mutex_init(&full_lock_mutex, NULL);
    pthread_mutex_init(&lock_mutex, NULL);
    pthread_mutex_init(&count_mutex, NULL);


    pthread_cond_init (&carIsReady_cv, NULL);
    pthread_cond_init (&boardCar_cv, NULL);
    pthread_cond_init (&unboardCar_cv, NULL);

    pthread_cond_init (&fullLoading_cv, NULL);
    pthread_cond_init (&queueReset_cv, NULL);
    pthread_cond_init (&queueReset_Result_cv, NULL);
    pthread_cond_init (&passengers_unboard_cv, NULL);



    //pthread_t threads[numberOfPassengers+1];

    threads = (pthread_t*) malloc( (numberOfPassengers+1) * sizeof( pthread_t) );
    passengers_board_cv= (pthread_cond_t*) malloc( (numberOfPassengers+1) * sizeof( pthread_cond_t) );
    passengers_board_ready_cv= (pthread_cond_t*) malloc( (numberOfPassengers+1) * sizeof( pthread_cond_t) );
    pthread_cond_init (&passengers_board_cv[0], NULL); // no use!!
    pthread_cond_init (&passengers_board_ready_cv[0], NULL);// no use!!


    pthread_attr_t pattr;
    pthread_attr_init(&pattr);
    pthread_attr_setdetachstate(&pattr,PTHREAD_CREATE_JOINABLE);

    struct ThreadInfo threadInfos[numberOfPassengers+1];

    threadInfos[0].id = 0;
    pthread_create(&threads[0],&pattr,carAction,&threadInfos[0]);

    for (i=1;i< (numberOfPassengers+1);i++){
        threadInfos[i].id = i;
        pthread_cond_init (&passengers_board_cv[i], NULL);
        pthread_cond_init (&passengers_board_ready_cv[i], NULL);
        pthread_create(&threads[i],&pattr,passengerAction,&threadInfos[i]);
    }


     for (i = 0; i < (numberOfPassengers+1); i++)
    {
        pthread_join(threads[i], NULL);
    }


    pthread_mutex_lock(&lock_mutex);
    printf("Total Waiting time: Gettimeofday time(%llu) = %llu.%03llu milisecond;  %llu.%06llu sec \n",waitingDuration, waitingDuration/1000, waitingDuration%1000, waitingDuration/1000000, waitingDuration%1000000  );
    printf("Average Waiting time: (persons=%d), Gettimeofday time = %llu.%03llu milisecond;  %llu.%06llu sec \n", waitingPeople, (waitingDuration/waitingPeople) /1000 , (waitingDuration/waitingPeople) %1000, (waitingDuration/waitingPeople) /1000000, (waitingDuration/waitingPeople) %1000000  );

     #if logEnable
     fprintf(logFp,"Total Waiting time: Gettimeofday time(%llu) = %llu.%03llu milisecond;  %llu.%06llu sec \n",waitingDuration, waitingDuration/1000, waitingDuration%1000, waitingDuration/1000000, waitingDuration%1000000  );
    fprintf(logFp,"Average Waiting time: (persons=%d), Gettimeofday time = %llu.%03llu milisecond;  %llu.%06llu sec \n", waitingPeople, (waitingDuration/waitingPeople) /1000 , (waitingDuration/waitingPeople) %1000, (waitingDuration/waitingPeople) /1000000, (waitingDuration/waitingPeople) %1000000  );
    fflush(stderr);
     #endif
    pthread_mutex_unlock(&lock_mutex);




    printf("Finish!!!\n");
     #if logEnable
    fflush(stderr);
    fclose(logFp);
     #endif


     free(threads);

    pthread_attr_destroy(&pattr);
    pthread_mutex_destroy(&lock_mutex);
    pthread_cond_destroy(&boardCar_cv);
    pthread_cond_destroy(&unboardCar_cv);

    pthread_exit (NULL);

    return 0;
}

