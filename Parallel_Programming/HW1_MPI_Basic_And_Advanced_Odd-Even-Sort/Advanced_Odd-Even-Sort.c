#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <unistd.h> /// linux
#include <sys/time.h>
#include <time.h>
//#include <windows.h>  /// windows
#define debug 0
#define  MASTER		0
#define  NOT_NUMBER -987654321
#define MY_RESULTS_FILE "my_results"

//32位 	-2,147,483,648 至 2,147,483,647
//-2147483648  2147483647

clock_t startTime;
clock_t endTime;
clock_t ioStartTime;
clock_t ioEndTime;
double ioTime = 0.0;

clock_t commuteStartTime;
clock_t commuteEndTime;
double commuteTime = 0.0;



struct timeval startT,endT, ioStartT, ioEndT, commuteStartT , commuteEndT;
double commuteT = 0.0, spanT = 0.0;



void initNumber(int*, int );
int cmpfunc (const void * , const void * );




void initNumber(int *buf, int bufsize){
    int i=0;
    for(i=0;i<bufsize;i++){
        buf[i]=NOT_NUMBER;
    }
}


int cmpfunc (const void * a, const void * b){
    // 1.a>b , 0.a=b -1. a<b
    int c,d;
    c = *(int*)a;
    d = *(int*)b;

    if (c>d) return 1;
    else if(c ==d) return 0;
    else return -1;
}




void mergeArray(int *result,int *a,int *b, int aSize, int bSize){
    int i = 0;
    int j = 0;
    int k = 0;
    // Merging starts
    while (i < aSize && j < bSize) {
        if (a[i] <= b[j]) {
            result[k] = a[i];
            i++;
            k++;
        } else {
            result[k] = b[j];
            k++;
            j++;
        }
    }

    // Some elements in array 'arr1' are still remaining
	 // where as the array 'arr2' is exhausted */
	while (i < aSize) {
		result[k] = a[i];
		i++;
		k++;
	}

	// Some elements in array 'arr2' are still remaining
	 //where as the array 'arr1' is exhausted
	while (j < bSize) {
		result[k] = b[j];
		k++;
		j++;
	}


}

int myrank, numprocs, countSize,  count, len,rc;
char hostname[MPI_MAX_PROCESSOR_NAME];
MPI_File thefile;
MPI_Status status;
MPI_Offset filesize;
FILE *fp;


int oldSwap = 1;
int subSwap = 0;
int swap =0;
int comingSwap = 0;
int isConverge = 0;
int tag1 =1, tag2=2;
char init_Tag[40] =  "Init Array";
char merge_Tag[40] = "Merge Array";
int sendRevCount =0;
int firstIndexForBasicTranspose=0;
int lastIndexForBasicTranspose=0;
int skipEvenFirstIndex = 0;
int skipEvenLastIndex =0;
int skipOddFirstIndex = 0;
int skipOddLastIndex =0;
int firstSwap=0;
int lastSwap=0;


void showArrayInfo(char info[],int *buf, int bufsize){
    int i=0;
    for(i=0;i<bufsize;i++){
        printf("[%s] hostname=%s, myrank=%d, buf[%d]=%d,\n",info,hostname,myrank,i,buf[i]);
        fprintf( fp, "[%s] hostname=%s, myrank=%d, buf[%d]=%d,\n",info, hostname,myrank,i,buf[i]);
        fflush(stderr);
    }
}

void initBasicTraspose(){
   firstIndexForBasicTranspose = (myrank * countSize);
   if (count <= 1){
       lastIndexForBasicTranspose = firstIndexForBasicTranspose;
   }else{
        lastIndexForBasicTranspose = (myrank * countSize)+count -1 ;
   }

   if ((firstIndexForBasicTranspose % 2)==1){
        skipEvenFirstIndex = 1;
   }else{
        skipOddFirstIndex = 1;
   }

   if ((lastIndexForBasicTranspose %2) ==0){
       skipEvenLastIndex =1;
   }else{
       skipOddLastIndex =1;
   }


    # if debug
   printf("[initBasicTraspose] Name=%s, rank=%d\n, firstIndex=%d, lastIndex=%d, skipEvenFirst=%d, skipEvenLast=%d, skipOddFirst=%d, skipOddLast=%d \n",hostname,myrank,firstIndexForBasicTranspose,
          lastIndexForBasicTranspose,skipEvenFirstIndex,skipEvenLastIndex,skipOddFirstIndex,skipOddLastIndex);
   fprintf(fp,"[initBasicTraspose] Name=%s, rank=%d\n, firstIndex=%d, lastIndex=%d, skipEvenFirst=%d, skipEvenLast=%d, skipOddFirst=%d, skipOddLast=%d \n",hostname,myrank,firstIndexForBasicTranspose,
           lastIndexForBasicTranspose,skipEvenFirstIndex,skipEvenLastIndex,skipOddFirstIndex,skipOddLastIndex);
   fflush(stderr);
   # endif // debug

}

void swapArray(int data[],int index1,int index2){
    int temp =0;
    temp = data[index1];
    data[index1] = data[index2];
    data[index2] = temp;
}


void doBasicEvenTranspose(int *buf, int *inBuf){
    # if debug
   showArrayInfo( "doBasicEvenTranspose **** buf Array ****",buf,count);
   showArrayInfo( "doBasicEvenTranspose **** inBuf Array ****",inBuf,countSize);
 # endif // debug

   int temp =NOT_NUMBER;
   MPI_Request request;

   subSwap =0;
   lastSwap =0 ;
   firstSwap =0;
   if (countSize ==1 ){
       /// because each process only has the ONLY one data in each process,
       /// its behavior is similar as doAdvanceEvenTraspose();

        # if debug
       printf("[doAdvanceEvenTranspose] countSize =1, Name=%s, rank=%d\n",hostname,myrank);
       fprintf(fp,"[doAdvanceEvenTranspose] countSize =1, Name=%s, rank=%d\n",hostname,myrank);
      fflush(stderr);
        # endif // debug
       doAdvanceEvenTranspose(buf,inBuf);
       return;
   }

   if (count ==0 ){
       ///do coverge
        /// even order!! but it is belong the last process and it cannot swap by others.

         # if debug
        printf("[Basic Even  MPI_Reduce(4), process no work] Name=%s, rank=%d\n",hostname,myrank);
        fprintf(fp,"[Basic Even MPI_Reduce(4), process no work] Name=%s, rank=%d\n",hostname,myrank);
        fflush(stderr);
          # endif // debug
        subSwap = 0;
        commuteStartTime = clock();
         gettimeofday(&commuteStartT,NULL);
        rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
        commuteEndTime = clock();
        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

         gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;

        commuteStartTime = clock();
         gettimeofday(&commuteStartT,NULL);
        rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
        commuteEndTime = clock();
        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

         gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;

        MPI_Barrier(MPI_COMM_WORLD);
       return;
   }

   if (skipEvenFirstIndex){
       # if debug
      printf("[do EvenFirstIndex] Name=%s, rank=%d, \n",hostname,myrank);
      fprintf(fp,"[do EvenFirstIndex] Name=%s, rank=%d, \n",hostname,myrank);
      fflush(stderr);
  # endif // debug

      /// do receive from previous process, and send back
        commuteStartTime = clock();
         gettimeofday(&commuteStartT,NULL);
      rc = MPI_Recv(inBuf, 1, MPI_INT, (myrank-1), tag1, MPI_COMM_WORLD, &status);
      commuteEndTime = clock();
      commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

       gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;


      if (rc != MPI_SUCCESS) {
             printf("[Even First Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
             fprintf(stderr, "[Even First Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
             fflush(stderr);
             MPI_Abort(MPI_COMM_WORLD, 3);
      }

      MPI_Get_count(&status, MPI_INT, &sendRevCount);
        # if debug
      printf("[even first recv] Name=%s, rank=%d, revCount=%d, inBuf[0]=%d \n",hostname,myrank,sendRevCount,inBuf[0]);
      fprintf(fp,"[even first recv] Name=%s, rank=%d, revCount=%d, inBuf[0]=%d \n",hostname,myrank,sendRevCount,inBuf[0]);
      fflush(stderr);
       # endif // debug



      if (count >0){
              # if debug
            printf("[even first recv] Name=%s, rank=%d, buf[0]=%d, inBuf[0]=%d \n",hostname,myrank,buf[0],inBuf[0]);
            fprintf(fp,"[even first recv] Name=%s, rank=%d, buf[0]=%d, inBuf[0]=%d \n",hostname,myrank,buf[0],inBuf[0]);
            fflush(stderr);
        # endif // debug
            if (buf[0] < inBuf[0])
            {
                  # if debug
                printf("[Even first swap] hostname=%s, myrank=%d, buf[0]=%d, swap with inBuf[0]=%d,\n",hostname,myrank,buf[0],inBuf[0]);
                fprintf( fp, "[Even first swap] hostname=%s, myrank=%d, buf[0]=%d, swap with inBuf[0]=%d,\n",hostname,myrank,buf[0],inBuf[0]);
                fflush(stderr);
                 # endif // debug
                firstSwap=1;
                temp = buf[0];
                buf[0]=inBuf[0];
                inBuf[0] = temp;
            }else{
                firstSwap =0;
            }
        }else{
            firstSwap=0;
        }

           # if debug
        printf("[even first send Back value] Name=%s, rank=%d, inBuf[0]=%d \n",hostname,myrank,inBuf[0]);
        fprintf(fp,"[even first send Back value] Name=%s, rank=%d, inBuf[0]=%d \n",hostname,myrank,inBuf[0]);
        fflush(stderr);
         # endif // debug
        commuteStartTime = clock();
         gettimeofday(&commuteStartT,NULL);
        //rc = MPI_Send( inBuf, 1, MPI_INT, (myrank-1), tag2, MPI_COMM_WORLD);
        rc = MPI_Isend( inBuf, 1, MPI_INT, (myrank-1), tag2, MPI_COMM_WORLD, &request);
        commuteEndTime = clock();
        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

         gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;



        if (rc != MPI_SUCCESS)
        {
            printf("[Even first Send Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
            fprintf(stderr, "[Even first Sent Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
            fflush(stderr);
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
   }


   if (skipEvenLastIndex){
         # if debug
        printf("[do EvenLastIndex] Name=%s, rank=%d, \n",hostname,myrank);
      fprintf(fp,"[do EvenLastIndex] Name=%s, rank=%d, \n",hostname,myrank);
      fflush(stderr);
       # endif // debug

        /// do send to next process, and receive form next process.
      if ( ((myrank == (numprocs -1))) ||  ((myrank*countSize+count) >= filesize)  ){

               # if debug
             printf("[do Nothing in EvenLastIndex] Name=%s, rank=%d, \n",hostname,myrank);
             fprintf(fp,"[do Nothing in EvenLastIndex] Name=%s, rank=%d, \n",hostname,myrank);
             fflush(stderr);
              # endif // debug

          /// not switch because it's belong the last processor
          /// not switch because it's belong the last data
      }else{
             # if debug
            printf("[Even Last Send before (1)] Name=%s, rank=%d\n",hostname,myrank);
            fprintf(fp,"[Even Last Send before (1)] Name=%s, rank=%d\n",hostname,myrank);
            fflush(stderr);
             # endif // debug

             commuteStartTime = clock();
              gettimeofday(&commuteStartT,NULL);
            //rc = MPI_Send(&buf[count-1], 1, MPI_INT, (myrank+1), tag1, MPI_COMM_WORLD);
             rc = MPI_Isend(&buf[count-1], 1, MPI_INT, (myrank+1), tag1, MPI_COMM_WORLD,&request);
            commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

             gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;


            if (rc != MPI_SUCCESS)
            {
                printf("[Even Last Send Error (1)] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fprintf(stderr, "[Even Last Sent Error (1)] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fflush(stderr);
                MPI_Abort(MPI_COMM_WORLD, 3);
            }

              # if debug
            printf("[Even Last Recv before (2)] Name=%s, rank=%d\n",hostname,myrank);
            fprintf(fp,"[Even Last Recv before(2)] Name=%s, rank=%d\n",hostname,myrank);
            fflush(stderr);
             # endif // debug

            commuteStartTime = clock();
             gettimeofday(&commuteStartT,NULL);
            rc = MPI_Recv(inBuf, 1, MPI_INT, (myrank+1), tag2, MPI_COMM_WORLD, &status);
            commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

             gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;



            if (rc != MPI_SUCCESS)
            {
                printf("[Even MPI Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fprintf(stderr, "[Even MPI Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fflush(stderr);
                MPI_Abort(MPI_COMM_WORLD, 3);
            }

            MPI_Get_count(&status, MPI_INT, &sendRevCount);
             # if debug
            printf("[even last recv 1] Name=%s, rank=%d, revCount=%d, inBuf[0]=%d \n",hostname,myrank,sendRevCount,inBuf[0]);
            fprintf(fp,"[even last recv 1] Name=%s, rank=%d, revCount=%d, inBuf[0]=%d \n",hostname,myrank,sendRevCount,inBuf[0]);
            fflush(stderr);
            # endif // debug
            if (count >0)
            {
                  # if debug
                printf("[even last recv 2] Name=%s, rank=%d, buf[%d]=%d, inBuf[0]=%d \n",hostname,myrank,count-1,buf[count-1],inBuf[0]);
                fprintf(fp,"[even last recv 2] Name=%s, rank=%d, buf[%d]=%d, inBuf[0]=%d \n",hostname,myrank,count-1,buf[count-1],inBuf[0]);
                fflush(stderr);
                  # endif // debug
                if (buf[count-1] > inBuf[0])
                {
                     # if debug
                    printf("[Even last swap] hostname=%s, myrank=%d, buf[%d]=%d swap with inBuf[0]=%d,\n",hostname,myrank,(count-1),buf[count-1],inBuf[0]);
                    fprintf( fp, "[Even last swap] hostname=%s, myrank=%d, buf[%d]=%d swap with inBuf[0]=%d,\n",hostname,myrank,(count-1),buf[count-1],inBuf[0]);
                    fflush(stderr);
                     # endif // debug
                    lastSwap=1;
                    buf[count-1]=inBuf[0];
                }else{
                    lastSwap =0;
                }
            }else{
                lastSwap=0;
            }
      }
   }



   int i=0;
   for (i=0;i<(count-1);i++){
      if (i==0  && skipEvenFirstIndex){
              # if debug
        printf("[Even swap In Process (skip first)] Name=%s, rank=%d, skip buf[0]=%d ,index=%d, \n",hostname,myrank,buf[i],(myrank*countSize+i));
        fprintf(fp,"[Even swap In Process (skip first)] Name=%s, rank=%d, skip buf[0]=%d ,index=%d, \n",hostname,myrank,buf[i],(myrank*countSize+i));
        fflush(stderr);
           # endif // debug
        continue;
      }

      /*  Never go through.
      if (i == (count-1) && skipEvenLastIndex){
        continue;
      }*/

      if (buf[i] > buf[i+1]){
             # if debug
        printf("[Even swap In Process(1)] Name=%s, rank=%d, swap(%d,%d),buf value(%d,%d),index(%d,%d) \n",hostname,myrank,i,i+1,buf[i],buf[i+1],(myrank*countSize+i),(myrank*countSize+i+1) );
        fprintf(fp,"[Even swap In Process(1)] Name=%s, rank=%d, swap(%d,%d),buf value(%d,%d),index(%d,%d) \n",hostname,myrank,i,i+1,buf[i],buf[i+1],(myrank*countSize+i),(myrank*countSize+i+1) );
        fflush(stderr);
            # endif // debug
        swapArray(buf,i,i+1);
        subSwap =1;
      }
      i++;
   }

     if (firstSwap+lastSwap+subSwap >0){
        subSwap =1;
     }else{
         subSwap =0;
     }
        # if debug
    printf("[MPI_Reduce] Name=%s, rank=%d, \n",hostname,myrank);
    fprintf(fp,"[MPI_Reduce] Name=%s, rank=%d, \n",hostname,myrank);
    fflush(stderr);
     # endif // debug

    commuteStartTime = clock();
     gettimeofday(&commuteStartT,NULL);
    rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
    commuteEndTime = clock();
    commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

     gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;


    if (rc != MPI_SUCCESS)
    {
        printf("[Even basic MPI_Reduce Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
        fprintf(stderr, "[Even basic MPI MPI_Reduce 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
        fflush(stderr);
        MPI_Abort(MPI_COMM_WORLD, 3);
    }

    if (myrank == MASTER)
    {
        if (comingSwap == 0 &&  oldSwap ==0 )
        {
            isConverge = 1;

              commuteStartTime = clock();
               gettimeofday(&commuteStartT,NULL);
            rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
             commuteEndTime = clock();
             commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

              gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;

        }
        else
        {
            if (comingSwap >0)
            {
                oldSwap = 1;
            }
            else
            {
                oldSwap =0;
            }
            isConverge =0;

             # if debug
            printf("[Even basic MPI_Bcast 1] Name=%s, rank=%d, \n",hostname,myrank);
            fprintf(fp,"[Even basic MPI_Bcast 1] Name=%s, rank=%d, \n",hostname,myrank);
            fflush(stderr);
              # endif // debug

               commuteStartTime = clock();
                gettimeofday(&commuteStartT,NULL);
            rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
             commuteEndTime = clock();
             commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

              gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;

        }

        # if debug
        printf("[Even basic comingSwap] comingSwap=%d,\n",comingSwap);
        fprintf( fp, "[Even basic comingSwap] comingSwap=%d,\n",comingSwap);
        fflush(stderr);
          # endif // debug

        MPI_Barrier(MPI_COMM_WORLD);
    }
    else{
             # if debug
        printf("[Even basic MPI_Bcast 2] Name=%s, rank=%d, \n",hostname,myrank);
        fprintf(fp,"[Even basic MPI_Bcast 2] Name=%s, rank=%d, \n",hostname,myrank);
        fflush(stderr);
             # endif // debug

            commuteStartTime = clock();
             gettimeofday(&commuteStartT,NULL);
        rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
            commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

             gettimeofday(&commuteEndT,NULL);
      spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
      commuteT = commuteT + spanT;


        MPI_Barrier(MPI_COMM_WORLD);
    }

}


void doBasicOddTranspose(int *buf, int *inBuf){
       # if debug
   showArrayInfo( "doBasicOddTranspose **** buf Array ****",buf,count);
   showArrayInfo( "doBasicOddTranspose **** inBuf Array ****",inBuf,countSize);
      # endif // debug
   int temp =NOT_NUMBER;


   subSwap =0;
   lastSwap =0 ;
   firstSwap =0;
   MPI_Request request;


   if (countSize ==1 ){
       /// because each process only has the ONLY one data in each process,
       /// its behavior is similar as doAdvanceOddTraspose();
          # if debug
       printf("[doAdvanceOddTranspose] countSize =1, Name=%s, rank=%d\n",hostname,myrank);
       fprintf(fp,"[doAdvanceOddTranspose] countSize =1, Name=%s, rank=%d\n",hostname,myrank);
       fflush(stderr);
         # endif // debug
       doAdvanceOddTranspose(buf,inBuf);
       return;
   }

   if (count ==0 ){
       ///do coverge
        /// odd order!! but it is belong the last process and it cannot swap by others.
         # if debug
        printf("[Basic Odd  MPI_Reduce(4), process no work] Name=%s, rank=%d\n",hostname,myrank);
        fprintf(fp,"[Basic Odd MPI_Reduce(4), process no work] Name=%s, rank=%d\n",hostname,myrank);
        fflush(stderr);
          # endif // debug
        subSwap = 0;
        commuteStartTime = clock();
          gettimeofday(&commuteStartT,NULL);
        rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
        commuteEndTime = clock();
        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

        gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;


        commuteStartTime = clock();
          gettimeofday(&commuteStartT,NULL);
        rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
        commuteEndTime = clock();
        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

        gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

        MPI_Barrier(MPI_COMM_WORLD);
       return;
   }

   if (skipOddFirstIndex && (myrank !=0) ){
        # if debug
      printf("[do OddFirstIndex] Name=%s, rank=%d, \n",hostname,myrank);
      fprintf(fp,"[do OddFirstIndex] Name=%s, rank=%d, \n",hostname,myrank);
      fflush(stderr);
        # endif // debug

      /// do receive from previous process, and send back
      commuteStartTime = clock();
        gettimeofday(&commuteStartT,NULL);
      rc = MPI_Recv(inBuf, 1, MPI_INT, (myrank-1), tag1, MPI_COMM_WORLD, &status);
      commuteEndTime = clock();
      commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

      gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;


      if (rc != MPI_SUCCESS) {
             printf("[Odd First Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
             fprintf(stderr, "[Odd First Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
             fflush(stderr);
             MPI_Abort(MPI_COMM_WORLD, 3);
      }

      MPI_Get_count(&status, MPI_INT, &sendRevCount);
      # if debug
      printf("[Odd first recv] Name=%s, rank=%d, revCount=%d, inBuf[0]=%d \n",hostname,myrank,sendRevCount,inBuf[0]);
      fprintf(fp,"[Odd first recv] Name=%s, rank=%d, revCount=%d, inBuf[0]=%d \n",hostname,myrank,sendRevCount,inBuf[0]);
      fflush(stderr);
      # endif // debug


      if (count >0){
             # if debug
            printf("[Odd first recv] Name=%s, rank=%d, buf[0]=%d, inBuf[0]=%d \n",hostname,myrank,buf[0],inBuf[0]);
            fprintf(fp,"[Odd first recv] Name=%s, rank=%d, buf[0]=%d, inBuf[0]=%d \n",hostname,myrank,buf[0],inBuf[0]);
            fflush(stderr);
            # endif // debug
            if (buf[0] < inBuf[0])
            {
                # if debug
                printf("[Odd first swap] hostname=%s, myrank=%d, buf[0]=%d, swap with inBuf[0]=%d,\n",hostname,myrank,buf[0],inBuf[0]);
                fprintf( fp, "[Odd first swap] hostname=%s, myrank=%d, buf[0]=%d, swap with inBuf[0]=%d,\n",hostname,myrank,buf[0],inBuf[0]);
                fflush(stderr);
                  # endif // debug
                firstSwap=1;
                temp = buf[0];
                buf[0]=inBuf[0];
                inBuf[0] = temp;
            }else{
                firstSwap =0;
            }
        }else{
            firstSwap=0;
        }
          # if debug
        printf("[Odd first send Back value] Name=%s, rank=%d, inBuf[0]=%d \n",hostname,myrank,inBuf[0]);
        fprintf(fp,"[Odd first send Back value] Name=%s, rank=%d, inBuf[0]=%d \n",hostname,myrank,inBuf[0]);
        fflush(stderr);
          # endif // debug

          commuteStartTime = clock();
            gettimeofday(&commuteStartT,NULL);
        //rc = MPI_Send( inBuf, 1, MPI_INT, (myrank-1), tag2, MPI_COMM_WORLD);
        rc = MPI_Isend( inBuf, 1, MPI_INT, (myrank-1), tag2, MPI_COMM_WORLD,&request);
        commuteEndTime = clock();
        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

       gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

        if (rc != MPI_SUCCESS)
        {
            printf("[Odd first Send Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
            fprintf(stderr, "[Odd first Sent Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
            fflush(stderr);
            MPI_Abort(MPI_COMM_WORLD, 3);
        }
   }


   if (skipOddLastIndex){
        # if debug
        printf("[do OddLastIndex] Name=%s, rank=%d, \n",hostname,myrank);
      fprintf(fp,"[do OddLastIndex] Name=%s, rank=%d, \n",hostname,myrank);
      fflush(stderr);
        # endif // debug
        /// do send to next process, and receive form next process.
      if ( ((myrank == (numprocs -1))) ||  ((myrank*countSize+count) >= filesize)  ){
            # if debug
             printf("[do Nothing in OddLastIndex] Name=%s, rank=%d, \n",hostname,myrank);
             fprintf(fp,"[do Nothing in OddLastIndex] Name=%s, rank=%d, \n",hostname,myrank);
             fflush(stderr);
            # endif // debug
          /// not switch because it's belong the last processor
          /// not switch because it's belong the last data
      }else{
            # if debug
            printf("[Odd Last Send before (1)] Name=%s, rank=%d\n",hostname,myrank);
            fprintf(fp,"[Odd Last Send before (1)] Name=%s, rank=%d\n",hostname,myrank);
            fflush(stderr);
            # endif // debug
            commuteStartTime = clock();
              gettimeofday(&commuteStartT,NULL);
            //rc = MPI_Send(&buf[count-1], 1, MPI_INT, (myrank+1), tag1, MPI_COMM_WORLD);
            rc = MPI_Isend(&buf[count-1], 1, MPI_INT, (myrank+1), tag1, MPI_COMM_WORLD,&request);

            commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

           gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

            if (rc != MPI_SUCCESS)
            {
                printf("[Odd Last Send Error (1)] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fprintf(stderr, "[Odd Last Sent Error (1)] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fflush(stderr);
                MPI_Abort(MPI_COMM_WORLD, 3);
            }
            # if debug
            printf("[Odd Last Recv before (2)] Name=%s, rank=%d\n",hostname,myrank);
            fprintf(fp,"[Odd Last Recv before(2)] Name=%s, rank=%d\n",hostname,myrank);
            fflush(stderr);
             # endif // debug
             commuteStartTime = clock();
               gettimeofday(&commuteStartT,NULL);
            rc = MPI_Recv(inBuf, 1, MPI_INT, (myrank+1), tag2, MPI_COMM_WORLD, &status);
           commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

       gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

            if (rc != MPI_SUCCESS)
            {
                printf("[Odd MPI Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fprintf(stderr, "[Odd MPI Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fflush(stderr);
                MPI_Abort(MPI_COMM_WORLD, 3);
            }

            MPI_Get_count(&status, MPI_INT, &sendRevCount);
             # if debug
            printf("[Odd last recv 1] Name=%s, rank=%d, revCount=%d, inBuf[0]=%d \n",hostname,myrank,sendRevCount,inBuf[0]);
            fprintf(fp,"[Odd last recv 1] Name=%s, rank=%d, revCount=%d, inBuf[0]=%d \n",hostname,myrank,sendRevCount,inBuf[0]);
            fflush(stderr);
             # endif // debug

            if (count >0)
            {
                # if debug
                printf("[Odd last recv 2] Name=%s, rank=%d, buf[%d]=%d, inBuf[0]=%d \n",hostname,myrank,count-1,buf[count-1],inBuf[0]);
                fprintf(fp,"[Odd last recv 2] Name=%s, rank=%d, buf[%d]=%d, inBuf[0]=%d \n",hostname,myrank,count-1,buf[count-1],inBuf[0]);
                fflush(stderr);
                  # endif // debug

                if (buf[count-1] > inBuf[0])
                {
                     # if debug
                    printf("[Odd last swap] hostname=%s, myrank=%d, buf[%d]=%d swap with inBuf[0]=%d,\n",hostname,myrank,(count-1),buf[count-1],inBuf[0]);
                    fprintf( fp, "[Odd last swap] hostname=%s, myrank=%d, buf[%d]=%d swap with inBuf[0]=%d,\n",hostname,myrank,(count-1),buf[count-1],inBuf[0]);
                    fflush(stderr);
                      # endif // debug
                    lastSwap=1;
                    buf[count-1]=inBuf[0];
                }else{
                    lastSwap =0;
                }
            }else{
                lastSwap=0;
            }
      }
   }



   int i=0;
   for (i=0;i<(count-1);i++){
      if (i==0  && skipOddFirstIndex){
              # if debug
        printf("[Odd swap In Process (skip first)] Name=%s, rank=%d, skip buf[0]=%d ,index=%d, \n",hostname,myrank,buf[i],(myrank*countSize+i));
        fprintf(fp,"[Odd swap In Process (skip first)] Name=%s, rank=%d, skip buf[0]=%d ,index=%d, \n",hostname,myrank,buf[i],(myrank*countSize+i));
        fflush(stderr);
          # endif // debug
        continue;
      }

      /*  Never go through.
      if (i == (count-1) && skipEvenLastIndex){
        continue;
      }*/

      if (buf[i] > buf[i+1]){
             # if debug
        printf("[Odd swap In Process(1)] Name=%s, rank=%d, swap(%d,%d),buf value(%d,%d),index(%d,%d) \n",hostname,myrank,i,i+1,buf[i],buf[i+1],(myrank*countSize+i),(myrank*countSize+i+1) );
        fprintf(fp,"[Odd swap In Process(1)] Name=%s, rank=%d, swap(%d,%d),buf value(%d,%d),index(%d,%d) \n",hostname,myrank,i,i+1,buf[i],buf[i+1],(myrank*countSize+i),(myrank*countSize+i+1) );
        fflush(stderr);
           # endif // debug
        swapArray(buf,i,i+1);
        subSwap =1;
      }
      i++;
   }

     if (firstSwap+lastSwap+subSwap >0){
        subSwap =1;
     }else{
         subSwap =0;
     }

       # if debug
    printf("[Odd basic MPI_Reduce] Name=%s, rank=%d, \n",hostname,myrank);
    fprintf(fp,"[Odd basic ] Name=%s, rank=%d, \n",hostname,myrank);
    fflush(stderr);
      # endif // debug

     commuteStartTime = clock();
       gettimeofday(&commuteStartT,NULL);
    rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
    commuteEndTime = clock();
    commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

   gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;


    if (rc != MPI_SUCCESS)
    {
        printf("[Odd basic MPI_Reduce Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
        fprintf(stderr, "[Odd basic MPI MPI_Reduce 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
        fflush(stderr);
        MPI_Abort(MPI_COMM_WORLD, 3);
    }

    if (myrank == MASTER)
    {
        if (comingSwap == 0 &&  oldSwap ==0 )
        {
            isConverge = 1;
            commuteStartTime = clock();
              gettimeofday(&commuteStartT,NULL);
            rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
            commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

           gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;
        }
        else
        {
            if (comingSwap >0)
            {
                oldSwap = 1;
            }
            else
            {
                oldSwap =0;
            }
            isConverge =0;

             # if debug
            printf("[Odd basic MPI_Bcast 1] Name=%s, rank=%d, \n",hostname,myrank);
            fprintf(fp,"[Odd basic MPI_Bcast 1] Name=%s, rank=%d, \n",hostname,myrank);
            fflush(stderr);
             # endif // debug


            commuteStartTime = clock();
              gettimeofday(&commuteStartT,NULL);
            rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
            commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

            gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;
        }

          # if debug
        printf("[Odd basic comingSwap] comingSwap=%d,\n",comingSwap);
        fprintf( fp, "[Odd basic comingSwap] comingSwap=%d,\n",comingSwap);
        fflush(stderr);
         # endif // debug
        MPI_Barrier(MPI_COMM_WORLD);
    }
    else{
              # if debug
        printf("[Odd basic MPI_Bcast 2] Name=%s, rank=%d, \n",hostname,myrank);
        fprintf(fp,"[Odd basic MPI_Bcast 2] Name=%s, rank=%d, \n",hostname,myrank);
        fflush(stderr);
            # endif // debug

        commuteStartTime = clock();
          gettimeofday(&commuteStartT,NULL);
        rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
        commuteEndTime = clock();
        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

       gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

        MPI_Barrier(MPI_COMM_WORLD);
    }

}




void doAdvanceEvenTranspose(int *buf, int *inBuf){
        # if debug
   showArrayInfo( "doAdvanceEvenTranspose **** buf Array ****",buf,count);
   showArrayInfo( "doAdvanceEvenTranspose **** inBuf Array ****",inBuf,countSize);
        # endif // debug

   MPI_Request request;

   int i=0;
   if ((myrank % 2) ==0){
             # if debug
            printf("even order \n");
            fprintf(fp,"even order \n");
            fflush(stderr);
             # endif // debug
          // even order
         if (myrank < (numprocs-1) ){

               fprintf(fp,"6\n");
               fflush(stderr);
               if (count >1){
                  ///qsort( buf, count, sizeof(int), cmpfunc);
               }


                     # if debug
                printf("[even send] Name=%s, rank=%d, sendCount=%d,\n",hostname,myrank,count);
                fprintf(stderr, "[even send] Name=%s, rank=%d, sendCount=%d,\n",hostname,myrank,count);
                fflush(stderr);

                printf("[Even send(1)] Name=%s, rank=%d\n",hostname,myrank);
                fprintf(fp,"[Even send(1)] Name=%s, rank=%d\n",hostname,myrank);
                fflush(stderr);

                showArrayInfo( "**** Even Send Array ****",buf,count);
                       # endif // debug
                       commuteStartTime = clock();
                        gettimeofday(&commuteStartT,NULL);
                //rc = MPI_Send(buf, count, MPI_INT, (myrank+1), tag1, MPI_COMM_WORLD);
                rc = MPI_Isend(buf, count, MPI_INT, (myrank+1), tag1, MPI_COMM_WORLD, &request);

                commuteEndTime = clock();
                commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

                gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

                if (rc != MPI_SUCCESS) {
                    printf("[Even MPI Send Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fprintf(stderr, "[Even MPI Sebd Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fflush(stderr);
                    MPI_Abort(MPI_COMM_WORLD, 3);
                }
                   # if debug
                printf("[Even recv(2)] Name=%s, rank=%d\n",hostname,myrank);
                fprintf(fp,"[Even recv(2)] Name=%s, rank=%d\n",hostname,myrank);
                fflush(stderr);
                  # endif // debug
                    commuteStartTime = clock();
                     gettimeofday(&commuteStartT,NULL);
                rc = MPI_Recv(inBuf, count, MPI_INT, (myrank+1), tag2, MPI_COMM_WORLD, &status);
                commuteEndTime = clock();
                commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

                gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

                if (rc != MPI_SUCCESS) {
                    printf("[Even MPI Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fprintf(stderr, "[Even MPI Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fflush(stderr);
                    MPI_Abort(MPI_COMM_WORLD, 3);
                }

                MPI_Get_count(&status, MPI_INT, &sendRevCount);
                   # if debug
                printf("[even recv] Name=%s, rank=%d, revCount=%d, \n",hostname,myrank,sendRevCount);
                fprintf(fp,"[even recv] Name=%s, rank=%d, revCount=%d, \n",hostname,myrank,sendRevCount);
                fflush(stderr);

                showArrayInfo( "Even Recv Array",inBuf,sendRevCount);
                    # endif // debug
                if (count >0){
                         # if debug
                    printf("[even recv] Name=%s, rank=%d, buf[0]=%d, inBuf[0]=%d, buf[%d]=%d, inBuf[%d]=%d \n",
                           hostname,myrank,buf[0],inBuf[0],count-1,buf[count-1],sendRevCount-1,inBuf[sendRevCount-1]);
                    fprintf(fp,"[even recv] Name=%s, rank=%d, buf[0]=%d, inBuf[0]=%d, buf[%d]=%d, inBuf[%d]=%d \n",
                            hostname,myrank,buf[0],inBuf[0],count-1,buf[count-1],sendRevCount-1,inBuf[sendRevCount-1]);
                    fflush(stderr);
                         # endif // debug
                    if ((buf[0] != inBuf[0]) || (buf[count-1] != inBuf[sendRevCount-1]) ){
                        subSwap=1;

                        //assign value by sorting result
                        for (i=0;i<count;i++){
                            buf[i]=inBuf[i];
                            //printf("[Even assign Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                            //fprintf( fp, "[Even assign Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                            //fflush(stderr);
                        }
                    }else{
                        subSwap =0;
                    }
                }else{
                    subSwap=0;
                }

                /*

                 for (i=0;i<count;i++){
                    buf[i]=inBuf[i];
                    printf("[Even Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                    fprintf( fp, "[Even Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                    fflush(stderr);
                }*/

                  # if debug
                printf("[Even] hostname=%s, myrank=%d, swap[%d]=%d,\n",hostname,myrank,i,swap);
                fprintf( fp, "[Even] hostname=%s, myrank=%d, swap[%d]=%d,\n",hostname,myrank,i,swap);
                fflush(stderr);

                showArrayInfo("Even Sorted Array",inBuf,countSize);

                printf("[Even MPI_Reduce(3)] Name=%s, rank=%d\n",hostname,myrank);
                fprintf(fp,"[Even MPI_Reduce(3)] Name=%s, rank=%d\n",hostname,myrank);
                fflush(stderr);
                    # endif // debug
                    commuteStartTime = clock();
                     gettimeofday(&commuteStartT,NULL);
                rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
                commuteEndTime = clock();
                commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

                gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

                if (rc != MPI_SUCCESS) {
                    printf("[Even MPI_Reduce Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fprintf(stderr, "[Even MPI MPI_Reduce 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fflush(stderr);
                    MPI_Abort(MPI_COMM_WORLD, 3);
                }



                if (myrank == MASTER){
                    if (comingSwap == 0 &&  oldSwap ==0 ){
                       isConverge = 1;
                        commuteStartTime = clock();
                         gettimeofday(&commuteStartT,NULL);
                       rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
                        commuteEndTime = clock();
                        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

                      gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

                    }else{
                        if (comingSwap >0){
                            oldSwap = 1;
                        }else{
                            oldSwap =0;
                        }
                        isConverge =0;
                         commuteStartTime = clock();
                          gettimeofday(&commuteStartT,NULL);
                        rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
                         commuteEndTime = clock();
                          commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

                         gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;
                    }
                    # if debug
                    printf("[comingSwap] comingSwap=%d,\n",comingSwap);
                    fprintf( fp, "[comingSwap] comingSwap=%d,\n",comingSwap);
                    fflush(stderr);
                     # endif // debug
                    MPI_Barrier(MPI_COMM_WORLD);
                }else{
                    commuteStartTime = clock();
                     gettimeofday(&commuteStartT,NULL);
                   rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
                     commuteEndTime = clock();
                    commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

                   gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;


                   MPI_Barrier(MPI_COMM_WORLD);
                }

         }else {
              # if debug
             /// even order!! but it is belong the last process and it cannot swap by others.
             printf("[Even MPI_Reduce(4), process no work] Name=%s, rank=%d\n",hostname,myrank);
             fprintf(fp,"[Even MPI_Reduce(4), process no work] Name=%s, rank=%d\n",hostname,myrank);
             fflush(stderr);
               # endif // debug
             subSwap = 0;
             commuteStartTime = clock();
              gettimeofday(&commuteStartT,NULL);
             rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
             commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );


            gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

            commuteStartTime = clock();
             gettimeofday(&commuteStartT,NULL);
             rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
             commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

           gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

             MPI_Barrier(MPI_COMM_WORLD);
         }
     }else{
          //printf("odd order, Name=%s, rank=%d,\n");
          //fprintf(fp,"odd order, Name=%s, rank=%d,\n");
          //fflush(stderr);
         //odd order
         if (count >1){
               ///qsort( buf, count, sizeof(int), cmpfunc);
         }

            # if debug
         printf("[Odd Recv(2)] Name=%s, rank=%d, original countSize=%d\n",hostname,myrank,countSize);
         fprintf(fp,"[Odd Recv(2)] Name=%s, rank=%d, original countSize=%d\n",hostname,myrank,countSize);
         fflush(stderr);
            # endif // debug
            commuteStartTime = clock();
             gettimeofday(&commuteStartT,NULL);
         rc = MPI_Recv(inBuf, countSize, MPI_INT, (myrank-1), tag1, MPI_COMM_WORLD, &status);
            commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

             gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;


         if (rc != MPI_SUCCESS) {
            fprintf(stderr, "[Odd MPI Recv Error 1] Name=%s, rank=%d%, error reading input file\n",hostname,myrank);
            fflush(stderr);
            MPI_Abort(MPI_COMM_WORLD, 3);
         }



         //printf("7.111 \n");
         //fprintf(fp,"7.111 \n");
         //fflush(stderr);

         MPI_Get_count(&status, MPI_INT, &sendRevCount);

            # if debug
         printf("[odd] Name=%s, rank=%d, recvCount=%d, \n",hostname,myrank,sendRevCount);
         fprintf(fp,"[odd] Name=%s, rank=%d, recvCount=%d, \n",hostname,myrank,sendRevCount);

        showArrayInfo( "**** Receive Send Array ****",inBuf,sendRevCount);
            # endif // debug
         /*
         int i=0;
         for(i=0;i<sendRevCount;i++){
            printf("[Odd Recv Array] hostname=%s, myrank=%d, inBuf[%d]=%d,\n",hostname,myrank,i,inBuf[i]);
            fprintf( fp, "[Odd Recv Array] hostname=%s, myrank=%d, inBuf[%d]=%d,\n",hostname,myrank,i,inBuf[i]);
            fflush(stderr);
        }*/




                # if debug
         showArrayInfo("Odd Recv Array",inBuf,countSize);
         fprintf(fp,"=====================================\n");
         fflush(stderr);
                # endif // debug
         //int result[sendRevCount+count];
         int *result = (int*) malloc( (sendRevCount+count)*sizeof(int) );
         mergeArray(result,inBuf,buf,sendRevCount,count);

         /*
         for(i=0;i<(sendRevCount+count);i++){
            printf("[Odd Merge Array] hostname=%s, myrank=%d, result[%d]=%d,\n",hostname,myrank,i,result[i]);
            fprintf( fp, "[Odd Merge Array] hostname=%s, myrank=%d, result[%d]=%d,\n",hostname,myrank,i,result[i]);
            fflush(stderr);
        }*/

              # if debug
          showArrayInfo(&merge_Tag,&result,(sendRevCount+count));
               # endif // debug

         //assign value by sorting result
         for (i=0;i<count;i++){
            buf[i]=result[sendRevCount+i];
            //printf("[Odd assign Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
            //fprintf( fp, "[Odd assign Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
            //fflush(stderr);
         }

              # if debug
         showArrayInfo(  "Odd Sorted Array",buf,countSize);

         printf("[Odd Send(3)] Name=%s, rank=%d, sendCount=%d \n",hostname,myrank,sendRevCount);
         fprintf(fp,"[Odd Send(3)] Name=%s, rank=%d, sendCount=%d \n",hostname,myrank,sendRevCount);
         fflush(stderr);
              # endif // debug

           commuteStartTime = clock();
            gettimeofday(&commuteStartT,NULL);
          //rc = MPI_Send(result, sendRevCount, MPI_INT, (myrank-1), tag2, MPI_COMM_WORLD);
          rc = MPI_Isend(result, sendRevCount, MPI_INT, (myrank-1), tag2, MPI_COMM_WORLD,&request);

            commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

            gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

          if (rc != MPI_SUCCESS) {
                printf("[Odd MPI Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fprintf(stderr, "[Odd MPI Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                fflush(stderr);
                MPI_Abort(MPI_COMM_WORLD, 3);
          }

          subSwap = 0;
          commuteStartTime = clock();
           gettimeofday(&commuteStartT,NULL);
          rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
           commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

            gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

            commuteStartTime = clock();
             gettimeofday(&commuteStartT,NULL);
           rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
           commuteEndTime = clock();
            commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

            gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

           MPI_Barrier(MPI_COMM_WORLD);
           free(result);
     }
}



void doAdvanceOddTranspose(int *buf, int *inBuf){
     # if debug
   showArrayInfo( "doAdvanceOddTranspose **** buf Array ****",buf,count);
   showArrayInfo( "doAdvanceOddTranspose **** inBuf Array ****",inBuf,countSize);
     # endif // debug
     MPI_Request request;
   int i=0;
   if ((myrank % 2) == 1){
               # if debug
            printf("odd order \n");
            fprintf(fp,"odd order \n");
            fflush(stderr);
                # endif // debug
          // odd order
         if (myrank < (numprocs-1) ){
                 # if debug
               fprintf(fp,"6\n");
               fflush(stderr);
                   # endif // debug
               if (count >1){
                  ///qsort( buf, count, sizeof(int), cmpfunc);
               }


                     # if debug
                printf("[odd send] Name=%s, rank=%d, sendCount=%d,\n",hostname,myrank,count);
                fprintf(stderr, "[odd send] Name=%s, rank=%d, sendCount=%d,\n",hostname,myrank,count);
                fflush(stderr);

                printf("[odd send(1)] Name=%s, rank=%d\n",hostname,myrank);
                fprintf(fp,"[odd send(1)] Name=%s, rank=%d\n",hostname,myrank);
                fflush(stderr);
                        # endif // debug

                  commuteStartTime = clock();
                  gettimeofday(&commuteStartT,NULL);


                //rc = MPI_Send(buf, count, MPI_INT, (myrank+1), tag1, MPI_COMM_WORLD);
                rc = MPI_Isend(buf, count, MPI_INT, (myrank+1), tag1, MPI_COMM_WORLD,&request);


                commuteEndTime = clock();
                commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

              gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

                if (rc != MPI_SUCCESS) {
                    printf("[odd MPI Send Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fprintf(stderr, "[odd MPI Sebd Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fflush(stderr);
                    MPI_Abort(MPI_COMM_WORLD, 3);
                }
                  # if debug
                printf("[odd recv(2)] Name=%s, rank=%d\n",hostname,myrank);
                fprintf(fp,"[odd recv(2)] Name=%s, rank=%d\n",hostname,myrank);
                fflush(stderr);
                   # endif // debug

                   commuteStartTime = clock();
                   gettimeofday(&commuteStartT,NULL);

                rc = MPI_Recv(inBuf, count, MPI_INT, (myrank+1), tag2, MPI_COMM_WORLD, &status);
                 commuteEndTime = clock();
              commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

               gettimeofday(&commuteEndT,NULL);
        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
         commuteT = commuteT + spanT;

                if (rc != MPI_SUCCESS) {
                    printf("[odd MPI Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fprintf(stderr, "[odd MPI Recv Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fflush(stderr);
                    MPI_Abort(MPI_COMM_WORLD, 3);
                }

                MPI_Get_count(&status, MPI_INT, &sendRevCount);
                  # if debug
                printf("[odd recv] Name=%s, rank=%d, revCount=%d, \n",hostname,myrank,sendRevCount);
                fprintf(fp,"[odd recv] Name=%s, rank=%d, revCount=%d, \n",hostname,myrank,sendRevCount);
                fflush(stderr);

                showArrayInfo( "odd Recv Array",inBuf,sendRevCount);
                    # endif // debug
                if (count >0){
                           # if debug
                    printf("[odd recv] Name=%s, rank=%d, buf[0]=%d, inBuf[0]=%d, buf[%d]=%d, inBuf[%d]=%d \n",
                           hostname,myrank,buf[0],inBuf[0],count-1,buf[count-1],sendRevCount-1,inBuf[sendRevCount-1]);
                    fprintf(fp,"[odd recv] Name=%s, rank=%d, buf[0]=%d, inBuf[0]=%d, buf[%d]=%d, inBuf[%d]=%d \n",
                            hostname,myrank,buf[0],inBuf[0],count-1,buf[count-1],sendRevCount-1,inBuf[sendRevCount-1]);
                    fflush(stderr);
                           # endif // debug
                    if ((buf[0] != inBuf[0]) || (buf[count-1] != inBuf[sendRevCount-1]) ){
                        subSwap=1;

                        //assign value by sorting result
                        for (i=0;i<count;i++){
                            buf[i]=inBuf[i];
                            //printf("[Even assign Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                            //fprintf( fp, "[Even assign Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                            //fflush(stderr);
                        }
                    }else{
                        subSwap =0;
                    }
                }else{
                    subSwap=0;
                }

                /*

                 for (i=0;i<count;i++){
                    buf[i]=inBuf[i];
                    printf("[Even Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                    fprintf( fp, "[Even Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                    fflush(stderr);
                }*/
                   # if debug
                printf("[odd] hostname=%s, myrank=%d, swap[%d]=%d,\n",hostname,myrank,i,swap);
                fprintf( fp, "[odd] hostname=%s, myrank=%d, swap[%d]=%d,\n",hostname,myrank,i,swap);
                fflush(stderr);

                showArrayInfo("odd Sorted Array",inBuf,countSize);

                printf("[odd MPI_Reduce(3)] Name=%s, rank=%d\n",hostname,myrank);
                fprintf(fp,"[odd MPI_Reduce(3)] Name=%s, rank=%d\n",hostname,myrank);
                fflush(stderr);
                   # endif // debug
                   commuteStartTime = clock();
                   gettimeofday(&commuteStartT,NULL);

                rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
                 commuteEndTime = clock();
                  gettimeofday(&commuteEndT,NULL);

                commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
                spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                commuteT = commuteT + spanT;

                if (rc != MPI_SUCCESS) {
                    printf("[odd MPI_Reduce Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fprintf(stderr, "[odd MPI MPI_Reduce 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                    fflush(stderr);
                    MPI_Abort(MPI_COMM_WORLD, 3);
                }
                    commuteStartTime = clock();
                    gettimeofday(&commuteStartT,NULL);

                 rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
                 commuteEndTime = clock();
                 gettimeofday(&commuteEndT,NULL);

                commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
                spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                commuteT = commuteT + spanT;

                 MPI_Barrier(MPI_COMM_WORLD);

        }else {
               # if debug
             // even order!! but it is belong the last process and it cannot swap by others.
             printf("[odd MPI_Reduce(4), process no work] Name=%s, rank=%d\n",hostname,myrank);
             fprintf(fp,"[odd MPI_Reduce(4), process no work] Name=%s, rank=%d\n",hostname,myrank);
              # endif // debug
             subSwap = 0;

             commuteStartTime = clock();
              gettimeofday(&commuteStartT,NULL);

             rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
             commuteEndTime = clock();
             gettimeofday(&commuteEndT,NULL);

             commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
             spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
             commuteT = commuteT + spanT;

             commuteStartTime = clock();
              gettimeofday(&commuteStartT,NULL);
             rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
             commuteEndTime = clock();
              gettimeofday(&commuteEndT,NULL);

             commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
              spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
             commuteT = commuteT + spanT;

             MPI_Barrier(MPI_COMM_WORLD);
         }
     }else{

         subSwap = 0;
         if (myrank == MASTER){
                    commuteStartTime = clock();
                    gettimeofday(&commuteStartT,NULL);
                    rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
                    commuteEndTime = clock();
                    gettimeofday(&commuteEndT,NULL);

                    commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
                    spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                    commuteT = commuteT + spanT;


                    if (rc != MPI_SUCCESS) {
                        printf("[Even MPI_Reduce Error 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                        fprintf(stderr, "[Even MPI MPI_Reduce 1] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                        fflush(stderr);
                        MPI_Abort(MPI_COMM_WORLD, 3);
                    }


                    if (comingSwap == 0 &&  oldSwap ==0 ){
                       isConverge = 1;
                       commuteStartTime = clock();
                        gettimeofday(&commuteStartT,NULL);

                       rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
                       commuteEndTime = clock();
                          gettimeofday(&commuteEndT,NULL);
                        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
                         spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                    commuteT = commuteT + spanT;

                    }else{
                        if (comingSwap >0){
                            oldSwap = 1;
                        }else{
                            oldSwap =0;
                        }
                        isConverge =0;
                        commuteStartTime = clock();
                        gettimeofday(&commuteStartT,NULL);

                        rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
                        commuteEndTime = clock();
                         gettimeofday(&commuteEndT,NULL);

                        commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

                        spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                        commuteT = commuteT + spanT;

                    }

                      # if debug
                    printf("[comingSwap] comingSwap=%d,\n",comingSwap);
                    fprintf( fp, "[comingSwap] comingSwap=%d,\n",comingSwap);
                    fflush(stderr);
                        # endif // debug
                    MPI_Barrier(MPI_COMM_WORLD);

         }else{

                //printf("odd order, Name=%s, rank=%d,\n");
                //fprintf(fp,"odd order, Name=%s, rank=%d,\n");
                //fflush(stderr);
                 //odd order
                 if (count >1){
                       ///qsort( buf, count, sizeof(int), cmpfunc);
                 }

                    # if debug
                 printf("[Even Recv(2)] Name=%s, rank=%d, original countSize=%d\n",hostname,myrank,countSize);
                 fprintf(fp,"[Even Recv(2)] Name=%s, rank=%d, original countSize=%d\n",hostname,myrank,countSize);
                 fflush(stderr);
                      # endif // debug
                      commuteStartTime = clock();
                      gettimeofday(&commuteStartT,NULL);

                 rc = MPI_Recv(inBuf, countSize, MPI_INT, (myrank-1), tag1, MPI_COMM_WORLD, &status);
                    commuteEndTime = clock();
                    gettimeofday(&commuteEndT,NULL);

                    commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
                     spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                    commuteT = commuteT + spanT;

                 if (rc != MPI_SUCCESS) {
                    fprintf(stderr, "[Even MPI Recv Error 1] Name=%s, rank=%d%, error reading input file\n",hostname,myrank);
                    fflush(stderr);
                    MPI_Abort(MPI_COMM_WORLD, 3);
                 }

                 //printf("7.111 \n");
                 //fprintf(fp,"7.111 \n");
                 //fflush(stderr);

                 MPI_Get_count(&status, MPI_INT, &sendRevCount);
                     # if debug
                 printf("[Even] Name=%s, rank=%d, recvCount=%d, \n",hostname,myrank,sendRevCount);
                 fprintf(fp,"[Even] Name=%s, rank=%d, recvCount=%d, \n",hostname,myrank,sendRevCount);
                     # endif // debug
                 /*
                 int i=0;
                 for(i=0;i<sendRevCount;i++){
                    printf("[Odd Recv Array] hostname=%s, myrank=%d, inBuf[%d]=%d,\n",hostname,myrank,i,inBuf[i]);
                    fprintf( fp, "[Odd Recv Array] hostname=%s, myrank=%d, inBuf[%d]=%d,\n",hostname,myrank,i,inBuf[i]);
                    fflush(stderr);
                }*/


                       # if debug
                 printf("7.222 \n");
                 fprintf(fp,"7.222 \n");
                 fflush(stderr);
                 showArrayInfo("Even Recv Array",inBuf,countSize);
                 fflush(stderr);
                     # endif // debug
                 //int result[sendRevCount+count];
                 int *result = (int*) malloc( (sendRevCount+count)*sizeof(int) );
                 mergeArray(result,inBuf,buf,sendRevCount,count);

                 /*
                 for(i=0;i<(sendRevCount+count);i++){
                    printf("[Odd Merge Array] hostname=%s, myrank=%d, result[%d]=%d,\n",hostname,myrank,i,result[i]);
                    fprintf( fp, "[Odd Merge Array] hostname=%s, myrank=%d, result[%d]=%d,\n",hostname,myrank,i,result[i]);
                    fflush(stderr);
                }*/

                  //showArrayInfo(&merge_Tag,&result,(sendRevCount+count));

                 //assign value by sorting result
                 for (i=0;i<count;i++){
                    buf[i]=result[sendRevCount+i];
                    //printf("[Odd assign Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                    //fprintf( fp, "[Odd assign Sorted Array] hostname=%s, myrank=%d, buf[%d]=%d,\n",hostname,myrank,i,buf[i]);
                    //fflush(stderr);
                 }
                       # if debug
                 showArrayInfo(  "Even Sorted Array",buf,countSize);

                 printf("[Even Send(3)] Name=%s, rank=%d, sendCount=%d \n",hostname,myrank,sendRevCount);
                 fprintf(fp,"[Even Send(3)] Name=%s, rank=%d, sendCount=%d \n",hostname,myrank,sendRevCount);
                 fflush(stderr);
                       # endif // debug

                       commuteStartTime = clock();
                         gettimeofday(&commuteStartT,NULL);
                  //rc = MPI_Send( result, sendRevCount, MPI_INT, (myrank-1), tag2, MPI_COMM_WORLD);
                  rc = MPI_Isend( result, sendRevCount, MPI_INT, (myrank-1), tag2, MPI_COMM_WORLD, &request);

                    commuteEndTime = clock();
                    gettimeofday(&commuteEndT,NULL);
                    commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
                    spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                    commuteT = commuteT + spanT;


                  if (rc != MPI_SUCCESS) {
                        printf("[Even MPI Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                        fprintf(stderr, "[Even MPI Error 2] Name=%s, rank=%d, error reading input file\n",hostname,myrank);
                        fflush(stderr);
                        MPI_Abort(MPI_COMM_WORLD, 3);
                  }

                  //subSwap = 0;
                  commuteStartTime = clock();
                   gettimeofday(&commuteStartT,NULL);

                  rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;
                  commuteEndTime = clock();
                   gettimeofday(&commuteEndT,NULL);

                  commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );
                  spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                  commuteT = commuteT + spanT;

                    commuteStartTime = clock();
                     gettimeofday(&commuteStartT,NULL);
                   rc = MPI_Bcast(&isConverge,1,MPI_INT,MASTER,MPI_COMM_WORLD);
                   commuteEndTime = clock();
                    gettimeofday(&commuteEndT,NULL);

                    commuteTime = commuteTime +  (double)(commuteEndTime - commuteStartTime  );

                    spanT = commuteEndT.tv_sec-commuteStartT.tv_sec + (commuteEndT.tv_usec-commuteStartT.tv_usec)/1000000.0; // sec
                    commuteT = commuteT + spanT;


                   MPI_Barrier(MPI_COMM_WORLD);
                   free(result);
            }
     }
}

void sequentialEvenOddSort(int *buf,int count){

    int sorted = 0;
    while(!sorted)
    {
        sorted=1;
        int i=0;
        for( i = 1; i < count-1; i += 2)
        {
            if(buf[i] > buf[i+1])
            {
                swapArray(buf, i, i+1);
                sorted = 0;
            }
        }

        for( i = 0; i < count-1; i += 2)
        {
            if(buf[i] > buf[i+1])
            {
                swapArray(buf, i, i+1);
                sorted = 0;
            }
        }
    }

}

char *string_concat(char *str1, char *str2) {

    int length=strlen(str1)+strlen(str2)+1;

    char *result = (char*)malloc(sizeof(char) * length);

    // 複製第一個字串至新的陣列空間
    strcpy(result, str1);
    // 串接第二個字串至新的陣列空間
    strcat(result, str2);

    return result;
}


int main(int argc, char *argv[])
{



    if (argc != 4){
        printf("Please key in argument: data size,input_file,output_file.\n");
        //fprintf( fp, "Please key in argument: data size,input_file,output_file.\n");
        //fflush(stderr);
        return -1;
    }

    fp = freopen("./log.txt","wr", stderr);
    fprintf( fp,"argc=%d\n",argc);
    fprintf( fp,"data size, argv[1]=#%s#\n",argv[1]);
    fprintf( fp,"input file, argv[2]=#%s#\n",argv[2]);
    fprintf( fp,"output file, argv[3]=#%s#\n",argv[3]);
    fflush(stderr);


    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Get_processor_name(hostname, &len);

    startTime = clock();
    ioStartTime = clock();

    gettimeofday(&startT,NULL);
    gettimeofday(&ioStartT,NULL);

    MPI_File_open(MPI_COMM_WORLD, argv[2], MPI_MODE_RDONLY,
		  MPI_INFO_NULL, &thefile);

    MPI_File_get_size(thefile, &filesize); // in bytes
    filesize = filesize / sizeof(int);     // in number of ints

    countSize = filesize / numprocs;

    if ((filesize%numprocs) != 0){
        countSize = countSize +1;
    }


    int *buf = (int*) malloc( countSize*sizeof(int) );
    //int buf[countSize] ;
    initNumber(buf,countSize);


    //buf = (int *) malloc (countSize * sizeof(int));
    //initNumber(buf,countSize);

      # if debug
    printf("4\n");
    fprintf(fp,"4\n");
    fflush(stderr);
       # endif // debug
    rc = MPI_File_set_view(thefile, myrank * countSize * sizeof(int),
		      MPI_INT, MPI_INT, "native", MPI_INFO_NULL);

    if (rc != MPI_SUCCESS) {
        fprintf(stderr, "error setting file view on input file\n");
        MPI_Abort(MPI_COMM_WORLD, 3);
        fflush(stderr);
    }
      # if debug
    printf("5\n");
    fprintf(fp,"5\n");
    fflush(stderr);
      # endif // debug
    rc =MPI_File_read_all(thefile, buf, countSize, MPI_INT, &status);

    if (rc != MPI_SUCCESS) {
        fprintf(stderr, "error reading input file\n");
        MPI_Abort(MPI_COMM_WORLD, 3);
        fflush(stderr);
    }

    MPI_Get_count(&status, MPI_INT, &count);
    //printf("1.myrank=%d, countSize=%d, count=%d, filesize=%d,numprocs=%d\n",myrank,countSize,count,filesize,numprocs);
    //fprintf(fp,"1.myrank=%d, countSize=%d,count=%d, filesize=%d,numprocs=%d\n",myrank, countSize,count,filesize,numprocs);

    # if debug
    /*
    int i=0;
    while (i<countSize){
       if (i<count){
          printf("Name=%s, rank=%d, count=%d, data[%d]=%d \n",hostname,myrank,count,i,buf[i]);
          fprintf(fp,"Name=%s, rank=%d, count=%d, data[%d]=%d \n",hostname,myrank,count,i,buf[i]);
          fflush(stderr);
       }else{
          printf("<<ignore>> Name=%s, rank=%d, data [%d]=%d, count=%d, countSize=%d, \n",hostname, myrank,i,buf[i],count,countSize);
          fprintf(fp,"<<ignore>> Name=%s, rank=%d, data [%d]=%d, count=%d, countSize=%d, \n",hostname, myrank,i,buf[i],count,countSize);
          fflush(stderr);
       }
       i++;
    }*/
    # endif // debug

    MPI_File_close(&thefile);

    ioEndTime = clock();
    gettimeofday(&ioEndT,NULL);
    spanT = ioEndT.tv_sec-ioStartT.tv_sec + (ioEndT.tv_usec-ioStartT.tv_usec)/1000000.0; // sec

    ioTime = (double)(ioEndTime-ioStartTime);
    fprintf(fp,"Name=%s, process=%d , #Read file end!# spend:(%lf), spend sec:(%lf), gettimeofday(%.12f)sec  \n",hostname, myrank, (double)(ioEndTime-ioStartTime), (double)(ioEndTime-ioStartTime) / CLOCKS_PER_SEC ,spanT );

    fflush(stderr);
    // do advance even-odd algorithms


    //int inBuf[countSize] ;
    int *inBuf = (int*) malloc( countSize*sizeof(int) );

    initNumber(inBuf,countSize);

    initBasicTraspose();

    //doBasicOddTranspose(buf,inBuf);
    //showArrayInfo( "<< BasicOddTrasponse Result >>",&buf,count);




    //printf("2.myrank=%d, countSize=%d, count=%d, filesize=%d,numprocs=%d\n",myrank,countSize,count,filesize,numprocs);
    //fprintf(fp,"2.myrank=%d, countSize=%d,count=%d, filesize=%d,numprocs=%d\n",myrank, countSize,count,filesize,numprocs);


    //showArrayInfo(&init_Tag,&inBuf,countSize);




     //printf("3.myrank=%d, countSize=%d, count=%d, filesize=%d,numprocs=%d\n",myrank,countSize,count,filesize,numprocs);
     //fprintf(fp,"3.myrank=%d, countSize=%d,count=%d, filesize=%d,numprocs=%d\n",myrank, countSize,count,filesize,numprocs);


     // do even order

     //showArrayInfo( "<< buf Array >>",&buf,count);
     //showArrayInfo( "<< inBuf Array >>",&inBuf,countSize);




    int evenTraspose =0;
    int oddTraspose =0;


    if (numprocs ==1){
        /// for basic sort
        //sequentialEvenOddSort(buf,countSize);
        /// for advance sort
        qsort( buf, count, sizeof(int), cmpfunc);
    }else{
        qsort( buf, count, sizeof(int), cmpfunc);

        //do even odd sort
        do{
            //even swap
            doAdvanceEvenTranspose(buf,inBuf);
            //doBasicEvenTranspose(buf,inBuf);
                ++evenTraspose;
            if (isConverge != 1){
                doAdvanceOddTranspose(buf,inBuf);
                //doBasicOddTranspose(buf,inBuf);
                ++oddTraspose;
            }
        }while( isConverge !=1);
    }

    //MPI_Barrier(MPI_COMM_WORLD);



    //printf("<<Result>> Name=%s, rank=%d, evenTraspose=%d, oddTraspose=%d \n",hostname, myrank,evenTraspose,oddTraspose);
    fprintf(fp,"\n <<Result>> Name=%s, rank=%d, evenTraspose=%d, oddTraspose=%d \n",hostname, myrank,evenTraspose,oddTraspose);
    fflush(stderr);

    //MPI_Barrier(MPI_COMM_WORLD);
       # if debug
    showArrayInfo( "<< buf Array >>",&buf,count);
        # endif // debug

    //rc = MPI_Reduce (&subSwap,&comingSwap,1,MPI_INT,MPI_SUM,MASTER,MPI_COMM_WORLD) ;

    //int gatherSize = count * numprocs ;
    //int gatherData[gatherSize];

    /*
    rc= MPI_Gather(buf, count, MPI_INT,gatherData,countSize,MPI_INT,MASTER,MPI_COMM_WORLD );

     if (rc != MPI_SUCCESS) {
        fprintf(stderr, "error setting file view on input file\n");
        MPI_Abort(MPI_COMM_WORLD, 3);
        fflush(stderr);
    }


    if (myrank == MASTER){
          showArrayInfo( "<<Result>> ",&gatherData, (countSize * numprocs) );
    }*/



    // Delete the output file if it exists so that striping can be set
    // on the output file.
    ioStartTime = clock();
    gettimeofday(&ioStartT,NULL);

    MPI_File outTheFile;
    MPI_Offset outOffset;

    rc = MPI_File_delete(argv[3], MPI_INFO_NULL);

    rc = MPI_File_open(MPI_COMM_WORLD, argv[3], MPI_MODE_WRONLY |
      MPI_MODE_CREATE, MPI_INFO_NULL, &outTheFile);

    if (rc != MPI_SUCCESS) {
        fprintf(stderr, "could not open results file\n");
        MPI_Abort(MPI_COMM_WORLD, 3);
    }

    if ( countSize != count){
       outOffset = myrank * countSize * sizeof(int);
    }else{
       outOffset = myrank * count * sizeof(int);
    }

    MPI_File_set_view(outTheFile , outOffset,  MPI_INT,  MPI_INT, "native", MPI_INFO_NULL);
    if (rc != MPI_SUCCESS) {
        fprintf(stderr, "error setting view on results file\n");
        MPI_Abort(MPI_COMM_WORLD, 4);
    }

    // MPI Collective Write
    rc = MPI_File_write_all(outTheFile, buf, count, MPI_INT, MPI_STATUS_IGNORE);
    if (rc != MPI_SUCCESS) {
        fprintf(stderr, "error writing results file\n");
        MPI_Abort(MPI_COMM_WORLD, 5);
    }

    // Close Files
    MPI_File_close(&outTheFile);
    ioEndTime = clock();
    ioTime = ioTime+ (double)(ioEndTime-ioStartTime);
    gettimeofday(&ioEndT,NULL);
    spanT = ioEndT.tv_sec-ioStartT.tv_sec + (ioEndT.tv_usec-ioStartT.tv_usec)/1000000.0; // sec


    fprintf(fp,"Name=%s, process=%d, ##Write file end!## spend:(%lf), spend sec:(%lf), gettimeofday=(%.12f) sec \n",hostname, myrank, (double)(ioEndTime-ioStartTime), (double)(ioEndTime-ioStartTime) / CLOCKS_PER_SEC ,spanT );
    fflush(stderr);

    endTime =clock();
    gettimeofday(&endT,NULL);
    spanT = endT.tv_sec-startT.tv_sec + (endT.tv_usec-startT.tv_usec)/1000000.0; // sec

    fprintf(fp,"Name=%s, process=%d, ###Total Run Time!### spend:(%lf), spend sec:(%lf),gettimeofday=(%.12f) sec; commuting time:%(%lf),commuting time sec(%lf) , gettimeofday=(%.12f) sec  \n",
            hostname, myrank,(double)(endTime-startTime), (double)(endTime-startTime) / CLOCKS_PER_SEC ,spanT, commuteTime ,  (double) commuteTime/ CLOCKS_PER_SEC , commuteT );
    fflush(stderr);

    fprintf(fp,"Name=%s, process=%d, ###Total Run Time!### I/o spend:(%lf),  sec:(%lf); Executing time:%(%lf), time sec(%lf) \n",
            hostname, myrank,ioTime , (double)ioTime / CLOCKS_PER_SEC ,   (double)(endTime-startTime) - (ioTime) - commuteTime ,  (double)((endTime-startTime) - (ioTime) - commuteTime )/CLOCKS_PER_SEC );
    fflush(stderr);

    MPI_Finalize();

    //printf("9\n");
    //fprintf(fp,"9\n");
    fflush(stderr);
    free(buf);
    printf("10\n");
    fprintf(fp,"10\n");
    fflush(stderr);
    fclose(fp);


    return 0;


}

