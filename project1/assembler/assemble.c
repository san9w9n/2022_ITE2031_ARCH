#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINELENGTH         1000
#define MAXINSTRUCTIONCOUNTS  1024

typedef unsigned int uint32_t;
typedef signed int int32_t;

typedef enum opcode_t
{
  ADD   = 0b000,
  NOR   = 0b001,
  LW    = 0b010,
  SW    = 0b011,
  BEQ   = 0b100,
  JALR  = 0b101,
  HALT  = 0b110,
  NOOP  = 0b111
} opcode_t;

typedef struct label_addr_t 
{
  char  label[MAXLINELENGTH];
  int   addr;
} label_addr_t;

typedef struct instruction_t 
{
  char opcode[MAXLINELENGTH];
  char arg0[MAXLINELENGTH];
  char arg1[MAXLINELENGTH];
  char arg2[MAXLINELENGTH];
} instruction_t;

void throwError(char *);
int readAndParse(FILE *, char *, char *, char *, char *, char *);
int isNumber(const char *);
void storeLabelAndAddress(char *);
int isEmptyString(const char *);
int findAddrOfLabel(const char *);
void setInstructions(void);
void debugInstructions(void);
opcode_t getOpcode(const char *);
void translateToMachineCode(void);
void processFormat(instruction_t *);
uint32_t translateToWhichFormat(instruction_t *);
uint32_t translateToO(opcode_t, const char *, const char *, const char *);
uint32_t translateToR(opcode_t, const char *, const char *, const char *);
uint32_t translateToI(opcode_t, const char *, const char *, const char *);
uint32_t translateToJ(opcode_t, const char *, const char *, const char *);

instruction_t instruction[MAXINSTRUCTIONCOUNTS];
label_addr_t label_addr[MAXINSTRUCTIONCOUNTS];

int countsOfLabel;
int lastAddress;
int currentAddress;

char *inFileString, *outFileString;
FILE *inFilePtr, *outFilePtr;

int 
main(int argc, char *argv[])
{ 
  if (argc != 3) {
    printf("error: usage: %s <assembly-code-file> <machine-code-file>\n", argv[0]);
    exit(1);
  }
  inFileString = argv[1];
  outFileString = argv[2];
  
  if (!(inFilePtr = fopen(inFileString, "r"))) {
    printf("error in opening %s\n", inFileString);
    exit(1);
  }
  if (!(outFilePtr = fopen(outFileString, "w"))) {
    printf("error in opening %s\n", outFileString);
    fclose(inFilePtr);
    exit(1);
  }

  setInstructions();
  translateToMachineCode();
  return 0;
}

int 
readAndParse(FILE *inFilePtr, char *label, char *opcode, 
          char *arg0, char *arg1, char *arg2)
{
  char line[MAXLINELENGTH];
  char *ptr = line;

  label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';
  if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
    return 0;
  }

  if (strchr(line, '\n') == NULL) {
    printf("error: line too long\n");
    exit(1);
  }

  ptr = line;
  if (sscanf(ptr, "%[^\t\n\r ]", label)) {
    ptr += strlen(label);
  }

  sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]", opcode, arg0, arg1, arg2);
  return 1;
}

int 
isNumber(const char *string)
{
  int i;
  return (sscanf(string, "%d", &i)) == 1;
}

void
throwError(char *string)
{
  printf("%s\n", string);
  fclose(inFilePtr);
  fclose(outFilePtr);
  exit(1);
}

int
isEmptyString(const char *string)
{
  return ((!string) || (string[0] == '\0'));
}

void
setInstructions(void)
{
  char label[MAXLINELENGTH];
  char opcode[MAXLINELENGTH];
  char arg0[MAXLINELENGTH];
  char arg1[MAXLINELENGTH];
  char arg2[MAXLINELENGTH];

  lastAddress = 0;
  for (; readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2); lastAddress++)
  {
    if (!isEmptyString(label)) {
      storeLabelAndAddress(label);
    }
    strcpy(instruction[lastAddress].opcode, opcode);
    strcpy(instruction[lastAddress].arg0, arg0);
    strcpy(instruction[lastAddress].arg1, arg1);
    strcpy(instruction[lastAddress].arg2, arg2);
  }
}

void
storeLabelAndAddress(char *label)
{
  uint32_t i;
  for (i = 0; i < countsOfLabel; i++) {
    if (!strcmp(label_addr[i].label, label)) {
      throwError("duplicate label.");
    }
  }
  label_addr[countsOfLabel].addr = lastAddress;
  strcpy(label_addr[countsOfLabel++].label, label);
}

int 
findAddrOfLabel(const char *label)
{
  uint32_t i;
  for (i = 0; i < countsOfLabel; i++)
  {
    int isEqual = !strcmp(label_addr[i].label, label);
    if (isEqual) {
      return label_addr[i].addr;
    }
  }
  throwError("non-exist label.");
  return -1;
}

void
debugInstructions(void)
{
  uint32_t i;
  printf("======================\n");
  for (i = 0; i < lastAddress; i++)
  {
    instruction_t *temp = &instruction[i];
    printf("%s %s %s %s\n", temp->opcode, temp->arg0, temp->arg1, temp->arg2);
  }

  for (i = 0; i < countsOfLabel; i++) {
    label_addr_t *temp = &label_addr[i];
    printf("%s %d\n", temp->label, temp->addr);
  }
}

void
translateToMachineCode(void)
{
  currentAddress = 0;
  for (; currentAddress < lastAddress; currentAddress++)
    processFormat(&instruction[currentAddress]);
}

void
processFormat(instruction_t *instr)
{
  char *opcode, *arg0, *arg1, *arg2;
  int32_t intArg0, intArg1, intArg2;
  uint32_t machine_code;
  if (!instr) {
    throwError("null pointer exception. (processFormat)");
  }
  
  opcode = instr->opcode;
  arg0 = instr->arg0;
  arg1 = instr->arg1;
  arg2 = instr->arg2;

  if (!strcmp(opcode, ".fill")) {
    if (strlen(arg0) == 0) {
      throwError("Not enough arguments.");
    }
    intArg0 = isNumber(arg0) ? atoi(arg0) : findAddrOfLabel(arg0);
    fprintf(outFilePtr, "%d\n", intArg0);
  } else {
    machine_code = translateToWhichFormat(instr);
    fprintf(outFilePtr, "%d\n", machine_code);
  }
}


uint32_t
translateToWhichFormat(instruction_t* instr) {
  const char *opcode = instr->opcode;
  const char *arg0 = instr->arg0;
  const char *arg1 = instr->arg1;
  const char *arg2 = instr->arg2;

  if (isEmptyString(opcode)) {
    throwError("Empty opcode. (translateToWhichFormat)");
  }
  opcode_t OPCODE = getOpcode(opcode);
  if (OPCODE <= 1)
    return translateToR(OPCODE, arg0, arg1, arg2);
  if (OPCODE <= 4)
    return translateToI(OPCODE, arg0, arg1, arg2);
  if (OPCODE == 5)
    return translateToJ(OPCODE, arg0, arg1, arg2);
  return translateToO(OPCODE, arg0, arg1, arg2);
}

opcode_t getOpcode(const char* opcode) {
  if (!strcmp(opcode, "add"))
    return ADD;
  if (!strcmp(opcode, "nor"))
    return NOR;
  if (!strcmp(opcode, "lw"))
    return LW;
  if (!strcmp(opcode, "sw"))
    return SW;
  if (!strcmp(opcode, "beq"))
    return BEQ;
  if (!strcmp(opcode, "jalr"))
    return JALR;
  if (!strcmp(opcode, "halt"))
    return HALT;
  if (!strcmp(opcode, "noop"))
    return NOOP;
  throwError("Unrecognized opcode.");
}

uint32_t
translateToR(opcode_t OPCODE, const char* arg0, const char* arg1, const char* arg2)
{
  uint32_t machine_code;
  if (isEmptyString(arg0) || isEmptyString(arg1) || isEmptyString(arg2)) {
    throwError("Not Enough argument.");
  }
  if (!isNumber(arg0) || !isNumber(arg1) || !isNumber(arg2)) {
    throwError("Not Number argument.");
  }
  machine_code = 0;
  machine_code |= (OPCODE << 22);
  machine_code |= (atoi(arg0) << 19);
  machine_code |= (atoi(arg1) << 16);
  machine_code |= atoi(arg2);
  return machine_code;
}

uint32_t
translateToI(opcode_t OPCODE, const char* arg0, const char* arg1, const char* arg2)
{
  uint32_t machine_code;
  if (isEmptyString(arg0) || isEmptyString(arg1) || isEmptyString(arg2)) {
    throwError("Not Enough argument.");
  }
  if (!isNumber(arg0) || !isNumber(arg1)) {
    throwError("Not Number argument.");
  }
  machine_code = 0;
  if (isNumber(arg2)) {
    int32_t offset = atoi(arg2);
    if ((offset > 32767) || (offset < -32768)) {
      throwError("Offset is out of range.");
    }
    int32_t mask = 0b1111111111111111;
    machine_code |= offset & mask;
  } else {
    int32_t labelAddr = findAddrOfLabel(arg2);
    
    int16_t offset = (OPCODE != BEQ) ? labelAddr : labelAddr - currentAddress - 1;
    int32_t mask = 0b1111111111111111;
    machine_code |= offset & mask;
  }
  machine_code |= (OPCODE << 22);
  machine_code |= (atoi(arg0) << 19);
  machine_code |= (atoi(arg1) << 16);
  return machine_code;
}

uint32_t
translateToJ(opcode_t OPCODE, const char* arg0, const char* arg1, const char* arg2)
{
  uint32_t machine_code;
  if (isEmptyString(arg0) || isEmptyString(arg1)) {
    throwError("Not Enough argument.");
  }
  if (!isNumber(arg0) || !isNumber(arg1)) {
    throwError("Not Number argument.");
  }
  machine_code = 0;
  machine_code |= (OPCODE << 22);
  machine_code |= (atoi(arg0) << 19);
  machine_code |= (atoi(arg1) << 16);
  return machine_code;
}

uint32_t
translateToO(opcode_t OPCODE, const char* arg0, const char* arg1, const char* arg2)
{
  uint32_t machine_code;
  machine_code = 0;
  machine_code |= (OPCODE << 22);
  return machine_code;
}