#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#define DEBUG_ENABLE 0
#define V 7000
#define INF 1000000
///const int INF = 1000000;
///const int V = 7000;
void input(char *inFileName);
void output(char *outFileName);

void block_APSP(int B);
int iceil(int a, int b);
void cal(int B, int Round, int block_start_x, int block_start_y, int block_width, int block_height);

int n, m;	// Number of vertices, edges
int Dist[V][V];
///int Dist[7000][7000];
FILE *logFp;
char *in1;
char *out1;
int main(int argc, char* argv[])
{
    logFp = freopen("./log.txt","wr", stderr);

//    in1= "D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\tiny_test_case";
//    out1="D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\output\\tiny_test_case_out";
    //in1= "./Testcase/in1";
    //out1="./output/tiny_test_case_out";

    ///char *in1= "D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\in3";
    ///char *out1="D:\\c\\codeblock\\c\\parallel_programming\\hw4\\hw4\\Testcase\\output\\out1";



    ///input( in1 );
    input( argv[1] );

    fprintf(logFp, "\n");
    int i,j=0;
    for ( i = 0; i < n; ++i) {
        for ( j = 0; j < n; ++j) {
            if (Dist[i][j] >= INF)	fprintf(logFp, "INF ");
            else					fprintf(logFp, "%d ", Dist[i][j]);
        }
        fprintf(logFp, "\n");
    }

    int B = 1;

    if(argc > 3) {
        sscanf(argv[3],"%d",&B);
    }

    printf("## B=%d, source=%s, output=%s,\n",B,argv[1],argv[2]);

    ///block_APSP(B);
    block_APSP_Time(B);

    output(argv[2]);
    ///output( out1 );

    fclose(logFp);
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

int iceil(int a, int b){
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


int block_APSP_Time(int B){
     struct timeval tv, tv2;

    clock_t endTime;
    unsigned long long start_utime, end_utime;


    endTime =clock();
    gettimeofday(&tv, NULL);

    block_APSP(B);

     gettimeofday(&tv2, NULL);
    endTime =clock() - endTime ;

    start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
	end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

    printf("Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );

     return 0;

}


void block_APSP(int B){
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
        cal(B,	r,	  r,	r,	            1,	           1);

#if DEBUG_ENABLE
        fprintf(logFp,"[Phase2] r=%d \n",r);
        fprintf(logFp,"         r=%d 1. \n",r);
#endif
        ///* Phase 2*/  /// (y,x) ==> (column,row) ==> (r,0)
        cal(B, r,     r,     0,             r,             1); ///front row
#if DEBUG_ENABLE
        fprintf(logFp,"         r=%d 2. \n",r);
#endif
        cal(B, r,     r,  r +1,  round - r -1,             1); /// back row
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
        cal(B, r,  r +1,     0,            r,  round - r -1);   /// 3 quadrant
#if DEBUG_ENABLE
        fprintf(logFp,"         r=%d 4. \n",r);
#endif
        cal(B, r,  r +1,  r +1,  round -r -1,  round - r -1);   /// 4 quadrant
    }
}
static void calKernelCPU(int B,int Round,int b_i,int b_j){
//////////////////////
            int k=0;
            /// To calculate B*B elements in the block (b_i, b_j)
            /// For each block, it need to compute B times
            int block_internal_start_x 	= b_i * B;
            int block_internal_end_x 	= (b_i +1) * B;
            int block_internal_start_y = b_j * B;
            int block_internal_end_y 	= (b_j +1) * B;
#if DEBUG_ENABLE
            fprintf(logFp,"b_i=%d, b_j=%d, k=%d,B=%d,  \n",b_i,b_j,k,B);
            fprintf(logFp,"block_internal_start_x=(b_i * B=)%d, block_internal_end_x=%d, block_internal_start_y=%d,block_internal_end_y=%d,  \n",block_internal_start_x,block_internal_end_x,block_internal_start_y,block_internal_end_y);
#endif
            if (block_internal_end_x > n)	block_internal_end_x = n;
            if (block_internal_end_y > n)	block_internal_end_y = n;
            for ( k = Round * B; k < (Round +1) * B && k < n; ++k) { ///
                int i,j;
                /// To calculate original index of elements in the block (b_i, b_j)
                /// For instance, original index of (0,0) in block (1,2) is (2,5) for V=6,B=2
                for ( i = block_internal_start_x; i < block_internal_end_x; ++i) {
                    for ( j = block_internal_start_y; j < block_internal_end_y; ++j) {
#if DEBUG_ENABLE
                        fprintf(logFp,"Dist[i=%d][k=%d]=%d + Dist[k=%d][j=%d]=%d, Dist[i=%d][j=%d]=%d  \n",i,k,Dist[i][k],k,j,Dist[k][j],i,j, Dist[i][j]);
#endif
                        if (Dist[i][k] + Dist[k][j] < Dist[i][j])
                            Dist[i][j] = Dist[i][k] + Dist[k][j];
                    }
                }
            }
}
static void calLauncher(int B,int Round,int x,int y,int w,int h){
    int b_i,b_j;
//    for ( b_i =  block_start_x; b_i < block_end_x; ++b_i) {
//        for ( b_j = block_start_y; b_j < block_end_y; ++b_j) {
    for ( b_i =  0; b_i < h; ++b_i) {
        for ( b_j = 0; b_j < w; ++b_j) {

            calKernelCPU(B,Round,b_i+x,b_j+y);
        }
    }
}
void cal(int B, int Round, int x,int y,int w,int h)
{
    int i,j=0;
    int block_end_x = x + h ;
    int block_end_y = y + w;
#if DEBUG_ENABLE
    fprintf(logFp,"B=%d, Round=%d, block_start_x=%d, block_start_y=%d, block_width=%d, block_height=%d, \n",B,Round,x,y,w,h);
#endif
    #if DEBUG_ENABLE
    fprintf(logFp,"block_end_x=%d, block_end_y=%d,\n",block_end_x,block_end_y);
#endif
    calLauncher(B,Round,x,y,w,h);
    /*
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
    */
}

