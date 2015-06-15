#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include<string.h>
#define LOG_ENABLE 1

///#define STORAGE_SIZE 1085440
#define LOG_ENABLE 1
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
#define U16_NOT_NUMBER  65000 /// 0~ 65535  (%hu)
typedef unsigned char uchar;  ///1byte
typedef uint32_t u32;         ///4bytes, 32bits
typedef uint16_t u16;

/*
u32 COUNT_LIMIT= 800000000;
u32 FILE_NOT_EXIST=901000000;
u32 U32_NOT_NUMBER=900000000;  ///%u
u16 U16_NOT_NUMBER=65000; /// 0~ 65535(六萬五千) (%hu)
*/
///uchar *volume;

uchar bitVector[MAX_NUM_OF_FILE];  /// 1K
uchar *file_content;  /// 1024K
u16 fileCreatedTimeCount =0;  /// 16 bytes
u16 fileModifiedTimeCount =0; ///  16 bytes

#pragma pack(push) ///把原本的對齊設定push進stack
#pragma pack(1)
typedef struct FCB{    /// 30byte * 1024 =30KB
   uchar file_name[fileNameLength];  ///20 byte
   u32 fileSize;              ///4 byte
   u16 fileCreatedTime;        /// 2K
   u16 fileModifiedTime;  ///2K
   u16 file_content_index;   ///2K
};
#pragma pack(pop)  ///把原本的對齊設定pop出來

struct FCB *fcbs;

FILE *logFp;

void initCharVaule(uchar *names,uchar value, int size ){
   int i=0;
   for (i=0;i<=size;i++){
       names[i]=value;
   }
}

void init_volume(){

  int i=0;
  for (i=0;i<MAX_NUM_OF_FILE;i++ ){
      ///printf("i=%d ",i);
      bitVector[i]=FREE_BIT;   /// 0 != '0'
      /// if (bitVector[i] == 0) {}
      ///printf("bitVector[%d]=%d; ",i,bitVector[i]); ///%d or %u


      initCharVaule(fcbs[i].file_name,'\0',fileNameLength );   /// 0 == '\0'  but != '0'
      fcbs[i].fileSize=U32_NOT_NUMBER;
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

int stringLength(char* symbols){
    int i=0;
    for (i=0;symbols[i] !='\0';i++ ){
    }
    return i;

}


void gpuStrncpy(uchar *source, uchar *destination, int size  ){
    int i=0;

    while( *(source+i) !='\0' && i<size  ){
        *destination = *(source+i);
        destination++;
        i++;
    }

    ///if ( *(source+i) == '\0'){
       *destination = '\0';
    ///}

}

int gpuStrcmp(uchar *str1, uchar *str2){

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


void mainAAA(){

     char *r5="abcdefghijklmnopqrstuv\0";
     char *r6="abcdefghijklmnopqrstuv\0";

     printf("r5[]=%s, %c,\n", r5+1, *(r5+1));

     char c5[]="aAe\0";
   char c6[]="aAd\0";
   ///printf("c5 > c6 , %d \n",gpuStrcmp(c5,c6));

   char *e4= "ab\0" ;
   char e5[4];


   gpuStrncpy(e4,e5,6);
   printf("gpuStrncpy e4=%s, %c, e5=%s, e4 length=%d, e5 length=%d,\n",e4,*(e4+1),e5,stringLength(e4),stringLength(e5));


   char c3[]="aAc\0";
   char c333[]="aAc";




   printf("c3 length=%d, c333 length=%d,\n",stringLength(c3),stringLength(c333));


   char c4[]="aAd\0";
   printf("c3 < c4 , %d \n",gpuStrcmp(c3,c4));

     char c1[2]="a";
   char c2[1];
   printf("c1 == c2 , %d \n",gpuStrcmp(c1,c2));





}

u16 findFreeFCB(){
    int i=0;
    for (i=0;i<MAX_NUM_OF_FILE;i++){
        if (fcbs[i].file_name[0] =='\0'){
            return i;
        }
    }
    return U16_NOT_NUMBER;
}

u16 findFileByName(uchar *fileName) {
   int i=0;

   for (i=0;i<MAX_NUM_OF_FILE;i++){
       if ( fcbs[i].file_name[0] == '\0') {
            #if LOG_ENABLE
             //fprintf(logFp,"file_name=0, fcbs[%d].file_name=%s,\n",i,fcbs[i].file_name);
             //fflush(stderr);
            #endif
            continue; /// unused file.
       }
       if (gpuStrcmp(fileName, fcbs[i].file_name) ==0 ){
            #if LOG_ENABLE
             fprintf(logFp,"find fileName=%s, fcbs[%d].file_name=%s, index=%hu\n",fileName,i,fcbs[i].file_name,fcbs[i].file_content_index);
             fflush(stderr);
            #endif
           return i;

       }
   }
   return U16_NOT_NUMBER;
}

u16 findFreeBitVector(){
    int i=0;
    for (i=0;i<MAX_NUM_OF_FILE;i++){
        if (bitVector[i] == FREE_BIT){
            return i;
        }
    }

    return U16_NOT_NUMBER;
}


void shiftFileCreatedTimeCountForOverFlow(){

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


u16 createFile(u16 bitLocation, uchar *fileName){

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
    fprintf(logFp ,"file_name_length=%d, fcbs[%d].file_name=%s, fileCreatedTime=%hu, fileSize=%u, file_content_index=%hu, \n",file_name_length,fcbIndex,fcbs[fcbIndex].file_name,fcbs[fcbIndex].fileCreatedTime, fcbs[fcbIndex].fileSize , fcbs[fcbIndex].file_content_index );
    fflush(stderr);
    #endif

    if (fileCreatedTimeCount >= U16_NOT_NUMBER){
        shiftFileCreatedTimeCountForOverFlow();
    }



    return fcbIndex;
}

u32 open(uchar *fileName, int accessMode){

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

void clearFileContent(u32 fcbIndex){
    u16 fb = fcbIndex;
    int i=0;

    if (fcbs[fb].fileSize ==0) {
       #if LOG_ENABLE
       fprintf(logFp , "##clearFileContent## fileSize==0, fcbIndex=%u, fb=%hu, fileSize=%u\n",fcbIndex,fb,fcbs[fb].fileSize );
        fflush(stderr);
       #endif
       return;
    }

    i = fcbs[fb].file_content_index * KBytes;

    #if LOG_ENABLE
    fprintf(logFp , "##clearFileContent## fcbIndex=%u, fb=%hu, fileSize=%u, file_content_index=%hu, real location= (file_content_index * KBytes)=%d,  \n",fcbIndex,fb,fcbs[fb].fileSize,fcbs[fb].file_content_index, i );
     fflush(stderr);
    #endif

    for (;i<fcbs[fb].fileSize;i++){
        file_content[i]='\0';
    }

}


void shiftFileModifiedTimeCountForOverFlow(){

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


u32 write(uchar *input, u32 size, u32 fcbIndex){
    u16 fb = U16_NOT_NUMBER;
    int i =0;
    int fileBlockLocation =0;


    if (fcbIndex >= U32_NOT_NUMBER) {
          # if LOG_ENABLE
        fprintf(logFp ,"[Error:write] fcbIndex=%u, fb=%hu >= U32_NOT_NUMBER\n",fcbIndex,fb );
          fflush(stderr);
         #endif

        return fcbIndex;
    }

    clearFileContent(fcbIndex);

    fileModifiedTimeCount++;

    fb = fcbIndex;

    fileBlockLocation =  fcbs[fb].file_content_index * KBytes;

    if (size > KBytes){
         # if LOG_ENABLE
        fprintf(logFp,"[Waring] fcbIndex=%u, fb=%hu, size=%u > 1024,\n",fcbIndex,fb,size );
         fflush(stderr);
         #endif
        size = KBytes;
    }

    for (;i<size;i++){
        file_content[fileBlockLocation + i] = input[i];
    }

    fcbs[fb].fileModifiedTime= fileModifiedTimeCount;
    fcbs[fb].fileSize = size;


    if (fileModifiedTimeCount >= U16_NOT_NUMBER){
        shiftFileModifiedTimeCountForOverFlow();
    }

    return size;
}


u32 read(uchar *output, u32 size, u32 fcbIndex){
    u16 fb = fcbIndex;
    int i =0;
    int fileBlockLocation =0;

    if (fcbIndex >= U32_NOT_NUMBER) {
         # if LOG_ENABLE
        fprintf(logFp,"[Error:read] fcbIndex=%u, fb=%hu >= U32_NOT_NUMBER\n",fcbIndex,fb );
        fflush(stderr);
         #endif
        return fcbIndex;
    }

    fileBlockLocation =  fcbs[fb].file_content_index * KBytes;

    if (size > KBytes){
         # if LOG_ENABLE
        fprintf(logFp,"[Waring.1] fcbIndex=%u, fb=%hu, size=%u > 1024,\n",fcbIndex,fb,size );
        fflush(stderr);
         #endif
        size = KBytes;
    }

    if (size > fcbs[fb].fileSize ){
         # if LOG_ENABLE
        fprintf(logFp,"[Waring.2] fcbIndex=%u, fb=%hu, size=%u > fileSize=%u,\n",fcbIndex,fb,size,fcbs[fb].fileSize );
        fflush(stderr);
         #endif

        size = fcbs[fb].fileSize;
    }

     for (;i<size;i++){
          output[i] = file_content[ (fileBlockLocation + i)];
     }
     return size;

}

u32 gsys2(int mode, uchar *fileName ){
    /// do remove file
    u16 fcbIndex = findFileByName(fileName);

    if (fcbIndex == U16_NOT_NUMBER){
         # if LOG_ENABLE
        fprintf(logFp,"[Error:gsy(RM)] List file Error. fcbIndex=%hu,\n",fcbIndex );
        fflush(stderr);
         #endif

        return ERROR_LIST_FILE;
    }

    clearFileContent(fcbIndex);

    initCharVaule(fcbs[fcbIndex].file_name,'\0',fileNameLength );
    fcbs[fcbIndex].fileSize  = U32_NOT_NUMBER;
    fcbs[fcbIndex].fileCreatedTime=U16_NOT_NUMBER;
    fcbs[fcbIndex].fileModifiedTime=U16_NOT_NUMBER;

    /// free bitVector before free fcbs.file_content_index
    bitVector[fcbs[fcbIndex].file_content_index] = FREE_BIT;


    fcbs[fcbIndex].file_content_index=U16_NOT_NUMBER;




    return 1;
}


void swapFCB(int iFcbIndex,int jFcbIndex){
    # if LOG_ENABLE
       fprintf(logFp,"[swapFCB] iFcbIndex=%d,jFcbIndex=%d,\n",iFcbIndex,jFcbIndex);
       fflush(stderr);
      #endif

    struct FCB fcb;
    initCharVaule(fcb.file_name,'\0',fileNameLength );   /// 0 == '\0'  but != '0'
    fcb.fileSize=U32_NOT_NUMBER;
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
    ///fprintf(logFp,"     j=%d puts into temp, filename=%s, filesize=%u, file_content_index=%hu, fileCreatedTime=%hu, fileModifiedTime=%hu, \n",jFcbIndex,fcb.file_name ,fcb.fileSize ,fcb.file_content_index , fcb.fileCreatedTime,  fcb.fileModifiedTime );
    ///fflush(stderr);
     #endif

    /// copy i data to j
    gpuStrncpy(fcbs[iFcbIndex].file_name, fcbs[jFcbIndex].file_name , stringLength(fcbs[iFcbIndex].file_name));
    fcbs[jFcbIndex].fileSize = fcbs[iFcbIndex].fileSize;
    fcbs[jFcbIndex].file_content_index = fcbs[iFcbIndex].file_content_index;
    fcbs[jFcbIndex].fileCreatedTime = fcbs[iFcbIndex].fileCreatedTime;
    fcbs[jFcbIndex].fileModifiedTime = fcbs[iFcbIndex].fileModifiedTime;

     # if LOG_ENABLE
    ///fprintf(logFp,"     i=%d put into j=%d, filename=%s, filesize=%u, file_content_index=%hu, fileCreatedTime=%hu, fileModifiedTime=%hu, \n",iFcbIndex,jFcbIndex,fcbs[jFcbIndex].file_name ,fcbs[jFcbIndex].fileSize ,fcbs[jFcbIndex].file_content_index , fcbs[jFcbIndex].fileCreatedTime,  fcbs[jFcbIndex].fileModifiedTime );
    ///fflush(stderr);
     #endif

    /// copy temp data to i
    gpuStrncpy(fcb.file_name, fcbs[iFcbIndex].file_name , stringLength(fcb.file_name));
    fcbs[iFcbIndex].fileSize = fcb.fileSize;
    fcbs[iFcbIndex].file_content_index = fcb.file_content_index;
    fcbs[iFcbIndex].fileCreatedTime = fcb.fileCreatedTime;
    fcbs[iFcbIndex].fileModifiedTime = fcb.fileModifiedTime;

     # if LOG_ENABLE
    ///fprintf(logFp,"     put temp to i=%d, filename=%s, filesize=%u, file_content_index=%hu, fileCreatedTime=%hu, fileModifiedTime=%hu, \n",iFcbIndex,fcbs[iFcbIndex].file_name ,fcbs[iFcbIndex].fileSize ,fcbs[iFcbIndex].file_content_index , fcbs[iFcbIndex].fileCreatedTime,  fcbs[iFcbIndex].fileModifiedTime );
     ///fflush(stderr);
      #endif
}


u32 gsys(int mode){
   int i,j=0;

   if (mode == LS_D){
       # if LOG_ENABLE
       fprintf(logFp,"[gsy LS_D]\n");
       fflush(stderr);
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
       fprintf(logFp,"[gsy LS_S]\n");
       fflush(stderr);
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
                    fprintf(logFp,"i=%d, j=%d, size the same. size=%u,\n",i,j,fcbs[i].fileSize);
                    fflush(stderr);
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
            printf("%s %u\n",fcbs[i].file_name, fcbs[i].fileSize );
        }
   }

   return 1;

}



void mykernel222(uchar *input,uchar *output){
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
     gsys2(RM,"t.txt\0");
     gsys(LS_S);
     //####kernel end####
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


void mykernel(uchar *input,uchar *output){
     //####kernel start####

     int i=0;
     u32 fp;
     char idString[10];
     char *rankIDString;

     for (i=0;i<1024;i++){

         sprintf(idString,"%d",i);
         rankIDString = stringConcat(idString,  "_t.txt\0");
         fp =open(rankIDString,G_WRITE);
         write(input+(i*1024),1024, fp);
     }


     for (i=0;i<1024;i++){
         if (i%2 == 1) continue;
         sprintf(idString,"%d",i);
         rankIDString = stringConcat(idString,  "_t.txt\0");
         fp =open(rankIDString,G_READ);
         read(output+(i*1024),40, fp);
     }



     gsys(LS_S);

     //gsys(LS_D);
     //gsys(LS_S);



     /*
     fp = open("t.txt\0",G_READ);
     read(output, 32,fp);
     gsys(LS_D);
     gsys(LS_S);
     fp = open("b.txt\0",G_WRITE);
     write(input+64,12, fp);
     gsys(LS_S);
     gsys(LS_D);
     gsys2(RM,"t.txt\0");
     gsys(LS_S);
     */
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
    fprintf(logFp,"[load_binaryFile] input size=%d, readResultSize=%d\n",size,readResultSize);
    fflush(stderr);
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
    ///fprintf(logFp,"[write_binaryFile] input size=%d, writeResultSize=%d\n",size,writeResultSize);
    ///fflush(stderr);
    #endif

    fclose(outfp);
}




int mainAAAB(){

    uchar  *output;
    output = (uchar*)malloc(sizeof(uchar) * MAX_FILE_SIZE);
    int i=0;
    int j=0;

    unsigned  char a= 2;
    a= 'A'+25;
    a= 'a'+25;
    a = '0'+1;
    printf("a=%c,%d, \n",a,a);

    for (i=0;i<1024;i++){
        for (j=0;j<1024;j++){


          if (j < 25*20){
              int c =j/20;
              a ='A'+c;
          }else if (j<50*20 ){
              int c =(j -(25*20))/20;
              a ='a'+c;
          }else if (j<1010){
              a ='0';
          }else if (j<1020){
              a ='1';
          }else{
              a ='2';
          }

          output[i*MAX_NUM_OF_FILE+j]=a;

        }
    }



    FILE *outfp;
    outfp = fopen("./peter_sample.txt","wb");

    size_t writeResultSize = fwrite ( &output[0], sizeof(uchar), MAX_FILE_SIZE, outfp);
    fclose(outfp);


}

int main()
{
    logFp = freopen("./log.txt","wr", stderr);

    file_content = (uchar*)malloc(sizeof(uchar) * MAX_FILE_SIZE);
    fcbs =  (struct FCB*)malloc(sizeof(struct FCB) * MAX_NUM_OF_FILE);

    init_volume();

    uchar *input, *output;
    input = (uchar*)malloc(sizeof(uchar) * MAX_FILE_SIZE);
    output = (uchar*)malloc(sizeof(uchar) * MAX_FILE_SIZE);

    int i=0;
    for(i=0;i<MAX_FILE_SIZE;i++){
        output[i]=0;
    }

    load_binaryFile(DATAFILE,input,MAX_FILE_SIZE);

    mykernel(input,output);

    write_binaryFile(OUTFILE,output,MAX_FILE_SIZE);


    printf("Finish!\n");
    # if LOG_ENABLE
    fprintf(logFp,"Finish!\n");
    fflush(stderr);
    #endif

    getchar();
    return 0;
}





void bubble_sort(int *array, int N)
{
    int i, j, temp;
    for ( i=0; i<N; ++i) // N改為N-1更精準
        for ( j=0; j<N-i-1; ++j)
            if (array[j] > array[j+1]){
                 temp = array[j];
                 array[j] = array[j+1];
                 array[j+1] = temp;
            }

    i=0;
    for (i=0;i<N;i++){
      printf("%d,",array[i]);
   }
   printf("\n");
}

void bubblesort(int *data, int n)
{
    int i, j, temp;
    for (i = 0; i < n; i++){
        for (j = 0; j <n; j++){
            /// from small to large <
            if (data[i] < data[j] ){

                temp = data[j];
                data[j] = data[i];
                data[i] = temp;
            }
        }
    }
}


int mainAAAC(){

   int n=5;

   ///int *data = (int *)malloc(sizeof(int) * n );
   int data[5];
   data[0]=2;
   data[1]=3;
   data[2]=5;
   data[3]=1;
   data[4]=7;

   bubblesort(&data[0],n);

   int i=0;
   for (i=0;i<n;i++){
      printf("%d,",data[i]);
   }

   char aa1=0;    /// aa1 == aa2 == 0
   char aa2='\0';  /// 空字元，用於字串的結束
   char aa3='0';   /// aa3 = 48

   if (aa1 == aa2){
      printf("aa1 == aa2\n");
   }if (aa1 == aa3){
      printf("aa1 == aa3\n");
   }

   printf("aa1=%d, aa2=%d, aa3=%d \n",aa1,aa2,aa3);




   char a=0;
   char bb[10];
   printf("a size=%d, bb size=%d, ",sizeof(a), sizeof(bb) );


   char c[] = "bb.txt";
   char c2[] = "bb.txt\0";
   printf("c size=%d, c2 size=%d, ", (sizeof(c) /sizeof(c[0])) , (sizeof(c2) /sizeof(c2[0])) );


   printf("c=%s length=%d;  c2=%s length=%d",c,strlen(c),c2,strlen(c2));

   getchar();
}



typedef struct
{
    char sAcNo[7];
    char sCompanyNo[6];
    char sDepNo[4];
    char sSellerNo[4];
    char sDebitStockNo[9];
    char sYdayQty[12];
    char sUnYdayQty[12];
    char sFiller[46];
}STTSS;

STTSS* pstTss;

void Sub_Read_Tss_File(char *psData)
{
    pstTss = (STTSS*) psData;
    printf("2:%s\n",pstTss->sDepNo);//可以直接秀出sDepNo開始的位置
    printf("3:%s\n",pstTss->sUnYdayQty);

}

int mainAAAA (int argc, char *argv[]){

    argv[1]="D:\\c\\codeblock\\c\\operation system\\cuda_hw4\\test.txt";

    FILE* fp = NULL;
    char line[101]="";

    if((fp=fopen(argv[1], "r"))==NULL)
    {
     printf("file cannot be opened\n");
     return 0;
    }
    while( fgets(line,100,fp) )
    {
     printf("1:%s\n",line);
     Sub_Read_Tss_File(line);
     memset(line,0,sizeof(line));
    }
    return 0;
}

