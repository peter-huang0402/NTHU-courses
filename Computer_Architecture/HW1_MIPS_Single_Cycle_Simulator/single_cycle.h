

#ifndef __SINGLE_CYCLE_SIMULATOR__
#define __SINGLE_CYCLE_SIMULATOR__
/** error type for error handler to know  **/
typedef enum { ERROR_EXIT_NOW, ERROR_WRITE_R_ZERO, ERROR_NUMBER_OVERFLOW, ERROR_MEMORY_ADDR_OVERFLOW , ERROR_DATA_MISALIGNED } ErrorHandlerCode;
void initInstrSpec(void);  /** initial R,I,J,S instruction SPEC **/
void initializeAction(void); /** initial action to be loaded at begin **/
void finalizeAction(void);  /** before program finishes, some final jobs should be done**/
void outputDataMemory(void);  /** output Dmemory data for debugging only**/
void outputSnapshot(void);    /** output snapShot for each cycle **/
int* loadFile(const char *path );  /** load file return int[] **/
void convertLitteEndianToBigEndian(int *data ,const int dataSize ); /** convert encoding to match project encoding **/
void fetchInstruction(void);  /** when dimage & image's sources binary codes loaded into memory, it uses to load the relative data in relative location **/
void errorHandler( ErrorHandlerCode errorCode); /** error handler to handling error when error happens **/
void undoSetting(void); /** some setting be undo before each cycle **/
void convertBinaryToInstr(int binaryCode);  /**convert binary code into R,I,J,S instruction's format **/
void decoding(void); /** decoding binary code , it will call convertBinaryToInstr() **/
void checkInstrMemoryError(void); /** check I memory has overflowed or misaligned **/
void checkMemoryAccessError(int howManyBytes); /** check memory access error **/
short loadShortFromMemory(int memoryAddress); /** load 2 bytes form memory **/
char loadByteFromMemory(int memoryAddress);  /** load 1 byte from memory **/
void saveShortIntoMemory(int memoryAddress, int rtIndex ); /** save 2 bytes into memory **/
void saveByteIntoMemory(int memoryAddress, int rtIndex); /** save 1 bytes into memory **/
void executeS(void); /** execute S instruction's job **/
void executeJ(void); /** execute J instruction's job **/
void executeI(void); /** execute I instruction's job **/
void executeR(void); /** execute R instruction's job **/
void execution(void); /** execute R,I,J,S job **/
#endif
