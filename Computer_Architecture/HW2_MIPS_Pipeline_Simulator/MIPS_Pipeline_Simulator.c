#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "pipeline.h"
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
#define DMEMORY_DUMP_FILE  "./dMemory_dump.rpt"  /** for print out Dmemory for debugging only**/
#define LOG_FILE "./log.txt"  /** log file for debugging only **/
#define MAX_CYCLE 500000
#define LOG_ENABLE 0
#define D_MEMEMORY_LOG_ENABLE 0

/** control if the porgram should be flush instruction or
    stall or jump to somewhere, without follow original programCounter value.**/
typedef struct ControlUnitStructure{
    bool isFlush;
    bool isStall;
    int updatePC;
}  ControlUnit;

/** Recording all the forwarding information.
    On ID & EXE Stages there are forwarding action happening.
    isForwardRsToID ==> means in ID stage rs need to be forwarding from last 2 stages.
    fromStage_ForwardRtToID ==> recording which stage forwarding value to the ID stage.
  **/
typedef struct ForwardingUnitStructure{
    bool isForwardRsToID;
    bool isForwardRtToID;

    char *fromStage_ForwardRsToID;
    char *fromStage_ForwardRtToID;

    bool isForwardRsToEXE;
    bool isForwardRtToEXE;

    char *fromStage_ForwardRsToEXE;
    char *fromStage_ForwardRtToEXE;
} ForwardingUnit;

const char DM_WB_STAGE[20] = "DM-WB";
const char EX_DM_STAGE[20] = "EX-DM";

Instruction NOP = {R,0,0,0,0,0,0,0,0,0,"NOP"};
PipelineBuffer initStage = { {R,0,0,0,0,0,0,0,0,0,"NOP"},0 };

/***
  there are 4 stages pipeline register which just follow book's design.
  However, the last stage WB I need to recording its value for printing out some log information.
  So I add WB stage for program's print out.
***/
PipelineBuffer IF_ID_Buffer, ID_EXE_Buffer , EXE_DM_Buffer , DM_WB_Buffer, WB_Buffer;

ControlUnit controlUnit = {false,false, -1};
ForwardingUnit forwardingUnit = {false, false, -1,-1,NULL,NULL,false, false, -1,-1,NULL,NULL };

/** 4 type R,I,J,S insturction's SPEC stuct to find mapping OP code & funct code **/
typedef struct InstructionSpec{
    InstructionType instrType;
    int op;
    char *instrName;
    int r_funct;
} InstrSpec;


int *dImageFile;  /// source for dimage.bin
int *iImageFile;  /// source for iimage.bin

FILE *logFp;
FILE *dMemoryFp;  /// for debugging to output D-memory information
FILE *snapshotFp;
FILE *errorDumpFp;

int cycleCount =0;
bool isHalt =false;


int dataMemory[256]; /// D-Memory 1K limit => 1024 / 4byte = 256 (1word, MIPS uses a word as a unit for access.)
int instrMemory[256]; /// I-Memory
int programCounter =0;   /// program counter,programCounter
int registers[32];
int registers_Init_Stages[32];

InstrSpec  rInstrSpec[R_INSTRSPEC_COUNT]; /** for R instruction SPEC **/
InstrSpec  iInstrSpec[I_INSTRSPEC_COUNT]; /** for I instruction SPEC **/
InstrSpec  jInstrSpec[J_INSTRSPEC_COUNT]; /** for J instruction SPEC **/
InstrSpec  sInstrSpec[S_INSTRSPEC_COUNT]; /** for S instruction SPEC **/

/** initial each stage with nop instruction **/
void getInitStage(PipelineBuffer *buffer){
    (*buffer) = initStage;
}

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

/** get instruction's name for printing out instruction.**/
char* getInstrName( InstructionType instrType, int opCode_Or_functCode){
    char *name;
    int i=0;
    switch(instrType){
            case R:
                  for(i=0;i<R_INSTRSPEC_COUNT;i++ ){
                     if (rInstrSpec[i].r_funct == opCode_Or_functCode ){
                        name= rInstrSpec[i].instrName;
                     }
                  }
                  break;
            case I:
                  for(i=0;i<I_INSTRSPEC_COUNT;i++ ){
                     if (iInstrSpec[i].op == opCode_Or_functCode ){
                        name= iInstrSpec[i].instrName;
                     }
                  }
                  break;
            case J:
                  for(i=0;i<J_INSTRSPEC_COUNT;i++ ){
                     if (jInstrSpec[i].op == opCode_Or_functCode ){
                        name= jInstrSpec[i].instrName;
                     }
                  }
                  break;
            case S:
                  for(i=0;i<S_INSTRSPEC_COUNT;i++ ){
                     if (sInstrSpec[i].op == opCode_Or_functCode ){
                        name= sInstrSpec[i].instrName;
                     }
                  }
                  break;
    }

    int len = strlen(name);
    if (len <=0){
        return NULL;
    }

    /**  debugging, using the same reference
    char returnName[len];
    for(i=0;i<len;i++){
        returnName[i]=name[i];
    }
    return returnName;
    **/

    /** avoiding using the reference **/
    char *returnName = malloc(len);
    strcpy(returnName, name);
    return returnName;
}

 /** initial action to be loaded at begin
     include:
              1.load all instruction SPEC
              2.get snapshot , eror_dump, D-memory_dump 's FILE pointer
              3. initialize all arrays like D & I memory
 **/
void initializeAction(){

   # if LOG_ENABLE
      logFp = freopen(LOG_FILE,"wr", stderr);
   #endif

   initInstrSpec();

   snapshotFp = fopen(SNAPSHOT_FILE,"wb");
   if (snapshotFp == NULL){
       fprintf(stderr,"[Error][init] Cannot get file=%s.\n",SNAPSHOT_FILE);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
   }
   errorDumpFp = fopen(ERROR_DUMP_FILE,"wb");
   if (errorDumpFp == NULL){
       fprintf(stderr,"[Error][init] Cannot get file=%s.\n",ERROR_DUMP_FILE);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
   }

   #if D_MEMEMORY_LOG_ENABLE
   dMemoryFp = fopen(DMEMORY_DUMP_FILE,"wb");
   #endif // D_MEMEMORY_LOG_ENABLE

   memset(dataMemory,0,sizeof(dataMemory));   /// init dataMemory
   memset(instrMemory,0,sizeof(instrMemory)); /// init instrMemory
   memset(registers,0, sizeof(registers));  /// init register


   getInitStage(&IF_ID_Buffer);
   getInitStage(&ID_EXE_Buffer);
   getInitStage(&EXE_DM_Buffer);
   getInitStage(&DM_WB_Buffer);
   getInitStage(&WB_Buffer);

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
        instrMemory[(programCounter/4)+i] = iImageFile[i+2];  /// get instruction with index start from 2
    }

    i=0;
    for (i; i<dataCount ;i++){
        dataMemory[i] = dImageFile[i+2];  /// get data value with index sarting from 2
    }
}


/** Because we want to print out the register value before each stage,
    we need to keep the registers' value on each stage before pipeline running.
    Once pipeLine running all the register's value has been updated!!
**/
void copy_RegistersValue_Into_RegistersInitStages(){
    int i=0;
    for(i=0;i<32;i++){
        registers_Init_Stages[i] = registers[i];
    }
}

/** output snapShot for each cycle
     Show Register content form 0-31 **/
void outputSnapshot(){

   fprintf(snapshotFp,"cycle %d\n",cycleCount);
   int i=0;
   for (i=0;i<32;i++){
       fprintf(snapshotFp,"$%02d: 0x%08X\n",i,registers_Init_Stages[i]);
   }
   fprintf(snapshotFp,"PC: 0x%08X\n",programCounter);

   fprintf(snapshotFp,"IF: 0x%08X", IF_ID_Buffer.instruction.binaryCode );
   if ( controlUnit.isStall ){
      fprintf(snapshotFp, " to_be_stalled");
   }else if ( controlUnit.isFlush  ){
      fprintf(snapshotFp, " to_be_flushed");
   }
   fprintf(snapshotFp, "\n");

   fprintf(snapshotFp, "ID: %s",  ID_EXE_Buffer.instruction.name);
   if ( controlUnit.isStall ){
      fprintf(snapshotFp, " to_be_stalled");
   }else if ( forwardingUnit.isForwardRsToID ==true &&  forwardingUnit.isForwardRtToID  == false ){
       fprintf(snapshotFp, " fwd_%s_rs_$%d",  forwardingUnit.fromStage_ForwardRsToID ,  ID_EXE_Buffer.instruction.r_i_rs   );
   }else if( forwardingUnit.isForwardRtToID == true && forwardingUnit.isForwardRsToID  == false  ){
        fprintf(snapshotFp, " fwd_%s_rt_$%d", forwardingUnit.fromStage_ForwardRtToID ,ID_EXE_Buffer.instruction.r_i_rt  );
   }else if( forwardingUnit.isForwardRtToID == true && forwardingUnit.isForwardRsToID  == true  ){
       fprintf(snapshotFp, " fwd_%s_rs_$%d fwd_%s_rt_$%d", forwardingUnit.fromStage_ForwardRsToID   , ID_EXE_Buffer.instruction.r_i_rs  , forwardingUnit.fromStage_ForwardRtToID, ID_EXE_Buffer.instruction.r_i_rt  );
   }
   fprintf(snapshotFp, "\n");


   fprintf(snapshotFp, "EX: %s",  EXE_DM_Buffer.instruction.name );
   if (  forwardingUnit.isForwardRsToEXE == true && forwardingUnit.isForwardRtToEXE  == false  ){
        fprintf(snapshotFp, " fwd_%s_rs_$%d",  forwardingUnit.fromStage_ForwardRsToEXE ,  EXE_DM_Buffer.instruction.r_i_rs );
   }else if( forwardingUnit.isForwardRtToEXE  == true && forwardingUnit.isForwardRsToEXE  == false ){
        fprintf(snapshotFp, " fwd_%s_rt_$%d", forwardingUnit.fromStage_ForwardRtToEXE , EXE_DM_Buffer.instruction.r_i_rt );
   }else if ( forwardingUnit.isForwardRtToEXE  == true && forwardingUnit.isForwardRsToEXE  == true ){
        fprintf(snapshotFp, " fwd_%s_rs_$%d fwd_%s_rt_$%d", forwardingUnit.fromStage_ForwardRsToEXE   , EXE_DM_Buffer.instruction.r_i_rs , forwardingUnit.fromStage_ForwardRtToEXE , EXE_DM_Buffer.instruction.r_i_rt  );
   }
   fprintf(snapshotFp, "\n");


   fprintf(snapshotFp, "DM: %s\n",  DM_WB_Buffer.instruction.name );
   fprintf(snapshotFp, "WB: %s\n\n\n", WB_Buffer.instruction.name );

}

/** error handler to handling error when error happens **/
void errorHandler( ErrorHandlerCode errorCode){
     bool exitNow =false;

     switch (errorCode){
            case   ERROR_WRITE_R_ZERO:
                   fprintf( errorDumpFp , "In cycle %d: Write $0 Error\n", cycleCount+1);
                   break;
            case   ERROR_NUMBER_OVERFLOW:
                   fprintf(errorDumpFp , "In cycle %d: Number Overflow\n", cycleCount+1);
                   break;
            case   ERROR_MEMORY_ADDR_OVERFLOW:
                   fprintf(errorDumpFp , "In cycle %d: Address Overflow\n", cycleCount+1);
                   isHalt = true;
                   break;
            case   ERROR_DATA_MISALIGNED:
                   fprintf(errorDumpFp , "In cycle %d: Misalignment Error\n", cycleCount+1);
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

    controlUnit.isFlush =false;
    controlUnit.isStall=false;
    controlUnit.updatePC = -1;


    forwardingUnit.fromStage_ForwardRsToEXE = NULL;
    forwardingUnit.fromStage_ForwardRsToID = NULL;
    forwardingUnit.fromStage_ForwardRtToEXE = NULL;
    forwardingUnit.fromStage_ForwardRtToID = NULL;

    forwardingUnit.isForwardRsToEXE =false;
    forwardingUnit.isForwardRsToID =false;
    forwardingUnit.isForwardRtToEXE = false;
    forwardingUnit.isForwardRtToID =false;
}

/** Because we need to detect data hazard, however, some instruction which rs, rt or rd is
    unused to the instruction. So we need to avoid the unused rt,rs,rd not to be used
    for judging data hazard, which means we need to make unused rt,rs,rd to be zero!!!
    because when judge data hazard, we skip some data hazard once it will write back to $zero
    register. **/
void avoid_Wrong_DataHazard_By_Unused_Registers(Instruction* instruction){

    switch( (*instruction).instrType ){
        case R:
            if ( (*instruction).r_funct ==R_SLL_FUNCT || (*instruction).r_funct ==R_SRL_FUNCT ||
                (*instruction).r_funct ==R_SRA_FUNCT){
                    /// mark to be zero!! because if register will be updated
                    /// will never to be $zero, otherwise, the data hazard won't
                    /// happen!!!
                        (*instruction).r_i_rs = 0;
            }else if ( (*instruction).r_funct  == R_JR_FUNCT ){
                        (*instruction).r_i_rt = 0;
                        (*instruction).r_rd = 0;
            }
                break;
        case I:
            if ( (*instruction).op == I_LUI_OPCODE ){
                (*instruction).r_i_rs =0;
            }
                break;
    }
}

 /**convert binary code into R,I,J,S instruction's format **/
void convertBinaryToInstr( Instruction* instruction){
    /***
    Instruction *instruction = (Instruction*) malloc( sizeof(Instruction) );
    memset( instruction,0, sizeof(Instruction)); /// debugging before, should sizeOf(Instruction), but not sizeOf(instruction)
    ***/

     int  binaryCode = (*instruction).binaryCode;
     (*instruction).op = GET_OPCode(binaryCode);


     if ( (*instruction).op > S_HALT_OPCODE){
       /// error case handler when opcode > 0x3f
       fprintf(stderr,"[Error][convertBinaryToInstr] opCode=0x%02X, Cannot find match opCode > S_HALT_OPCODE:(0x%02X) \n",(*instruction).op ,S_HALT_OPCODE);
       fflush(stderr);
       errorHandler(ERROR_EXIT_NOW);
    }else if ( (*instruction).op == S_HALT_OPCODE ){
        (*instruction).instrType =S;
        (*instruction).j_s_addr =GET_J_S_Address(binaryCode);
        (*instruction).name =  getInstrName(S, (*instruction).op );
    }else if ( (*instruction).op == R_OPCODE){
        (*instruction).instrType = R;
        (*instruction).r_i_rs = GET_R_I_RsCode(binaryCode);
        (*instruction).r_i_rt = GET_R_I_RtCode(binaryCode);
        (*instruction).r_rd = GET_R_RdCode(binaryCode);
        (*instruction).r_shamt = GET_R_ShamtCode(binaryCode);
        (*instruction).r_funct = GET_R_FunctCode(binaryCode);
        (*instruction).name =  getInstrName(R, (*instruction).r_funct );
        avoid_Wrong_DataHazard_By_Unused_Registers(instruction);

    }else if ( (*instruction).op == J_J_OPCODE ||  (*instruction).op == J_JAL_OPCODE){
        (*instruction).instrType =J;
        (*instruction).j_s_addr =GET_J_S_Address(binaryCode);
        (*instruction).name =  getInstrName(J, (*instruction).op );

    }else{
        /// I instruction
        (*instruction).instrType =I;
        (*instruction).r_i_rs = GET_R_I_RsCode(binaryCode);
        (*instruction).r_i_rt = GET_R_I_RtCode(binaryCode);
        (*instruction).i_immediate =   GET_I_Immediate(binaryCode);
        (*instruction).name =  getInstrName(I, (*instruction).op );
         avoid_Wrong_DataHazard_By_Unused_Registers(instruction);
    }
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
void checkMemoryAccessError(int howManyBytes, Instruction *instruction , int memoryAddress ){

    /** check Number overflow first, the order follows the Error Detection SPEC **/
    /**  Should be marked !!! because in pipeline the DM will not check  number overflow which is
         in EXE stage.

    int rsSign = GET_Sign_Bit(  registers[(*instruction).r_i_rs]  );
    int immSign =  GET_Sign_Bit(  (*instruction).i_immediate );
    int memoryAddress = registers[ (*instruction).r_i_rs ] + (*instruction).i_immediate ;

    if (rsSign == immSign){
        int memAddrSign = GET_Sign_Bit( memoryAddress );
        if (memAddrSign != immSign ){
            errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when signs are not the same.
        }
    }

    **/


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


/** load 2 bytes form memory **/
short loadShortFromMemory(int memoryAddress){
    short value= 0;
    int dataMemoryIndex = memoryAddress /4;

    int index = memoryAddress % 4;

    switch (index){
        case 0:
            /// fisrt 2 bytes
            value = (short) ((dataMemory[dataMemoryIndex] & 0xffff0000) >> 16);
            break;
        case 2:
            /// the last 2 bytes=> memoryAddress % 2 == 0
            value = (short) (dataMemory[dataMemoryIndex] & 0xffff);
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
             value = (char) ((dataMemory[dataMemoryIndex] & 0xff000000) >>24);
             break;;
        case 1:
            value = (char) ((dataMemory[dataMemoryIndex] & 0x00ff0000) >> 16);
              break;
        case 2:
            value = (char) ((dataMemory[dataMemoryIndex] & 0x0000ff00) >>8);
              break;
        case 3:
             value = (char) (dataMemory[dataMemoryIndex] & 0x000000ff) ;
              break;
    }
}

/** save 2 bytes into memory **/
void saveShortIntoMemory(int memoryAddress, int rtIndex ){

     int dataMemoryIndex = memoryAddress /4;
     int index = memoryAddress % 4;
     int last16bits = registers[rtIndex] & 0xffff;


     switch (index){
        case 0:
            /// fisrt 2 bytes
            dataMemory[dataMemoryIndex]  = (last16bits << 16) | (dataMemory[dataMemoryIndex] & 0xffff)  ;
            break;
        case 2:
            ///the last 2 bytes=> memoryAddress % 2 == 0
            dataMemory[dataMemoryIndex]  = (dataMemory[dataMemoryIndex] & 0xffff0000) | last16bits;
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
             dataMemory[dataMemoryIndex] = (dataMemory[dataMemoryIndex] & 0x00ffffff ) | (last8Bits <<  24);
             break;;
        case 1:
             dataMemory[dataMemoryIndex] = (dataMemory[dataMemoryIndex] & 0xff00ffff ) | (last8Bits <<  16);
              break;
        case 2:
             dataMemory[dataMemoryIndex] = (dataMemory[dataMemoryIndex] & 0xffff00ff ) | (last8Bits <<  8);
              break;
        case 3:
             dataMemory[dataMemoryIndex] = (dataMemory[dataMemoryIndex] & 0xffffff00 ) | last8Bits;
              break;
    }

}

/** doing WB stage job**/
void WB(){
   PipelineBuffer tmp_WB_Buffer = DM_WB_Buffer;

   switch (tmp_WB_Buffer.instruction.instrType){
       case R:
             if ( tmp_WB_Buffer.instruction.r_funct == R_ADD_FUNCT ||  tmp_WB_Buffer.instruction.r_funct == R_SUB_FUNCT ||  tmp_WB_Buffer.instruction.r_funct == R_AND_FUNCT ||
                tmp_WB_Buffer.instruction.r_funct == R_OR_FUNCT ||  tmp_WB_Buffer.instruction.r_funct == R_XOR_FUNCT ||  tmp_WB_Buffer.instruction.r_funct == R_NOR_FUNCT ||
                tmp_WB_Buffer.instruction.r_funct == R_NAND_FUNCT ||  tmp_WB_Buffer.instruction.r_funct == R_SLT_FUNCT ||
                tmp_WB_Buffer.instruction.r_funct == R_SRL_FUNCT ||  tmp_WB_Buffer.instruction.r_funct == R_SRA_FUNCT || tmp_WB_Buffer.instruction.r_funct == R_SLL_FUNCT
                 ){

                 if ( tmp_WB_Buffer.instruction.r_rd == R_ZERO ){
                     if (  tmp_WB_Buffer.instruction.r_funct == R_SLL_FUNCT   ){
                          if ( tmp_WB_Buffer.instruction.r_i_rt != R_ZERO ||  tmp_WB_Buffer.instruction.r_shamt !=0 ){
                              /** For specifal case. If case is sll $0, $0, $0 , we just run it normally.  **/
                              errorHandler(ERROR_WRITE_R_ZERO);
                          }
                     }else {
                            /** cannot return , because one instruction may cause several errors.**/
                            /// return; /// do nothing when writing register $zero
                            errorHandler(ERROR_WRITE_R_ZERO);
                      }
                 }else{
                       registers[ tmp_WB_Buffer.instruction.r_rd ] = tmp_WB_Buffer.result;
                 }

             }else{
                /// JR don't write back to register!!!
                /// donothing.
             }
             break;
       case I:

             if (  tmp_WB_Buffer.instruction.op == I_ADDI_OPCODE ||  tmp_WB_Buffer.instruction.op == I_LW_OPCODE ||  tmp_WB_Buffer.instruction.op == I_LH_OPCODE ||
                  tmp_WB_Buffer.instruction.op == I_LHU_OPCODE ||  tmp_WB_Buffer.instruction.op == I_LB_OPCODE ||  tmp_WB_Buffer.instruction.op == I_LBU_OPCODE ||
                  tmp_WB_Buffer.instruction.op == I_LUI_OPCODE ||  tmp_WB_Buffer.instruction.op == I_ANDI_OPCODE ||  tmp_WB_Buffer.instruction.op == I_ORI_OPCODE ||
                  tmp_WB_Buffer.instruction.op == I_NORI_OPCODE ||  tmp_WB_Buffer.instruction.op == I_SLTI_OPCODE ){

                   if (  tmp_WB_Buffer.instruction.r_i_rt ==R_ZERO ){
                        errorHandler(ERROR_WRITE_R_ZERO);
                   }else{
                       registers[ tmp_WB_Buffer.instruction.r_i_rt ] = tmp_WB_Buffer.result;
                   }

             }else{
                ///   I_SW_OPCODE    I_SH_OPCODE   I_SB_OPCODE   I_BEQ_OPCODE  I_BNE_OPCODE
                /// donothing.
             }

             break;
       case J:
             if (  tmp_WB_Buffer.instruction.op ==  J_JAL_OPCODE){
                   registers[R_RA] = tmp_WB_Buffer.result;
             }
             break;
       case S:
            /** special case, because spec ask to halt once five "halt" instruction fill all pipeline stage **/
            /**
             case S_HALT_OPCODE:
                  isHalt = true;
             **/
             break;
   }

    WB_Buffer = tmp_WB_Buffer;
}


/** doing DM stage job**/
void DM(){

   PipelineBuffer tmp_DM_WB_Buffer = EXE_DM_Buffer;

   switch (tmp_DM_WB_Buffer.instruction.op){
         case I_LW_OPCODE:
                checkMemoryAccessError(4, (&tmp_DM_WB_Buffer.instruction) , tmp_DM_WB_Buffer.result  );
                if (isHalt) {
                    ///return;
                    break;
                } /** key avoids to make the program shut down by accessing wrong data **/
                tmp_DM_WB_Buffer.result = dataMemory[ (tmp_DM_WB_Buffer.result/4 ) ];
                break;
         case I_LH_OPCODE:
                checkMemoryAccessError(2 , (&tmp_DM_WB_Buffer.instruction) , tmp_DM_WB_Buffer.result );
                if (isHalt) {
                    ///return;
                    break;
                } /** key avoids to make the program shut down by accessing wrong data **/
                tmp_DM_WB_Buffer.result = loadShortFromMemory(tmp_DM_WB_Buffer.result );
                break;
         case I_LHU_OPCODE:
                checkMemoryAccessError(2, (&tmp_DM_WB_Buffer.instruction) , tmp_DM_WB_Buffer.result );
                if (isHalt) {
                    ///return;
                    break;
                } /** key avoids to make the program shut down by accessing wrong data **/
                tmp_DM_WB_Buffer.result = 0x0000ffff & loadShortFromMemory(tmp_DM_WB_Buffer.result );
                break;
         case I_LB_OPCODE:
                checkMemoryAccessError(1, (&tmp_DM_WB_Buffer.instruction) , tmp_DM_WB_Buffer.result );
                if (isHalt) {
                    ///return;
                    break;
                } /** key avoids to make the program shut down by accessing wrong data **/
                tmp_DM_WB_Buffer.result =  loadByteFromMemory(tmp_DM_WB_Buffer.result );
                break;
         case I_LBU_OPCODE:
                checkMemoryAccessError(1 , (&tmp_DM_WB_Buffer.instruction) , tmp_DM_WB_Buffer.result );
                if (isHalt) {
                    ///return;
                    break;
                } /** key avoids to make the program shut down by accessing wrong data **/
                 tmp_DM_WB_Buffer.result = 0x000000ff & loadByteFromMemory( tmp_DM_WB_Buffer.result );
                break;
         case I_SW_OPCODE:
                checkMemoryAccessError(4 , (&tmp_DM_WB_Buffer.instruction) , tmp_DM_WB_Buffer.result );
                if (isHalt) {
                    ///return;
                    break;
                } /** key avoids to make the program shut down by accessing wrong data **/
                dataMemory[ (tmp_DM_WB_Buffer.result /4) ] = registers[ (tmp_DM_WB_Buffer.instruction.r_i_rt )   ];
                break;

         case I_SH_OPCODE:
                checkMemoryAccessError(2 , (&tmp_DM_WB_Buffer.instruction) , tmp_DM_WB_Buffer.result );
                if (isHalt) {
                    ///return;
                    break;
                } /** key avoids to make the program shut down by accessing wrong data **/
                /*** from register= 0x01020304 to  memory= 0x05060708
                                 new memory= 0x03040708  ***/
                saveShortIntoMemory(tmp_DM_WB_Buffer.result   , tmp_DM_WB_Buffer.instruction.r_i_rt );
                break;

         case I_SB_OPCODE:
                checkMemoryAccessError(1 , (&tmp_DM_WB_Buffer.instruction) , tmp_DM_WB_Buffer.result );
                if (isHalt) {
                    ///return;
                    break;
                } /** key avoids to make the program shut down by accessing wrong data **/
                /*** from register= 0x01020304 to  memory= 0x05060708
                                 new memory= 0x04060708  ***/
                saveByteIntoMemory( tmp_DM_WB_Buffer.result  , tmp_DM_WB_Buffer.instruction.r_i_rt );
                break;
         default:
                ///return;
                break;
   }
   DM_WB_Buffer =  tmp_DM_WB_Buffer;

}

/** detect data hazard in EXE stage **/
void dataHazardDetection_For_EXE(PipelineBuffer *tmp_EXE_DM_Buffer, int *rs_Value , int *rt_Value ){

        switch ( (*tmp_EXE_DM_Buffer).instruction.instrType ){
            case R:
                     switch(WB_Buffer.instruction.instrType ){
                        case R:
                               if ( WB_Buffer.instruction.r_rd !=0  &&  WB_Buffer.instruction.r_rd == (*tmp_EXE_DM_Buffer).instruction.r_i_rs ){
                                    (*rs_Value) = WB_Buffer.result; /// forwarding new rs value
                                    forwardingUnit.isForwardRsToEXE =true;
                                    forwardingUnit.fromStage_ForwardRsToEXE = DM_WB_STAGE;
                               }
                               if ( WB_Buffer.instruction.r_rd !=0  &&  WB_Buffer.instruction.r_rd == (*tmp_EXE_DM_Buffer).instruction.r_i_rt ){
                                    (*rt_Value) = WB_Buffer.result;
                                    forwardingUnit.isForwardRtToEXE =true;
                                    forwardingUnit.fromStage_ForwardRtToEXE = DM_WB_STAGE;
                               }
                               break;
                        case I:
                              if ( WB_Buffer.instruction.op != I_SW_OPCODE && WB_Buffer.instruction.op != I_SH_OPCODE &&
                                   WB_Buffer.instruction.op != I_SB_OPCODE && WB_Buffer.instruction.op != I_BEQ_OPCODE &&
                                   WB_Buffer.instruction.op != I_BNE_OPCODE ){

                                    if (WB_Buffer.instruction.r_i_rt !=0 &&  WB_Buffer.instruction.r_i_rt == (*tmp_EXE_DM_Buffer).instruction.r_i_rs  ){
                                        (*rs_Value) = WB_Buffer.result;
                                        forwardingUnit.isForwardRsToEXE =true;
                                        forwardingUnit.fromStage_ForwardRsToEXE = DM_WB_STAGE;
                                    }

                                    if (WB_Buffer.instruction.r_i_rt !=0 &&  WB_Buffer.instruction.r_i_rt == (*tmp_EXE_DM_Buffer).instruction.r_i_rt  ){
                                         (*rt_Value) = WB_Buffer.result;
                                        forwardingUnit.isForwardRtToEXE =true;
                                        forwardingUnit.fromStage_ForwardRtToEXE = DM_WB_STAGE;
                                    }
                                }
                               break;
                        case J:
                                  /*** JAL --- register:31 ra
                                 ex1: jal  0x001
                                      add  $x , $31, $x  => addi $rd, $rs, $rt

                                   => jal  0x001
                                      nop
                                      add  $x , $31, $x  (forwarding $31)
                                  **/
                                if( WB_Buffer.instruction.op  == J_JAL_OPCODE ){
                                            if ( (*tmp_EXE_DM_Buffer).instruction.r_i_rs == R_RA){
                                                   (*rs_Value) = WB_Buffer.result;
                                                   forwardingUnit.fromStage_ForwardRsToEXE = DM_WB_STAGE;
                                                   forwardingUnit.isForwardRsToEXE =true;
                                            }
                                            if ( (*tmp_EXE_DM_Buffer).instruction.r_i_rt == R_RA){
                                                   (*rt_Value) = WB_Buffer.result;
                                                   forwardingUnit.fromStage_ForwardRtToEXE = DM_WB_STAGE;
                                                   forwardingUnit.isForwardRtToEXE =true;
                                            }
                                }

                               break;
                        case S:
                               break;

                     }

                     switch(EXE_DM_Buffer.instruction.instrType ){
                        case R:
                               if ( EXE_DM_Buffer.instruction.r_rd !=0  &&  EXE_DM_Buffer.instruction.r_rd == (*tmp_EXE_DM_Buffer).instruction.r_i_rs ){
                                    (*rs_Value) = EXE_DM_Buffer.result;
                                    forwardingUnit.isForwardRsToEXE =true;
                                    forwardingUnit.fromStage_ForwardRsToEXE = EX_DM_STAGE;
                               }
                               if ( EXE_DM_Buffer.instruction.r_rd  !=0  &&  EXE_DM_Buffer.instruction.r_rd == (*tmp_EXE_DM_Buffer).instruction.r_i_rt ){
                                    (*rt_Value) = EXE_DM_Buffer.result;
                                    forwardingUnit.isForwardRtToEXE =true;
                                    forwardingUnit.fromStage_ForwardRtToEXE = EX_DM_STAGE;
                               }
                               break;
                        case I:
                              /** because this instruction need to be calculated in EXE stage , so we need to get new values in register.
                                  And other I type instruction don't need to be calculated in EXE stage for rs & rt register.**/
                              if ( EXE_DM_Buffer.instruction.op == I_ADDI_OPCODE || EXE_DM_Buffer.instruction.op == I_LUI_OPCODE ||
                                   EXE_DM_Buffer.instruction.op == I_ANDI_OPCODE || EXE_DM_Buffer.instruction.op == I_ORI_OPCODE ||
                                   EXE_DM_Buffer.instruction.op == I_NORI_OPCODE || EXE_DM_Buffer.instruction.op == I_SLTI_OPCODE  ){

                                    if (EXE_DM_Buffer.instruction.r_i_rt  !=0 &&  EXE_DM_Buffer.instruction.r_i_rt == (*tmp_EXE_DM_Buffer).instruction.r_i_rs  ){
                                        (*rs_Value) = EXE_DM_Buffer.result;
                                        forwardingUnit.isForwardRsToEXE =true;
                                        forwardingUnit.fromStage_ForwardRsToEXE = EX_DM_STAGE;
                                    }

                                    if (EXE_DM_Buffer.instruction.r_i_rt !=0 &&  EXE_DM_Buffer.instruction.r_i_rt == (*tmp_EXE_DM_Buffer).instruction.r_i_rt  ){
                                         (*rt_Value) = EXE_DM_Buffer.result;
                                         forwardingUnit.isForwardRtToEXE =true;
                                         forwardingUnit.fromStage_ForwardRtToEXE = EX_DM_STAGE;
                                    }
                                }
                               break;
                        case J:
                               /** because when JAL , there will insert one nop instruction!!
                                  this is why we don't take care of R_RA forwarding for JAL **/
                               break;
                        case S:
                               break;

                     }
                  break;
            case I:
                        switch( WB_Buffer.instruction.instrType ){
                             case R:
                                   if ( WB_Buffer.instruction.r_rd !=0  &&  WB_Buffer.instruction.r_rd == (*tmp_EXE_DM_Buffer).instruction.r_i_rs ){
                                        (*rs_Value) = WB_Buffer.result;
                                        forwardingUnit.isForwardRsToEXE =true;
                                        forwardingUnit.fromStage_ForwardRsToEXE = DM_WB_STAGE;
                                    }
                                    /** I type 裡的rt，如果有data hazard 要偵測，並記錄forwarding.
                                        然而在 sw, sb, sh 在exe stage, 我們並不會真的計算他。
                                        而是等到 DM stage 去memory load rt 的值。
                                        且 I type 裡，RT 的值 ， 會被正確 讀到 。
                                        ex:  add $s1, $s2, $s3      IF  ID  EXE  MEM  [WB]       前半部寫入register
                                             sw  $s1, 0($t0)            IF  ID   EXE  [MEM]  WB  後半部才讀入 register
                                    **/
                                    if( (*tmp_EXE_DM_Buffer).instruction.op == I_SW_OPCODE || (*tmp_EXE_DM_Buffer).instruction.op == I_SH_OPCODE || (*tmp_EXE_DM_Buffer).instruction.op == I_SB_OPCODE ){
                                        if ( WB_Buffer.instruction.r_rd !=0  &&  WB_Buffer.instruction.r_rd == (*tmp_EXE_DM_Buffer).instruction.r_i_rt ){
                                            (*rt_Value) = WB_Buffer.result;
                                            forwardingUnit.isForwardRtToEXE = true;
                                            forwardingUnit.fromStage_ForwardRtToEXE = DM_WB_STAGE;
                                        }
                                    }
                                   break;
                             case I:
                                   if ( WB_Buffer.instruction.op != I_SW_OPCODE && WB_Buffer.instruction.op != I_SH_OPCODE && WB_Buffer.instruction.op != I_SB_OPCODE &&
                                        WB_Buffer.instruction.op !=  I_BEQ_OPCODE && WB_Buffer.instruction.op != I_BNE_OPCODE ){

                                        if ( WB_Buffer.instruction.r_i_rt !=0  &&  WB_Buffer.instruction.r_i_rt == (*tmp_EXE_DM_Buffer).instruction.r_i_rs ){
                                            (*rs_Value) = WB_Buffer.result;
                                            forwardingUnit.isForwardRsToEXE =true;
                                            forwardingUnit.fromStage_ForwardRsToEXE = DM_WB_STAGE;
                                        }

                                       /** I type 裡的rt，如果有data hazard 要偵測，並記錄forwarding.
                                        然而在 sw, sb, sh 在exe stage, 我們並不會真的計算他。
                                        而是等到 DM stage 去memory load rt 的值。
                                        且 I type 裡，RT 的值 ， 會被正確 讀到 。
                                        ex:  add $s1, $s2, $s3      IF  ID  EXE  MEM  [WB]       前半部寫入register
                                             sw  $s1, 0($t0)            IF  ID   EXE  [MEM]  WB  後半部才讀入 register
                                       **/
                                        if ( (*tmp_EXE_DM_Buffer).instruction.op ==I_SW_OPCODE || (*tmp_EXE_DM_Buffer).instruction.op ==I_SH_OPCODE || (*tmp_EXE_DM_Buffer).instruction.op ==I_SB_OPCODE  ){
                                             if ( WB_Buffer.instruction.r_i_rt !=0  &&  WB_Buffer.instruction.r_i_rt == (*tmp_EXE_DM_Buffer).instruction.r_i_rt ){
                                                  (*rt_Value) = WB_Buffer.result;
                                                  forwardingUnit.isForwardRtToEXE = true;
                                                  forwardingUnit.fromStage_ForwardRtToEXE = DM_WB_STAGE;
                                             }
                                        }
                                   }
                                   break;
                             case J:
                                    /*** JAL --- register:31 ra
                                 ex1: jal  0x001
                                      addi $x , $31, 112  => addi $rt, $rs, $C

                                   => jal  0x001
                                      nop
                                      addi $1 , $31, 112  (forwarding $31)

                                ex2: jal  0x001
                                     sw  $31, 112($X)   =>   sw $rt, C($rs)

                                   => jal  0x001
                                      nop
                                      sw $31 , 112($X)  (forwarding $31)
                               ***/
                               if( WB_Buffer.instruction.op  == J_JAL_OPCODE  ){
                                    if ( (*tmp_EXE_DM_Buffer).instruction.r_i_rs == R_RA ){
                                                     (*rs_Value) =  WB_Buffer.result;
                                                     forwardingUnit.isForwardRsToEXE =true;
                                                     forwardingUnit.fromStage_ForwardRsToEXE = DM_WB_STAGE;
                                    }

                                    if ( (*tmp_EXE_DM_Buffer).instruction.op  == I_SW_OPCODE ||  (*tmp_EXE_DM_Buffer).instruction.op  == I_SH_OPCODE ||
                                          (*tmp_EXE_DM_Buffer).instruction.op  == I_SB_OPCODE){
                                            if ( (*tmp_EXE_DM_Buffer).instruction.r_i_rt == R_RA ){
                                                       ( *rt_Value) =  WB_Buffer.result;
                                                       forwardingUnit.isForwardRtToEXE = true;
                                                       forwardingUnit.fromStage_ForwardRtToEXE = DM_WB_STAGE;
                                                }
                                    }
                               }
                                   break;
                             case S:
                                   break;
                        }

                        switch( EXE_DM_Buffer.instruction.instrType ){
                             case R:
                                    if ( EXE_DM_Buffer.instruction.r_rd !=0  &&  EXE_DM_Buffer.instruction.r_rd == (*tmp_EXE_DM_Buffer).instruction.r_i_rs ){
                                        (*rs_Value) = EXE_DM_Buffer.result;
                                        forwardingUnit.isForwardRsToEXE =true;
                                        forwardingUnit.fromStage_ForwardRsToEXE = EX_DM_STAGE;
                                    }
                                    /** I type 裡的rt，如果有data hazard 要偵測，並記錄forwarding.
                                        然而在 sw, sb, sh 在exe stage, 我們並不會真的計算他。
                                        而是等到 DM stage 去memory load rt 的值。
                                        且 I type 裡，RT 的值 ， 會被正確 讀到 。
                                        ex:  add $s1, $s2, $s3      IF  ID  EXE  MEM  [WB]       前半部寫入register
                                             sw  $s1, 0($t0)            IF  ID   EXE  [MEM]  WB  後半部才讀入 register
                                    **/
                                    if( (*tmp_EXE_DM_Buffer).instruction.op == I_SW_OPCODE || (*tmp_EXE_DM_Buffer).instruction.op == I_SH_OPCODE || (*tmp_EXE_DM_Buffer).instruction.op == I_SB_OPCODE ){
                                        if ( EXE_DM_Buffer.instruction.r_rd !=0  &&  EXE_DM_Buffer.instruction.r_rd == (*tmp_EXE_DM_Buffer).instruction.r_i_rt ){
                                            (*rt_Value) = EXE_DM_Buffer.result;
                                            forwardingUnit.isForwardRtToEXE = true;
                                            forwardingUnit.fromStage_ForwardRtToEXE = EX_DM_STAGE;
                                        }
                                    }
                                   break;
                             case I:
                                   if ( EXE_DM_Buffer.instruction.op != I_SW_OPCODE && EXE_DM_Buffer.instruction.op != I_SH_OPCODE && EXE_DM_Buffer.instruction.op != I_SB_OPCODE &&
                                        EXE_DM_Buffer.instruction.op !=  I_BEQ_OPCODE && EXE_DM_Buffer.instruction.op != I_BNE_OPCODE ){

                                        if ( EXE_DM_Buffer.instruction.r_i_rt !=0  &&  EXE_DM_Buffer.instruction.r_i_rt == (*tmp_EXE_DM_Buffer).instruction.r_i_rs ){
                                            (*rs_Value) = EXE_DM_Buffer.result;
                                            forwardingUnit.isForwardRsToEXE =true;
                                            forwardingUnit.fromStage_ForwardRsToEXE = EX_DM_STAGE;
                                        }

                                        /** I type 裡的rt，如果有data hazard 要偵測，並記錄forwarding.
                                        然而在 sw, sb, sh 在exe stage, 我們並不會真的計算他。
                                        而是等到 DM stage 去memory load rt 的值。
                                        且 I type 裡，RT 的值 ， 會被正確 讀到 。
                                        ex:  add $s1, $s2, $s3      IF  ID  EXE  MEM  [WB]       前半部寫入register
                                             sw  $s1, 0($t0)            IF  ID   EXE  [MEM]  WB  後半部才讀入 register
                                       **/
                                        if ((*tmp_EXE_DM_Buffer).instruction.op ==I_SW_OPCODE || (*tmp_EXE_DM_Buffer).instruction.op ==I_SH_OPCODE || (*tmp_EXE_DM_Buffer).instruction.op ==I_SB_OPCODE  ){
                                             if ( EXE_DM_Buffer.instruction.r_i_rt !=0  &&  EXE_DM_Buffer.instruction.r_i_rt == (*tmp_EXE_DM_Buffer).instruction.r_i_rt ){
                                                  (*rt_Value) = EXE_DM_Buffer.result;
                                                  forwardingUnit.isForwardRtToEXE = true;
                                                  forwardingUnit.fromStage_ForwardRtToEXE = EX_DM_STAGE;
                                             }
                                        }
                                   }
                                   break;
                             case J: /** because when JAL , there will insert one nop instruction!!
                                         this is why we don't take care of R_RA forwarding for JAL **/
                                   break;
                             case S:
                                   break;
                        }
        }
}

/** doing EXE stage job**/
void EXE(){
    PipelineBuffer tmp_EXE_DM_Buffer  = ID_EXE_Buffer;

    int rs_Value =0;
    int rt_Value =0;

    if ( tmp_EXE_DM_Buffer.instruction.r_funct == R_JR_FUNCT  || tmp_EXE_DM_Buffer.instruction.binaryCode == 0 ||
        tmp_EXE_DM_Buffer.instruction.op == J_J_OPCODE ||  tmp_EXE_DM_Buffer.instruction.op == J_JAL_OPCODE ||
        tmp_EXE_DM_Buffer.instruction.op ==   I_BEQ_OPCODE || tmp_EXE_DM_Buffer.instruction.op == I_BNE_OPCODE  ) {

    }else{

        int rsSign=0,rtSign=0, rdSign =0, immSign=0;
        switch ( tmp_EXE_DM_Buffer.instruction.instrType ){
            case R:
                     /// detect data hazard !!
                     dataHazardDetection_For_EXE(&tmp_EXE_DM_Buffer,&rs_Value , &rt_Value );
                     /// do ALU operation
                      if ( forwardingUnit.isForwardRsToEXE == false){
                           rs_Value = registers[ tmp_EXE_DM_Buffer.instruction.r_i_rs  ];
                      }

                      if ( forwardingUnit.isForwardRtToEXE == false){
                           rt_Value = registers[ tmp_EXE_DM_Buffer.instruction.r_i_rt  ];
                      }

                      switch (tmp_EXE_DM_Buffer.instruction.r_funct ){
                           case R_ADD_FUNCT:
                                            rsSign = GET_Sign_Bit( rs_Value   );
                                            rtSign =  GET_Sign_Bit(  rt_Value  );
                                            tmp_EXE_DM_Buffer.result = rs_Value  +  rt_Value;

                                            if (rsSign == rtSign){
                                                rdSign = GET_Sign_Bit(  tmp_EXE_DM_Buffer.result  );
                                                if (rtSign != rdSign ){
                                                    errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when signs are not the same.
                                                }
                                            }
                                            break;
                           case R_SUB_FUNCT:

                                            rsSign = GET_Sign_Bit( rs_Value   );
                                            rtSign =  GET_Sign_Bit(  rt_Value  );
                                            int substractedNumber =  rt_Value ;
                                            int value = 0;

                                            if (substractedNumber== 0x80000000){
                                                    /// for special case
                                                    /// 0x00000004 - 0x80000000 = 0x80000004
                                                    /// b=0x80000000是負數
                                                    /// -b=0x80000000還是負數
                                                    /// 0x00000004-0x80000000是正加負，不會有number overflow

                                                    value =  rs_Value + rt_Value ;
                                                    rdSign = GET_Sign_Bit( value );
                                                    tmp_EXE_DM_Buffer.result = value;

                                                    if (rsSign == rtSign){
                                                        if (rtSign != rdSign ){
                                                            errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when signs are not the same.
                                                        }
                                                    }
                                            }else{
                                                    value = rs_Value - rt_Value ;
                                                    rdSign = GET_Sign_Bit( value );
                                                    tmp_EXE_DM_Buffer.result = value;

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
                                            tmp_EXE_DM_Buffer.result = rs_Value  & rt_Value;
                                            break;
                           case R_OR_FUNCT:
                                            tmp_EXE_DM_Buffer.result = rs_Value  | rt_Value;
                                            break;
                           case R_XOR_FUNCT:
                                            tmp_EXE_DM_Buffer.result = rs_Value  ^ rt_Value;
                                            break;
                           case R_NOR_FUNCT:
                                            tmp_EXE_DM_Buffer.result = ~( rs_Value  | rt_Value );
                                            break;
                           case R_NAND_FUNCT:
                                            tmp_EXE_DM_Buffer.result = ~( rs_Value  & rt_Value );
                                            break;
                           case R_SLT_FUNCT:
                                            tmp_EXE_DM_Buffer.result = ( rs_Value  < rt_Value );
                                            break;
                           case R_SLL_FUNCT:
                                            tmp_EXE_DM_Buffer.result = ( rt_Value  << tmp_EXE_DM_Buffer.instruction.r_shamt );
                                            break;
                           case R_SRL_FUNCT:
                                             /**  Shit right logic
                                                  0xFF020304 >> 8 => 0x00FF0203   **/
                                            tmp_EXE_DM_Buffer.result = ( (unsigned int) rt_Value)  >> tmp_EXE_DM_Buffer.instruction.r_shamt ;
                                            break;
                           case R_SRA_FUNCT:
                                             /**  Shit right arithmetic
                                                  0x81020304 >> 8 => 0xff810203   **/
                                            tmp_EXE_DM_Buffer.result = (  rt_Value)  >> tmp_EXE_DM_Buffer.instruction.r_shamt ;
                                            break;
                      }

                  break;

            case I:
                        /// detect data hazard !!
                        dataHazardDetection_For_EXE(&tmp_EXE_DM_Buffer,&rs_Value , &rt_Value );

                        /// do ALU operation

                            int memoryAddress=0,dataMemoryIndex=0;
                            int oldPCSign=0, newPCSign=0, newProgramCounter=0;

                            if ( forwardingUnit.isForwardRsToEXE == false  ){
                                    rs_Value = registers[tmp_EXE_DM_Buffer.instruction.r_i_rs ];
                            }

                            /**  在exe stage, 我們並不會去用 Rt 的值去計算。
                            if ( forwardingUnit.isForwardRtToEXE == false  ){
                                    rt_Value = registers[tmp_EXE_DM_Buffer.instruction.r_i_rt ];
                            }**/



                            switch(tmp_EXE_DM_Buffer.instruction.op ){
                                    case I_ADDI_OPCODE:

                                        rsSign = GET_Sign_Bit( rs_Value  );
                                        immSign =  GET_Sign_Bit(  tmp_EXE_DM_Buffer.instruction.i_immediate  );
                                        tmp_EXE_DM_Buffer.result =  rs_Value + tmp_EXE_DM_Buffer.instruction.i_immediate  ;

                                        if (rsSign == immSign){
                                            rtSign = GET_Sign_Bit( tmp_EXE_DM_Buffer.result );
                                            if (rtSign != immSign ){
                                                errorHandler(ERROR_NUMBER_OVERFLOW); /// overflow when signs are not the same.
                                            }
                                        }
                                        break;

                                    case I_LW_OPCODE:
                                    case I_LH_OPCODE:
                                    case I_LHU_OPCODE:
                                    case I_LB_OPCODE:
                                    case I_LBU_OPCODE:
                                    case I_SW_OPCODE:
                                    case I_SH_OPCODE:
                                    case I_SB_OPCODE:
                                        rsSign = GET_Sign_Bit(  rs_Value  );
                                        immSign =  GET_Sign_Bit(  tmp_EXE_DM_Buffer.instruction.i_immediate );
                                        tmp_EXE_DM_Buffer.result =  rs_Value  + tmp_EXE_DM_Buffer.instruction.i_immediate ;

                                        if (rsSign == immSign){
                                            int memAddrSign = GET_Sign_Bit( tmp_EXE_DM_Buffer.result );
                                            if (memAddrSign != immSign ){
                                                errorHandler(ERROR_NUMBER_OVERFLOW); /** overflow when signs are not the same. **/
                                            }
                                        }
                                        break;
                                    case I_LUI_OPCODE:
                                        /** put immediate in first 16 bits, and clear last 16 bits **/
                                        tmp_EXE_DM_Buffer.result = tmp_EXE_DM_Buffer.instruction.i_immediate << 16 & 0xffff0000;
                                        break;
                                    case I_ANDI_OPCODE:
                                        /** debugging before immediate is 16 bit needs to and with 0xffff , then to do logic operation**/
                                        tmp_EXE_DM_Buffer.result  =  rs_Value  &  (  tmp_EXE_DM_Buffer.instruction.i_immediate & 0xffff ) ;
                                        break;
                                    case I_ORI_OPCODE:
                                        tmp_EXE_DM_Buffer.result  = rs_Value |  ( tmp_EXE_DM_Buffer.instruction.i_immediate & 0xffff ) ;
                                        break;
                                    case I_NORI_OPCODE:
                                        tmp_EXE_DM_Buffer.result  = ~( rs_Value |  ( tmp_EXE_DM_Buffer.instruction.i_immediate & 0xffff )) ;
                                        break;
                                    case I_SLTI_OPCODE:
                                        tmp_EXE_DM_Buffer.result  = rs_Value  < tmp_EXE_DM_Buffer.instruction.i_immediate ;
                                        break;
                            }
                  break;
            case J:
                    break;
            case S:
                    break;
        }
    }

    EXE_DM_Buffer = tmp_EXE_DM_Buffer;

}



/** detect load-use or data hazard which will make stage stall  in ID stage **/
void loadUseDataHazardDetection_For_ID(PipelineBuffer *tmp_ID_EXE_Buffer  ){
     /**
     if (cycleCount ==27){
        int a=1;
     }**/

     /// stall for load-use data hazard  & data hazard which must solve by stall like jr & beq
     switch ( (*tmp_ID_EXE_Buffer).instruction.instrType  ){
      case R:
             switch( DM_WB_Buffer.instruction.instrType  ){
                  case I:
                      if ( (*tmp_ID_EXE_Buffer).instruction.r_funct == R_JR_FUNCT ){

                           if ( DM_WB_Buffer.instruction.op == I_LW_OPCODE ||  DM_WB_Buffer.instruction.op == I_LH_OPCODE ||
                                       DM_WB_Buffer.instruction.op == I_LHU_OPCODE ||  DM_WB_Buffer.instruction.op == I_LB_OPCODE ||
                                       DM_WB_Buffer.instruction.op == I_LBU_OPCODE  ){

                                 if ( DM_WB_Buffer.instruction.r_i_rt  != 0 && (DM_WB_Buffer.instruction.r_i_rt  ==  (*tmp_ID_EXE_Buffer).instruction.r_i_rs )){
                                       controlUnit.isStall =true;
                                       return;
                                  }
                            }
                      }
                      break;
              }

              switch(EXE_DM_Buffer.instruction.instrType){
                   case R:
                          if ( (*tmp_ID_EXE_Buffer).instruction.r_funct == R_JR_FUNCT ){
                                 if (EXE_DM_Buffer.instruction.r_rd !=0 &&
                                    (  EXE_DM_Buffer.instruction.r_rd  == (*tmp_ID_EXE_Buffer).instruction.r_i_rs ||
                                      EXE_DM_Buffer.instruction.r_rd  == (*tmp_ID_EXE_Buffer).instruction.r_i_rt  )  ){
                                               controlUnit.isStall =true;
                                               return;
                                 }
                          }
                          break;
                   case I:
                          if ( EXE_DM_Buffer.instruction.r_i_rt !=0 && (
                              EXE_DM_Buffer.instruction.op == I_LW_OPCODE ||  EXE_DM_Buffer.instruction.op == I_LH_OPCODE ||
                              EXE_DM_Buffer.instruction.op == I_LHU_OPCODE ||  EXE_DM_Buffer.instruction.op == I_LB_OPCODE ||
                               EXE_DM_Buffer.instruction.op == I_LBU_OPCODE  ) &&
                              (EXE_DM_Buffer.instruction.r_i_rt  ==  (*tmp_ID_EXE_Buffer).instruction.r_i_rs   || EXE_DM_Buffer.instruction.r_i_rt  == (*tmp_ID_EXE_Buffer).instruction.r_i_rt  )
                              ){
                                controlUnit.isStall =true;
                                return;
                               }

                           if ( (*tmp_ID_EXE_Buffer).instruction.r_funct == R_JR_FUNCT ){   /*** detection ***/
                                 if ( EXE_DM_Buffer.instruction.r_i_rt  !=0 &&
                                      ( EXE_DM_Buffer.instruction.r_i_rt  == (*tmp_ID_EXE_Buffer).instruction.r_i_rs) &&
                                      ( EXE_DM_Buffer.instruction.op != I_SW_OPCODE &&  EXE_DM_Buffer.instruction.op != I_SH_OPCODE &&
                                        EXE_DM_Buffer.instruction.op != I_SB_OPCODE &&  EXE_DM_Buffer.instruction.op != I_BEQ_OPCODE &&
                                        EXE_DM_Buffer.instruction.op != I_BNE_OPCODE  )
                                       ){
                                              controlUnit.isStall =true;
                                              return;
                                       }
                           }
                          break;
                   case S:
                          break;
                   case J:
                          break;
              }
            break;
      case I:

             /// stall for load-use data hazard
              switch (DM_WB_Buffer.instruction.instrType   ){
                    case I:
                         if ( (*tmp_ID_EXE_Buffer).instruction.op == I_BEQ_OPCODE ||  (*tmp_ID_EXE_Buffer).instruction.op == I_BNE_OPCODE  ){
                             if( DM_WB_Buffer.instruction.op == I_LW_OPCODE ||
                                 DM_WB_Buffer.instruction.op == I_LH_OPCODE ||
                                 DM_WB_Buffer.instruction.op == I_LHU_OPCODE ||
                                 DM_WB_Buffer.instruction.op == I_LB_OPCODE ||
                                 DM_WB_Buffer.instruction.op == I_LBU_OPCODE
                                ){
                                      if ( DM_WB_Buffer.instruction.r_i_rt != 0 && ( DM_WB_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rs  ||
                                           DM_WB_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rt ) ){
                                                controlUnit.isStall =true;
                                                return;
                                      }
                                 }
                         }
                         break;
              }

              switch(EXE_DM_Buffer.instruction.instrType){
                   case R:
                          if ( (*tmp_ID_EXE_Buffer).instruction.op == I_BEQ_OPCODE ||  (*tmp_ID_EXE_Buffer).instruction.op == I_BNE_OPCODE  ){
                              if (EXE_DM_Buffer.instruction.r_rd != 0 && (  EXE_DM_Buffer.instruction.r_rd == (*tmp_ID_EXE_Buffer).instruction.r_i_rt || EXE_DM_Buffer.instruction.r_rd == (*tmp_ID_EXE_Buffer).instruction.r_i_rs )){
                                    controlUnit.isStall =true;
                                    return;
                              }
                          }
                          break;
                   case I:
                          if ( EXE_DM_Buffer.instruction.op == I_LW_OPCODE ||  EXE_DM_Buffer.instruction.op == I_LH_OPCODE ||
                          EXE_DM_Buffer.instruction.op == I_LHU_OPCODE ||  EXE_DM_Buffer.instruction.op == I_LB_OPCODE ||
                          EXE_DM_Buffer.instruction.op == I_LBU_OPCODE  ){

                              if ( EXE_DM_Buffer.instruction.r_i_rt !=0 && EXE_DM_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rs ){
                                  controlUnit.isStall =true;
                                  return;
                              }
                              if ( (*tmp_ID_EXE_Buffer).instruction.op == I_SW_OPCODE  || (*tmp_ID_EXE_Buffer).instruction.op == I_SH_OPCODE || (*tmp_ID_EXE_Buffer).instruction.op == I_SB_OPCODE  ){
                                  if ( EXE_DM_Buffer.instruction.r_i_rt !=0  && EXE_DM_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rt ){
                                      controlUnit.isStall =true;
                                      return;
                                  }
                              }
                        }

                         if ( (*tmp_ID_EXE_Buffer).instruction.op == I_BEQ_OPCODE ||  (*tmp_ID_EXE_Buffer).instruction.op == I_BNE_OPCODE  ){

                              if ( EXE_DM_Buffer.instruction.op != I_SW_OPCODE  && EXE_DM_Buffer.instruction.op != I_SH_OPCODE && EXE_DM_Buffer.instruction.op != I_SB_OPCODE &&
                                   EXE_DM_Buffer.instruction.op != I_BEQ_OPCODE  && EXE_DM_Buffer.instruction.op != I_BNE_OPCODE   ){
                                   if ( EXE_DM_Buffer.instruction.r_i_rt !=0  && EXE_DM_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rt ){
                                       controlUnit.isStall =true;
                                       return;
                                  }
                                  if ( EXE_DM_Buffer.instruction.r_i_rt !=0  && EXE_DM_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rs ){
                                       controlUnit.isStall =true;
                                       return;
                                  }
                              }
                         }
                          break;
                   case J:
                          break;
              }
             break;
      case J:
            break;
      case S:
             break;
   }
}

/** detect data hazard in ID stage **/
void dataHazardDetection_For_ID(PipelineBuffer *tmp_ID_EXE_Buffer, int *rs_Value , int *rt_Value ){

     switch ( (*tmp_ID_EXE_Buffer).instruction.instrType  ){
      case R:

                if ( (*tmp_ID_EXE_Buffer).instruction.r_funct == R_JR_FUNCT ){
                        /// forward test
                    switch( DM_WB_Buffer.instruction.instrType  ){
                            case R:
                                  if ( DM_WB_Buffer.instruction.r_rd != 0 && DM_WB_Buffer.instruction.r_rd ==  (*tmp_ID_EXE_Buffer).instruction.r_i_rs ){
                                               (*rs_Value) = DM_WB_Buffer.result;
                                                forwardingUnit.isForwardRsToID = true;
                                                forwardingUnit.fromStage_ForwardRsToID = EX_DM_STAGE;
                                  }
                                  break;
                            case I:
                                  if (  DM_WB_Buffer.instruction.op != I_SW_OPCODE &&  DM_WB_Buffer.instruction.op != I_SH_OPCODE &&
                                              DM_WB_Buffer.instruction.op != I_SB_OPCODE &&  DM_WB_Buffer.instruction.op != I_BEQ_OPCODE &&
                                              DM_WB_Buffer.instruction.op != I_BNE_OPCODE   ){
                                              if ( DM_WB_Buffer.instruction.r_i_rt !=0  && DM_WB_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rs ) {
                                                    forwardingUnit.isForwardRsToID = true;
                                                   (*rs_Value) = DM_WB_Buffer.result;
                                                    forwardingUnit.fromStage_ForwardRsToID = EX_DM_STAGE;
                                              }
                                  }
                                  break;
                            case J:
                                  if (  DM_WB_Buffer.instruction.op  == J_JAL_OPCODE ){
                                      if ( (*tmp_ID_EXE_Buffer).instruction.r_i_rs == R_RA ){
                                          forwardingUnit.isForwardRsToID =true;
                                          forwardingUnit.fromStage_ForwardRsToID = EX_DM_STAGE;
                                          (*rs_Value) = DM_WB_Buffer.result;
                                      }
                                  }
                                  break;
                            case S:
                                  break;
                    }
               }

             break;
      case I:
              /** for beq and bne forwarding **/
              if ( (*tmp_ID_EXE_Buffer).instruction.op == I_BEQ_OPCODE ||  (*tmp_ID_EXE_Buffer).instruction.op == I_BNE_OPCODE  ){

                   switch (DM_WB_Buffer.instruction.instrType   ){
                            case R:

                                    if (DM_WB_Buffer.instruction.r_rd != 0 && DM_WB_Buffer.instruction.r_rd == (*tmp_ID_EXE_Buffer).instruction.r_i_rs ){
												forwardingUnit.isForwardRsToID =true;
												(*rs_Value) = DM_WB_Buffer.result;
												forwardingUnit.fromStage_ForwardRsToID = EX_DM_STAGE;
                                    }
                                    if (DM_WB_Buffer.instruction.r_rd != 0 && DM_WB_Buffer.instruction.r_rd  == (*tmp_ID_EXE_Buffer).instruction.r_i_rt){
												forwardingUnit.isForwardRtToID =true;
												(*rt_Value) = DM_WB_Buffer.result;
												forwardingUnit.fromStage_ForwardRtToID = EX_DM_STAGE;
                                    }
                                break;
                             case I:
                                    if(  DM_WB_Buffer.instruction.op != I_SW_OPCODE && DM_WB_Buffer.instruction.op != I_SH_OPCODE &&
												DM_WB_Buffer.instruction.op != I_SB_OPCODE && DM_WB_Buffer.instruction.op != I_BEQ_OPCODE &&
												DM_WB_Buffer.instruction.op != I_BNE_OPCODE){
												if ( DM_WB_Buffer.instruction.r_i_rt != 0 && DM_WB_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rs ){
												    forwardingUnit.isForwardRsToID = true;
													(*rs_Value) = DM_WB_Buffer.result;
													forwardingUnit.fromStage_ForwardRsToID = EX_DM_STAGE;
												}
                                                if ( DM_WB_Buffer.instruction.r_i_rt != 0 && DM_WB_Buffer.instruction.r_i_rt == (*tmp_ID_EXE_Buffer).instruction.r_i_rt ){
                                                    forwardingUnit.isForwardRtToID = true;
                                                    (*rt_Value) = DM_WB_Buffer.result;
                                                    forwardingUnit.fromStage_ForwardRtToID = EX_DM_STAGE;
												}
                                      }
											break;
                                case J:
                                        if( DM_WB_Buffer.instruction.op == J_JAL_OPCODE  ){
												if ( (*tmp_ID_EXE_Buffer).instruction.r_i_rs == 31  ){
												    forwardingUnit.isForwardRsToID = true;
                                                    (*rs_Value) = DM_WB_Buffer.result;
                                                    forwardingUnit.fromStage_ForwardRsToID = EX_DM_STAGE;
												}
												if ( (*tmp_ID_EXE_Buffer).instruction.r_i_rt == 31){
                                                    forwardingUnit.isForwardRtToID = true;
                                                    (*rt_Value) = DM_WB_Buffer.result;
                                                    forwardingUnit.fromStage_ForwardRtToID = EX_DM_STAGE;
												}
                                         }
											break;
                        }
              }
             break;
      case J:
            break;
      case S:
             break;
   }
}

/** doing ID stage job**/
void ID(){

   PipelineBuffer tmp_ID_EXE_Buffer = IF_ID_Buffer;
   int rs_Value =0;
   int rt_Value =0;

   convertBinaryToInstr( &tmp_ID_EXE_Buffer);

    if ( tmp_ID_EXE_Buffer.instruction.binaryCode == 0
         ||
         ( tmp_ID_EXE_Buffer.instruction.op==0 &&
            tmp_ID_EXE_Buffer.instruction.r_rd==0 &&
            tmp_ID_EXE_Buffer.instruction.r_i_rt==0 &&
            tmp_ID_EXE_Buffer.instruction.r_shamt==0 &&
            tmp_ID_EXE_Buffer.instruction.r_funct ==0
            )
        ){
            /// && tmp_ID_EXE_Buffer.instruction.r_i_rs==0 )
        tmp_ID_EXE_Buffer.instruction.name= NOP.name;
        ID_EXE_Buffer = tmp_ID_EXE_Buffer;
        return;
   }


   /** again first, because some of action will return ,
       so I need update ID_EXE_Buffer beforehand.   ***/
   ID_EXE_Buffer = tmp_ID_EXE_Buffer;

   if (tmp_ID_EXE_Buffer.instruction.op   == S_HALT_OPCODE ) {
        return;
   }


   bool isMatchR =false; bool isMatchI =false;
   bool isMatchJ =false; bool isMatchS =false;
   int i=0;
   switch ( tmp_ID_EXE_Buffer.instruction.instrType  ){
      case R:
             isMatchR =false;
             for (i=0;i<R_INSTRSPEC_COUNT;i++){
                    if ( tmp_ID_EXE_Buffer.instruction.r_funct != rInstrSpec[i].r_funct) continue;
                    isMatchR = true;
                    break;
            }
            if (!isMatchR){
                fprintf(stderr,"[Error][executeR] R-instruction OPCode(0x%02X) is not matched in R_InstrSpec.\n",tmp_ID_EXE_Buffer.instruction.op);
                fflush(stderr);
                errorHandler(ERROR_EXIT_NOW);
            }

            loadUseDataHazardDetection_For_ID(&tmp_ID_EXE_Buffer);
            if (controlUnit.isStall){
                return;
            }

             if ( tmp_ID_EXE_Buffer.instruction.r_funct == R_JR_FUNCT ){
                        /// forward test
                    dataHazardDetection_For_ID( &tmp_ID_EXE_Buffer, &rs_Value, &rt_Value );

                    if ( forwardingUnit.isForwardRsToID ==false ){
                        rs_Value = registers[tmp_ID_EXE_Buffer.instruction.r_i_rs];
                    }
                    controlUnit.updatePC =rs_Value;
                    controlUnit.isFlush = true;
                    return ;
             }

             break;
      case I:
             isMatchI =false;
             for (i=0;i<I_INSTRSPEC_COUNT;i++){
                  if ( tmp_ID_EXE_Buffer.instruction.op   != iInstrSpec[i].op) continue;
                  isMatchI = true;
                  break;
            }
            if (!isMatchI){
                fprintf(stderr,"[Error][executeI] I-instruction OPCode(0x%02X) is not matched in I_InstrSpec.\n", tmp_ID_EXE_Buffer.instruction.op );
                fflush(stderr);
                errorHandler(ERROR_EXIT_NOW);
            }


            loadUseDataHazardDetection_For_ID( &tmp_ID_EXE_Buffer);
            if (controlUnit.isStall){
                return;
            }

              /** for beq and bne forwarding **/
              if ( tmp_ID_EXE_Buffer.instruction.op == I_BEQ_OPCODE ||  tmp_ID_EXE_Buffer.instruction.op == I_BNE_OPCODE  ){

                  dataHazardDetection_For_ID(&tmp_ID_EXE_Buffer, &rs_Value, &rt_Value );

                  if ( tmp_ID_EXE_Buffer.instruction.op  == I_BEQ_OPCODE ){

                         if (forwardingUnit.isForwardRsToID ==false ){
                               rs_Value  = registers[tmp_ID_EXE_Buffer.instruction.r_i_rs];
                          }
                          if (forwardingUnit.isForwardRtToID ==false ){
                               rt_Value  = registers[tmp_ID_EXE_Buffer.instruction.r_i_rt];
                          }
                          if (rs_Value  == rt_Value  ){
                                controlUnit.updatePC = programCounter +  (tmp_ID_EXE_Buffer.instruction.i_immediate * 4) ;
                                controlUnit.isFlush = true;
                          }

                  }else{

                          if (forwardingUnit.isForwardRsToID ==false ){
                               rs_Value  = registers[tmp_ID_EXE_Buffer.instruction.r_i_rs];
                          }
                          if (forwardingUnit.isForwardRtToID ==false ){
                               rt_Value  = registers[tmp_ID_EXE_Buffer.instruction.r_i_rt];
                          }
                          if (rs_Value  != rt_Value  ){
                                controlUnit.updatePC = programCounter + (tmp_ID_EXE_Buffer.instruction.i_immediate *4);
                                controlUnit.isFlush = true;
                          }
                  }

              }
             break;
      case J:
              isMatchJ =false;
              for (i=0;i<J_INSTRSPEC_COUNT;i++){
                    if ( tmp_ID_EXE_Buffer.instruction.op  != jInstrSpec[i].op) continue;
                    isMatchJ = true;
                    break;
             }
             if (!isMatchJ){
                    fprintf(stderr,"[Error][executeJ] J-instruction OPCode(0x%02X) is not matched in J_InstrSpec.\n", tmp_ID_EXE_Buffer.instruction.op);
                    fflush(stderr);
                    errorHandler(ERROR_EXIT_NOW);
             }

            controlUnit.isFlush = true;
            controlUnit.updatePC  = ( (programCounter +4) & 0xf0000000) | ( tmp_ID_EXE_Buffer.instruction.j_s_addr *4);
            if ( tmp_ID_EXE_Buffer.instruction.op == J_JAL_OPCODE ){
                tmp_ID_EXE_Buffer.instruction.r_i_rs = R_RA;
                tmp_ID_EXE_Buffer.result = programCounter;
            }
            break;
      case S:
             isMatchS =false;
             for (i=0;i<S_INSTRSPEC_COUNT;i++){
                    if (  tmp_ID_EXE_Buffer.instruction.op  != sInstrSpec[i].op) continue;
                    isMatchS = true;
                    break;
            }
            if (!isMatchS){
                fprintf(stderr,"[Error][executeS] S-instruction OPCode(0x%02X) is not matched in S_InstrSpec.\n", tmp_ID_EXE_Buffer.instruction.op  );
                fflush(stderr);
                errorHandler(ERROR_EXIT_NOW);
            }
             break;
   }

   /** assign again because in JAL, the tmp_ID_EXE_Buffer has been updated again. **/
   ID_EXE_Buffer = tmp_ID_EXE_Buffer;

}


/** doing IF stage job**/
void IF(){
   PipelineBuffer tmp_IF_ID_Buffer;
   checkInstrMemoryError();

   tmp_IF_ID_Buffer.instruction.binaryCode = instrMemory[ (programCounter/4) ];
   IF_ID_Buffer = tmp_IF_ID_Buffer;
}



int main(){

    initializeAction();

    iImageFile =loadFile( I_IMAGE_FILE );   /// load iImage.bin
    dImageFile = loadFile( D_IMAGE_FILE );  /// load dImage.bin

    fetchInstruction();
    copy_RegistersValue_Into_RegistersInitStages();

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

            /**
             if ( cycleCount == 290   ){
                int aa=0;
            }**/

            WB();
            DM();
            EXE();

            /**
            if ( cycleCount == 19   ){
                int aa=0;
            }**/

            ID();
            IF();
            outputSnapshot();

            /**
            if (cycleCount >=100){
                isHalt =true;
                continue;
            }**/



            if (WB_Buffer.instruction.op == S_HALT_OPCODE  &&
                DM_WB_Buffer.instruction.op == S_HALT_OPCODE  &&
                EXE_DM_Buffer.instruction.op == S_HALT_OPCODE  &&
                ID_EXE_Buffer.instruction.op == S_HALT_OPCODE
             ){
                  int opcode_IF_ID = GET_OPCode(IF_ID_Buffer.instruction.binaryCode) ;
                  if ( opcode_IF_ID == S_HALT_OPCODE){
                     /// printf("#### 5 stage with halt #### cycleCount=%d\n",cycleCount);
                     isHalt=true;
                     continue;
                  }
            }


            /**
            if (cycleCount > 158 ){
                 isHalt=true;
                 continue;
            }**/


            copy_RegistersValue_Into_RegistersInitStages();
            cycleCount++;

            /** key, before executing instruction,
                we should let programCounter pointers to next instruction location **/
            /// programCounter = programCounter +4;
            if (controlUnit.isFlush ){
                /** flush IF_ID stage which is approaching to ID_EXE stage
                    and is ready to be used for :ID_EXE stage **/
                getInitStage(&IF_ID_Buffer);

                programCounter = controlUnit.updatePC;

            }else if ( controlUnit.isStall ){

                 /** when stall happens, because the IF_ID_Buffer instruction
                     has been run to next stage ID_EXE_Buffer,
                     we need to load it back to IF_ID stage.
                     Otherwise, we will execute skip the IF_ID stage's instruction. **/
                 PipelineBuffer buffer = ID_EXE_Buffer;
                 IF_ID_Buffer = buffer;
                 /** make ID_EXE stage to be nop instruction !! **/
                 getInitStage(&ID_EXE_Buffer );

                 programCounter = programCounter;
            }else{
                programCounter = programCounter +4;
            }

    }


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

