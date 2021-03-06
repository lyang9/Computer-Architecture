#include "cpu.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define DATA_LEN 6

unsigned char cpu_ram_read(struct cpu *cpu, unsigned char mar)
{
  return cpu->ram[mar];
}

void cpu_ram_write(struct cpu *cpu, unsigned char mar, unsigned char mdr)
{
  cpu->ram[mar] = mdr;
}

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *filename)
{
  FILE *fp = fopen(filename, "r");
  char line[1024];
  unsigned char addr = 0x00;

  if (fp == NULL) {
    fprintf(stderr, "error opening file %s\n", filename);
    exit(2);
  }

  while (fgets(line, sizeof line, fp) != NULL) {
    char *endptr = NULL;

    unsigned char b = strtoul(line, &endptr, 2);

    if (endptr == line) {
      // we got no numbers
      continue;
    }

    cpu_ram_write(cpu, addr++, b);
  }

  fclose(fp);

#if 0
  char data[DATA_LEN] = {
    // From print8.ls8
    0b10000010, // LDI R0,8
    0b00000000,
    0b00001000,
    0b01000111, // PRN R0
    0b00000000,
    0b00000001  // HLT
  };

  int address = 0;

  for (int i = 0; i < DATA_LEN; i++) {
    cpu->ram[address++] = data[i];
  }

  // TODO: Replace this with something less hard-coded
#endif
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{
  switch (op) {
    case ALU_MUL:
      // TODO
      cpu->reg[regA] *= cpu->reg[regB];
      break;

    // TODO: implement more ALU ops
    case ALU_ADD:
      cpu->reg[regA] += cpu->reg[regB];
      break;

    // SPRINT
    // case ALU_CMP:
    //   if (cpu->reg[regA] == cpu->reg[regB]) {
    //     cpu->FL = 1;
    //   } 
    //   if (cpu->reg[regA] > cpu->reg[regB]) {
    //     cpu->FL = 2;
    //   } 
    //   if (cpu->reg[regA] < cpu->reg[regB]) {
    //     cpu->FL = 4;
    //   }
    //   break;
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction
  unsigned char IR, operandA, operandB;

  while (running) {
    // TODO
    // 1. Get the value of the current instruction (in address PC).
    IR = cpu_ram_read(cpu, cpu->PC);
    operandA = cpu_ram_read(cpu, (cpu->PC+1) & 0xff);
    operandB = cpu_ram_read(cpu, (cpu->PC+2) & 0xff);

    int add_to_pc = (IR >> 6) + 1;

    printf("TRACE: %02X | %02X %02X %02X |", cpu->PC, IR, operandA, operandB);

    for (int i = 0; i < 8; i++) {
      printf(" %02X", cpu->reg[i]);
    }

    printf("\n");

    // 2. Figure out how many operands this next instruction requires
    // 3. Get the appropriate value(s) of the operands following this instruction
    // 4. switch() over it to decide on a course of action.
    switch(IR) {
      case HLT:
        running = 0;
        // cpu->PC += 1;
        break;

      case LDI:
        cpu->reg[operandA] = operandB;
        // cpu->PC += 3;
        break;

      case PRN:
        printf("%d\n", cpu->reg[operandA & 7]);
        // cpu->PC += 2;
        break;

      case MUL:
        alu(cpu, ALU_MUL, operandA, operandB);
        break;

      case ADD:
        alu(cpu, ALU_ADD, operandA, operandB);
        break;

      case POP:
        cpu->reg[operandA & 7] = cpu_ram_read(cpu, cpu->reg[SP]);
        cpu->reg[SP]++;
        break;

      case PUSH:
        cpu->reg[SP]--;
        cpu_ram_write(cpu, cpu->reg[SP], cpu->reg[operandA & 7]);
        break;

      case CALL:
        cpu->reg[SP]--;
        cpu_ram_write(cpu, cpu->reg[SP], cpu->PC + add_to_pc);
        break;

      // SPRINT 
      case CMP:
        if (cpu->reg[operandA & 7] == cpu->reg[operandB & 7]) {
          cpu->FL = 1; 
        } else if (cpu->reg[operandA & 7] > cpu->reg[operandB & 7]) {
          cpu->FL = 2;
        } else {
          cpu->FL = 4;
        } 
        break;

      case JMP:
        cpu->PC = cpu->reg[operandA & 7];
        add_to_pc = 0;
        break;

      case JEQ:
        if (cpu->FL == 1) {
          cpu->PC = cpu->reg[operandA & 7];
        }
        break;

       case JNE:
        if (cpu->FL == 0) {
          cpu->PC = cpu->reg[operandA & 7];
        }
        break;

      default:
        printf("Unknown instruction %02x\n", IR);
        // exit(3);
    }
    // 5. Do whatever the instruction should do according to the spec.
    // 6. Move the PC to the next instruction.
    cpu->PC += add_to_pc;
    cpu->PC &= 0xff;
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->PC = 0;
  cpu->FL = 0;

  // Zero registers and RAM
  memset(cpu->ram, 0, sizeof cpu->ram);
  memset(cpu->reg, 0, sizeof cpu->reg);
}