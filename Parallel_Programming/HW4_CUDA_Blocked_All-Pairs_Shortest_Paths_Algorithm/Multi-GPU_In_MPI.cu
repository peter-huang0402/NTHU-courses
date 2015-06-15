#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <mpi.h>
#define DEBUG_ENABLE 0
#define V 7000
#define INF 1000000
///const int INF = 1000000;
///const int V = 7000;
#define DATARANGE (n*V)
void input(char *inFileName);
void output(char *outFileName);

void block_APSP(int B);
int iceil(int a, int b);
void cal(int B, int Round, int block_start_x, int block_start_y, int block_width, int block_height);

int n, m;	// Number of vertices, edges
int Dist[V][V];
int* devDist;
char* devChange;
char change[V][V];
char otherChange[V][V];
int otherDist[V][V];
int gpuID=0;
///int Dist[7000][7000];
FILE *logFp;
char *in1;
char *out1;
static int totalCUDADevice = 0;
static int totalNode = 1;
static int MPIID=0;
static int currentDev=0;
typedef struct Timer {
    char name[256];
    struct timeval begin;
    struct timeval end;
} Timer;
Timer* timer_memcpy,*timer_commu,*timer_compute;
Timer *timer_phase3, *timer_main;

Timer* timer_init(Timer* t,const char* name);
Timer* timer_new(const char* name);
void timer_start(Timer* t);
void timer_end(Timer* t);
void timer_add(Timer* t1, const Timer* t2);
double timer_seconds(const Timer* t);
void timer_print(const Timer* t,FILE* stream);
void timer_delete(Timer* t);


Timer* timer_init(Timer* t,const char* name ) {
    if(t) {
        memset (t,0,sizeof(Timer));
        strncpy(t->name,name,256);
    }
    return t;
}

Timer* timer_new(const char* name) {
    Timer* t;
    t = (Timer*)malloc(sizeof(Timer));
    return timer_init(t,name);
}
void timer_start(Timer* t) {
    if(!t) return;
    gettimeofday(&t->begin,0);
}
void timer_end(Timer* t) {
    if(!t) return;
    gettimeofday(&t->end,0);
}
void timer_add(Timer* t1, const Timer* t2) {
    if(!t1 || !t2) return;
    t1->end.tv_sec+=(t2->end.tv_sec-t2->begin.tv_sec);
    t1->end.tv_usec+=(t2->end.tv_usec-t2->begin.tv_usec);
}

double timer_seconds(const Timer* t) {
    if(!t) return 0;
    return (double)(t->end.tv_sec-t->begin.tv_sec)+(1e-6*(t->end.tv_usec-t->begin.tv_usec));
}
void timer_delete(Timer* t) {
    if(!t) return;
    free(t);
}
void timer_print(const Timer* t, FILE* stream) {
    if(!t) return;
    fprintf(stream,"%s : %f(sec)\n",t->name,timer_seconds(t));
    //printf("%s : %f(sec)\n",t->name,timer_seconds(t));

}

static void __debugCUDACall(cudaError_t err, const char* expr, const char* file, int line) {
    if(err != cudaSuccess) {
        fprintf(stdout,"in File %s Line %d:%s\n",file,line,expr);
        fprintf(stdout,"%s \n",cudaGetErrorString(err));
    }
}
#define debugCUDACall(X) __debugCUDACall((X),#X,__FILE__,__LINE__)


char *stringConcat(char *str1, char *str2) {

    int length=strlen(str1)+strlen(str2)+1;

    char *result = (char*)malloc(sizeof(char) * length);

    // 複製第一個字串至新的陣列空間
    strcpy(result, str1);
    // 串接第二個字串至新的陣列空間
    strcat(result, str2);

    return result;
}


void initCUDADevice(int gpuID)
{
    // Task 1: Device Initialization
    debugCUDACall(cudaGetDeviceCount(&totalCUDADevice));
    printf("totalCUDADevice=%d, \n",totalCUDADevice);


    if (totalCUDADevice == 0) {
        printf("No CUDA device found.\n\n");
    } else if (gpuID < totalCUDADevice) {
        printf("set CUDA device=%d, \n",gpuID );
        debugCUDACall(cudaSetDevice(gpuID));
    } else {
        gpuID =0;
        printf("set CUDA device=%d, \n",gpuID );
        debugCUDACall(cudaSetDevice(gpuID));
    }

}

int mainRun(int argc, char* argv[])
{

    if(argc > 4) {
        sscanf(argv[4],"%d",&gpuID);
    }
    MPI_Comm_size(MPI_COMM_WORLD,&totalNode);
    MPI_Comm_rank(MPI_COMM_WORLD,&MPIID);

    initCUDADevice(MPIID);

    timer_memcpy = timer_new("Memcpy");
    timer_commu = timer_new("Communication");
    timer_compute = timer_new("Compute");
    timer_phase3 = timer_new("Phase3");

    timer_main = timer_new("block_APSP");

    logFp = freopen("./mpi_log.txt","wr", stderr);

//    in1= "D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\tiny_test_case";
//    out1="D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\output\\tiny_test_case_out";
    //in1= "./Testcase/in2";
    //out1="./output/tiny_test_case_out";

    ///char *in1= "D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\in3";
    ///char *out1="D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\output\\out1";
    ///input( in1 );
    if(MPIID == 0){
         input( argv[1] );
    }

    timer_start(timer_commu);
    MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);
    timer_end(timer_commu);

    timer_start(timer_phase3);
    timer_end(timer_phase3);
    timer_start(timer_compute);
    timer_end(timer_compute);

    printf("node %d n=%d\n",MPIID,n);

    if(totalCUDADevice > 0) {
        debugCUDACall(cudaMalloc((void**)&devDist,sizeof(Dist)));
        debugCUDACall(cudaMalloc((void**)&devChange, sizeof(change)));
    }

    /*
    if(MPIID == 0){
       fprintf(logFp, "\n");
       int i,j=0;
       for ( i = 0; i < n; ++i) {
           for ( j = 0; j < n; ++j) {
               if (Dist[i][j] >= INF)	fprintf(logFp, "INF ");
               else					fprintf(logFp, "%d ", Dist[i][j]);
           }
           fprintf(logFp, "\n");
       }
    }
    */

    if(totalCUDADevice > 0 && MPIID == 0) {
        timer_start(timer_memcpy);
        debugCUDACall(cudaMemcpy(devDist,&Dist[0][0],sizeof(int)*DATARANGE,cudaMemcpyHostToDevice));
        timer_end(timer_memcpy);
    }


    int B = 128;
    if(argc > 3) {
        sscanf(argv[3],"%d",&B);
    }

    printf("*** B=%d, source=%s, output=%s,\n",B,argv[1],argv[2]);

    timer_start(timer_main);
    block_APSP(B);
    timer_end(timer_main);


    if(totalCUDADevice >0) {
        Timer tempMemcpy;
        timer_init(&tempMemcpy,"");
        timer_start(&tempMemcpy);
        if(MPIID == 0){
           debugCUDACall(cudaMemcpy(&Dist[0][0],devDist,sizeof(int)*DATARANGE,cudaMemcpyDeviceToHost));
        }
        timer_end(&tempMemcpy);
        timer_add(timer_memcpy,&tempMemcpy);
        debugCUDACall(cudaFree(devDist));
        debugCUDACall(cudaFree(devChange));
    }

    if(MPIID==0){
          output(argv[2]);
    }
    ///output( out1 );
    if(MPIID==0)
    {
        timer_print(timer_memcpy,stdout);
        timer_print(timer_commu,stdout);
        timer_print(timer_compute,stdout);
        timer_print(timer_phase3,stdout);
        timer_print(timer_main,stdout);
        fflush(stdout);
    }
    fclose(logFp);
    timer_delete(timer_memcpy);
    timer_delete(timer_commu);
    timer_delete(timer_compute);
    timer_delete(timer_phase3);
    timer_delete(timer_main);

    return 0;
}

void input(char *inFileName)
{   FILE *infile = fopen(inFileName, "r");
    fscanf(infile, "%d %d", &n, &m);

    printf("n=%d, m=%d \n",n,m);
    int i,j=0;
    for ( i = 0; i < n; ++i) {
        for ( j = 0; j < n; ++j) {
            if (i == j)	Dist[i][j] = 0;
            else		Dist[i][j] = INF;
        }
    }

    while (--m >= 0) {
        int a, b, v;
        fscanf(infile, "%d %d %d", &a, &b, &v);
        if (m== 49) printf("m=%d, a=%d, b=%d, v=%d \n",m,a,b,v);
        --a, --b;
        Dist[a][b] = v;
    }
}

void output(char *outFileName)
{   FILE *outfile = fopen(outFileName, "w");
    int i,j=0;
    for ( i = 0; i < n; ++i) {
        for ( j = 0; j < n; ++j) {
            if (Dist[i][j] >= INF)	fprintf(outfile, "INF ");
            else					fprintf(outfile, "%d ", Dist[i][j]);
        }
        fprintf(outfile, "\n");
    }
}

int iceil(int a, int b) {
    return (a + b -1)/b;
}
///  (y,x) ==> (column,row)
///        |       |
///  (0,0) | (0,1) | (0,2)
///  ______|_______|______
///  (1,0) | (1,1) | (1,2)
///  ______|_______|______
///  (2,0) | (2,1) | (2,2)
///        |       |

void readChangeAndData() {
    if(totalCUDADevice > 0) {
        Timer tempMemcpy;
        timer_init(&tempMemcpy,"");
        timer_start(&tempMemcpy);

        debugCUDACall(cudaMemcpy(Dist,devDist,sizeof(int)*DATARANGE,cudaMemcpyDeviceToHost));
        debugCUDACall(cudaMemcpy(change,devChange,sizeof(char)*DATARANGE,cudaMemcpyDeviceToHost));
        timer_end(&tempMemcpy);
        timer_add(timer_memcpy,&tempMemcpy);

    }
}
void sendDataAndChange(int other) {
    Timer temp;
    timer_init(&temp,"");

    readChangeAndData();

    timer_start(&temp);
    MPI_Send(Dist,DATARANGE,MPI_INT,other,0x1234,MPI_COMM_WORLD);
    MPI_Send(change,DATARANGE,MPI_CHAR,other,0x1234,MPI_COMM_WORLD);
    memset(change,0,sizeof(char)*DATARANGE);
    timer_end(&temp);
    timer_add(timer_commu,&temp);
}


void applyOtherDataByChange(int other) {
    MPI_Status status;
    {
       Timer temp;
       timer_init(&temp,"");
       timer_start(&temp);
       MPI_Recv(otherDist,DATARANGE,MPI_INT,other,0x1234,MPI_COMM_WORLD,&status);
       MPI_Recv(otherChange,DATARANGE,MPI_CHAR,other,0x1234,MPI_COMM_WORLD,&status);
       if(totalCUDADevice > 0) {
           Timer temp;
           timer_init(&temp,"");
           timer_start(&temp);
           debugCUDACall(cudaMemcpy(Dist,devDist,sizeof(int)*DATARANGE,cudaMemcpyDeviceToHost));
           timer_end(&temp);
           timer_add(timer_memcpy,&temp);
       }

       #pragma omp parallel for
       for(int iL=0; iL<DATARANGE; ++iL) {
           int i = iL%V;
           int j = iL/V;
           if(otherChange[j][i] && Dist[j][i]!=otherDist[j][i]) {
     //          printf("Dist[%d][%d]:%d->%d\n",j,i,Dist[j][i],otherDist[j][i]);
               Dist[j][i]=otherDist[j][i];
           }
       }
       timer_end(&temp);
       timer_add(timer_commu,&temp);
    }
    if(totalCUDADevice > 0) {
        Timer temp;
        timer_init(&temp,"");
        timer_start(&temp);

        debugCUDACall(cudaMemcpy(devDist,Dist,sizeof(int)*DATARANGE,cudaMemcpyHostToDevice));
        debugCUDACall(cudaMemset(devChange,0,sizeof(char)*DATARANGE));
        timer_end(&temp);
        timer_add(timer_memcpy,&temp);
    }
}

void block_APSP(int B) {
    int round = iceil(n, B);
#if DEBUG_ENABLE
    fprintf(logFp,"round=%d ====================================\n",round);
#endif
    int r=0;
    for ( r = 0; r < round; ++r) {
        ///* Phase 1*/
#if DEBUG_ENABLE
        fprintf(logFp,"[Phase1] r=%d ====================================\n",r);
#endif

        /// B, Round, block_start_x, block_start_y, block_width, block_height
        if(MPIID == 0){
            cal(B,	r,	  r,	r,	            1,	           1); // phase 1

#if DEBUG_ENABLE
            fprintf(logFp,"[Phase2] r=%d \n",r);
            fprintf(logFp,"         r=%d 1. \n",r);
#endif
            ///* Phase 2*/  /// (y,x) ==> (column,row) ==> (r,0)

            // vertical
            // B Round   x      y              w              h
            cal(B, r,     r,     0,             r,             1); ///front row
#if DEBUG_ENABLE
            fprintf(logFp,"         r=%d 2. \n",r);
#endif
            cal(B, r,     r,  r +1,  round - r -1,             1); /// back row
            // horizontal
#if DEBUG_ENABLE
            fprintf(logFp,"         r=%d 3. \n",r);
#endif
            cal(B, r,     0,     r,             1,             r); /// up column
#if DEBUG_ENABLE
            fprintf(logFp,"         r=%d 4. \n",r);
#endif
            cal(B, r,  r +1,     r,             1,  round - r -1); /// down column
#if DEBUG_ENABLE
            fprintf(logFp,"[Phase3] r=%d \n",r);
#endif
        }

        if(totalNode > 1){
           if(totalCUDADevice > 0 && MPIID == 0){
              Timer tempMemcpy;
              timer_init(&tempMemcpy,"");
              timer_start(&tempMemcpy);
              debugCUDACall(cudaMemcpy(Dist,devDist,sizeof(int)*DATARANGE,cudaMemcpyDeviceToHost));
              timer_end(&tempMemcpy);
              timer_add(timer_memcpy,&tempMemcpy);
           }

           MPI_Barrier(MPI_COMM_WORLD);

           Timer temp;
           timer_init(&temp,"");
           timer_start(&temp);
           MPI_Bcast(Dist,DATARANGE,MPI_INT,0,MPI_COMM_WORLD);
           timer_end(&temp);
           timer_add(timer_commu,&temp);

           memset(change,0,sizeof(char)*DATARANGE);
           if(totalCUDADevice > 0){
              if(MPIID==1){
                 Timer tempMemcpy;
                 timer_init(&tempMemcpy,"");
                 timer_start(&tempMemcpy);
                 debugCUDACall(cudaMemcpy(devDist,Dist,sizeof(int)*DATARANGE,cudaMemcpyHostToDevice));
                 debugCUDACall(cudaMemset(devChange,0,sizeof(char)*DATARANGE));
                 timer_end(&tempMemcpy);
                 timer_add(timer_memcpy,&tempMemcpy);
              }
           }
        }
        // here broadcast

        Timer tempTime;
        timer_init(&tempTime,"");
        timer_start(&tempTime);

        if(MPIID == 0){
#if DEBUG_ENABLE
            ///* Phase 3*/
            fprintf(logFp,"         r=%d 1. \n",r);
#endif
            cal(B, r,     0,     0,            r,             r);  ///2 quadrant
#if DEBUG_ENABLE
            fprintf(logFp,"         r=%d 2. \n",r);
#endif
            cal(B, r,     0,  r +1,  round -r -1,             r);  /// 1 quadrant
#if DEBUG_ENABLE
            fprintf(logFp,"         r=%d 3. \n",r);
#endif
            if(totalNode < 2){
               cal(B, r,  r +1,     0,            r,  round - r -1);   /// 3 quadrant
               cal(B, r,  r +1,  r +1,  round -r -1,  round - r -1);   /// 4 quadrant
            }

        }else{
           //memset(change,0,sizeof(char)*DATARANGE);
           cal(B, r,  r +1,     0,            r,  round - r -1);   /// 3 quadrant
           cal(B, r,  r +1,  r +1,  round -r -1,  round - r -1);   /// 4 quadrant

#if DEBUG_ENABLE
            fprintf(logFp,"         r=%d 4. \n",r);
#endif
        }

        timer_end(&tempTime);
        timer_add(timer_phase3,&tempTime);

        //MPI_Barrier(MPI_COMM_WORLD);
        if(totalNode > 1){
           if(MPIID==0){
              applyOtherDataByChange(1);
           }
           else if(MPIID==1){
              sendDataAndChange(0);
           }
           //MPI_Bcast(Dist,DATARANGE,MPI_INT,0,MPI_COMM_WORLD);
           //MPI_Barrier(MPI_COMM_WORLD);
        }
    }
}
static __global__ void calKernelGPU(int B,int Round,int x,int y,int n,int* dDist,int k,char* dchange) {
//////////////////////
    //int Bpow2=B*B;
    int b_i = blockIdx.x+x;
    int b_j = blockIdx.y+y;
    int valIK,valKJ,valIJ;
    for(int bid=0; bid<B; bid+=1) {
        int threadIdx_x=bid;
        int threadIdx_y=threadIdx.x;
        int i=b_i*B+threadIdx_x;
        int j=b_j*B+threadIdx_y;
        if (i > n) continue;
        if (j > n) continue;
        valIK=dDist[i*V+k];
        valKJ=dDist[k*V+j];
        valIJ=dDist[i*V+j];
        if (valIK + valKJ < valIJ) {
            valIJ = valIK + valKJ;
            dDist[i*V+j]=valIJ;
            dchange[i*V+j]=1;  // mark a change bit
        }
        //__threadfence();
    }
}

static void calKernelCPU(int B,int Round,int b_i,int b_j) {
//////////////////////
    int k=0;
    /// To calculate B*B elements in the block (b_i, b_j)
    /// For each block, it need to compute B times
    int block_internal_start_x 	= b_i * B;
    int block_internal_end_x 	= (b_i +1) * B;
    int block_internal_start_y = b_j * B;
    int block_internal_end_y 	= (b_j +1) * B;
    if (block_internal_end_x > n)	block_internal_end_x = n;
    if (block_internal_end_y > n)	block_internal_end_y = n;
    for ( k = Round * B; k < (Round +1) * B && k < n; ++k) { ///
        int i,j;
        /// To calculate original index of elements in the block (b_i, b_j)
        /// For instance, original index of (0,0) in block (1,2) is (2,5) for V=6,B=2
        for ( i = block_internal_start_x; i < block_internal_end_x; ++i) {
            for ( j = block_internal_start_y; j < block_internal_end_y; ++j) {
                if (Dist[i][k] + Dist[k][j] < Dist[i][j]) {
                    Dist[i][j] = Dist[i][k] + Dist[k][j];
                    change[i][j]=1; // mark a change bit
                }
            }
        }
    }
}
static void calLauncherCPU(int B,int Round,int x,int y,int w,int h) {
    int b_i,b_j;
    for ( b_i =  0; b_i < h; ++b_i) {
        for ( b_j = 0; b_j < w; ++b_j) {
            calKernelCPU(B,Round,b_i+x,b_j+y);
        }
    }

}
static struct cudaDeviceProp prop;
static int devicePropGot=0;
static void getProp() {
    if(!devicePropGot) {
        devicePropGot=1;
        cudaGetDeviceProperties(&prop,currentDev);
    }
}

void calLauncher(int B,int Round,int x,int y,int w,int h) {
    dim3 gdim(h,w,1);
    dim3 bdim(B,1,1);
    cudaError_t err;
    if(totalCUDADevice == 0) {
        printf("run in cpu ,because totalCUDADevice=%d\n",totalCUDADevice );
        calLauncherCPU(B,Round,x,y,w,h);
        return;
    }
    int mink=Round*B;
    int maxk=mink+B;
    if(maxk>n) maxk=n;
    getProp();
    if(bdim.x > prop.maxThreadsPerBlock) {
        bdim.x=prop.maxThreadsPerBlock;
    }

    for (int k = mink; k < maxk; ++k) { ///

        Timer tempTime;
        timer_init(&tempTime,"");
        timer_start(&tempTime);
        calKernelGPU<<<gdim,bdim>>>(B,Round,x,y,n,devDist,k,devChange);
        err=cudaDeviceSynchronize();
        timer_end(&tempTime);
        timer_add(timer_compute,&tempTime);

        if(err != cudaSuccess) {
            fprintf(stderr,"%s(gdim=%d,%d,%d)(bid=%d,%d,%d)\n",
                    cudaGetErrorString(err),
                    gdim.x,gdim.y,gdim.z,bdim.x,bdim.y,bdim.z);
        }
    }
}
void cal(int B, int Round, int x,int y,int w,int h)
{
#if DEBUG_ENABLE
    int i,j=0;
    int block_end_x = x + h ;
    int block_end_y = y + w;
    fprintf(logFp,"B=%d, Round=%d, block_start_x=%d, block_start_y=%d, block_width=%d, block_height=%d, \n",B,Round,x,y,w,h);
    fprintf(logFp,"block_end_x=%d, block_end_y=%d,\n",block_end_x,block_end_y);
#endif
    calLauncher(B,Round,x,y,w,h);
#if DEBUG_ENABLE
    fprintf(logFp, "\n");
    i,j=0;
    for ( i = 0; i < n; ++i) {
        for ( j = 0; j < n; ++j) {
            if (Dist[i][j] >= INF)	fprintf(logFp, "INF ");
            else					fprintf(logFp, "%d ", Dist[i][j]);
        }
        fprintf(logFp, "\n");
    }
    fprintf(logFp, "------------------------------------------------\n");
#endif
}


int main(int argc, char* argv[]) {
    struct timeval tv, tv2;

    clock_t endTime;
    unsigned long long start_utime, end_utime;
    MPI_Init(&argc, &argv);


    endTime =clock();
    gettimeofday(&tv, NULL);

    mainRun( argc, argv);

    gettimeofday(&tv2, NULL);
    endTime =clock() - endTime ;

    start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
    end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

    if(MPIID==0){
        printf("Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );
    }

    MPI_Finalize();

    return 0;

}

