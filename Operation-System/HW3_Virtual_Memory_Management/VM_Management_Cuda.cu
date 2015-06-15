#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

///page size is 32bytes
#define PAGESIZE 32
///32 KB in shared memory
#define PHYSICAL_MEM_SIZE 32768
///128KB in global memory
#define STORAGE_SIZE 131072

#define DATAFILE "./data.bin"
#define OUTFILE "./snapshot.bin"

#define PAGE_COUNT_LIMIT 4000000000
#define NOT_NUMBER       4200000000
#define ZERO 0
#define LOG_ENABLE 0

/// 32bit-->  0 到 4294967295



typedef unsigned char uchar;
typedef uint32_t u32;

///page table entries---page table 有幾個entries
 int PAGE_ENTRIES =0;
///COUNT THE pagefault times
 int PAGEFAULT=0;
///secondary memory ---是secondary memory in global memory -> 128KB
 uchar storage[STORAGE_SIZE];

///data input and output

 uchar results[STORAGE_SIZE]; ///results ---單純放results，之後output 到snapshot.bin要用
 uchar input[STORAGE_SIZE]; /// input---單純放input data，從data.bin裡面load data進來
 FILE *logFp;

///page table
  u32 *pt;  ///page table in shared memory -> 16KB
  u32 *count;
  u32 pageCount =0;  ///262154
  ///u32 pageCount =PAGE_COUNT_LIMIT -260000;  ///262154


u32 findVirtualAddress(u32 physicalAddressIndex){
    int i=0;
    for(;i<PAGE_ENTRIES;i++){
        if (pt[i] == physicalAddressIndex) return i;
    }
    return NOT_NUMBER;
}

u32 findVictimFromPhysicalAddress(){
    u32 minValue=NOT_NUMBER;
    u32 minValueIndex;
    int i=0;
    for (i=0;i<PAGE_ENTRIES;i++){
        if (count[i]<=minValue) {
            minValue = count[i];
            minValueIndex = i;
        }
    }
    return minValueIndex;
}

u32 findAvailablePhysicalAddress(){
    int i=0;
    for (;i<PAGE_ENTRIES;i++){
        if (pt[i]==NOT_NUMBER){
            return i;
        }
    }
    return NOT_NUMBER;
}

void moveDataFromDiskToMemory(uchar *buffer,u32 diskIndex,u32 memoryIndex){
     /// get page data from disk to memory
    int d_index=diskIndex*PAGESIZE;
    int d_limit= d_index+PAGESIZE;
    int m_index = memoryIndex *PAGESIZE;

    for ( ;d_index< d_limit ;d_index++ ){
            buffer[m_index] = storage[d_index];
             //# if LOG_ENABLE
             fprintf(logFp,"d_index=%lu, m_index=%lu, data[%d]=%c, storage[%d]=%c\n",d_index,m_index, m_index,buffer[m_index] ,d_index ,storage[d_index]);
             fflush(stderr);
            //#endif
            m_index++;
    }
}

void  moveDataFromMemoryToDisk(uchar *buffer,u32 diskIndex,u32 memoryIndex){
     int m_index = memoryIndex *PAGESIZE;
     int m_limit= m_index+PAGESIZE;
     int d_index=diskIndex*PAGESIZE;

     for ( ;m_index< m_limit ;m_index++ ){
            uchar oldDataInMemory = buffer[m_index];
           storage[d_index]=buffer[m_index];
       # if LOG_ENABLE
           fprintf(logFp,"[moveDataFromMemoryToDisk] d_index=%lu, m_index=%lu, oldDataInMemory[%d]=%c, data[%d]=%c, storage[%d]=%c\n",d_index,m_index, m_index,oldDataInMemory, m_index,buffer[m_index] ,d_index ,storage[d_index]);
           fflush(stderr);
            #endif
           d_index++;
    }

}

void shitPageCountWhenBeyondLimit(){


    u32 minValue=NOT_NUMBER;

    int i=0;
    for (i=0;i<PAGE_ENTRIES;i++){
        if (count[i]<=minValue) {
            minValue = count[i];
        }
    }

     # if LOG_ENABLE
    fprintf(logFp,"###shitPageCountWhenBeyondLimit###   minValue=%lu in count array \n",minValue);
    fflush(stderr);
    #endif

    u32 oldPageCount;
    for (i=0;i<PAGE_ENTRIES;i++){
         oldPageCount = count[i];
         count[i] = count[i]-minValue;
         # if LOG_ENABLE
         fprintf(logFp,"old count[%d]=%lu --> count[%d]=%lu \n",i,oldPageCount, i, count[i]);
         fflush(stderr);
         #endif
    }

    oldPageCount = pageCount;
    pageCount = pageCount -minValue;

     # if LOG_ENABLE
     fprintf(logFp,"## old PageCount=%lu --> pageCount=%lu, minValue=%lu in count array \n",oldPageCount,pageCount, minValue);
     fflush(stderr);
     #endif


}

 u32 pagingOLD(uchar *buffer,u32 frame_num,u32 offset){

    if (pageCount > PAGE_COUNT_LIMIT){
        shitPageCountWhenBeyondLimit();
    }

    pageCount++;

    int dataIndex=0;
    u32 addr;

    u32 physicalAddressIndex = findVirtualAddress(frame_num);

    # if LOG_ENABLE
    fprintf(logFp,"[paging] frame_num=%lu, offset=%lu, pageCount=%lu, physicalAddressIndex=%lu\n",frame_num,offset, pageCount,physicalAddressIndex);
    fflush(stderr);
    #endif


    if ( physicalAddressIndex != NOT_NUMBER){
        dataIndex = physicalAddressIndex * PAGESIZE;
        count[physicalAddressIndex]= pageCount;
        return addr = dataIndex+offset;
    }

    PAGEFAULT++;

    u32 availablePtIndex= findAvailablePhysicalAddress();

    # if LOG_ENABLE
    fprintf(logFp,"[paging] PAGEFAULT=%d, availablePtIndex=%lu\n",PAGEFAULT,availablePtIndex);
    fflush(stderr);
    #endif

    if ( availablePtIndex != NOT_NUMBER){
        /// find unused physical address
        physicalAddressIndex = availablePtIndex;
    }else{
        /// physical addrass exhausted
        u32 victimIndex =findVictimFromPhysicalAddress();
        physicalAddressIndex = victimIndex;
        # if LOG_ENABLE
        fprintf(logFp,"[paging] victimIndex=%lu \n",victimIndex);
        fflush(stderr);
        #endif

        u32 diskIndex= pt[physicalAddressIndex];

        fprintf(logFp,"[Swap data from memory to disk] victimIndex=%d, diskIndex=%u\n",victimIndex,diskIndex);
        fflush(stderr);
        moveDataFromMemoryToDisk(buffer, diskIndex,physicalAddressIndex);

    }



    pt[physicalAddressIndex] = frame_num;
    count[physicalAddressIndex]= pageCount;
    # if LOG_ENABLE
    fprintf(logFp,"pt[%lu]=%lu, count[%lu]=%lu\n##moveDataFromDiskToMemory \n",physicalAddressIndex,pt[physicalAddressIndex], physicalAddressIndex, count[physicalAddressIndex]);
    fflush(stderr);
    #endif
    moveDataFromDiskToMemory(buffer,frame_num,physicalAddressIndex );

    dataIndex = physicalAddressIndex*PAGESIZE;
    addr = dataIndex+offset;

     # if LOG_ENABLE
    fprintf(logFp,"frame_num=%lu, offset=%lu, dataIndex=%d, addr=%lu  \n",frame_num,offset, dataIndex, addr);
    fflush(stderr);
    #endif

    return addr;
}



 u32 paging(uchar *buffer,u32 frame_num,u32 offset){

    if (pageCount > PAGE_COUNT_LIMIT){
        shitPageCountWhenBeyondLimit();
    }

    pageCount++;

    int dataIndex=0;
    u32 addr;

    ///  for gpu performance issue!!
    u32 physicalAddressIndex =NOT_NUMBER;
    u32 availablePtIndex=NOT_NUMBER;
    int isRuningForAvailablePtIndex =1;
    u32 victimIndex =NOT_NUMBER;
    u32 minValue=NOT_NUMBER;
    u32 minValueIndex=0;

    int i=0;

    for(;i<PAGE_ENTRIES;i++){
        /// findVirtualAddress
        if (pt[i] == physicalAddressIndex){
            physicalAddressIndex = i;
            break;
        }

        /// findAvailablePhysicalAddress
        if (isRuningForAvailablePtIndex && pt[i]==NOT_NUMBER){
            availablePtIndex= i;
            isRuningForAvailablePtIndex =0;
        }

        /// findVictimFromPhysicalAddress
        if (count[i] < minValue) {
            minValue = count[i];
            minValueIndex = i;
        }
    }

    victimIndex = minValueIndex;

    /// u32 physicalAddressIndex = findVirtualAddress(frame_num);

    # if LOG_ENABLE
    fprintf(logFp,"[paging] frame_num=%lu, offset=%lu, pageCount=%lu, physicalAddressIndex=%lu\n",frame_num,offset, pageCount,physicalAddressIndex);
    fflush(stderr);
    #endif


    if ( physicalAddressIndex != NOT_NUMBER){
        dataIndex = physicalAddressIndex * PAGESIZE;
        count[physicalAddressIndex]= pageCount;
        return addr = dataIndex+offset;
    }

    PAGEFAULT++;

    /// u32 availablePtIndex= findAvailablePhysicalAddress();

    # if LOG_ENABLE
    fprintf(logFp,"[paging] PAGEFAULT=%d, availablePtIndex=%lu\n",PAGEFAULT,availablePtIndex);
    fflush(stderr);
    #endif

    if ( availablePtIndex != NOT_NUMBER){
        /// find unused physical address
        physicalAddressIndex = availablePtIndex;
    }else{
        /// physical addrass exhausted
       /// u32 victimIndex =findVictimFromPhysicalAddress();
        physicalAddressIndex = victimIndex;
        # if LOG_ENABLE
        fprintf(logFp,"[paging] victimIndex=%lu \n",victimIndex);
        fflush(stderr);
        #endif

        u32 diskIndex= pt[physicalAddressIndex];

        fprintf(logFp,"[Swap data from memory to disk] victimIndex=%d, diskIndex=%u\n",victimIndex,diskIndex);
        fflush(stderr);
        moveDataFromMemoryToDisk(buffer, diskIndex,physicalAddressIndex);

    }



    pt[physicalAddressIndex] = frame_num;
    count[physicalAddressIndex]= pageCount;
    # if LOG_ENABLE
    fprintf(logFp,"pt[%lu]=%lu, count[%lu]=%lu\n##moveDataFromDiskToMemory \n",physicalAddressIndex,pt[physicalAddressIndex], physicalAddressIndex, count[physicalAddressIndex]);
    fflush(stderr);
    #endif
    moveDataFromDiskToMemory(buffer,frame_num,physicalAddressIndex );

    dataIndex = physicalAddressIndex*PAGESIZE;
    addr = dataIndex+offset;

     # if LOG_ENABLE
    fprintf(logFp,"frame_num=%lu, offset=%lu, dataIndex=%d, addr=%lu  \n",frame_num,offset, dataIndex, addr);
    fflush(stderr);
    #endif

    return addr;
}

void initNumber(u32 *buf, int bufsize, u32 value){
    int i=0;
    for(i=0;i<bufsize;i++){
        buf[i]=value;
    }
}

void init_pageTable(int pt_entries){
    PAGE_ENTRIES = pt_entries;
    pt = (u32*)malloc(sizeof(u32) * pt_entries);
    initNumber(pt,PAGE_ENTRIES,NOT_NUMBER);
    //memset (pt, NOT_NUMBER, sizeof(pt));

    count = (u32*)malloc(sizeof(u32) * pt_entries);
    initNumber(count,PAGE_ENTRIES,ZERO);
    //memset (count, ZERO, sizeof(count));

    copyInputToDiskStorage();
}

void copyInputToDiskStorage(){
    int i=0;
    for (i=0;i<STORAGE_SIZE;i++){
        storage[i]=input[i];
    }
}

 uchar Gread(uchar *buffer, u32 addr){
   u32 frame_num = addr/PAGESIZE;
   u32 offset = addr % PAGESIZE;

   addr = paging(buffer, frame_num,offset);
   return buffer[addr];
}

 void Gwrite(uchar *buffer, u32 addr, uchar value){
   u32 frame_num = addr/PAGESIZE;
   u32 offset = addr % PAGESIZE;

   addr=paging(buffer,frame_num,offset);
   buffer[addr]= value;
}

 void snapshot(uchar *results, uchar *buffer, int offset, int input_size){
    int i =0;
    for(i=0;i<input_size;i++){
        results[i]= Gread(buffer,i+offset);
    }
}

 void mykernel(int input_size){
  /// take shared memory as physical memory -> 32KB
   uchar data[PHYSICAL_MEM_SIZE];
  /// get page table entries
  int pt_entries = PHYSICAL_MEM_SIZE/PAGESIZE;

  /// before first Gwrite or Gread
  init_pageTable(pt_entries);

  //####Gwrite/Gread code section start####
  int i=0;
  for (i=0;i<input_size;i++ ){
    Gwrite(data,i,input[i]);
  }
  for (i=input_size-1;i>=input_size-10;i--){
     int value = Gread(data,i);
  }

  ///the last line of Gwrite/Gread code section should be snapshot()
  snapshot(results,data,0,input_size);
  //####Gwrite/Gread code section end####


  printf("pagefault times = %d\n",PAGEFAULT);

  # if LOG_ENABLE
  fprintf(logFp,"pagefault times = %d\n",PAGEFAULT);
  fflush(stderr);
  #endif


}



int load_binaryFile(char *path, uchar *buffer, int size ){

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


	return (int)readResultSize;

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
    fprintf(logFp,"[write_binaryFile] input size=%d, writeResultSize=%d\n",size,writeResultSize);
    fflush(stderr);
    #endif

    fclose(outfp);
}



int main()
{
    //u32 a= 4294967295;
    //printf("a=%d , a=%lu\n",a,a);
    //return 0;


    logFp = freopen("./log.txt","wr", stderr);

    int input_size = load_binaryFile(DATAFILE,input, STORAGE_SIZE);

    printf("input_size=%d \n",input_size);
    # if LOG_ENABLE
    fprintf(logFp,"input_size=%d \n",input_size);
    fflush(stderr);
    #endif // LOG_ENABLE

    //cudaSetDevice(1);
    //mykernel<<<1,1,16384>>>(input_size);

    mykernel(input_size);

    //cudaDeviceSynchronize();
    //cudaDeviceReset();
    write_binaryFile(OUTFILE, results, input_size);
   // write_binaryFile(OUTFILE, input, input_size);

    fclose(logFp);

    return 0;
}
