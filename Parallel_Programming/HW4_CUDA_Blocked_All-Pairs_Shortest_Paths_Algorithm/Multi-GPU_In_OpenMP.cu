#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <omp.h>
#include <sys/types.h>
#define DEBUG_ENABLE 0
#define ERROR_TRACING 0
#define V 7000
#define INF 1000000
///const int INF = 1000000;
///const int V = 7000;
void input(char *inFileName);
void output(char *outFileName);

void openMP_Block_APSP(int B);
int iceil(int a, int b);
void cal(int B, int Round, int block_start_x, int block_start_y, int block_width, int block_height , int gpuDeviceNo);

int n, m;	// Number of vertices, edges
int Dist[V][V];
int* devDist;
int* devOtherDist;
char* devOtherChange;

///char change[V][V];
char otherChange[V][V];
int otherDist[V][V];

int gpuID=0;
int  otherDeviceNo =0;

FILE *logFp;
char *in1;
char *out1;

static int totalCUDADevice = 0;
static int currentDev=0;


typedef struct Timer {
    char name[256];
    struct timeval begin;
    struct timeval end;
} Timer;

Timer* timer_memcpy,*timer_commu,*timer_compute;
Timer *timer_phase3, *timer_allPhase, *timer_main, *subcard_compute;

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
}

static void __debugCUDACall(cudaError_t err, const char* expr, const char* file, int line) {
    if(err != cudaSuccess) {
        fprintf(stderr,"###Error in File %s Line %d:%s\n",file,line,expr);
        fprintf(stderr,"%s \n",cudaGetErrorString(err));
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
    cudaGetDeviceCount(&totalCUDADevice);
    printf("totalCUDADevice=%d, \n",totalCUDADevice);


    if (totalCUDADevice == 0) {
        printf("No CUDA device found.\n\n");
    } else if (gpuID < totalCUDADevice) {
        printf("set CUDA device=%d, \n",gpuID );
        cudaSetDevice(gpuID);
    } else {
        gpuID =0;
        printf("set CUDA device=%d, \n",gpuID );
        cudaSetDevice(gpuID);
    }

    if (gpuID ==0){
       otherDeviceNo =1;
    }else{
        otherDeviceNo =0;
    }
}

int mainRun(int argc, char* argv[])
{


    if(argc > 4) {
        sscanf(argv[4],"%d",&gpuID);
    }

    initCUDADevice(gpuID);



    omp_set_num_threads(totalCUDADevice);

    timer_memcpy = timer_new("Memcpy");
    timer_commu = timer_new("Communication");
    timer_compute = timer_new("Compute");
    subcard_compute = timer_new("SubCard_Compute");


    timer_phase3 = timer_new("Phase3");

    timer_allPhase= timer_new("All Phase");

    timer_main = timer_new("openMP_Block_APSP");

    logFp = freopen("./omp_log.txt","wr", stderr);

//    in1= "D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\tiny_test_case";
//    out1="D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\output\\tiny_test_case_out";
    //in1= "./Testcase/in2";
    //out1="./output/tiny_test_case_out";

    ///char *in1= "D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\in3";
    ///char *out1="D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\output\\out1";
    ///input( in1 );

    input( argv[1] );
    if(totalCUDADevice > 0) {
        cudaMalloc((void**)&devDist,sizeof(Dist));

        debugCUDACall(cudaSetDevice(otherDeviceNo));
        debugCUDACall(cudaMalloc((void**)&devOtherChange, sizeof(otherChange)));
        debugCUDACall(cudaMalloc((void**)&devOtherDist,sizeof(otherDist)));
        debugCUDACall(cudaSetDevice(gpuID));
    }

    /*
    fprintf(logFp, "\n");
    int i,j=0;
    for ( i = 0; i < n; ++i) {
        for ( j = 0; j < n; ++j) {
            if (Dist[i][j] >= INF)	fprintf(logFp, "INF ");
            else					fprintf(logFp, "%d ", Dist[i][j]);
        }
        fprintf(logFp, "\n");
    }
    */


    if(totalCUDADevice > 0) {
        timer_start(timer_memcpy);
        debugCUDACall(cudaMemcpy(devDist,&Dist[0][0],sizeof(int)*V*n,cudaMemcpyHostToDevice));
        timer_end(timer_memcpy);
    }


    int B = 128;
    if(argc > 3) {
        sscanf(argv[3],"%d",&B);
    }

    printf("*** B=%d, source=%s, output=%s,\n",B,argv[1],argv[2]);



      timer_start(timer_allPhase);
    timer_end(timer_allPhase);

    timer_start(subcard_compute);
    timer_end(subcard_compute);

    timer_start(timer_phase3);
    timer_end(timer_phase3);

    timer_start(timer_compute);
    timer_end(timer_compute);


    timer_start(timer_main);
    openMP_Block_APSP(B);
    timer_end(timer_main);


    if(totalCUDADevice >0) {
        Timer tempMemcpy;
        timer_init(&tempMemcpy,"");
        timer_start(&tempMemcpy);
        debugCUDACall(cudaMemcpy(&Dist[0][0],devDist,sizeof(int)*n*V,cudaMemcpyDeviceToHost));
        timer_end(&tempMemcpy);
        timer_add(timer_memcpy,&tempMemcpy);
        debugCUDACall(cudaFree(devDist));
    }

    output(argv[2]);
    ///output( out1 );
    timer_print(timer_memcpy,stdout);
    timer_print(timer_commu,stdout);
    timer_print(timer_allPhase,stdout);
    timer_print(timer_compute,stdout);


    timer_print(timer_phase3,stdout);
    timer_print(subcard_compute,stdout);
    timer_print(timer_main,stdout);

    fclose(logFp);
    timer_delete(timer_memcpy);
    timer_delete(timer_commu);
    timer_delete(timer_allPhase);
    timer_delete(timer_compute);
    timer_delete(timer_phase3);
     timer_delete(subcard_compute);
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

void readChangeAndData() {
    if(totalCUDADevice > 1) {
         # if ERROR_TRACING
        printf("readChangeAndData \n");
        fprintf(logFp, "readChangeAndData \n");
         #endif

        Timer tempMemcpy;
        timer_init(&tempMemcpy,"");
        timer_start(&tempMemcpy);

        debugCUDACall(cudaMemcpy(otherDist,devOtherDist,sizeof(int)*n*V,cudaMemcpyDeviceToHost));
        debugCUDACall(cudaMemcpy(otherChange,devOtherChange,sizeof(char)*n*V,cudaMemcpyDeviceToHost));
        timer_end(&tempMemcpy);
        timer_add(timer_memcpy,&tempMemcpy);

    }
}

void copyFormSubGPUToMainGPU(int round){
    Timer temp;
    timer_init(&temp,"");

    debugCUDACall(cudaSetDevice(otherDeviceNo));
    readChangeAndData();


    Timer tempMemcpy;
    timer_init(&tempMemcpy,"");
    timer_start(&tempMemcpy);
    /// copy data form device first from main gpu
    debugCUDACall(cudaSetDevice(gpuID));
    debugCUDACall(cudaMemcpy(Dist,devDist,sizeof(int)*n*V,cudaMemcpyDeviceToHost));

    timer_end(&tempMemcpy);
    timer_add(timer_memcpy,&tempMemcpy);


    timer_start(&temp);
    # if ERROR_TRACING
     fprintf(logFp, "round=%d, otherChange[0][2]=%d \n",round,otherChange[0][2]);
    fprintf(logFp, "\n\n####round=%d,Change Change Change Dist \n",round);
    #endif

    #pragma omp parallel for
    for(int iL=0; iL<V*n; ++iL) {
        int i = iL%V;
        int j = iL/V;
        if (i >= n) continue;
        if(otherChange[j][i]) {
               Dist[j][i]=otherDist[j][i];
               # if ERROR_TRACING
               if (round ==0) fprintf(logFp, "[%d][%d]=%d, ",j,i,Dist[j][i]);
               #endif
        }
    }


    # if ERROR_TRACING
    if (round ==1 ){
        fprintf(logFp, "\n\n####round=%d,Total Dist \n", round);
        ///#pragma omp parallel for
        for(int iL=0; iL<V*n; ++iL) {
            int i = iL%V;
            int j = iL/V;
            if (j ==0 && i== 2){
                 fprintf(logFp, "\n### [%d][%d], iL=%d, \n",j,i,iL);
            }
            if (i >= n) continue;
            fprintf(logFp, "[%d][%d]=%d, ",j,i,Dist[j][i]);
        }
    }
    #endif

    ///memset(otherChange,0,sizeof(char)*n*V);
    ///debugCUDACall(cudaMemset(devOtherChange,0,sizeof(char)*n*V));

    timer_end(&temp);
    timer_add(timer_commu,&temp);


    if(totalCUDADevice > 1) {
        Timer temp;
        timer_init(&temp,"");
        timer_start(&temp);
        debugCUDACall(cudaMemcpy(devDist,Dist,sizeof(int)*n*V,cudaMemcpyHostToDevice));
        timer_end(&temp);
        timer_add(timer_memcpy,&temp);
    }

}

void validateVaule(int round){


     if (totalCUDADevice > 1 ){

            debugCUDACall(cudaMemcpy(Dist,devDist,sizeof(int)*n*V,cudaMemcpyDeviceToHost));
            char *fileName = "omp_in3_dev_";
            char *extFile =".txt";
            char roundString[10];

            sprintf(roundString,"%d",round);
            fileName = stringConcat(fileName, roundString);
            fileName = stringConcat(fileName, extFile);

            FILE *outfile = fopen(fileName, "w");
            fprintf(outfile, "round=%d, \n",round);

            int i,j=0;
            for ( i = 0; i < n; ++i) {
                for ( j = 0; j < n; ++j) {
                    fprintf(outfile, "[%d][%d]=%d, ",i,j,Dist[i][j]);
                }
                fprintf(outfile, "\n");
            }

            fclose(outfile);
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

void openMP_Block_APSP(int B) {
    int round = iceil(n, B);
#if DEBUG_ENABLE
    fprintf(logFp,"round=%d ====================================\n",round);
#endif


    int r=0;
    for ( r = 0; r < round; ++r) {

        debugCUDACall(cudaSetDevice(gpuID));

         Timer phase1_2Time;
         timer_init(&phase1_2Time,"");
         timer_start(&phase1_2Time);

        ///* Phase 1*/
#if DEBUG_ENABLE
        fprintf(logFp,"[Phase1] r=%d ====================================\n",r);
#endif
        /// B, Round, block_start_x, block_start_y, block_width, block_height
        cal(B,	r,	  r,	r,	            1,	           1 , gpuID);

#if DEBUG_ENABLE
        fprintf(logFp,"[Phase2] r=%d \n",r);
        fprintf(logFp,"         r=%d 1. \n",r);
#endif
        ///* Phase 2*/  /// (y,x) ==> (column,row) ==> (r,0)
        cal(B, r,     r,     0,             r,             1 , gpuID); ///front row
#if DEBUG_ENABLE
        fprintf(logFp,"         r=%d 2. \n",r);
#endif
        cal(B, r,     r,  r +1,  round - r -1,             1 , gpuID); /// back row
#if DEBUG_ENABLE
        fprintf(logFp,"         r=%d 3. \n",r);
#endif
        cal(B, r,     0,     r,             1,             r , gpuID); /// up column
#if DEBUG_ENABLE
        fprintf(logFp,"         r=%d 4. \n",r);
#endif
        cal(B, r,  r +1,     r,             1,  round - r -1 , gpuID); /// down column
#if DEBUG_ENABLE
        fprintf(logFp,"[Phase3] r=%d \n",r);
#endif

        timer_end(&phase1_2Time);
        timer_add(timer_allPhase, &phase1_2Time);



        if(totalCUDADevice > 1){
              debugCUDACall(cudaMemcpy(Dist,devDist,sizeof(int)*n*V,cudaMemcpyDeviceToHost));

              # if ERROR_TRACING
              if (r ==1){
                fprintf(logFp, "\n\nround=%d, #####Finish pharse2!!\n",r);
                int i,j=0;
                for ( i = 0; i < n; ++i) {
                    for ( j = 0; j < n; ++j) {
                        fprintf(logFp, "[%d][%d]=%d, ",i,j,Dist[i][j]);
                    }
                    fprintf(logFp, "\n\n\n");
                }
              }
              # endif

              ///copy main device data to another device
              debugCUDACall(cudaSetDevice(otherDeviceNo));
              memset(otherChange,0,sizeof(char)*n*V);

              Timer tempMemcpy;
              timer_init(&tempMemcpy,"");
              timer_start(&tempMemcpy);
              debugCUDACall(cudaMemset(devOtherChange,0,sizeof(char)*n*V));
              debugCUDACall(cudaMemcpy(devOtherDist,Dist,sizeof(int)*n*V,cudaMemcpyHostToDevice));
              timer_end(&tempMemcpy);
              timer_add(timer_memcpy,&tempMemcpy);

              debugCUDACall(cudaSetDevice(gpuID));
        }

        Timer tempTime;
        timer_init(&tempTime,"");
        timer_start(&tempTime);
         /// Phase 3/
         #pragma omp parallel
         {
            unsigned int cpu_thread_id = omp_get_thread_num();
            unsigned int num_cpu_threads = omp_get_num_threads();

            ///printf("Start CPU thread=%d, (total threads=%d)\n",cpu_thread_id, num_cpu_threads);

            if (cpu_thread_id == gpuID){

                 debugCUDACall(cudaSetDevice(gpuID));
                 cal(B, r,     0,     0,            r,             r , gpuID);  ///2 quadrant
                 cal(B, r,     0,  r +1,  round -r -1,             r , gpuID);  /// 1 quadrant

                 if (totalCUDADevice ==1){
                       # if ERROR_TRACING
                     printf("### Error totalCUDADevice=%d \n",totalCUDADevice);
                     fprintf(logFp, "### Error totalCUDADevice=%d \n",totalCUDADevice);
                       # endif
                     cal(B, r,  r +1,     0,            r,  round - r -1 , otherDeviceNo);   /// 3 quadrant
                     cal(B, r,  r +1,  r +1,  round -r -1,  round - r -1 , otherDeviceNo);   /// 4 quadrant
                 }

            }else{
                     # if ERROR_TRACING
                 printf("Another GPU, round=%d, 3 quardrant \n",r);
                 fprintf(logFp, "Another GPU, round=%d, 3 quardrant \n",r);
                    # endif
                 debugCUDACall(cudaSetDevice(otherDeviceNo));
                 cal(B, r,  r +1,     0,            r,  round - r -1 , otherDeviceNo);   /// 3 quadrant

                   # if ERROR_TRACING
                 printf("Another GPU, round=%d, 4 quardrant \n",r);
                 fprintf(logFp, "Another GPU, round=%d, 4 quardrant \n",r);
                   # endif
                 cal(B, r,  r +1,  r +1,  round -r -1,  round - r -1 , otherDeviceNo);   /// 4 quadrant
            }
             ///fprintf(logFp,"End CPU thread=%d, (total threads=%d)\n",cpu_thread_id, num_cpu_threads);
        }

        timer_end(&tempTime);
        timer_add(timer_phase3,&tempTime);
        timer_add(timer_allPhase, &tempTime);

         # if ERROR_TRACING
        fprintf(logFp,"AAAA---------------------------BBBB\n");
         # endif


        if(totalCUDADevice > 1){
          copyFormSubGPUToMainGPU(r);

          # if ERROR_TRACING
          debugCUDACall(cudaSetDevice(gpuID));
          validateVaule(r);
          # endif
        }

    }

}

static __global__ void calKernelGPU(int B,int Round,int x,int y,int n,int* dDist,int k, char* dchange) {
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

        # if ERROR_TRACING
        if (Round ==1 && i==0 && j ==2 && valIJ !=1000000 ){
            printf("###[%d][%d]=valIJ=%d ",i,j, valIJ);
            if (dchange != NULL ){
                printf("dchange[%d][%d]=%d, ",i,j,dchange[i*V+j]);
            }
        # endif

        if (valIK + valKJ < valIJ) {
            valIJ = valIK + valKJ;
            dDist[i*V+j]=valIJ;

             if (dchange != NULL ){
                dchange[i*V+j]=1;  // mark a change bit
            }

            # if ERROR_TRACING
            if (dchange != NULL ){
                dchange[i*V+j]=1;  // mark a change bit
                if (Round ==1 && i==0 && j ==2) printf("sub [%d][%d]=%d, ",i,j,valIJ);
            }else{
                 if (Round ==1 && i==0 && j ==2) printf("main [%d][%d]=%d, ",i,j,valIJ);
            }
            # endif
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
                if (Dist[i][k] + Dist[k][j] < Dist[i][j])
                    Dist[i][j] = Dist[i][k] + Dist[k][j];
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

void calLauncher(int B,int Round,int x,int y,int w,int h, int deviceID) {
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
        if (deviceID == gpuID){
            Timer tempTime;
            timer_init(&tempTime,"");
            timer_start(&tempTime);
            calKernelGPU<<<gdim,bdim>>>(B,Round,x,y,n,devDist,k, NULL );
            err=cudaDeviceSynchronize();
            timer_end(&tempTime);
            timer_add(timer_compute,&tempTime);

        }else if (deviceID == otherDeviceNo){
            Timer tempTime;
            timer_init(&tempTime,"");
            timer_start(&tempTime);

            calKernelGPU<<<gdim,bdim>>>(B,Round,x,y,n,devOtherDist,k, devOtherChange );
             err=cudaDeviceSynchronize();

            timer_end(&tempTime);
            timer_add(subcard_compute,&tempTime);

        }else{
           printf("### Error Error Error ####");
        }

        err=cudaDeviceSynchronize();
        if(err != cudaSuccess) {
            fprintf(stderr,"%s(gdim=%d,%d,%d)(bid=%d,%d,%d)\n",
                    cudaGetErrorString(err),
                    gdim.x,gdim.y,gdim.z,bdim.x,bdim.y,bdim.z);
        }
    }
}


void cal(int B, int Round, int x,int y,int w,int h, int deviceID)
{
#if DEBUG_ENABLE
    int i,j=0;
    int block_end_x = x + h ;
    int block_end_y = y + w;
    fprintf(logFp,"B=%d, Round=%d, block_start_x=%d, block_start_y=%d, block_width=%d, block_height=%d, \n",B,Round,x,y,w,h);
    fprintf(logFp,"block_end_x=%d, block_end_y=%d,\n",block_end_x,block_end_y);
#endif
    calLauncher(B,Round,x,y,w,h, deviceID);
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


    endTime =clock();
    gettimeofday(&tv, NULL);

    mainRun( argc, argv);

    gettimeofday(&tv2, NULL);
    endTime =clock() - endTime ;

    start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
    end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

    printf("Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );

    return 0;
}
