#ifndef PIPELINE_H_INCLUDED
#define PIPELINE_H_INCLUDED
/** error type for error handler to know  **/
typedef enum { ERROR_EXIT_NOW, ERROR_WRITE_R_ZERO, ERROR_NUMBER_OVERFLOW, ERROR_MEMORY_ADDR_OVERFLOW , ERROR_DATA_MISALIGNED } ErrorHandlerCode;
typedef enum { R,I,J,S } InstructionType; /** 4 type R,I,J,S insturction's stuct **/
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
    char * name;
} Instruction;

/**
   PipeRegister Buffer for doing pipeline action in 5 stage.
**/
typedef struct PipelineBufferStructure{
    Instruction instruction;
    int result;
} PipelineBuffer;

void getInitStage(PipelineBuffer *buffer);  /** initial each stage with nop instruction **/
void initInstrSpec(void);  /** initial R,I,J,S instruction SPEC **/
char* getInstrName( InstructionType instrType, int opCode_Or_functCode); /** get instruction's name for printing out instruction.**/
void initializeAction(void); /** initial action to be loaded at begin **/
void finalizeAction(void);  /** before program finishes, some final jobs should be done**/
void outputDataMemory(void);  /** output Dmemory data for debugging only**/

/** Because we want to print out the register value before each stage,
    we need to keep the registers' value on each stage before pipeline running.
    Once pipeLine running all the register's value has been updated!! **/
void copy_RegistersValue_Into_RegistersInitStages();

void outputSnapshot(void);    /** output snapShot for each cycle **/
int* loadFile(const char *path );  /** load file return int[] **/

/** Because we need to detect data hazard, however, some instruction which rs, rt or rd is
    unused to the instruction. So we need to avoid the unused rt,rs,rd not to be used
    for judging data hazard, which means we need to make unused rt,rs,rd to be zero!!!
    because when judge data hazard, we skip some data hazard once it will write back to $zero  register. **/
void avoid_Wrong_DataHazard_By_Unused_Registers(Instruction* instruction);
void convertLitteEndianToBigEndian(int *data ,const int dataSize ); /** convert encoding to match project encoding **/
void fetchInstruction(void);  /** when dimage & image's sources binary codes loaded into memory, it uses to load the relative data in relative location **/
void errorHandler( ErrorHandlerCode errorCode); /** error handler to handling error when error happens **/
void undoSetting(void); /** some setting be undo before each cycle **/
void convertBinaryToInstr(  Instruction* instruction );  /**convert binary code into R,I,J,S instruction's format **/
void decoding(void); /** decoding binary code , it will call convertBinaryToInstr() **/
void checkInstrMemoryError( void  ); /** check I memory has overflowed or misaligned **/
void checkMemoryAccessError(int howManyBytes, Instruction *instruction , int memoryAddress); /** check memory access error **/
short loadShortFromMemory(int memoryAddress); /** load 2 bytes form memory **/
char loadByteFromMemory(int memoryAddress);  /** load 1 byte from memory **/
void saveShortIntoMemory(int memoryAddress, int rtIndex ); /** save 2 bytes into memory **/
void saveByteIntoMemory(int memoryAddress, int rtIndex); /** save 1 bytes into memory **/
void WB(void); /** doing WB stage job**/
void DM(void);  /** doing DM stage job**/
void dataHazardDetection_For_EXE(PipelineBuffer *tmp_EXE_DM_Buffer, int *rs_Value , int *rt_Value ); /** detect data hazard in EXE stage **/
void EXE(void);  /** doing EXE stage job**/
void loadUseDataHazardDetection_For_ID(PipelineBuffer *tmp_ID_EXE_Buffer  ); /** detect load-use or data hazard which will make stage stall  in ID stage **/
void dataHazardDetection_For_ID(PipelineBuffer *tmp_ID_EXE_Buffer, int *rs_Value , int *rt_Value ); /** detect data hazard in ID stage **/
void ID(void);  /** doing ID stage job**/
void IF(void);  /** doing IF stage job**/


#endif // PIPELINE_H_INCLUDED
