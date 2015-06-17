#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
typedef enum { ERROR_EXIT_NOW, ERROR_WRITE_R_ZERO, ERROR_NUMBER_OVERFLOW, ERROR_MEMORY_ADDR_OVERFLOW , ERROR_DATA_MISALIGNED } ErrorHandlerCode;
/**** instruction spec  start  ***/
/** I don't want hard code in program, so I make symbol for each of them,
once some SPEC has been changed, I just update
it only without check all my codes. **/
#define R_ADD  (0)
#define R_SUB  (1)
#define R_AND  (2)
#define R_OR   (3)
#define R_XOR  (4)
#define R_NOR  (5)
#define R_NAND (6)
#define R_SLT  (7)
#define R_SLL  (8)
#define R_SRL  (9)
#define R_SRA   (10)
#define R_JR   (11)
#define R_INSTRSPEC_COUNT   (12)

#define R_OPCODE  (0x00)
#define R_ADD_FUNCT  (0x20)
#define R_SUB_FUNCT  (0x22)
#define R_AND_FUNCT  (0x24)
#define R_OR_FUNCT   (0x25)
#define R_XOR_FUNCT  (0x26)
#define R_NOR_FUNCT  (0x27)
#define R_NAND_FUNCT (0x28)
#define R_SLT_FUNCT  (0x2A)
#define R_SLL_FUNCT  (0x00)
#define R_SRL_FUNCT  (0x02)
#define R_SRA_FUNCT   (0x03)
#define R_JR_FUNCT   (0x08)


#define I_ADDI      (0)
#define I_LW        (1)
#define I_LH        (2)
#define I_LHU       (3)
#define I_LB        (4)
#define I_LBU       (5)
#define I_SW        (6)
#define I_SH        (7)
#define I_SB        (8)
#define I_LUI       (9)
#define I_ANDI      (10)
#define I_ORI       (11)
#define I_NORI      (12)
#define I_SLTI      (13)
#define I_BEQ       (14)
#define I_BNE       (15)
#define I_INSTRSPEC_COUNT   (16)

#define I_ADDI_OPCODE      (0x08)
#define I_LW_OPCODE        (0x23)
#define I_LH_OPCODE        (0x21)
#define I_LHU_OPCODE       (0x25)
#define I_LB_OPCODE        (0x20)
#define I_LBU_OPCODE       (0x24)
#define I_SW_OPCODE        (0x2B)
#define I_SH_OPCODE        (0x29)
#define I_SB_OPCODE        (0x28)
#define I_LUI_OPCODE       (0x0F)
#define I_ANDI_OPCODE      (0x0C)
#define I_ORI_OPCODE       (0x0D)
#define I_NORI_OPCODE      (0x0E)
#define I_SLTI_OPCODE      (0x0A)
#define I_BEQ_OPCODE       (0x04)
#define I_BNE_OPCODE       (0x05)

#define J_J     (0)
#define J_JAL   (1)
#define J_INSTRSPEC_COUNT   (2)

#define J_J_OPCODE     (0x02)
#define J_JAL_OPCODE   (0x03)

#define S_HALT (0)
#define S_INSTRSPEC_COUNT   (1)

#define S_HALT_OPCODE (0x3F)
/**** instruction spec  end  ***/
/** in MIPS there are the same length in each R,I,J,S instruction.
So the decoding binary code form MIPS instruction has been defined by different the number of bits.
I just generalize them to get match fields by different instructions.
**/
#define GET_OPCode(x)  ( (x>> 26) & 0x3f)     /** debug before, becasue opcode 6bit => not 0xff MASK **/
#define GET_R_I_RsCode(x)  ( (x >> 21) & 0x1f)
#define GET_R_I_RtCode(x)  ( (x >> 16) & 0x1f)
#define GET_R_RdCode(x)     ( (x >> 11) & 0x1f)
#define GET_R_ShamtCode(x) ( (x >> 6) & 0x1f)
#define GET_R_FunctCode(x) (x & 0x3f)
#define GET_I_Immediate(x) (( (x & 0xffff) <<16) >> 16)  /**debugging before, for signed and unsigned extension**/
#define GET_J_S_Address(x) ( x & 0x03ffffff)
#define GET_Sign_Bit(x)  ( (x >> 31) & 0x1 )

/** specific Register number & available access address index**/
#define R_ZERO (0)
#define R_SP   (29)
#define R_RA   (31)
#define MEMORY_ADDR_FIRST   (0)
#define MEMORY_ADDR_LAST    (1023)

/** relating log file **/
#define I_IMAGE_FILE "./iimage.bin"
#define D_IMAGE_FILE "./dimage.bin"
#define SNAPSHOT_FILE "./snapshot.rpt"
#define ERROR_DUMP_FILE "./error_dump.rpt"
#define HITRATE_DUMP_FILE "./report.rpt"
#define DMEMORY_DUMP_FILE  "./dMemory_dump.rpt"  /** for print out Dmemory for debugging only**/
#define LOG_FILE "./log.txt"  /** log file for debugging only **/
#define MAX_CYCLE 500000
#define LOG_ENABLE 0
#define D_MEMEMORY_LOG_ENABLE 0

/** 4 type R,I,J,S insturction's stuct **/
typedef enum { R,I,J,S } InstructionType;
typedef struct InstructionStructure {
	InstructionType instrType;
	int binaryCode;
	int op;
	int r_i_rs;
	int r_i_rt;
	int r_rd;
	int r_shamt;
	int r_funct;
	int i_immediate;
	int j_s_addr;

} Instruction;

/** 4 type R,I,J,S insturction's SPEC stuct to find mapping OP code & funct code **/
typedef struct InstructionSpec{
    InstructionType instrType;
    int op;
    char *instrName;
    int r_funct;
} InstrSpec;

/** for cache memory pageable simulator **/
#define LRU_SEQ_NO_LIMIT 0x6fffffff
#define MIN_VALUE 0x7fffffff

/** recording memory setting argument **/
typedef struct MemorySettingStructure{
    int memorySize;
    int pageSize;
    int cacheSize;
    int cacheBlockSize;
    int cacheSetAssociativity;
}MemorySetting;
MemorySetting instr_MemorySetting;
MemorySetting data_MemorySetting;

/** memory structure  **/
typedef struct MemoryStructure{
    int memLen;
	int valid[1024];
	int seqNO_LRU[1024];
	int current_SeqNO_LRU;

    int dataLen;
    int data[1024];
}Memory;
Memory instr_Memory;
Memory data_Memory;

/** recording hit rate **/
typedef struct HitRateStructure{
    int hit;
    int miss;
}HitRate;

/** cache structure  **/
typedef struct CacheStructure{
    int cacheLen;
	int valid[1024];
	int tag[1024];
	int seqNO_LRU[1024];
	int current_SeqNO_LRU;

	int dataLen;
	int data[1024];
	HitRate hitRate;
	int cacheBlockAddr[1024]; /** for debugging to validate if my calculated formula for cacheBlockAddr is right. **/
}Cache;
Cache instr_Cache;
Cache data_Cache;

/** Pagetable structure  **/
typedef struct PageTableStructure{
    int pageTableLen;
	int content[1024];
	int valid[1024];
	HitRate hitRate;
}PageTable;
PageTable instr_PageTable;
PageTable data_PageTable;

/** TLB  structure  **/
typedef struct TLBStructure{
    int tlbLen;
	int content[1024];
	int valid[1024];
	int tag[1024];
	int seqNO_LRU[1024];
	int current_SeqNO_LRU;
	HitRate hitRate;
}TLB;
TLB instr_TLB;
TLB data_TLB;

int *dImageFile;  /// source for dimage.bin
int *iImageFile;  /// source for iimage.bin

FILE *logFp;
FILE *dMemoryFp;  /// for debugging to output D-memory information
FILE *snapshotFp;
FILE *errorDumpFp;
FILE *hitRateDumpFp;

int cycleCount =0;
bool isHalt =false;

int data_Disk[256]; /// D-Memory 1K limit => 1024 / 4byte = 256 (1word, MIPS uses a word as a unit for access.)
int instr_Disk[256]; /// I-Memory


int programCounter =0;   /// program counter,programCounter
int registers[32];

Instruction *instruction;
InstrSpec  rInstrSpec[R_INSTRSPEC_COUNT]; /** for R instruction SPEC **/
InstrSpec  iInstrSpec[I_INSTRSPEC_COUNT]; /** for I instruction SPEC **/
InstrSpec  jInstrSpec[J_INSTRSPEC_COUNT]; /** for J instruction SPEC **/
InstrSpec  sInstrSpec[S_INSTRSPEC_COUNT]; /** for S instruction SPEC **/


/**
  int b =0x1235  => in memory it presents by 16 bits as a[0]=00, a[1]=00, a[2]=12, a[3]=35
  sw: big endian in memory    => Mem低 00 00 12 35 高  (放最高位元:a[0] 至最低的記憶體 (least significat memory address) )
  sw: little endian in memory => Mem低 35 12 00 00 高  (放最低位元:a[3] 至最低的記憶體 (least significat memory address) )
  sb: 從暫器取值時，awlays 先取least significat的位置，也就是先取 a[3]=35  -> 存到 in memory
  sh: 從暫器取值時，awlays 先取least significat的位置，也就是先取 a[2]=12 ,a[3]=35 -> 存到b in memory
**/

/** initial R,I,J,S instruction SPEC **/
void initInstrSpec(){

   /** init R instruction SPEC **/
   int i=0;
   for (i=0;i<R_INSTRSPEC_COUNT;i++){

        rInstrSpec[i].instrType= R;
        rInstrSpec[i].op = R_OPCODE;

        switch(i){
            case R_ADD:
                        rInstrSpec[i].instrName ="ADD";
                        rInstrSpec[i].r_funct = R_ADD_FUNCT;
                        break ;
            case R_SUB :
                        rInstrSpec[i].instrName ="SUB";
                        rInstrSpec[i].r_funct = R_SUB_FUNCT;
                        break;
            case R_AND :
                         rInstrSpec[i].instrName ="AND";
                        rInstrSpec[i].r_funct = R_AND_FUNCT;
                        break;
            case R_OR  :
                         rInstrSpec[i].instrName ="OR";
                        rInstrSpec[i].r_funct = R_OR_FUNCT;
                        break;
            case R_XOR :
                         rInstrSpec[i].instrName ="XOR";
                        rInstrSpec[i].r_funct = R_XOR_FUNCT;
                        break;
            case R_NOR :
                         rInstrSpec[i].instrName ="NOR";
                        rInstrSpec[i].r_funct = R_NOR_FUNCT;
                        break;
            case R_NAND :
                         rInstrSpec[i].instrName ="NAND";
                        rInstrSpec[i].r_funct = R_NAND_FUNCT;
                        break;
            case R_SLT :
                         rInstrSpec[i].instrName ="SLT";
                        rInstrSpec[i].r_funct = R_SLT_FUNCT;
                        break;
            case R_SLL :
                         rInstrSpec[i].instrName ="SLL";
                        rInstrSpec[i].r_funct = R_SLL_FUNCT;
                        break;
            case R_SRL :
                         rInstrSpec[i].instrName ="SRL";
                        rInstrSpec[i].r_funct = R_SRL_FUNCT;
                        break;
            case R_SRA  :
                         rInstrSpec[i].instrName ="SRA";
                        rInstrSpec[i].r_funct = R_SRA_FUNCT;
                        break;
            case R_JR  :
                        rInstrSpec[i].instrName ="JR";
                        rInstrSpec[i].r_funct = R_JR_FUNCT;
                        break;
            default:
                   fprintf(stderr,"[Error][initInstrSpec] Cannot find match R instruction code. i=%d \n",i);
                   fflush(stderr);
                   errorHandler(ERROR_EXIT_NOW);
                        break;

        }
   }

    /** init I instruction SPEC **/
    i=0;
    for (i=0;i<I_INSTRSPEC_COUNT;i++){
        iInstrSpec[i].r_funct =0;
        iInstrSpec[i].instrType= I;

         switch(i){
            case I_ADDI :
                       iInstrSpec[i].instrName ="ADDI";
                       iInstrSpec[i].op = I_ADDI_OPCODE;
                        break;
            case I_LW :
                        iInstrSpec[i].instrName ="LW";
                       iInstrSpec[i].op = I_LW_OPCODE;
                        break;
            case I_LH :
                        iInstrSpec[i].instrName ="LH";
                       iInstrSpec[i].op = I_LH_OPCODE;
                        break;
            case I_LHU :
                        iInstrSpec[i].instrName ="LHU";
                       iInstrSpec[i].op = I_LHU_OPCODE;
                        break;
            case I_LB :
                        iInstrSpec[i].instrName ="LB";
                       iInstrSpec[i].op = I_LB_OPCODE;
                        break;
            case I_LBU:
                        iInstrSpec[i].instrName ="LBU";
                       iInstrSpec[i].op = I_LBU_OPCODE;
                        break;
            case I_SW :
                        iInstrSpec[i].instrName ="SW";;
                       iInstrSpec[i].op = I_SW_OPCODE;
                        break;
            case I_SH :
                        iInstrSpec[i].instrName ="SH";
                       iInstrSpec[i].op = I_SH_OPCODE;
                        break;
            case I_SB :
                        iInstrSpec[i].instrName ="SB";
                       iInstrSpec[i].op = I_SB_OPCODE;
                        break;
            case I_LUI:
                        iInstrSpec[i].instrName ="LUI";
                       iInstrSpec[i].op = I_LUI_OPCODE;
                        break;
            case I_ANDI:
                        iInstrSpec[i].instrName ="ANDI";
                       iInstrSpec[i].op = I_ANDI_OPCODE;
                        break;
            case I_ORI :
                        iInstrSpec[i].instrName ="ORI";
                       iInstrSpec[i].op = I_ORI_OPCODE;
                        break;
            case I_NORI:
                        iInstrSpec[i].instrName ="NORI";
                       iInstrSpec[i].op = I_NORI_OPCODE;
                        break;
            case I_SLTI:
                        iInstrSpec[i].instrName ="SLTI";
                       iInstrSpec[i].op = I_SLTI_OPCODE;
                        break;
            case I_BEQ :
                        iInstrSpec[i].instrName ="BEQ";
                       iInstrSpec[i].op = I_BEQ_OPCODE;
                        break;
            case I_BNE :
                        iInstrSpec[i].instrName ="BNE";
                       iInstrSpec[i].op = I_BNE_OPCODE;
                        break;
             default:
                   fprintf(stderr,"[Error][initInstrSpec] Cannot find match I instruction code. i=%d \n",i);
                   fflush(stderr);
                   errorHandler(ERROR_EXIT_NOW);
                        break;
            }

    }

     /** init J instruction SPEC **/
     i=0;
    for (i=0;i<J_INSTRSPEC_COUNT;i++){
        jInstrSpec[i].r_funct =0;
        jInstrSpec[i].instrType= J;
          switch(i){
            case J_J :
                  jInstrSpec[i].instrName ="J";
                  jInstrSpec[i].op = J_J_OPCODE;
                break;
            case J_JAL :
                  jInstrSpec[i].instrName ="JAL";
                  jInstrSpec[i].op = J_JAL_OPCODE;
                break;
            default:
                fprintf(stderr,"[Error][initInstrSpec] Cannot find match J instruction code. i=%d \n",i);
                fflush(stderr);
                errorHandler(ERROR_EXIT_NOW);
                 break;
          }
    }

    /** init S instruction SPEC **/
    i=0;
    for (i=0;i<S_INSTRSPEC_COUNT;i++){
            sInstrSpec[i].r_funct =0;
            sInstrSpec[i].instrType= S;
            switch(i){
                case S_HALT :
                  sInstrSpec[i].instrName ="HALT";
                  sInstrSpec[i].op = S_HALT_OPCODE;
                break;
            default:
                fprintf(stderr,"[Error][initInstrSpec] Cannot find match S instruction code. i=%d \n",i);
                fflush(stderr);
                errorHandler(ERROR_EXIT_NOW);
                 break;
            }
    }


}
 /** initial action to be loaded at begin
     include:
              1.load all instruction SPEC
              2.get snapshot , eror_dump, D-memory_dump 's FILE pointer
              3. initialize all arrays like D & I memory
 **/
void initializeAction(){

   initInstrSpec();

   snapshotFp = fopen(SNAPSHOT_FILE,"wb");
   if (snapshotFp == NULL){
       fprintf(stderr,"[Error][init] Cannot get file=%s.\n",SNAPSHOT_FILE);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
   }

   /** project 3 , cancel output error_dump !!
   errorDumpFp = fopen(ERROR_DUMP_FILE,"wb");
   if (errorDumpFp == NULL){
       fprintf(stderr,"[Error][init] Cannot get file=%s.\n",ERROR_DUMP_FILE);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
   }**/

   hitRateDumpFp = fopen(HITRATE_DUMP_FILE,"wb");
   if (hitRateDumpFp == NULL){
       fprintf(stderr,"[Error][init] Cannot get file=%s.\n",HITRATE_DUMP_FILE);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
   }

   #if D_MEMEMORY_LOG_ENABLE
   dMemoryFp = fopen(DMEMORY_DUMP_FILE,"wb");
   #endif // D_MEMEMORY_LOG_ENABLE

   memset(data_Disk,0,sizeof(data_Disk));   /// init data disk
   memset(instr_Disk,0,sizeof(instr_Disk)); /// init instr disk
   memset(registers,0, sizeof(registers));  /// init register

   instruction = (Instruction*) malloc(sizeof(Instruction));
   memset(instruction,0, sizeof(Instruction)); /** debugging before, should sizeOf(Instruction), but not sizeOf(instruction) **/

}

/** before program finishes, some final jobs should be done
    include:
          close all FILE pointer
**/
void finalizeAction(){

    if (snapshotFp !=NULL){
        fclose(snapshotFp);
    }

    if (errorDumpFp != NULL){
        fclose(errorDumpFp);
    }

    if (hitRateDumpFp != NULL){
        fclose(hitRateDumpFp);
    }


    # if D_MEMEMORY_LOG_ENABLE
    if (dMemoryFp != NULL){
        fclose(dMemoryFp);
    }
    #endif // D_MEMEMORY_LOG_ENABLE


    # if LOG_ENABLE
    if (logFp!=NULL ) fclose(logFp);
    # endif
}


 /** output Dmemory data for debugging only**/
void outputDataMemory(){
    #if D_MEMEMORY_LOG_ENABLE
    if (dMemoryFp != NULL){
        fprintf(dMemoryFp,"cycle %d\n",cycleCount);
        int i=0;
        for (i=0;i<10;i++){
            fprintf(dMemoryFp,"dataMemory[%02d]: 0x%08X\n",i,dataMemory[i]);
        }
        fprintf(dMemoryFp,"\n\n");
    }
    #endif // D_MEMEMORY_LOG_ENABLE
}

 /** output snapShot for each cycle
     Show Register content form 0-31 **/
void outputSnapshot(){

   fprintf(snapshotFp,"cycle %d\n",cycleCount);
   int i=0;
   for (i=0;i<32;i++){
       fprintf(snapshotFp,"$%02d: 0x%08X\n",i,registers[i]);
   }
   fprintf(snapshotFp,"PC: 0x%08X\n\n\n",programCounter);
}

/** give file path , then load file content  and return int[] **/
int* loadFile(const char *path ){

   FILE *fp;
   fp = fopen(path,"rb");

   if (fp == NULL){
       fprintf(stderr,"[Error][loadFile] Cannot get file=%s.\n",path);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
   }

   fseek(fp,0, SEEK_END);
   int totalSize = ftell(fp);
   rewind(fp);
   int dataSize = totalSize/ sizeof(int);

   int *data = (int*)malloc(dataSize * sizeof(int));



   if (data == NULL){
       fprintf(stderr,"[Error][loadFile] Cannot malloc memory, file=%s.\n",path);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
   }

   size_t readResultSize  = fread(&data[0], sizeof(int) ,dataSize, fp);

    # if LOG_ENABLE
   printf(logFp,"before change, (little) pc=0x%08X , %d\n",data[0],data[0]);
    fflush(stderr);
    # endif

   convertLitteEndianToBigEndian(data, dataSize);

   # if LOG_ENABLE
   printf("after change, (big) pc=0x%08X , %d\n",data[0],data[0]);
   fflush(stderr);
    # endif


   fclose(fp);


    # if LOG_ENABLE
    fprintf(logFp,"[loadFile] file=%s, totalSize=%d, dataSize=%d, readResultSize=%d\n",path,totalSize,dataSize,readResultSize);
    fflush(stderr);
    # endif


    return data;


}

/** convert encoding to match project encoding
    Because the CPU's default save int with little format,
    which means in INT its least significant bit has been put in the start index like 0
    , so I need to convert them to match Big endian's format.
 **/
void convertLitteEndianToBigEndian(int *data ,const int dataSize ){
    int i=0;

    for(i=0;i<dataSize;i++){
        data[i] = data[i] << 24 | ((data[i] << 8) & 0xff0000) | ((data[i] >> 8) & 0xff00) | ((data[i] >> 24) & 0xff);
    }

     /** printf("pc=0x%08 , %d\n",data[0],data[0]);**/

}

/** when dimage & image's sources binary codes loaded into memory,
 it uses to load the relative data in relative location **/
void fetchInstruction(){
    programCounter = iImageFile[0];  /// programCounter - program counter
    registers[R_SP] = dImageFile[0]; /// $sp - stack pointer

    int instrCount = 0;
    int dataCount =0;

    instrCount = iImageFile[1];
    dataCount = dImageFile[1];


    checkInstrMemoryError();

    if ( ((programCounter/4) + instrCount)> 256 ){
       fprintf(stderr,"[Error][fetchInstruction] PC=0x%08X, ( (programCounter/4)=%d + instrCount=%d) > 256  (0-255). Instr Memory is overflowed.\n",programCounter,(programCounter/4),instrCount);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
    }

    /***
    if ( registers[R_SP] % 4 !=0){
       fprintf(stderr,"[Error][fetchInstruction] $sp, Register[29]=0x08X are misaligned. programCounter=0x08X.\n",Register[29],programCounter);
       fflush(stderr);
       errorHandler(ERROR_SIGNAL_EXIT);
    } ***/

    ///*  should check sp overflowed ??? */

    if (  dataCount > 256 ){
       fprintf(stderr,"[Error][fetchInstruction] dataCount=%d > 256. data Memory is overflowed.\n",instrCount);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
    }


    int i=0;
    for (i; i<instrCount ;i++){
        instr_Disk[(programCounter/4)+i] = iImageFile[i+2];  /// get instruction with index start from 2
    }

    i=0;
    for (i; i<dataCount ;i++){
        data_Disk[i] = dImageFile[i+2];  /// get data value with index sarting from 2
    }
}


void outputHitRate(){
    /** for debuggin only  **/
    ///outputMemoryDetails( &instr_TLB, &instr_Memory, &instr_PageTable, &instr_Cache);
    ///outputMemoryDetails( &data_TLB, &data_Memory, &data_PageTable, &data_Cache);

	fprintf(hitRateDumpFp,"ICache :\n# hits: %d\n# misses: %d\n\n", instr_Cache.hitRate.hit, instr_Cache.hitRate.miss);
	fprintf(hitRateDumpFp,"DCache :\n# hits: %d\n# misses: %d\n\n", data_Cache.hitRate.hit,  data_Cache.hitRate.miss);
	fprintf(hitRateDumpFp,"ITLB :\n# hits: %d\n# misses: %d\n\n", instr_TLB.hitRate.hit  , instr_TLB.hitRate.miss  );
	fprintf(hitRateDumpFp,"DTLB :\n# hits: %d\n# misses: %d\n\n", data_TLB.hitRate.hit  , data_TLB.hitRate.miss  );
	fprintf(hitRateDumpFp,"IPageTable :\n# hits: %d\n# misses: %d\n\n", instr_PageTable.hitRate.hit  , instr_PageTable.hitRate.miss  );
	fprintf(hitRateDumpFp,"DPageTable :\n# hits: %d\n# misses: %d\n\n", data_PageTable.hitRate.hit  , data_PageTable.hitRate.miss  );
}

/** debugging tool for tracing the details memory info in each Program counter **/
void outputMemoryDetails( TLB *tlb, Memory *memory,PageTable *pageTable, Cache *cache ){
    fprintf(hitRateDumpFp,"####################################\n");
    fprintf(hitRateDumpFp,"PC: 0x%08X\n",programCounter);
    int i=0;

    for (i = 0 ; i < (*tlb).tlbLen ; i++){
        if ((*tlb).valid[i] != 1) continue;
        fprintf(hitRateDumpFp,"TLB[%d] valid=1, tag=%d, content=%d, lru_seq_no=%d\n",i,(*tlb).tag[i],(*tlb).content[i],(*tlb).seqNO_LRU[i]);
    }

    fprintf(hitRateDumpFp,"\n");

    for (i = 0 ; i < (*memory).memLen  ; i++){
        if ((*memory).valid[i] != 1) continue;
        fprintf(hitRateDumpFp,"Memory[%d] valid=1, lru_seq_no=%d\n",i, (*memory).seqNO_LRU[i]);

    }

    fprintf(hitRateDumpFp,"\n");

    for (i = 0 ; i < (*pageTable).pageTableLen  ; i++){
        if ((*pageTable).valid[i] != 1) continue;
        fprintf(hitRateDumpFp,"PageTable[%d] valid=1, content=%d\n",i,(*pageTable).content[i] );
    }

    fprintf(hitRateDumpFp,"\n");
    for (i = 0 ; i < (*cache).cacheLen  ; i++){

        if ((*cache).valid[i] != 1) continue;
        fprintf(hitRateDumpFp,"Cache[%d] valid=1, tag=%d, lru_seq_no=%d, cacheBlockAddr=%d\n",i,(*cache).tag[i] ,(*cache).seqNO_LRU[i], (*cache).cacheBlockAddr[i]);
    }

}

/** error handler to handling error when error happens **/
void errorHandler( ErrorHandlerCode errorCode){
     bool exitNow =false;

     switch (errorCode){
            case   ERROR_WRITE_R_ZERO:
                   fprintf( ((errorDumpFp ==NULL) ? stderr : errorDumpFp) , "In cycle %d: Write $0 Error\n", cycleCount);
                   break;
            case   ERROR_NUMBER_OVERFLOW:
                   fprintf(((errorDumpFp ==NULL) ? stderr : errorDumpFp)  , "In cycle %d: Number Overflow\n", cycleCount);
                   break;
            case   ERROR_MEMORY_ADDR_OVERFLOW:
                   fprintf(((errorDumpFp ==NULL) ? stderr : errorDumpFp)  , "In cycle %d: Address Overflow\n", cycleCount);
                   isHalt = true;
                   break;
            case   ERROR_DATA_MISALIGNED:
                   fprintf(((errorDumpFp ==NULL) ? stderr : errorDumpFp)  , "In cycle %d: Misalignment Error\n", cycleCount);
                   isHalt = true;
                   break;
            case   ERROR_EXIT_NOW:
                    exitNow = true;
                    break;
            default:
                    break;
     }

     if (exitNow){
         finalizeAction();
         exit(0);
     }
}

/** some setting be undo before each cycle **/
void undoSetting(){
    /** because it may happen write to $zero ,
        so the value may be updated. **/
    registers[R_ZERO] =0;
}


 /**convert binary code into R,I,J,S instruction's format **/
void convertBinaryToInstr(int binaryCode){
    memset(instruction,0, sizeof(Instruction)); /** debugging before, should sizeOf(Instruction), but not sizeOf(instruction) **/

    (*instruction).binaryCode = binaryCode;

    (*instruction).op = GET_OPCode(binaryCode);


    if ( (*instruction).op > S_HALT_OPCODE){
       /// error case handler when opcode > 0x3f
       fprintf(stderr,"[Error][convertBinaryToInstr] opCode=0x%02X, Cannot find match opCode > S_HALT_OPCODE:(0x%02X) \n",(*instruction).op ,S_HALT_OPCODE);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
    }else if ( (*instruction).op == S_HALT_OPCODE ){
        (*instruction).instrType =S;
        (*instruction).j_s_addr =GET_J_S_Address(binaryCode);
        return;
    }else if ( (*instruction).op == R_OPCODE){
        (*instruction).instrType = R;
        (*instruction).r_i_rs = GET_R_I_RsCode(binaryCode);
        (*instruction).r_i_rt = GET_R_I_RtCode(binaryCode);
        (*instruction).r_rd = GET_R_RdCode(binaryCode);
        (*instruction).r_shamt = GET_R_ShamtCode(binaryCode);
        (*instruction).r_funct = GET_R_FunctCode(binaryCode);

    }else if ( (*instruction).op == J_J_OPCODE ||  (*instruction).op == J_JAL_OPCODE){
        (*instruction).instrType =J;
        (*instruction).j_s_addr =GET_J_S_Address(binaryCode);

    }else{
        /// I instruction
        (*instruction).instrType =I;
        (*instruction).r_i_rs = GET_R_I_RsCode(binaryCode);
        (*instruction).r_i_rt = GET_R_I_RtCode(binaryCode);
        (*instruction).i_immediate =   GET_I_Immediate(binaryCode);
    }
}

 /** decoding binary code , it will call convertBinaryToInstr() **/
void decoding(){

     /** before decoding we check our programCounter value again,
     to make sure our programCounter value won't be overflowed or misaligned,
     by the update of previous instruction action **/

    checkInstrMemoryError();

    int instr = instr_Disk[ (programCounter/4) ];
    lookupTLB(programCounter,instr_Disk, &instr_Memory, &instr_MemorySetting, &instr_TLB,&instr_PageTable,&instr_Cache );

    /// int instr = 0x00223344;
    /** do decoding action by converting sequential binary code into
        different fields like rt,rs,rd **/
    convertBinaryToInstr(instr);


}


/** check I memory has overflowed or misaligned **/
void checkInstrMemoryError(){
    /*** instrMemory Address overflow ***/
    bool isIMemError =false;
    if ( (programCounter /4 ) <0 || (programCounter/4) >255 ){
        fprintf(stderr,"[Error][checkInstrMemoryError] instrMemory is overflow, programCounter=0x%08X (%d), (programCounter/4)=%d  <0 || > 255\n",programCounter,programCounter,(programCounter/4));
        fflush(stderr);
        isIMemError = true;
    }

     /*** instrMemory is misligned.***/
    if (programCounter % 4 != 0){
        fprintf(stderr,"[Error][checkInstrMemoryError] instrMemory is misligned (programCounter%4 !=0) , programCounter=0x%08X (%d), (programCounter%4)=%d\n",programCounter,programCounter,(programCounter %4));
        fflush(stderr);
        isIMemError = true;
    }

    if (isIMemError){
        errorHandler(ERROR_EXIT_NOW);
    }

}

/** check D memory access error **/
void checkMemoryAccessError(int howManyBytes){
    /** check Number overflow first, the order follows the Error Detection SPEC **/
    int rsSign = GET_Sign_Bit(  registers[(*instruction).r_i_rs]  );
    int immSign =  GET_Sign_Bit(  (*instruction).i_immediate );
    int memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;

    if (rsSign == immSign){
        int memAddrSign = GET_Sign_Bit( memoryAddress );
        if (memAddrSign != immSign ){
            errorHandler(ERROR_NUMBER_OVERFLOW); /** overflow when signs are not the same. **/
        }
    }

    /** check Memory address overflow later **/
    if (memoryAddress < MEMORY_ADDR_FIRST || memoryAddress > MEMORY_ADDR_LAST  ){
         errorHandler(ERROR_MEMORY_ADDR_OVERFLOW);
    }else if ( (memoryAddress + howManyBytes -1 ) > MEMORY_ADDR_LAST ){
         /** when memory address is available. however, it access bytes
             + memory address is larger than available address **/
         errorHandler(ERROR_MEMORY_ADDR_OVERFLOW);
    }


    /** check Memory misalignment error **/
    bool isMisaligment = false;
    switch( (*instruction).op ){
       case I_LW_OPCODE:
       case I_SW_OPCODE:
                        if (memoryAddress % 4 != 0) isMisaligment = true;
                        break;
       case I_LH_OPCODE:
       case I_LHU_OPCODE:
       case I_SH_OPCODE:
                        if (memoryAddress % 2 != 0) isMisaligment = true;
                        break;
       case I_LB_OPCODE:
       case I_LBU_OPCODE:
       case I_SB_OPCODE:
                        break;
       default:
                        break;
    }

    if (isMisaligment){
        errorHandler(ERROR_DATA_MISALIGNED);
    }
}


 /** execute S instruction's job **/
void executeS(){
     /** check op code format **/
    int i=0;
    bool isMatchS =false;
    for (i=0;i<S_INSTRSPEC_COUNT;i++){
        if ((*instruction).op != sInstrSpec[i].op) continue;
        isMatchS = true;
        break;
    }

    if (!isMatchS){
            fprintf(stderr,"[Error][executeS] S-instruction OPCode(0x%02X) is not matched in S_InstrSpec.\n",(*instruction).op);
            fflush(stderr);
            errorHandler(ERROR_EXIT_NOW);
    }

     switch( (*instruction).op   ){
                        case S_HALT_OPCODE:
                            isHalt = true;
                            break;
     }

}


/** execute J instruction's job **/
void executeJ(){

    /** check op code format **/
    int i=0;
    bool isMatchJ =false;
    for (i=0;i<J_INSTRSPEC_COUNT;i++){
        if ((*instruction).op != jInstrSpec[i].op) continue;
        isMatchJ = true;
        break;
    }

    if (!isMatchJ){
            fprintf(stderr,"[Error][executeJ] J-instruction OPCode(0x%02X) is not matched in J_InstrSpec.\n",(*instruction).op);
            fflush(stderr);
            errorHandler(ERROR_EXIT_NOW);
    }


     int address = (*instruction).j_s_addr;

     switch( (*instruction).op   ){
        case J_J_OPCODE:
                        /** Jump **/
                        ///programCounter = (programCounter & 0xf0000000) | ( address <<2 );
                        programCounter = (programCounter & 0xf0000000) | ( address *4);
                        break;
        case J_JAL_OPCODE:
                        /** Jump And Link **/
                        registers[R_RA] = programCounter;
                        ///programCounter = (programCounter & 0xf0000000) | ( address <<2 );
                        programCounter = (programCounter & 0xf0000000) | (address * 4 );
                        break;
     }

}

/** load 2 bytes form memory **/
short loadShortFromMemory(int memoryAddress){
    short value= 0;
    int dataMemoryIndex = memoryAddress /4;

    int index = memoryAddress % 4;

    switch (index){
        case 0:
            /// fisrt 2 bytes
            value = (short) ((data_Disk[dataMemoryIndex] & 0xffff0000) >> 16);
            break;
        case 2:
            /// the last 2 bytes=> memoryAddress % 2 == 0
            value = (short) (data_Disk[dataMemoryIndex] & 0xffff);
            break;
    }

    return value;
}

/** load 1 byte from memory **/
char loadByteFromMemory(int memoryAddress){
    char value = '\0';
    int dataMemoryIndex = memoryAddress /4;
    int index = memoryAddress % 4;

    switch (index){
        case 0:
             value = (char) ((data_Disk[dataMemoryIndex] & 0xff000000) >>24);
             break;;
        case 1:
            value = (char) ((data_Disk[dataMemoryIndex] & 0x00ff0000) >> 16);
              break;
        case 2:
            value = (char) ((data_Disk[dataMemoryIndex] & 0x0000ff00) >>8);
              break;
        case 3:
             value = (char) (data_Disk[dataMemoryIndex] & 0x000000ff) ;
              break;
    }

    return value;
}

/** save 2 bytes into memory **/
void saveShortIntoMemory(int memoryAddress, int rtIndex ){

     int dataMemoryIndex = memoryAddress /4;
     int index = memoryAddress % 4;
     int last16bits = registers[rtIndex] & 0xffff;


     switch (index){
        case 0:
            /// fisrt 2 bytes
            data_Disk[dataMemoryIndex]  = (last16bits << 16) | (data_Disk[dataMemoryIndex] & 0xffff)  ;
            break;
        case 2:
            ///the last 2 bytes=> memoryAddress % 2 == 0
            data_Disk[dataMemoryIndex]  = (data_Disk[dataMemoryIndex] & 0xffff0000) | last16bits;
            break;
    }
}

/** save 1 bytes into memory **/
void saveByteIntoMemory(int memoryAddress, int rtIndex){
     int dataMemoryIndex = memoryAddress /4;
     int index = memoryAddress % 4;

     int last8Bits = registers[rtIndex] & 0xff;


      switch (index){
        case 0:
             data_Disk[dataMemoryIndex] = (data_Disk[dataMemoryIndex] & 0x00ffffff ) | (last8Bits <<  24);
             break;;
        case 1:
             data_Disk[dataMemoryIndex] = (data_Disk[dataMemoryIndex] & 0xff00ffff ) | (last8Bits <<  16);
              break;
        case 2:
             data_Disk[dataMemoryIndex] = (data_Disk[dataMemoryIndex] & 0xffff00ff ) | (last8Bits <<  8);
              break;
        case 3:
             data_Disk[dataMemoryIndex] = (data_Disk[dataMemoryIndex] & 0xffffff00 ) | last8Bits;
              break;
    }

}



/** execute I instruction's job **/
void executeI(){
   /** check op code format **/
   int i=0;
   bool isMatchI =false;
   for (i=0;i<I_INSTRSPEC_COUNT;i++){
      if ((*instruction).op != iInstrSpec[i].op) continue;
      isMatchI = true;
      break;
   }

   if (!isMatchI){
        fprintf(stderr,"[Error][executeI] I-instruction OPCode(0x%02X) is not matched in I_InstrSpec.\n",(*instruction).op);
        fflush(stderr);
        errorHandler(ERROR_EXIT_NOW);
   }

   /** write register 0 **/

   if ( (*instruction).r_i_rt == R_ZERO ){
       if (  (*instruction).op == I_ADDI_OPCODE ||  (*instruction).op == I_LW_OPCODE ||  (*instruction).op == I_LH_OPCODE ||
             (*instruction).op == I_LHU_OPCODE ||  (*instruction).op == I_LB_OPCODE ||  (*instruction).op == I_LBU_OPCODE ||
             (*instruction).op == I_LUI_OPCODE ||  (*instruction).op == I_ANDI_OPCODE ||  (*instruction).op == I_ORI_OPCODE ||
             (*instruction).op == I_NORI_OPCODE ||  (*instruction).op == I_SLTI_OPCODE ){
                 errorHandler(ERROR_WRITE_R_ZERO);
                  /** cannot return , because one instruction may cause several errors.**/
                  /// return; /// do nothing when writing register $zero
             }
   }

   int memoryAddress=0,dataMemoryIndex=0;

   int rsSign=0, immSign=0,rtSign =0;

    int oldPCSign=0, newPCSign=0, newProgramCounter=0;

   switch( (*instruction).op ){
        case I_ADDI_OPCODE:

                            rsSign = GET_Sign_Bit( registers[ (*instruction).r_i_rs ]);
                            immSign =  GET_Sign_Bit(  (*instruction).i_immediate  );
                            registers[ (*instruction).r_i_rt ] =  registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;

                            if (rsSign == immSign){
                                rtSign = GET_Sign_Bit( registers[ (*instruction).r_i_rt ] );
                                if (rtSign != immSign ){
                                    errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when signs are not the same.
                                }
                            }
                            break;
        case I_LW_OPCODE:
                            checkMemoryAccessError(4);
                            if (isHalt) return; /** key avoids to make the program shut down by accessing wrong data **/
                            memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;
                            lookupTLB(memoryAddress,data_Disk, &data_Memory,&data_MemorySetting,&data_TLB, &data_PageTable, &data_Cache);
                            dataMemoryIndex = memoryAddress /4;
                            registers[ (*instruction).r_i_rt ]  = data_Disk[dataMemoryIndex];
                            break;
        case I_LH_OPCODE:
                            checkMemoryAccessError(2);
                            if (isHalt) return; /** key avoids to make the program shut down by accessing wrong data **/
                            memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;
                            lookupTLB(memoryAddress,data_Disk, &data_Memory,&data_MemorySetting,&data_TLB, &data_PageTable, &data_Cache);
                            registers[ (*instruction).r_i_rt ]  =  loadShortFromMemory(memoryAddress);
                            break;
        case I_LHU_OPCODE:
                            checkMemoryAccessError(2);
                            if (isHalt) return; /** key avoids to make the program shut down by accessing wrong data **/
                            memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;
                            lookupTLB(memoryAddress,data_Disk, &data_Memory,&data_MemorySetting,&data_TLB, &data_PageTable, &data_Cache);
                            registers[ (*instruction).r_i_rt ]  =  0x0000ffff & loadShortFromMemory(memoryAddress);
                            break;
        case I_LB_OPCODE:
                            checkMemoryAccessError(1);
                            if (isHalt) return; /** key avoids to make the program shut down by accessing wrong data **/
                            memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;
                            lookupTLB(memoryAddress,data_Disk, &data_Memory,&data_MemorySetting,&data_TLB, &data_PageTable, &data_Cache);
                            registers[ (*instruction).r_i_rt ]  = loadByteFromMemory( memoryAddress ) ;
                            break;
        case I_LBU_OPCODE:
                            checkMemoryAccessError(1);
                            if (isHalt) return; /** key avoids to make the program shut down by accessing wrong data **/
                            memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;
                            lookupTLB(memoryAddress,data_Disk, &data_Memory,&data_MemorySetting,&data_TLB, &data_PageTable, &data_Cache);
                            registers[ (*instruction).r_i_rt ]  = 0x000000ff & loadByteFromMemory(memoryAddress);
                            break;
        case I_SW_OPCODE:
                            checkMemoryAccessError(4);
                            if (isHalt) return; /** key avoids to make the program shut down by accessing wrong data **/
                            memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;
                            lookupTLB(memoryAddress,data_Disk, &data_Memory,&data_MemorySetting,&data_TLB, &data_PageTable, &data_Cache);
                            dataMemoryIndex = memoryAddress /4;
                            data_Disk[dataMemoryIndex] = registers[ (*instruction).r_i_rt ];
                            break;
        case I_SH_OPCODE:
                            checkMemoryAccessError(2);
                            if (isHalt) return; /** key avoids to make the program shut down by accessing wrong data **/
                            /*** from register= 0x01020304 to  memory= 0x05060708
                                 new memory= 0x03040708  ***/
                            memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;
                            lookupTLB(memoryAddress,data_Disk, &data_Memory,&data_MemorySetting,&data_TLB, &data_PageTable, &data_Cache);
                            saveShortIntoMemory(memoryAddress, (*instruction).r_i_rt );
                            break;
        case I_SB_OPCODE:
                            checkMemoryAccessError(1);
                            if (isHalt) return; /** key avoids to make the program shut down by accessing wrong data **/
                            /*** from register= 0x01020304 to  memory= 0x05060708
                                 new memory= 0x04060708  ***/
                            memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;
                            lookupTLB(memoryAddress,data_Disk, &data_Memory,&data_MemorySetting,&data_TLB, &data_PageTable, &data_Cache);
                            saveByteIntoMemory(memoryAddress, (*instruction).r_i_rt );
                            break;
        case I_LUI_OPCODE:
                            /** put immediate in first 16 bits, and clear last 16 bits **/
                            registers[ (*instruction).r_i_rt ] = (*instruction).i_immediate << 16 & 0xffff0000;
                            break;
        case I_ANDI_OPCODE:
                            /** debugging before immediate is 16 bit needs to and with 0xffff , then to do logic operation**/
                            registers[ (*instruction).r_i_rt ] = registers[(*instruction).r_i_rs] &  ( (*instruction).i_immediate & 0xffff ) ;
                            break;
        case I_ORI_OPCODE:
                           registers[ (*instruction).r_i_rt ] = registers[(*instruction).r_i_rs] |  ( (*instruction).i_immediate & 0xffff ) ;
                            break;
        case I_NORI_OPCODE:
                            registers[ (*instruction).r_i_rt ] = ~(registers[(*instruction).r_i_rs] |  ( (*instruction).i_immediate & 0xffff )) ;
                            break;
        case I_SLTI_OPCODE:
                            registers[ (*instruction).r_i_rt ] =  registers[(*instruction).r_i_rs] < (*instruction).i_immediate;
                            break;
        case I_BEQ_OPCODE:

                             oldPCSign = GET_Sign_Bit(  programCounter );
                             immSign =  GET_Sign_Bit(  ((*instruction).i_immediate * 4) );

                            /// current programCounter value + offset (relative instruction's interval )
                            /// programCounter = programCounter + ((*instruction).i_immediate <<2 );
                             newProgramCounter = programCounter + (*instruction).i_immediate * 4;

                            if (oldPCSign == immSign){
                                 newPCSign = GET_Sign_Bit(  programCounter );
                                if (newPCSign != immSign ){
                                        errorHandler(ERROR_NUMBER_OVERFLOW); /** overflow when signs are not the same. **/
                                }
                            }

                            if ( registers[ (*instruction).r_i_rs ] == registers[ (*instruction).r_i_rt ] ){
                                programCounter = newProgramCounter;
                            }

                            break;
        case I_BNE_OPCODE:

                             oldPCSign = GET_Sign_Bit(  programCounter );
                             immSign =  GET_Sign_Bit(  ((*instruction).i_immediate * 4) );

                            /// current programCounter value + offset (relative instruction's interval)
                                /// programCounter = programCounter + ((*instruction).i_immediate <<2 );
                             newProgramCounter = programCounter + (*instruction).i_immediate * 4;


                            if (oldPCSign == immSign){
                                 newPCSign = GET_Sign_Bit(  programCounter );
                                if (newPCSign != immSign ){
                                        errorHandler(ERROR_NUMBER_OVERFLOW); /** overflow when signs are not the same. **/
                                }
                            }

                            if ( registers[ (*instruction).r_i_rs ] != registers[ (*instruction).r_i_rt ] ){
                                programCounter = newProgramCounter;
                            }
                            break;

   }

}

/** execute R instruction's job **/
void executeR(){
   /** check op code format **/
   int i=0;
   bool isMatchR =false;
   for (i=0;i<R_INSTRSPEC_COUNT;i++){
      if ((*instruction).r_funct != rInstrSpec[i].r_funct) continue;
      isMatchR = true;
      break;
   }

   if (!isMatchR){
        fprintf(stderr,"[Error][executeR] R-instruction OPCode(0x%02X) is not matched in R_InstrSpec.\n",(*instruction).op);
        fflush(stderr);
        errorHandler(ERROR_EXIT_NOW);
   }


   /** write register 0 **/
   if ( (*instruction).r_rd == R_ZERO ){
       if (  (*instruction).r_funct == R_ADD_FUNCT ||  (*instruction).r_funct == R_SUB_FUNCT ||  (*instruction).r_funct == R_AND_FUNCT ||
             (*instruction).r_funct == R_OR_FUNCT ||  (*instruction).r_funct == R_XOR_FUNCT ||  (*instruction).r_funct == R_NOR_FUNCT ||
             (*instruction).r_funct == R_NAND_FUNCT ||  (*instruction).r_funct == R_SLT_FUNCT ||
             (*instruction).r_funct == R_SRL_FUNCT ||  (*instruction).r_funct == R_SRA_FUNCT ){
                 errorHandler(ERROR_WRITE_R_ZERO);
                  /** cannot return , because one instruction may cause several errors.**/
                  /// return; /// do nothing when writing register $zero
      }else if ( (*instruction).r_funct == R_SLL_FUNCT && (  (*instruction).r_i_rt != R_ZERO ||  (*instruction).r_shamt !=0 ) ){
            /** For specifal case. If case is sll $0, $0, $0 , we just run it normally.  **/
             errorHandler(ERROR_WRITE_R_ZERO);
      }
   }

   int rsSign=0,rtSign=0, rdSign =0;
   switch( (*instruction).r_funct ){
        case R_ADD_FUNCT:

                            rsSign = GET_Sign_Bit( registers[(*instruction).r_i_rs] );
                            rtSign =  GET_Sign_Bit(  registers[ (*instruction).r_i_rt]  );
                            registers[ (*instruction).r_rd ] =  registers[ (*instruction).r_i_rs ] + registers[ (*instruction).r_i_rt ] ;

                            if (rsSign == rtSign){
                                rdSign = GET_Sign_Bit( registers[ (*instruction).r_rd ] );
                                if (rtSign != rdSign ){
                                    errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when signs are not the same.
                                }
                            }
                            break;
        case R_SUB_FUNCT:
                            rsSign = GET_Sign_Bit( registers[(*instruction).r_i_rs]  );
                            rtSign =  GET_Sign_Bit(  registers[(*instruction).r_i_rt]  );
                            int substractedNumber =  registers[(*instruction).r_i_rt] ;

                            int value = 0;
                            /// registers[ (*instruction).r_rd ] =  registers[ (*instruction).r_i_rs ] - registers[ (*instruction).r_i_rt ] ;
                            /// rdSign = GET_Sign_Bit( registers[ (*instruction).r_rd ] );

                            if (substractedNumber== 0x80000000){
                                /// for special case
                                /// 0x00000004 - 0x80000000 = 0x80000004
                                /// b=0x80000000是負數
                                /// -b=0x80000000還是負數
                                /// 0x00000004-0x80000000是正加負，不會有number overflow

                                value =  registers[ (*instruction).r_i_rs ] + registers[ (*instruction).r_i_rt ] ;
                                rdSign = GET_Sign_Bit( value );
                                registers[ (*instruction).r_rd ] =value;


                                if (rsSign == rtSign){
                                    if (rtSign != rdSign ){
                                        errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when signs are not the same.
                                    }
                                }

                            }else{

                                value = registers[ (*instruction).r_i_rs ] - registers[ (*instruction).r_i_rt ] ;
                                rdSign = GET_Sign_Bit( value );

                                registers[ (*instruction).r_rd ] =value;

                                if (rsSign ==0 ){
                                    if (rtSign ==1 && rdSign==1){
                                        errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when positive - negative = negative
                                    }
                                }else if (rsSign == 1){
                                    if (rtSign ==0 && rdSign ==0){
                                        errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when negative - positive = positive
                                    }
                                }
                            }

                            break;
        case R_AND_FUNCT:
                            registers[ (*instruction).r_rd ] = registers[ (*instruction).r_i_rs ] &  registers[ (*instruction).r_i_rt ];
                            break;
        case R_OR_FUNCT:
                            registers[ (*instruction).r_rd ] = registers[ (*instruction).r_i_rs ] | registers[ (*instruction).r_i_rt ];
                            break;
        case R_XOR_FUNCT:
                            registers[ (*instruction).r_rd ] = registers[ (*instruction).r_i_rs ] ^ registers[ (*instruction).r_i_rt ];
                            break;
        case R_NOR_FUNCT:
                            registers[ (*instruction).r_rd ] = ~(registers[ (*instruction).r_i_rs ] | registers[ (*instruction).r_i_rt ]);
                            break;
        case R_NAND_FUNCT:
                            registers[ (*instruction).r_rd ] = ~(registers[ (*instruction).r_i_rs ] & registers[ (*instruction).r_i_rt ]);
                            break;
        case R_SLT_FUNCT:
                            registers[ (*instruction).r_rd ] =  (registers[ (*instruction).r_i_rs ] < registers[ (*instruction).r_i_rt ]);
                            break;
        case R_SLL_FUNCT:
                            registers[ (*instruction).r_rd ] = registers[ (*instruction).r_i_rt ] <<  (*instruction).r_shamt;
                            break;
        case R_SRL_FUNCT:
                            /**  Shit right logic
                                 0xFF020304 >> 8 => 0x00FF0203   **/
                            registers[ (*instruction).r_rd ] = ((unsigned int) registers[ (*instruction).r_i_rt ]) >>  (*instruction).r_shamt;
                            break;
        case R_SRA_FUNCT:
                            /**  Shit right arithmetic
                                 0x81020304 >> 8 => 0xff810203   **/
                            registers[(*instruction).r_rd]  =   registers[(*instruction).r_i_rt ]  >>  (*instruction).r_shamt;
                            break;
        case R_JR_FUNCT:
                            /** Jump Register **/
                            programCounter = registers[ (*instruction).r_i_rs  ];
                            break;
   }


}


/** execute R,I,J,S job **/
void execution(){
    InstructionType type = (*instruction).instrType;
    switch (type){
        case R:
                executeR();
                break;
        case I:
                executeI();
                break;
        case J:
                executeJ();
                break;
        case S:
                executeS();
                break;
    }
}

void getMemorySetting(int argc, char **argv){
   /** I_memory_size, D_memory_size, I_page_size, D_page_size, I_cache_size
     I_cache_block_size, I_cache_set_associativity ,
     D_cache_block_size , D_cache_block_size  ,  D_cache_set_associativity
   **/


   int defaultSetting[] = {64, 32, 8, 16, 16, 4,  4, 16, 4,  1};


   ///int defaultSetting[] = {64, 32, 8, 16, 16, 4,  1, 16, 4,  1};
   ///int defaultSetting[10] = {64, 32, 8, 16, 16, 4, 2, 16, 4, 1};
   /// wrong!!  (128,64,16,8,32,8,1,32,8,2)
   /// 64,128,16,8,64,16,2,64,16,4
   /// wrong!! 32 16 16 4 512 128 1 32 8 2
   /// 64 32 8 16 16 4 2 16 4 1


   int i=0;
   if (argc == 11){
		for (i = 1 ; i < 11 ; i++){
			defaultSetting[i-1] = atoi(argv[i]);
			printf("[%d]=%d, ",i,defaultSetting[i-1]);

			if ( i == 7 || i == 10 ) continue;
            if  ( defaultSetting[i-1] % 4 != 0 ){
				printf("\nError: argv[%d]=%d is not be of multiple of four.\n", i, defaultSetting[i-1] );
                errorHandler(ERROR_EXIT_NOW);
            }else if((i-1) == 10){
                printf("\n");
            }
		}
	}else if (argc >1 ){
	    printf("Warning!! Becasue argc=%d is not 11 (not with full settings' parameters), we run the program with default setting.\n", argc);
	}

   instr_MemorySetting.memorySize= defaultSetting[0];
   instr_MemorySetting.pageSize =  defaultSetting[2];
   instr_MemorySetting.cacheSize =  defaultSetting[4];
   instr_MemorySetting.cacheBlockSize =  defaultSetting[5];
   instr_MemorySetting.cacheSetAssociativity =  defaultSetting[6];

   data_MemorySetting.memorySize= defaultSetting[1];
   data_MemorySetting.pageSize =  defaultSetting[3];
   data_MemorySetting.cacheSize =  defaultSetting[7];
   data_MemorySetting.cacheBlockSize =  defaultSetting[8];
   data_MemorySetting.cacheSetAssociativity =  defaultSetting[9];
}

void initializeMemory(){
        int i=0;
        for (i = 0; i < 1024; i++)	{
                instr_Memory.valid[i]=-1;
                instr_Memory.seqNO_LRU[i]=-1;
                instr_Memory.data[i]=-1;
                data_Memory.valid[i]=-1;
                data_Memory.seqNO_LRU[i]=-1;
                data_Memory.data[i] =-1;

                instr_Cache.valid[i]=-1;
                instr_Cache.tag[i]=-1;
                instr_Cache.seqNO_LRU[i]=-1;
                instr_Cache.data[i]=-1;
                instr_Cache.cacheBlockAddr[i]=-1;
                data_Cache.valid[i]=-1;
                data_Cache.tag[i]=-1;
                data_Cache.seqNO_LRU[i]=-1;
                data_Cache.data[i]=-1;
                data_Cache.cacheBlockAddr[i]=-1;  /** for debugging only to validate my calculated formula**/

                instr_PageTable.valid[i]=-1;
                instr_PageTable.content[i]=-1;
                data_PageTable.valid[i]=-1;
                data_PageTable.content[i]=-1;

                instr_TLB.valid[i]=-1;
                instr_TLB.tag[i]=-1;
                instr_TLB.seqNO_LRU[i]=-1;
                instr_TLB.content[i]=-1;
                data_TLB.valid[i]=-1;
                data_TLB.tag[i]=-1;
                data_TLB.seqNO_LRU[i]=-1;
                data_TLB.content[i]=-1;
        }


        instr_Memory.dataLen =  instr_MemorySetting.memorySize /sizeof(int);
        instr_Memory.memLen = instr_MemorySetting.memorySize / instr_MemorySetting.pageSize;
        instr_Memory.current_SeqNO_LRU=0;

        data_Memory.dataLen = data_MemorySetting.memorySize /sizeof(int);
        data_Memory.memLen = data_MemorySetting.memorySize / data_MemorySetting.pageSize;
        data_Memory.current_SeqNO_LRU=0;

        instr_PageTable.pageTableLen = 1024 /  instr_MemorySetting.pageSize;
        instr_PageTable.hitRate.hit=0;
        instr_PageTable.hitRate.miss=0;

        data_PageTable.pageTableLen = 1024 /  data_MemorySetting.pageSize;
        data_PageTable.hitRate.hit=0;
        data_PageTable.hitRate.miss=0;

        instr_TLB.tlbLen = (1024/instr_MemorySetting.pageSize)/4;
        instr_TLB.current_SeqNO_LRU =0;
        instr_TLB.hitRate.hit=0;
        instr_TLB.hitRate.miss=0;

        data_TLB.tlbLen = (1024 /data_MemorySetting.pageSize) /4;
        data_TLB.current_SeqNO_LRU =0;
        data_TLB.hitRate.hit=0;
        data_TLB.hitRate.miss=0;

        instr_Cache.cacheLen = instr_MemorySetting.cacheSize / instr_MemorySetting.cacheBlockSize;
        instr_Cache .dataLen = instr_MemorySetting.cacheSize /sizeof(int);
        instr_Cache.current_SeqNO_LRU=0;
        instr_Cache.hitRate.hit=0;
        instr_Cache.hitRate.miss=0;

		data_Cache.cacheLen = data_MemorySetting.cacheSize / data_MemorySetting.cacheBlockSize;
        data_Cache .dataLen = data_MemorySetting.cacheSize /sizeof(int);
        data_Cache.current_SeqNO_LRU=0;
        data_Cache.hitRate.hit=0;
        data_Cache.hitRate.miss=0;

}

void shiftLRUSeqNoWhenBeyondLimit( int indexLRU[], const int len, int * seqNoLRU ){


    if ( (*seqNoLRU) < LRU_SEQ_NO_LIMIT){
           return;
    }

    int minValue=MIN_VALUE;
    int i=0;
    for (i=0;i<(len);i++){
        if (indexLRU[i] <0 ) continue;
        if (indexLRU[i]<=minValue) {
            minValue = indexLRU[i];
        }
    }

    for (i=0;i<(len);i++){
         if (indexLRU[i] <0 ) continue;
         indexLRU[i] = indexLRU[i]-minValue;
    }

    (*seqNoLRU) = (*seqNoLRU) -minValue;
}


int findVictimByLeastSeqNoLRU(int valid[] ,int indexLRU[],const int start, const int end ){
    int minValue=MIN_VALUE;
    int minValueIndex =-1;

    int i=0;
    for (i=start;i<= end;i++){
        if (valid[i]<=0 ) continue;
        if (indexLRU[i]<0 ) continue;
        if (indexLRU[i]<=minValue) {
            minValue = indexLRU[i];
            minValueIndex = i;
        }
    }
    return minValueIndex;
}


void moveData(int fromStart, int fromEnd, int toStart, int from[], int to[] ){
  int i=fromStart;
  int j=toStart;
  for (; i<=fromEnd;i++,j++ ){
       to[j] = from[i];
  }
}

void lookupTLB(int virtualAddr, int disk[] , Memory *memory, MemorySetting *memorySetting,TLB *tlb, PageTable *pageTable, Cache *cache ){
     /**
     if (virtualAddr == 0x00000034  ){
            int a=1;
        }

    if (programCounter == 0x00000034  ){
            int a=1;
        }
     ***/

   int pageAddress = virtualAddr/ (*memorySetting).pageSize;
   int i=0;
   for(i=0;i<(*tlb).tlbLen;i++){
        if ((*tlb).valid[i] <=0 ) continue;
        /** tlb hit **/
        if (pageAddress == (*tlb).tag[i]){
            (*tlb).hitRate.hit++;

            /** tlb LRU updated **/
            (*tlb).current_SeqNO_LRU++;
            shiftLRUSeqNoWhenBeyondLimit(   (*tlb).seqNO_LRU, (*tlb).tlbLen  , &((*tlb).current_SeqNO_LRU)) ;
            (*tlb).seqNO_LRU[i]= (*tlb).current_SeqNO_LRU;

            /** get PA **/
            int pa = ( (*tlb).content[i] * (*memorySetting).pageSize);
            int offset = virtualAddr % ( *memorySetting).pageSize;
            pa = pa+offset;

            /** use pa to look up data in cache **/
            int cacheIndex = lookupCache( pa, disk , memory,  memorySetting,  pageTable,  cache);
            return;
        }
   }
   /** tlb miss **/
   (*tlb).hitRate.miss++;

   /** look up data from page table **/
   int invalidPageAddress =  lookupPageTable( virtualAddr, disk , memory,  memorySetting,  pageTable,  cache);


   /** update valid pageAddress to be invalid **/
   if (invalidPageAddress >=0){
       for (i = 0; i < (*tlb).tlbLen; i++){
            if ((*tlb).valid[i] <0 ) continue;
			if ((*tlb).tag[i] == invalidPageAddress){
                  (*tlb).seqNO_LRU[i]=-1;
                  (*tlb).tag[i] = -1;
                  (*tlb).valid[i] = -1;
            }
		}
   }

    /** find valid tlb and update tlb info **/
    for(i=0;i<(*tlb).tlbLen;i++){
        if ((*tlb).valid[i] >0 ) continue;
        (*tlb).tag[i]=pageAddress;
        (*tlb).valid[i]=1;
        (*tlb).content[i]= (*pageTable).content[pageAddress];
        /** tlb LRU updated **/
        (*tlb).current_SeqNO_LRU++;
        shiftLRUSeqNoWhenBeyondLimit(   (*tlb).seqNO_LRU, (*tlb).tlbLen  , &((*tlb).current_SeqNO_LRU));
        (*tlb).seqNO_LRU[i]=(*tlb).current_SeqNO_LRU;

        /** get pa **/
        int pa = ((*tlb).content[i] * (*memorySetting).pageSize);
        int offset = virtualAddr % (*memorySetting).pageSize;
        pa = pa+offset;

        /** use pa to look up data in cache **/
        int cacheIndex = lookupCache( pa, disk , memory,  memorySetting,  pageTable,  cache);
        return;
    }

    /** not find valid tlb, find victim by least LRU seqNO **/
    int victimIndex = findVictimByLeastSeqNoLRU((*tlb).valid,  (*tlb).seqNO_LRU, 0,((*tlb).tlbLen -1)) ;
    (*tlb).tag[victimIndex] = pageAddress;
    (*tlb).content[victimIndex] = (*pageTable).content[pageAddress];

    /** update tlb LRU seqNo **/
    (*tlb).current_SeqNO_LRU++;
    shiftLRUSeqNoWhenBeyondLimit(   (*tlb).seqNO_LRU, (*tlb).tlbLen  , &((*tlb).current_SeqNO_LRU) );
    (*tlb).seqNO_LRU[victimIndex]=(*tlb).current_SeqNO_LRU;

    /** get pa **/
    int pa = ( (*tlb).content[victimIndex] * (*memorySetting).pageSize);
    int offset = virtualAddr %  (*memorySetting).pageSize;
    pa = pa+offset;

    /** use pa to look up data in cache **/
    int cacheIndex = lookupCache( pa, disk , memory,  memorySetting,  pageTable,  cache);
    return;

}

/** return invalid pageAddress, whose mapping memory blocks' index: updatedMemoryBlockIndex has been replaced with new data. **/
int lookupPageTable(int virtualAddr, int disk[] ,Memory *memory, MemorySetting *memorySetting, PageTable *pageTable, Cache *cache){
    int pageAddress =virtualAddr/ (*memorySetting).pageSize;
    int invalidPageAddress = -1;

    /**
    if (virtualAddr == 0x00064){
            int a=1;
        }

        ***/

    /** pageTable hit **/
    if ( (*pageTable).valid[pageAddress] >0 ){
        (*pageTable).hitRate.hit++;
        return -1;
    }

    /** pageTable miss, data in disk**/
    (*pageTable).hitRate.miss++;
    (*pageTable).valid[pageAddress] =1;
    int pageSizeInWords = (*memorySetting).pageSize/sizeof(int) ;
    int updatedMemoryBlockIndex = -1;

    int i=0;
    bool isFind=false;
    for (i=0;i<(*memory).memLen && (!isFind) ;i++){
        if ( (*memory).valid[i] >0 ) continue;

        (*memory).valid[i] =1;
        /** move page-sized data form disk to memory **/
        int fromStart = virtualAddr/sizeof(int);
        int fromEnd =  fromStart + pageSizeInWords;
        int toStart= i * pageSizeInWords;
        moveData( fromStart,  fromEnd,  toStart, disk, (*memory).data );
        /** assign PA (memory blocks' index) to pagetable entry **/
        (*pageTable).content[pageAddress]=i;
        updatedMemoryBlockIndex=i;

        /** update memory LRU seqNo **/
        (*memory).current_SeqNO_LRU++;
        shiftLRUSeqNoWhenBeyondLimit(   (*memory).seqNO_LRU,(*memory).memLen  , &((*memory).current_SeqNO_LRU) );
        (*memory).seqNO_LRU[i]= (*memory).current_SeqNO_LRU;
        isFind=true;
    }


    /** no any valid space in memory , find victim by LRU seqNo.**/
    if(!isFind){
        if (virtualAddr ==0){
            int a=1;
        }
       int victimIndex = findVictimByLeastSeqNoLRU( (*memory).valid, (*memory).seqNO_LRU, 0,( (*memory).memLen -1)) ;
        /** move page-sized data form disk to memory **/
       int fromStart = virtualAddr/sizeof(int);
       int fromEnd =  fromStart + pageSizeInWords;
       int toStart=  victimIndex * pageSizeInWords;

       /**
       if (toStart <0){
        int a=1;
       }**/

       moveData( fromStart,  fromEnd,  toStart, disk,  (*memory).data );

        /** assign PA (memory blocks' index) to pagetable entry **/
        (*pageTable).content[pageAddress]=victimIndex;
        updatedMemoryBlockIndex=victimIndex;

        /** update memory LRU seqNo **/
         (*memory).current_SeqNO_LRU++;
         shiftLRUSeqNoWhenBeyondLimit(    (*memory).seqNO_LRU, (*memory).memLen  , &((*memory).current_SeqNO_LRU) ) ;
         (*memory).seqNO_LRU[victimIndex]= (*memory).current_SeqNO_LRU;
    }

    /** check if the same memory blocks' index in any page table entry of page table **/
    for(i=0;i<(*pageTable).pageTableLen;i++){
        if ( (*pageTable).valid[i] <=0) continue;
        if ( (*pageTable).content[i]!= updatedMemoryBlockIndex ) continue;
        if (i  !=  pageAddress){
            /** it means that PageTable[i] refer to this updatedMemoryBlockIndex before,
            however, the updatedMemoryBlockIndex's data has been updated now **/
            (*pageTable).valid[i] = -1;
            (*pageTable).content[i] =-1;

            /** also check cache by the updatedMemoryBlockIndex **/
            int j=0;
            for(j=0;j< (*cache).cacheLen;j++){
                if ((*cache).valid[j] <=0 ) continue;


                int convert_TagIndex_To_CacheBlockAddr = getCacheBlockAddrByCacheTagIndex(j, memorySetting , cache );
                ///int cache_pa = cache.tag[j] * (*memorySetting).cacheBlockSize;
                int cache_pa = convert_TagIndex_To_CacheBlockAddr * (*memorySetting).cacheBlockSize;
                int cache_pa_In_MemoryBlock_index = cache_pa / (*memorySetting).pageSize;
                if (cache_pa_In_MemoryBlock_index != updatedMemoryBlockIndex) continue;
                (*cache).valid[j]=-1;
                (*cache).tag[j]=-1;
                (*cache).seqNO_LRU[j]=-1;
                (*cache).cacheBlockAddr[j]=-1;
                /** should check if dirty bit is 1, write back to memory **/

                /** break; debugging fatal error!! there are more than one recording to be cleared !!**/
            }
            /** because pageTable index is arranged by 1024/page_size, which is also be page address **/
            invalidPageAddress = i;
            break;
        }
    }
    return invalidPageAddress;

}

/** convert cache tag & index into original cacheBlockAddress **/
int getCacheBlockAddrByCacheTagIndex(int cacheIndex,MemorySetting *memorySetting,Cache *cache){
    int cacheBlockAddr = -1;
    if ( (*memorySetting).cacheSetAssociativity == 1){
         /** direct map**/
         int tag = (*cache).tag[cacheIndex];
         cacheBlockAddr = (tag * (*cache).cacheLen ) +cacheIndex ;
    }else if (  (*memorySetting).cacheSetAssociativity == (*cache).cacheLen  ){
        /** fully associativity **/
        cacheBlockAddr = (*cache).tag[cacheIndex];
    }else{
        /** n-way associativity **/
        int oneCacheSetBlocks =  ( (*cache).cacheLen /  (*memorySetting).cacheSetAssociativity );
        int whichCacheSet = cacheIndex /  (*memorySetting).cacheSetAssociativity;
        int tag = (*cache).tag[cacheIndex];
        cacheBlockAddr = (tag * oneCacheSetBlocks )+ whichCacheSet;
    }
    return cacheBlockAddr;
}

/**  use PA to look up in cache,
  when find pa's location in cache,
  return cache index **/
int lookupCache(int physicalAddr,int disk[] ,Memory *memory, MemorySetting *memorySetting, PageTable *pageTable, Cache *cache){
   int cacheBlockAddr = physicalAddr / (*memorySetting).cacheBlockSize;
   int i=0;
   for (i=0;i< (*cache).cacheLen;i++){
       if ((*cache).valid[i]<=0 ) continue;

       int convert_TagIndex_To_CacheBlockAddr = getCacheBlockAddrByCacheTagIndex(i ,memorySetting , cache );

       /** for debugging only to validate if convert formula has been calculated wrongly**/
       /**  debugging only
       if ((*cache).cacheBlockAddr[i] == cacheBlockAddr){
           if (convert_TagIndex_To_CacheBlockAddr != cacheBlockAddr){
               printf("Fatal Error: (convert_TagIndex_To_CacheBlockAddr=%d) !=  (cacheBlockAddr=%d) .\n", convert_TagIndex_To_CacheBlockAddr, cacheBlockAddr );
               errorHandler(ERROR_EXIT_NOW);
           }
       }**/
       /** for debugging only to validate if convert formula has been caculated wrongly**/

       if ( convert_TagIndex_To_CacheBlockAddr == cacheBlockAddr ){

           (*cache).hitRate.hit++;

           if ( (*memorySetting).cacheSetAssociativity == 1){
               /** direct map, no LRU
                because find cache index by:

                 int index = cacheBlockAddr % (*cache).cacheLen;
                 int tag =   cacheBlockAddr /  (*cache).cacheLen;
                **/

           }else if ( (*memorySetting).cacheSetAssociativity == (*cache).cacheLen  ){
               /** fully associativity **/
               (*cache).current_SeqNO_LRU++;
               shiftLRUSeqNoWhenBeyondLimit(   (*cache).seqNO_LRU,(*cache).cacheLen  , &((*cache).current_SeqNO_LRU) );
               (*cache).seqNO_LRU[i]=(*cache).current_SeqNO_LRU;
           }else{
               /** n-way associativity **/
               (*cache).current_SeqNO_LRU++;
               shiftLRUSeqNoWhenBeyondLimit(   (*cache).seqNO_LRU,(*cache).cacheLen  , &((*cache).current_SeqNO_LRU) );
               (*cache).seqNO_LRU[i]=(*cache).current_SeqNO_LRU;
           }
           return i;
       }
   }

    (*cache).hitRate.miss++;
    int cacheBlockInWords = (*memorySetting).cacheBlockSize / sizeof(int);

    if ( (*memorySetting).cacheSetAssociativity == 1){
               /** direct map, no LRU **/
        int index = cacheBlockAddr % (*cache).cacheLen;
        int tag =   cacheBlockAddr /  (*cache).cacheLen;
        (*cache).valid[index] = 1;
        (*cache).tag[ index] = tag;
        (*cache).cacheBlockAddr[index]=cacheBlockAddr; /** for debugging only to avoid calculated wrongly**/

        /** when cache miss, adopt write allocate.  move data form memory to cache **/
        int fromStart = physicalAddr/sizeof(int);
        int fromEnd =  fromStart + cacheBlockInWords;
        int toStart=  index * cacheBlockInWords;
        moveData( fromStart,  fromEnd,  toStart, (*memory).data, (*cache).data );

        /** direct map no LRU, because it use index  to find the mapping location in cache.**/


    }else if ( (*memorySetting).cacheSetAssociativity == (*cache).cacheLen  ){
               /** fully associativity ,index will always to be zero, because
                   (cacheBlockAddr % 1 ) =0, so tag = (cacheBlockAddr /1 ) = cacheBlockAddr **/
           /** find valid space in cache **/
           for(i=0;i<(*cache).cacheLen ;i++){
              if ((*cache).valid[i] >0) continue;
              (*cache).valid[i] = 1;
              (*cache).tag[i]=cacheBlockAddr;
              (*cache).cacheBlockAddr[i]=cacheBlockAddr;  /** for debugging only to avoid calculated wrongly**/

              /** when cache miss, adopt write allocate.  move data form memory to cache **/
              int fromStart = physicalAddr/sizeof(int);
              int fromEnd =  fromStart + cacheBlockInWords;
              int toStart=  i * cacheBlockInWords;
              moveData( fromStart,  fromEnd,  toStart, (*memory).data, (*cache).data );

              /** update cache LRU **/
              (*cache).current_SeqNO_LRU++;
              shiftLRUSeqNoWhenBeyondLimit(   (*cache).seqNO_LRU,(*cache).cacheLen  , &((*cache).current_SeqNO_LRU));
              (*cache).seqNO_LRU[i]=(*cache).current_SeqNO_LRU;
              return i;
           }

           /** all valid space is full, find victim for replaced **/
           int victimIndex = findVictimByLeastSeqNoLRU( (*cache).valid, (*cache).seqNO_LRU, 0,((*cache).cacheLen -1)) ;
           (*cache).tag[victimIndex] = cacheBlockAddr;
           (*cache).cacheBlockAddr[victimIndex]=cacheBlockAddr;

           /** copy data form memory to cache**/
           int fromStart = physicalAddr/sizeof(int);
           int fromEnd =  fromStart + cacheBlockInWords;
           int toStart=  victimIndex * cacheBlockInWords;
           moveData( fromStart,  fromEnd,  toStart, (*memory).data, (*cache).data );

           /** update cache LRU **/
           (*cache).current_SeqNO_LRU++;
           shiftLRUSeqNoWhenBeyondLimit(   (*cache).seqNO_LRU,(*cache).cacheLen  , &((*cache).current_SeqNO_LRU));
           (*cache).seqNO_LRU[victimIndex]=(*cache).current_SeqNO_LRU;

           return victimIndex;

    }else{
               /** n-way associativity , find valid space in cache **/
            int oneCacheSetBlocks =  ( (*cache).cacheLen / (*memorySetting).cacheSetAssociativity );
            int whichCacheSet =  cacheBlockAddr % oneCacheSetBlocks;
            int tag =  cacheBlockAddr / oneCacheSetBlocks;

            int s = (whichCacheSet* (*memorySetting).cacheSetAssociativity);
            int e = s+ (*memorySetting).cacheSetAssociativity;
            for (i=s  ;i<e;i++ ){
                if ((*cache).valid[i] >0) continue;
                 (*cache).valid[i] = 1;
                 (*cache).tag[i]=tag;
                 (*cache).cacheBlockAddr[i]=cacheBlockAddr;  /** for debugging only to avoid calculated wrongly**/

                /** when cache miss, adopt write allocate.  copy data form memory to cache **/
                int fromStart = physicalAddr/sizeof(int);
                int fromEnd =  fromStart + cacheBlockInWords;
                int toStart=  i * cacheBlockInWords;
                moveData( fromStart,  fromEnd,  toStart, (*memory).data, (*cache).data );

                /** update cache LRU **/
                (*cache).current_SeqNO_LRU++;
                shiftLRUSeqNoWhenBeyondLimit(   (*cache).seqNO_LRU,(*cache).cacheLen  , &((*cache).current_SeqNO_LRU)  );
                (*cache).seqNO_LRU[i]=(*cache).current_SeqNO_LRU;
                return i;
            }

             /** all valid space is full, find victim for replaced **/
            s = (whichCacheSet*  (*memorySetting).cacheSetAssociativity);
            e = s+ (*memorySetting).cacheSetAssociativity -1;

            int victimIndex = findVictimByLeastSeqNoLRU( (*cache).valid, (*cache).seqNO_LRU, s,e) ;
            (*cache).tag[victimIndex] = tag;
            (*cache).cacheBlockAddr[victimIndex]=cacheBlockAddr;

            /** copy data form memory to cache**/
            int fromStart = physicalAddr/sizeof(int);
            int fromEnd =  fromStart + cacheBlockInWords;
            int toStart=  victimIndex * cacheBlockInWords;
            moveData( fromStart,  fromEnd,  toStart, (*memory).data, (*cache).data );

            /** update cache LRU**/
            (*cache).current_SeqNO_LRU++;
            shiftLRUSeqNoWhenBeyondLimit(   (*cache).seqNO_LRU,(*cache).cacheLen  , &(*cache).current_SeqNO_LRU);
            (*cache).seqNO_LRU[victimIndex]=(*cache).current_SeqNO_LRU;

            return victimIndex;
    }


}

void main(int argc, char **argv){


    # if LOG_ENABLE
    logFp = freopen(LOG_FILE,"wr", stderr);
    #endif

    initializeAction();

    iImageFile =loadFile( I_IMAGE_FILE );   /// load iImage.bin
    dImageFile = loadFile( D_IMAGE_FILE );  /// load dImage.bin

    fetchInstruction();
    getMemorySetting(argc, argv);
    initializeMemory();

    while( !isHalt ){



            undoSetting(); /** undo some register value like $zero's value may be updated **/
            if (cycleCount >MAX_CYCLE){
                printf(stderr,"[Error][main] cycleCount=%d is beyond limit:%d.\n",cycleCount,MAX_CYCLE);
                fflush(stderr);
                errorHandler(ERROR_EXIT_NOW);
            }

            #if D_MEMEMORY_LOG_ENABLE
                outputDataMemory();
            #endif // D_MEMEMORY_LOG_ENABLE

            outputSnapshot();
            /** must adding cycle before decoding,
            otherwise, I will get wrong cycle count when
            output error cycle in errorHandler.**/
            cycleCount++;

            /// for debugging only
            /*
            if ( cycleCount == 7
                || cycleCount == 8
                || cycleCount == 9
                || cycleCount == 10
                ||cycleCount == 11 ){
                printf("cycleCount=%d \n",cycleCount);
                int cccc=1;
            }*/


            decoding();

            ///** for debugging to trace each memory detain in each program counter!!!
            ///outputHitRate();


            /** key, before executing instruction,
            we should let programCounter pointers to next instruction location **/
            programCounter = programCounter +4;
            execution();

    }

     outputHitRate();
     finalizeAction();
}




/** The following functions are used for test and design only,
    I do not use it in simulator **/

bool isNumberOverflow(int number){
    long long min=-2147483648 ;
    long long max = 2147483647 ;

    long long longNumber = (long long) number;
     printf("number=%d,(0x%08X) -> longNumber=%ld, 0x%016X is overflowed \n",number,number,longNumber,longNumber) ;

    if (  longNumber > max || longNumber <min ){
        return true;
    }

    return false;

}

typedef struct DataStructure Data;
struct DataStructure{
        char *buffer;
        int (* getData) (Data*); /// function pointer
};


int getData(Data *data){
    int value  =  (data->buffer[0] << 24) | (data->buffer[1] << 16) | (data->buffer[2] << 8) | data->buffer[3];
}


void initData(Data *data, char *buffer){
  data->getData = getData;
  data->buffer =  buffer;
}


int mainAAAAA(void){

     char buf2[4];
    buf2[0] =0x01;
    buf2[1] =0x02;
    buf2[2] =0x03;
    buf2[3] =0x04;

    Data data;
    initData(&data,&buf2);

    printf("data.buffer[0]=0x%02X \n",data.buffer[0]);
    printf("data.buffer[1]=0x%02X \n",data.buffer[1]);
    printf("data.buffer[2]=0x%02X \n",data.buffer[2]);
    printf("data.buffer[3]=0x%02X \n",data.buffer[3]);
    printf("data.getData=0x%08X \n",data.getData(&data));

   return 0;
}

