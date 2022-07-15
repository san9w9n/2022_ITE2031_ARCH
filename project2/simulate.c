#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CONVERT_TO_32(NUM) \
  ((NUM) & (1 << 15)) ? ((NUM) - (1 << 16)) : (NUM)
#define ADD_OP(X, Y) ((X) + (Y))
#define NOR_OP(X, Y) (~((X) | (Y)))
#define BEQ_OP(X, Y) ((X) - (Y))
#define MOV_OP(X, Y) ((X) + (Y))

#define NUMREGS           8
#define NUMMEMORY         65536
#define MAXLINELENGTH     1000

#define ADD               0
#define NOR               1
#define LW                2
#define SW                3
#define BEQ               4
#define JALR              5
#define HALT              6
#define NOOP              7
#define NOOPINSTRUCTION   0x1c00000

typedef struct IFIDStruct { 
  int instr;
  int pcPlus1; 
} IFIDType;

typedef struct IDEXStruct { 
  int instr;
  int pcPlus1; 
  int readRegA; 
  int readRegB; 
  int offset;
} IDEXType;

typedef struct EXMEMStruct { 
  int instr;
  int branchTarget; 
  int aluResult;
  int readRegB; 
} EXMEMType;

typedef struct MEMWBStruct { 
  int instr;
  int writeData; 
} MEMWBType;

typedef struct WBENDStruct { 
  int instr;
  int writeData; 
} WBENDType;

typedef struct stateStruct { 
  int       pc;
  int       instrMem[NUMMEMORY]; 
  int       dataMem[NUMMEMORY]; 
  int       reg[NUMREGS];
  int       numMemory;
  IFIDType  IFID;
  IDEXType  IDEX;
  EXMEMType EXMEM;
  MEMWBType MEMWB;
  WBENDType WBEND;
  int       cycles;
} stateType;

void      printState(stateType*);
void      printInstruction(int);
void      initState(stateType*);
void      initIFID(IFIDType*);
void      initIDEX(IDEXType*);
void      initEXMEM(EXMEMType*);
void      initMEMWB(MEMWBType*);
void      initWBEND(WBENDType*);

void      IF_stage();
void      ID_stage();
void      EX_stage();
void      MEM_stage();
void      WB_stage();

int       field0(int);
int       field1(int);
int       field2(int);
int       opcode(int);

FILE      *filePtr;
stateType state;
stateType newState;

int 
main(int argc, char *argv[])
{
  char line[MAXLINELENGTH];
  int i;

  filePtr = 0;
  if (argc != 2) {
    printf("error: usage: %s <machine-code file>\n", argv[0]);
    exit(1);
  }

  filePtr = fopen(argv[1], "r");
  if (filePtr == NULL) {
    printf("error: can't open file %s", argv[1]);
    perror("fopen");
    exit(1);
  }
  
  initState(&state);
  initState(&newState);
  for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr); state.numMemory++) {
    if (sscanf(line, "%d", state.instrMem+state.numMemory) != 1) {
      printf("error in reading address %d\n", state.numMemory);
      exit(1);
    }
    printf("memory[%d]=%d\n", state.numMemory, state.instrMem[state.numMemory]);
    state.dataMem[state.numMemory] = state.instrMem[state.numMemory];
  }

  printf("%d memory words\n", state.numMemory);
  printf("\tinstruction memory:\n");
  for (i = 0; i < state.numMemory; i++) {
    printf("\t\tinstrMem[ %d ] ", i);
    printInstruction(state.instrMem[i]);
  }

  while (1) { 
    printState(&state);
    if (opcode(state.MEMWB.instr) == HALT) {
      printf("machine halted\n");
      printf("total of %d cycles executed\n", state.cycles); 
      exit(0);
    }
    newState = state; 
    newState.cycles++;

    /* --------------------- IF stage --------------------- */

    IF_stage(); 

    /* --------------------- ID stage --------------------- */

    ID_stage();

    /* --------------------- EX stage --------------------- */

    EX_stage();

    /* --------------------- MEM stage --------------------- */

    MEM_stage();

    /* --------------------- WB stage --------------------- */

    WB_stage();
      
    state = newState; /* this is the last statement before end of the loop.
                         It marks the end of the cycle and updates the 
                         current state with the values calculated in this
                         cycle */
  }
  return 0;
}

void
IF_stage()
{
  newState.IFID.instr = state.instrMem[state.pc];
  newState.IFID.pcPlus1 = state.pc + 1;
  newState.pc++;
}

void
ID_stage()
{
  int regA, regB, offset, regD;

  regA   = field0(state.IFID.instr);
  regB   = field1(state.IFID.instr);
  offset = field2(state.IFID.instr);
  
  newState.IDEX.instr = state.IFID.instr;
  newState.IDEX.pcPlus1 = state.IFID.pcPlus1;
  newState.IDEX.readRegA = state.reg[regA];
  newState.IDEX.readRegB = state.reg[regB];
  newState.IDEX.offset = CONVERT_TO_32(offset);
}

void
EX_stage()
{
  int operand0;
  int operand1;
  int IDEX_code;
  
  IDEX_code  = opcode(state.IDEX.instr);
  operand0 = state.IDEX.readRegA;
  operand1 = (IDEX_code == LW || IDEX_code == SW) ? state.IDEX.offset : state.IDEX.readRegB;

  switch (opcode(state.IDEX.instr)) {
    case ADD:
      newState.EXMEM.aluResult = ADD_OP(operand0, operand1);
      break;
    case NOR:
      newState.EXMEM.aluResult = NOR_OP(operand0, operand1);
      break;
    case BEQ:
      newState.EXMEM.aluResult = BEQ_OP(operand0, operand1);
      break;
    case LW:
    case SW:
      newState.EXMEM.aluResult = MOV_OP(operand0, operand1);
      break;
  }

  newState.EXMEM.instr = state.IDEX.instr;
  newState.EXMEM.branchTarget = state.IDEX.pcPlus1 + state.IDEX.offset;
  newState.EXMEM.readRegB = state.IDEX.readRegB;
}

void
MEM_stage()
{
  newState.MEMWB.instr = state.EXMEM.instr;

  switch (opcode(state.EXMEM.instr)) {
    case ADD:
    case NOR:
      newState.MEMWB.writeData = state.EXMEM.aluResult;
      break;
    case LW:
      newState.MEMWB.writeData = state.dataMem[state.EXMEM.aluResult];
      break;
    case SW:
      newState.dataMem[state.EXMEM.aluResult] = state.EXMEM.readRegB;
      break;
    case BEQ:
      if(state.EXMEM.aluResult == 0)
        newState.pc = state.EXMEM.branchTarget;
      break;
  }
}

void
WB_stage()
{
  switch(opcode(state.MEMWB.instr)) {
    case ADD:
    case NOR:
      newState.reg[field2(state.MEMWB.instr)] = state.MEMWB.writeData;
      break;
    case LW:
      newState.reg[field1(state.MEMWB.instr)] = state.MEMWB.writeData;
      break;
  }
  newState.WBEND.instr = state.MEMWB.instr;
  newState.WBEND.writeData = state.MEMWB.writeData;
}

void
printState(stateType *statePtr)
{
  int i;

  printf("\n@@@\nstate before cycle %d starts\n", statePtr->cycles); 
  printf("\tpc %d\n", statePtr->pc);
  printf("\tdata memory:\n");
  for (i = 0; i < statePtr->numMemory; i++) {
    printf("\t\tdataMem[ %d ] %d\n", i, statePtr->dataMem[i]); 
  }
  printf("\tregisters:\n");
  for (i = 0; i < NUMREGS; i++) {
    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]); 
  }

  printf("\tIFID:\n"); 
  printf("\t\tinstruction ");
  printInstruction(statePtr->IFID.instr);
  printf("\t\tpcPlus1 %d\n", statePtr->IFID.pcPlus1);

  printf("\tIDEX:\n");
  printf("\t\tinstruction "); 
  printInstruction(statePtr->IDEX.instr);
  printf("\t\tpcPlus1 %d\n", statePtr->IDEX.pcPlus1); 
  printf("\t\treadRegA %d\n", statePtr->IDEX.readRegA); 
  printf("\t\treadRegB %d\n", statePtr->IDEX.readRegB); 
  printf("\t\toffset %d\n", statePtr->IDEX.offset);

  printf("\tEXMEM:\n"); 
  printf("\t\tinstruction ");
  printInstruction(statePtr->EXMEM.instr);
  printf("\t\tbranchTarget %d\n", statePtr->EXMEM.branchTarget); 
  printf("\t\taluResult %d\n", statePtr->EXMEM.aluResult); 
  printf("\t\treadRegB %d\n", statePtr->EXMEM.readRegB);

  printf("\tMEMWB:\n");
  printf("\t\tinstruction "); 
  printInstruction(statePtr->MEMWB.instr); 
  printf("\t\twriteData %d\n", statePtr->MEMWB.writeData);

  printf("\tWBEND:\n"); 
  printf("\t\tinstruction ");
  printInstruction(statePtr->WBEND.instr); 
  printf("\t\twriteData %d\n", statePtr->WBEND.writeData);
}

int
field0(int instruction)
{
  return (instruction>>19) & 0x7;
}

int
field1(int instruction) 
{
  return (instruction>>16) & 0x7;
}

int
field2(int instruction) 
{
  return instruction & 0xFFFF;
}

int
opcode(int instruction) 
{
  return instruction >> 22;
}

void
printInstruction(int instr) 
{
  char opcodeString[10];

  if (opcode(instr) == ADD) 
    strcpy(opcodeString, "add");
  else if (opcode(instr) == NOR)
    strcpy(opcodeString, "nor");
  else if (opcode(instr) == LW)
    strcpy(opcodeString, "lw");
  else if (opcode(instr) == SW)
    strcpy(opcodeString, "sw");
  else if (opcode(instr) == BEQ)
    strcpy(opcodeString, "beq");
  else if (opcode(instr) == JALR)
    strcpy(opcodeString, "jalr");
  else if (opcode(instr) == HALT)
    strcpy(opcodeString, "halt");
  else if (opcode(instr) == NOOP)
    strcpy(opcodeString, "noop");
  else
    strcpy(opcodeString, "data");
  printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
  field2(instr));
}

void
initIFID(IFIDType* ifid)
{
  ifid->pcPlus1 = 0;
  ifid->instr   = NOOPINSTRUCTION;
}

void
initIDEX(IDEXType* idex)
{
  memset(idex, 0, sizeof(IDEXType));
  idex->instr = NOOPINSTRUCTION;
}

void
initEXMEM(EXMEMType* exmem)
{
  memset(exmem, 0, sizeof(exmem));
  exmem->instr = NOOPINSTRUCTION;
}

void
initMEMWB(MEMWBType* memwb)
{
  memwb->writeData = 0;
  memwb->instr = NOOPINSTRUCTION;
}

void
initWBEND(WBENDType* wbend)
{
  wbend->writeData = 0;
  wbend->instr = NOOPINSTRUCTION;
}

void
initState(stateType* st)
{
  initIFID(&st->IFID);
  initIDEX(&st->IDEX);
  initEXMEM(&st->EXMEM);
  initMEMWB(&st->MEMWB);
  initWBEND(&st->WBEND);
}