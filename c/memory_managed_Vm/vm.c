#include <stdio.h>
#include <stdlib.h>
#include "vm.h"

#define STACK_MAX 256
#define PROGRAM_MAX 256

typedef struct {
    Instruction program[PROGRAM_MAX];
    size_t ip;                 

    int stack[STACK_MAX];
    int sp;                    
} VM;

void push(VM *vm, int value){
    if(vm->sp >= STACK_MAX){
       fprintf(stderr,"stack overflow\n");
       exit(1);
    }
    vm->stack[vm->sp++] = value;
}

int pop(VM *vm){
    if(vm->sp <= 0){
       fprintf(stderr,"stack underflow\n");
       exit(1);
    }
    return vm->stack[--vm->sp];
}

void run(VM *vm) {
    for (;;) {
        Instruction instr = vm->program[vm->ip++];

        switch (instr.op) {

            case OP_PUSH_INT:
                push(vm, instr.operand);
                break;

            case OP_ADD: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a + b);
                break;
            }

            case OP_SUB: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a - b);
                break;
            }

            case OP_MUL: {
                int b = pop(vm);
                int a = pop(vm);
                push(vm, a * b);
                break;
            }

            case OP_DIV: {
                int b = pop(vm);
                int a = pop(vm);
                if (b == 0) {
                    fprintf(stderr, "Division by zero\n");
                    exit(1);
                }
                push(vm, a / b);
                break;
            }

            case OP_PRINT: {
                int value = pop(vm);
                printf("%d\n", value);
                break;
            }

            case OP_HALT:
                return;
        }
    }
}

int main(void) {
    VM vm = {0};

    vm.program[0] = (Instruction){ OP_PUSH_INT, 3 };
    vm.program[1] = (Instruction){ OP_PUSH_INT, 4 };
    vm.program[2] = (Instruction){ OP_ADD, 0 };
    vm.program[3] = (Instruction){ OP_PRINT, 0 };
    vm.program[4] = (Instruction){ OP_HALT, 0 };

    run(&vm);
    return 0;
}
