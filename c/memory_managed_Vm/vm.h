// cpu modeling opcode+operand

#ifndef VM_H
#define VM_H

typedef enum {
    OP_PUSH_INT,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_PRINT,
    OP_HALT

} OpCode;

typedef struct {
    OpCode op;
    int operand;   
} Instruction;

#endif 