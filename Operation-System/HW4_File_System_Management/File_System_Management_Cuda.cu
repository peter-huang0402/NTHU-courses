#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include<string.h>
#include <sys/time.h>
#include <time.h>


///#define STORAGE_SIZE 1085440
#define LOG_ENABLE 0
#define MAX_FILE_SIZE 1048576
#define DATAFILE "./data.bin"
#define OUTFILE "./snapshot.bin"
#define KBytes 1024
#define MAX_NUM_OF_FILE 1024
#define fileNameLength 20
#define FREE_BIT 0
#define USED_BIT 1

#define G_READ 0
#define G_WRITE 1
#define RM 2
#define LS_S 3
#define LS_D 4

#define ERROR_LIST_FILE       904000000
#define ERROR_FCB_FULL        903000000
#define ERROR_FILE_BLOCK_FULL 902000000
#define ERROR_FILE_NOT_EXIST  901000000
#define U32_NOT_NUMBER  900000000 ///%u
#define U16_NOT_NUMBER  65000 /// 0~ 65535(六萬五千) (%hu)

typedef unsigned char uchar;  ///1byte
typedef uint32_t u32;         ///4bytes, 32bits
typedef uint16_t u16;

///uchar *volume;

__device__ __managed__ uchar bitVector[MAX_NUM_OF_FILE];  /// 1K =  1* 1024
__device__ __managed__ uchar *file_content;  /// 1024K = 1024 * 1024
__device__ __managed__ u16 fileCreatedTimeCount =0;  /// 16 bytes
__device__ __managed__ u16 fileModifiedTimeCount =0; ///  16 bytes

#pragma pack(push) ///把原本的對齊設定push進stack
#pragma pack(1)
typedef struct FCB{         /// total=  28 byte
   char file_name[fileNameLength];  /// 20 byte
   u16 fileSize;              ///        2 byte
   u16 fileCreatedTime;        ///       2 byte
   u16 fileModifiedTime;  ///            2 byte
   u16 file_content_index;   ///         2 byte
};
#pragma pack(pop)  ///把原本的對齊設定pop出來

__device__ __managed__ struct FCB *fcbs;   /// 28K =28 *1024

__device__  void swapFCB(int iFcbIndex,int jFcbIndex);

__device__  void initCharVaule(char *names,char value, int size ){
   int i=0;
   for (i=0;i<=size;i++){
       names[i]=value;
   }
}

 __device__ void init_volume(){

  int i=0;
  for (i=0;i<MAX_NUM_OF_FILE;i++ ){
      ///printf("i=%d ",i);
      bitVector[i]=FREE_BIT;   /// 0 != '0'
      /// if (bitVector[i] == 0) {}
      ///printf("bitVector[%d]=%d; ",i,bitVector[i]); ///%d or %u


      initCharVaule(fcbs[i].file_name,'\0',fileNameLength );   /// 0 == '\0'  but != '0'
      fcbs[i].fileSize=U16_NOT_NUMBER;
      fcbs[i].fileCreatedTime=U16_NOT_NUMBER;
      fcbs[i].fileModifiedTime=U16_NOT_NUMBER;
      fcbs[i].file_content_index=U16_NOT_NUMBER;
  }

  ///printf("fileSize[%d]=%d; \n", (KBytes-1) ,fileSize[(KBytes-1)] ); ///%d or %u
  ///printf("fileCreatedTime[%d]=%d; \n", (KBytes-1) ,fileCreatedTime[(KBytes-1)] );
  ///printf("fileModifiedTimeSize[%d]=%d; \n", (KBytes-1) ,fileModifiedTimeSize[(KBytes-1)] );



  for (i=0;i<MAX_FILE_SIZE;i++ ){
        file_content[i]='\0';
  }

  ///printf("file_content[%d]=%d; \n",(MAX_FILE_SIZE-1),file_content[(MAX_FILE_SIZE-1)]); ///%d or %u


}


__device__  int stringLength(const char *symbols){
    int i=0;
    for (i=0;symbols[i] !='\0';i++ ){
    }
    return i;

}


__device__  void gpuStrncpy(const char *source,  char *destination, int size  ){

    int i=0;
    char sChar = *source;

     while( sChar !='\0' && i<size  ){
        *destination = sChar;
        destination++;
        source++;
        sChar = *source;
        i++;
    }

    /**
         /// Key !! when convert c code to cuda code.
         /// 1. in cuda cannot use pointer array!!! like  *(source+i)
         /// 2. *destination will be assigned value like destination = '\0',
         ///     so *destination   cannot declare const!!!

    while( *(source+i) !='\0' && i<size  ){
        *destination = *(source+i);
        destination++;
        i++;
    }**/

    ///if ( *(source+i) == '\0'){
       *destination = '\0';
    ///}

}

__device__  int gpuStrcmp( const char *str1, const char *str2){

   ///printf("str1=%s, str2=%s, *(str2+1)=%c, str2[1]=%c, *str2=%c,\n,",&str1[0],str2,*(str2+1),str2[1],*str2 );
   ///printf("str1=%s, str2=%s, str2[1]=%c\n,",&str1[0],str2,str2[1]);
   /// str1 == str2 = 0, str1 > str2 =  >0 , str1 <str2 =  <0
   char s1,s2;

   s1 = *str1;
   s2 = *str2;


   ///printf("1. s1=%c,s2=%c,s1= %d,s2=%d \n",s1,s2,s1,s2);
   ///if (s1 == NULL || s2 ==NULL){
   ///   printf("s1 == NULL or s2 == NULL\n");
   ///}



   while (s1 ==s2){
        ///printf("s1=%c,s2=%c,s1= %d,s2=%d \n",s1,s2,s1,s2);
       if (s1 == '\0' || s2 =='\0'){
           break;
       }
       ++str2;
       s2= *str2;
       ++str1;
       s1 = *str1;
   }

   return ((const unsigned char )s1 - (const unsigned char)(s2));

}



__device__  u16 findFreeFCB(){
    int i=0;
    for (i=0;i<MAX_NUM_OF_FILE;i++){
        if (fcbs[i].file_name[0] =='\0'){
            return i;
        }
    }
    return U16_NOT_NUMBER;
}

__device__  u16 findFileByName(const char* fileName) {
   int i=0;

   for (i=0;i<MAX_NUM_OF_FILE;i++){
       if ( fcbs[i].file_name[0] == '\0') {
            #if LOG_ENABLE
             ///printf("file_name=0, fcbs[%d].file_name=%s,\n",i,fcbs[i].file_name);
            #endif
            continue; /// unused file.
       }
       if (gpuStrcmp(fileName, fcbs[i].file_name) ==0 ){
            #if LOG_ENABLE
             printf("find fileName=%s, fcbs[%d].file_name=%s, index=%hu\n",fileName,i,fcbs[i].file_name,fcbs[i].file_content_index);
            #endif
           return i;

       }
   }
   return U16_NOT_NUMBER;
}

__device__  u16 findFreeBitVector(){
    int i=0;
    for (i=0;i<MAX_NUM_OF_FILE;i++){
        if (bitVector[i] == FREE_BIT){
            return i;
        }
    }

    return U16_NOT_NUMBER;
}

__device__  void shiftFileCreatedTimeCountForOverFlow(){

    #if LOG_ENABLE
    fprintf(logFp ,"[shiftFileCreatedTimeCountForOverFlow] fileCreatedTimeCount=%hu,\n",fileCreatedTimeCount );
    fflush(stderr);
    #endif

   int i,j=0;

   for (i=0;i< MAX_NUM_OF_FILE;i++){
         if (fcbs[i].file_name[0] == '\0' ) continue;
         for (j=0;j<MAX_NUM_OF_FILE ;j++){
             if (fcbs[j].file_name[0] == '\0' || i==j ) continue;
             /// from small to large <
             /// from large to small >
             if ( fcbs[i].fileCreatedTime  < fcbs[j].fileCreatedTime ){
                 swapFCB(i,j);
             }
         }
   }

   fileCreatedTimeCount =1;

    for (i=0;i< MAX_NUM_OF_FILE;i++){
         if (fcbs[i].file_name[0] == '\0' ) continue;

          #if LOG_ENABLE
          fprintf(logFp ,"fcbs[%d]=%s, fileModifiedTime=%hu, shift from fileCreatedTime=%hu, to fileCreatedTime=%hu\n",i,fcbs[i].file_name,fcbs[i].fileModifiedTime  ,fcbs[i].fileCreatedTime,fileCreatedTimeCount );
          fflush(stderr);
          #endif

          fcbs[i].fileCreatedTime =fileCreatedTimeCount;
          fileCreatedTimeCount++;
    }
}


__device__  u16 createFile(u16 bitLocation, const char *fileName){

    u16 fcbIndex = findFreeFCB();

    if (fcbIndex == U16_NOT_NUMBER){
        return U16_NOT_NUMBER;
    }

    fileCreatedTimeCount++;

    bitVector[bitLocation] = USED_BIT;

    int file_name_length = stringLength(fileName);
    gpuStrncpy(fileName,fcbs[fcbIndex].file_name, file_name_length);
    fcbs[fcbIndex].fileCreatedTime = fileCreatedTimeCount;
    fcbs[fcbIndex].fileModifiedTime = fileCreatedTimeCount;
    fcbs[fcbIndex].fileSize=0;
    fcbs[fcbIndex].file_content_index= bitLocation;

    #if LOG_ENABLE
    printf("file_name_length=%d, fcbs[%d].file_name=%s, fileCreatedTime=%hu, fileSize=%hu, file_content_index=%hu, \n",file_name_length,fcbIndex,fcbs[fcbIndex].file_name,fcbs[fcbIndex].fileCreatedTime, fcbs[fcbIndex].fileSize , fcbs[fcbIndex].file_content_index );
    #endif

     if (fileCreatedTimeCount >= U16_NOT_NUMBER){
        shiftFileCreatedTimeCountForOverFlow();
    }

    return fcbIndex;
}

__device__  u32 open(const char* fileName, int accessMode){

   u16 fcbIndex = findFileByName(fileName);
   u16 bitIndex = U16_NOT_NUMBER;


   if (fcbIndex ==U16_NOT_NUMBER){
       if (accessMode == G_READ){
            return  ERROR_FILE_NOT_EXIST;
       }else{
          bitIndex = findFreeBitVector();
          if (bitIndex == U16_NOT_NUMBER){
             return ERROR_FILE_BLOCK_FULL;
          }
          fcbIndex = createFile(bitIndex , fileName);
          if (fcbIndex == U16_NOT_NUMBER){
              return ERROR_FCB_FULL;
          }
       }
   }

   return (u32) fcbIndex;

}

__device__ void clearFileContent(u32 fcbIndex){
    u16 fb = fcbIndex;
    int i=0;

    if (fcbs[fb].fileSize ==0) {
       #if LOG_ENABLE
       printf( "##clearFileContent## fileSize==0, fcbIndex=%u, fb=%hu, fileSize=%hu\n",fcbIndex,fb,fcbs[fb].fileSize );
       #endif
       return;
    }

    i = fcbs[fb].file_content_index * KBytes;

    #if LOG_ENABLE
    printf("fcbIndex=%u, fb=%hu, fileSize=%hu, file_content_index=%hu, real location= (file_content_index * KBytes)=%d,  \n",fcbIndex,fb,fcbs[fb].fileSize,fcbs[fb].file_content_index, i );
    #endif

    for (;i<fcbs[fb].fileSize;i++){
        file_content[i]='\0';
    }

}


__device__ void shiftFileModifiedTimeCountForOverFlow(){

    #if LOG_ENABLE
    fprintf(logFp ,"[shiftFileModifiedTimeCountForOverFlow] fileModifiedTimeCount=%hu,\n",fileModifiedTimeCount );
    fflush(stderr);
    #endif

   int i,j=0;

   for (i=0;i< MAX_NUM_OF_FILE;i++){
         if (fcbs[i].file_name[0] == '\0' ) continue;
         for (j=0;j<MAX_NUM_OF_FILE ;j++){
             if (fcbs[j].file_name[0] == '\0' || i==j ) continue;
             /// from small to large <
             /// from large to small >
             if ( fcbs[i].fileModifiedTime  < fcbs[j].fileModifiedTime ){
                 swapFCB(i,j);
             }
         }
   }

   fileModifiedTimeCount =1;

    for (i=0;i< MAX_NUM_OF_FILE;i++){
         if (fcbs[i].file_name[0] == '\0' ) continue;
         #if LOG_ENABLE
         fprintf(logFp ,"fcbs[%d]=%s, fileCreatedTime=%hu, shift from fileModifiedTime=%hu, to fileModifiedTime=%hu\n",i,fcbs[i].file_name,fcbs[i].fileCreatedTime  ,fcbs[i].fileModifiedTime,fileModifiedTimeCount );
         fflush(stderr);
         #endif
         fcbs[i].fileModifiedTime =fileModifiedTimeCount;
         fileModifiedTimeCount++;
    }
}


__device__ u32 write(uchar *input, u32 size, u32 fcbIndex){
    u16 fb = U16_NOT_NUMBER;
    int i =0;
    int fileBlockLocation =0;


    if (fcbIndex >= U32_NOT_NUMBER) {
          # if LOG_ENABLE
        printf("[Error:write] fcbIndex=%u, fb=%hu >= U32_NOT_NUMBER\n",fcbIndex,fb );
         #endif

        return fcbIndex;
    }

    clearFileContent(fcbIndex);

    fileModifiedTimeCount++;

    fb = fcbIndex;

    fileBlockLocation =  fcbs[fb].file_content_index * KBytes;

    if (size > KBytes){
         # if LOG_ENABLE
        printf("[Waring] fcbIndex=%u, fb=%hu, size=%u > 1024,\n",fcbIndex,fb,size );
         #endif
        size = KBytes;
    }

    for (;i<size;i++){
        file_content[fileBlockLocation + i] = input[i];
    }

    fcbs[fb].fileModifiedTime= fileModifiedTimeCount;
    fcbs[fb].fileSize = (u16) size;


    if (fileModifiedTimeCount >= U16_NOT_NUMBER){
        shiftFileModifiedTimeCountForOverFlow();
    }


    return size;
}


__device__ u32 read(uchar *output, u32 size, u32 fcbIndex){
    u16 fb = fcbIndex;
    int i =0;
    int fileBlockLocation =0;

    if (fcbIndex >= U32_NOT_NUMBER) {
         # if LOG_ENABLE
        printf("[Error:read] fcbIndex=%u, fb=%hu >= U32_NOT_NUMBER\n",fcbIndex,fb );
         #endif
        return fcbIndex;
    }

    fileBlockLocation =  fcbs[fb].file_content_index * KBytes;

    if (size > KBytes){
         # if LOG_ENABLE
        printf("[Waring.1] fcbIndex=%u, fb=%hu, size=%u > 1024,\n",fcbIndex,fb,size );
         #endif
        size = KBytes;
    }

    if (size > fcbs[fb].fileSize ){
         # if LOG_ENABLE
        printf("[Waring.2] fcbIndex=%u, fb=%hu, size=%u > fileSize=%hu,\n",fcbIndex,fb,size,fcbs[fb].fileSize );
         #endif

        size = fcbs[fb].fileSize;
    }

     for (;i<size;i++){
          output[i] = file_content[ (fileBlockLocation + i)];
     }
     return size;

}

__device__  u32 gsys(int mode, const char* fileName ){
    /// do remove file
    u16 fcbIndex = findFileByName(fileName);

    if (fcbIndex == U16_NOT_NUMBER){
         # if LOG_ENABLE
        printf("[Error:gsy(RM)] List file Error. fcbIndex=%hu,\n",fcbIndex );
         #endif

        return ERROR_LIST_FILE;
    }

    clearFileContent(fcbIndex);

    initCharVaule(fcbs[fcbIndex].file_name,'\0',fileNameLength );
    fcbs[fcbIndex].fileSize  = U16_NOT_NUMBER;
    fcbs[fcbIndex].fileCreatedTime=U16_NOT_NUMBER;
    fcbs[fcbIndex].fileModifiedTime=U16_NOT_NUMBER;

    /// free bitVector before free fcbs.file_content_index
    bitVector[fcbs[fcbIndex].file_content_index] = FREE_BIT;


    fcbs[fcbIndex].file_content_index=U16_NOT_NUMBER;


    return 1;
}


__device__  void swapFCB(int iFcbIndex,int jFcbIndex){
    # if LOG_ENABLE
       printf("[swapFCB] iFcbIndex=%d,jFcbIndex=%d,\n",iFcbIndex,jFcbIndex);
      #endif

    struct FCB fcb;
    initCharVaule(fcb.file_name,'\0',fileNameLength );   /// 0 == '\0'  but != '0'
    fcb.fileSize=U16_NOT_NUMBER;
    fcb.fileCreatedTime=U16_NOT_NUMBER;
    fcb.fileModifiedTime=U16_NOT_NUMBER;
    fcb.file_content_index=U16_NOT_NUMBER;

    /// copy j data to temp
    gpuStrncpy(fcbs[jFcbIndex].file_name,fcb.file_name , stringLength(fcbs[jFcbIndex].file_name));
    fcb.fileSize = fcbs[jFcbIndex].fileSize;
    fcb.file_content_index = fcbs[jFcbIndex].file_content_index;
    fcb.fileCreatedTime = fcbs[jFcbIndex].fileCreatedTime;
    fcb.fileModifiedTime = fcbs[jFcbIndex].fileModifiedTime;

     # if LOG_ENABLE
    printf("j=%d puts into temp, filename=%s, filesize=%u, file_content_index=%hu, fileCreatedTime=%hu, fileModifiedTime=%hu, \n",jFcbIndex,fcb.file_name ,fcb.fileSize ,fcb.file_content_index , fcb.fileCreatedTime,  fcb.fileModifiedTime );
     #endif

    /// copy i data to j
    gpuStrncpy(fcbs[iFcbIndex].file_name, fcbs[jFcbIndex].file_name , stringLength(fcbs[iFcbIndex].file_name));
    fcbs[jFcbIndex].fileSize = fcbs[iFcbIndex].fileSize;
    fcbs[jFcbIndex].file_content_index = fcbs[iFcbIndex].file_content_index;
    fcbs[jFcbIndex].fileCreatedTime = fcbs[iFcbIndex].fileCreatedTime;
    fcbs[jFcbIndex].fileModifiedTime = fcbs[iFcbIndex].fileModifiedTime;

     # if LOG_ENABLE
    printf("i=%d put into j=%d, filename=%s, filesize=%u, file_content_index=%hu, fileCreatedTime=%hu, fileModifiedTime=%hu, \n",iFcbIndex,jFcbIndex,fcbs[jFcbIndex].file_name ,fcbs[jFcbIndex].fileSize ,fcbs[jFcbIndex].file_content_index , fcbs[jFcbIndex].fileCreatedTime,  fcbs[jFcbIndex].fileModifiedTime );
     #endif

    /// copy temp data to i
    gpuStrncpy(fcb.file_name, fcbs[iFcbIndex].file_name , stringLength(fcb.file_name));
    fcbs[iFcbIndex].fileSize = fcb.fileSize;
    fcbs[iFcbIndex].file_content_index = fcb.file_content_index;
    fcbs[iFcbIndex].fileCreatedTime = fcb.fileCreatedTime;
    fcbs[iFcbIndex].fileModifiedTime = fcb.fileModifiedTime;

     # if LOG_ENABLE
    printf("put temp to i=%d, filename=%s, filesize=%u, file_content_index=%hu, fileCreatedTime=%hu, fileModifiedTime=%hu, \n",iFcbIndex,fcbs[iFcbIndex].file_name ,fcbs[iFcbIndex].fileSize ,fcbs[iFcbIndex].file_content_index , fcbs[iFcbIndex].fileCreatedTime,  fcbs[iFcbIndex].fileModifiedTime );
      #endif
}


__device__  u32 gsys(int mode){
   int i,j=0;

   if (mode == LS_D){
       # if LOG_ENABLE
       printf("[gsy LS_D]\n");
      #endif
      /// sort by modified time
      for (i=0;i< MAX_NUM_OF_FILE;i++){
         if (fcbs[i].file_name[0] == '\0' ) continue;
         for (j=0;j<MAX_NUM_OF_FILE ;j++){
             if (fcbs[j].file_name[0] == '\0' || i==j ) continue;
             if ( fcbs[i].fileModifiedTime  > fcbs[j].fileModifiedTime ){
                 swapFCB(i,j);
             }
         }
      }

     printf("===sort by modified time===\n");

   }else if (mode == LS_S){
       # if LOG_ENABLE
       printf("[gsy LS_S]\n");
      #endif
      /// sort by size if size is the same, sort by create time
        for (i=0; i< MAX_NUM_OF_FILE; i++){
            if (fcbs[i].file_name[0] == '\0' ) continue;
            for (j=0; j<MAX_NUM_OF_FILE ; j++){
                if (fcbs[j].file_name[0] == '\0' || i==j ) continue;

                if ( fcbs[i].fileSize  > fcbs[j].fileSize ){
                    swapFCB(i,j);
                }else if ( fcbs[i].fileSize == fcbs[j].fileSize){
                     # if LOG_ENABLE
                    printf("i=%d, j=%d, size the same. size=%hu,\n",i,j,fcbs[i].fileSize);
                     #endif
                    if ( fcbs[i].fileCreatedTime  < fcbs[j].fileCreatedTime ){
                        swapFCB(i,j);
                    }
                }
            }
        }

        printf("===sort by file size===\n");
   }

   /// print sorted result
   for (i=0;i<MAX_NUM_OF_FILE;i++){
        if (fcbs[i].file_name[0] == '\0' ) continue;
        if (mode == LS_D){
            printf("%s\n",fcbs[i].file_name );
        }else if (mode == LS_S){
            printf("%s %hu\n",fcbs[i].file_name, fcbs[i].fileSize );
        }
   }

   return 1;

}



__global__  void mykernel(uchar *input,uchar *output){

     init_volume();


     //####kernel start####
     u32 fp =open("t.txt\0",G_WRITE);
     write(input,64, fp);
     fp =open("b.txt\0",G_WRITE);
     write(input+32,32, fp);
     fp = open("t.txt\0",G_WRITE);
     write(input+32,32, fp);
     fp = open("t.txt\0",G_READ);
     read(output, 32,fp);
     gsys(LS_D);
     gsys(LS_S);
     fp = open("b.txt\0",G_WRITE);
     write(input+64,12, fp);
     gsys(LS_S);
     gsys(LS_D);
     gsys(RM,"t.txt\0");
     gsys(LS_S);
     //####kernel end####
}

void load_binaryFile(char *path, uchar *buffer, int size ){

	FILE *fp;
	fp = fopen(path,"rb");
	//fseek (fp , 0 , SEEK_END);
	//int totalSize = ftell (fp);
	//rewind (fp);
	//int dataSize = totalSize / sizeof( uchar);
	size_t readResultSize = fread(&buffer[0], sizeof(uchar), size , fp);
	//printf("[load_binaryFile] input size=%d, dataSize=%d, readResultSize=%d\n",size,dataSize,readResultSize);
	//printf("[load_binaryFile] input size=%d, readResultSize=%d\n",size,readResultSize);
	fclose(fp);

	 # if LOG_ENABLE
    printf("[load_binaryFile] input size=%d, readResultSize=%d\n",size,readResultSize);
    #endif


    /*
    int* returnSize= (int*) malloc ( sizeof(int));
    returnSize[0] = (int)readResultSize;
    return returnSize[0];
    */
}

void write_binaryFile(char *path, uchar *buffer, int size){
    FILE *outfp;
    outfp = fopen(path,"wb");

    //fseek (outfp , 0 , SEEK_SET);
    //int totalSize = ftell (outfp);
	//rewind (outfp);
	//int dataSize = totalSize / sizeof( uchar);
    size_t writeResultSize = fwrite ( &buffer[0], sizeof(uchar), size, outfp);

    //printf("[write_binaryFile] input size=%d,dataSize=%d,writeResultSize=%d\n",size,dataSize,writeResultSize);
    //printf("[write_binaryFile] input size=%d, writeResultSize=%d\n",size,writeResultSize);

     # if LOG_ENABLE
    printf("[write_binaryFile] input size=%d, writeResultSize=%d\n",size,writeResultSize);
    #endif

    fclose(outfp);
}



int mainRun()
{

    cudaSetDevice(1);

    cudaMallocManaged(&file_content, MAX_FILE_SIZE);
    cudaMallocManaged(&fcbs, sizeof(struct FCB)* MAX_NUM_OF_FILE);
    printf("fcb size=%d bytes, total:(fcb*1024)=%d bytes,\n\n",sizeof(struct FCB),sizeof(struct FCB)* MAX_NUM_OF_FILE );


    uchar *input, *output;
    cudaMallocManaged(&input, MAX_FILE_SIZE);
    cudaMallocManaged(&output, MAX_FILE_SIZE);


    int i=0;
    for(i=0;i<MAX_FILE_SIZE;i++){
        output[i]=0;
    }

    load_binaryFile(DATAFILE,input,MAX_FILE_SIZE);


    mykernel<<<1,1>>>(input,output);
    cudaDeviceSynchronize();

    write_binaryFile(OUTFILE,output,MAX_FILE_SIZE);
    cudaDeviceReset();

    printf("\nFinish!\n");

    return 0;
}





int main(){
    struct timeval tv, tv2;

    clock_t endTime;
    unsigned long long start_utime, end_utime;


    endTime =clock();
    gettimeofday(&tv, NULL);

    mainRun();

    gettimeofday(&tv2, NULL);
    endTime =clock() - endTime ;

    start_utime = tv.tv_sec * 1000000 + tv.tv_usec;
	end_utime = tv2.tv_sec * 1000000 + tv2.tv_usec;

    printf("Clock=%f sec. ,  Gettimeofday time = %llu.%03llu milisecond;  %llu.%03llu sec \n",((float)endTime) /CLOCKS_PER_SEC, (end_utime - start_utime)/1000, (end_utime - start_utime)%1000, (end_utime - start_utime)/1000000, (end_utime - start_utime)%1000000  );

     return 0;

}
