#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define NUMMEMORY 65536
#define NUMREGS 8
#define MAXLINELENGTH 1000

typedef unsigned int uint32_t;
typedef signed int int32_t;

typedef struct stateStruct {
  int pc;
  int mem[NUMMEMORY]; 
  int reg[NUMREGS];
  int numMemory;
} stateType;

typedef enum opcode_t
{
  ADD = 0b000,
  NOR = 0b001,
  LW = 0b010,
  SW = 0b011,
  BEQ = 0b100,
  JALR = 0b101,
  HALT = 0b110,
  NOOP = 0b111
} opcode_t;

void throwError(char *string);
void printState(stateType *);
uint32_t getOpcode(uint32_t machine_code);
uint32_t getRegA(uint32_t machine_code);
uint32_t getRegB(uint32_t machine_code);
uint32_t getRegDest(uint32_t machine_code);
uint32_t getOffset(uint32_t machine_code);

void add(uint32_t code, stateType *state);
void nor(uint32_t code, stateType *state);
void lw(uint32_t code, stateType *state);
void sw(uint32_t code, stateType *state);
void beq(uint32_t code, stateType *state);
void jalr(uint32_t code, stateType *state);

void (*op[6])(uint32_t, stateType *) = {add, nor, lw, sw, beq, jalr};

FILE *filePtr;
int countsOfExecutedInstr;

int 
main(int argc, char *argv[])
{
  char line[MAXLINELENGTH]; 
  stateType state;
  uint32_t machine_code;
  uint32_t opcode;

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
  memset(&state, 0, sizeof(stateType));
  for (state.numMemory = 0; fgets(line, MAXLINELENGTH, filePtr); state.numMemory++) {
    if (sscanf(line, "%d", state.mem+state.numMemory) != 1) {
      printf("error in reading address %d\n", state.numMemory);
      exit(1); 
    }
    printf("memory[%d]=%d\n", state.numMemory, state.mem[state.numMemory]); 
  }

  
  countsOfExecutedInstr = 0;
  while (1) {
    printState(&state);

    machine_code = state.mem[state.pc];
    countsOfExecutedInstr++;
    opcode = getOpcode(machine_code);
    if (opcode <= 5 && opcode >= 0) {
      op[opcode](machine_code, &state);
    } else if (opcode == HALT) {
      state.pc++;
      break;
    } else if (opcode == NOOP) {
      state.pc++;
    } else {
      throwError("Unrecognized opcode.");
    }
    if (state.pc < 0 || state.pc >= NUMMEMORY) {
      throwError("PC out of memory");
    }
  }
  printf("machine halted\n");
  printf("total of %d instructions executed\n", countsOfExecutedInstr);
  printf("final state of machine:");
  printState(&state);

  return(0); 
}

uint32_t
getOpcode(uint32_t machine_code)
{
  uint32_t res = (machine_code >> 22);
  uint32_t mask = 0b111;
  return res & mask;
}

uint32_t
getRegA(uint32_t machine_code)
{
  uint32_t res = (machine_code >> 19);
  uint32_t mask = 0b111;
  return res & mask;
}

uint32_t
getRegB(uint32_t machine_code)
{
  uint32_t res = (machine_code >> 16);
  uint32_t mask = 0b111;
  return res & mask;
}

uint32_t
getRegDest(uint32_t machine_code)
{
  uint32_t mask = 0b111;
  return machine_code & mask;
}

uint32_t
getOffset(uint32_t machine_code)
{
  uint32_t mask = 0b1111111111111111;
  return machine_code & mask;
}

void
throwError(char *string)
{
  printf("%s\n", string);
  fclose(filePtr);
  exit(1);
}

void 
printState(stateType *statePtr)
{
  int i;

  printf("\n@@@\nstate:\n");
  printf("\tpc %d\n", statePtr->pc); 
  printf("\tmemory:\n");
  for (i=0; i<statePtr->numMemory; i++) {
    printf("\t\tmem[ %d ] %d\n", i, statePtr->mem[i]); 
  }
  printf("\tregisters:\n");
  for (i=0; i<NUMREGS; i++) {
    printf("\t\treg[ %d ] %d\n", i, statePtr->reg[i]); 
  }
  printf("end state\n");
}

void add(uint32_t code, stateType *state)
{
  uint32_t regA = getRegA(code);
  uint32_t regB = getRegB(code);
  uint32_t regDest = getRegDest(code);
  state->reg[regDest] = state->reg[regA] + state->reg[regB];
  state->pc++;
}

void nor(uint32_t code, stateType *state)
{
  uint32_t regA = getRegA(code);
  uint32_t regB = getRegB(code);
  uint32_t regDest = getRegDest(code);
  state->reg[regDest] = ~(state->reg[regA] | state->reg[regB]);
  state->pc++;
}

void lw(uint32_t code, stateType *state)
{
  uint32_t regA = getRegA(code);
  uint32_t regB = getRegB(code);
  int16_t offset = (int16_t)getOffset(code);
  state->reg[regB] = state->mem[state->reg[regA] + offset];
  state->pc++;
}

void sw(uint32_t code, stateType *state)
{
  uint32_t regA = getRegA(code);
  uint32_t regB = getRegB(code);
  int16_t offset = (int16_t)getOffset(code);
  state->mem[state->reg[regA] + offset] = state->reg[regB];
  state->pc++;
}

void beq(uint32_t code, stateType *state)
{
  uint32_t regA = getRegA(code);
  uint32_t regB = getRegB(code);
  int16_t offset = (int16_t)getOffset(code);
  state->pc += offset * (state->reg[regA] == state->reg[regB]) + 1;
}

void jalr(uint32_t code, stateType *state)
{
  uint32_t regA = getRegA(code);
  uint32_t regB = getRegB(code);
  state->reg[regB] = state->pc + 1;
  state->pc = state->reg[regA];
}
