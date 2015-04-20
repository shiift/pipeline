#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct instruction_struct{
    int size;
    unsigned int address[256];
    unsigned int code[256];
    unsigned int opcode[256];
    unsigned int funct[256];
    char instrformat[256]; // I, J, R
    char regdst[256];
    char jump[256];
    char branch[256];
    char memread[256];
    char memtoreg[256];
    char memwrite[256];
    char alusrc[256];
    char regwrite[256];
    unsigned int read_register_1[256];
    unsigned int read_register_2[256];
    unsigned int write_register[256];
    unsigned int sign_extend_output[256];
    unsigned int alu_control_output[256];
    unsigned int btarget[256];
    unsigned int jump_address[256];
} instruction_set;

// only works up to base 10
char *itoa(int i, int base){
    static char buf[1024];
    char *p = buf + 1023;
    do{
        *--p = '0' + (i % base);
        i /= base;
    }while (i != 0);
    return p;
}


void parse_input(instruction_set* inst, char* file){
    char input[256][256];

    FILE* plist = fopen(file, "r");
    int i, lines = 0;
    while(fgets(input[lines], sizeof(input[lines]), plist))
        lines++;
    for(i = 2; i < lines; i++){
        sscanf(input[i],"0x%X 0x%X", &inst->address[i-2], &inst->code[i-2]);
    }
    inst->size = i-2;
}

void calc_code(instruction_set* inst){
    int i, size = inst->size;
    for(i = 0; i < size; i++){
        unsigned int opcode = inst->code[i] >> 26;
        inst->opcode[i] = opcode;
        if(opcode == 0){
            inst->instrformat[i] = 'R';
        }else if(opcode == 2 || opcode == 3){
            inst->instrformat[i] = 'J';
        }else{
            inst->instrformat[i] = 'I';
        }
        inst->funct[i] = inst->code[i] & 0x3F;
    }
}

void calc_control(instruction_set* inst){
    int i, size = inst->size;
    for(i = 0; i < size; i++){
        unsigned int opcode = inst->opcode[i];
        switch(inst->instrformat[i]){
            case 'R':
                inst->regdst[i] = 1;
                inst->jump[i] = 0;
                inst->branch[i] = 0;
                inst->memread[i] = 0;
                inst->memtoreg[i] = 0;
                inst->memwrite[i] = 0;
                inst->alusrc[i] = 0;
                inst->regwrite[i] = 1;
                break;
            case 'I':
                inst->regdst[i] = 0;
                inst->jump[i] = 0;
                if(opcode == 0x23){           // lw
                    inst->branch[i] = 0;
                    inst->memread[i] = 1;
                    inst->memtoreg[i] = 1;
                    inst->memwrite[i] = 0;
                    inst->alusrc[i] = 1;
                    inst->regwrite[i] = 1;
                }else if(opcode == 0x2B){     // sw
                    inst->branch[i] = 0;
                    inst->memread[i] = 0;
                    inst->memtoreg[i] = 0;
                    inst->memwrite[i] = 1;
                    inst->alusrc[i] = 1;
                    inst->regwrite[i] = 0;
                }else if(opcode == 0x04){     // beq
                    inst->branch[i] = 1;
                    inst->memread[i] = 0;
                    inst->memtoreg[i] = 0;
                    inst->memwrite[i] = 0;
                    inst->alusrc[i] = 0;
                    inst->regwrite[i] = 0;
                }else{
                    printf("invalid instruction: %d\n", opcode);
                    exit(-1);
                }
                break;
            case 'J':
                inst->regdst[i] = 0;
                inst->jump[i] = 1;
                inst->branch[i] = 0;
                inst->memread[i] = 0;
                inst->memtoreg[i] = 0;
                inst->memwrite[i] = 0;
                inst->alusrc[i] = 0;
                inst->regwrite[i] = 0;
                break;
        }
    }
}

void get_registers(instruction_set* inst){
    int i, size = inst->size;
    for(i = 0; i < size; i++){
        unsigned int opcode = inst->opcode[i];
        unsigned int extension = 0;
        if(inst->code[i] & 0x8000)
            extension = 0xFFFF0000;
        inst->read_register_1[i] = (inst->code[i] >> 21) & 0x1F;
        inst->read_register_2[i] = (inst->code[i] >> 16) & 0x1F;
        if(inst->regdst[i])
            inst->write_register[i] = (inst->code[i] >> 11) & 0x1F;
        else
            inst->write_register[i] = inst->read_register_2[i];
        inst->sign_extend_output[i] = (inst->code[i] & 0xFFFF) + extension;
    }
}

void get_alu_control(instruction_set* inst){
    int i, size = inst->size;
    for(i = 0; i < size; i++){
        unsigned int opcode = inst->opcode[i];
        switch(opcode){
            case 0x23: // lw
                inst->alu_control_output[i] = 2;
                break;
            case 0x2B: // sw
                inst->alu_control_output[i] = 2;
                break;
            case 0x04: // branch
                inst->alu_control_output[i] = 6;
                break;
            case 0x00: // R-type
                switch(inst->funct[i]){
                    case 32: // add
                        inst->alu_control_output[i] = 2;
                        break;
                    case 34: // subtract
                        inst->alu_control_output[i] = 6;
                        break;
                    case 36: // AND
                        inst->alu_control_output[i] = 0;
                        break;
                    case 37: // OR
                        inst->alu_control_output[i] = 1;
                        break;
                    case 42: // SLT
                        inst->alu_control_output[i] = 7;
                        break;
                }
                break;
            default: // J-Type
                inst->alu_control_output[i] = 0;
        }
    }
}

void calc_btarget_jump(instruction_set* inst){
    int i, size = inst->size;
    for(i = 0; i < size; i++){
        unsigned int add1 = inst->address[i] + 4;
        unsigned int add2 = inst->sign_extend_output[i] << 2;
        inst->btarget[i] = add1 + add2;
        unsigned int inst_shift = (inst->code[i] & 0x3FFFFFF) << 2;
        inst->jump_address[i] = (add1 & 0xF0000000) + inst_shift; 
    }
}

int main(int argc, char* argv[]){

    if(argc != 2){
        printf("Format: ./signals input.txt\n");
        exit(0);
    }
    char* file = argv[1];

    instruction_set* inst = (instruction_set*)malloc(sizeof(instruction_set));

    parse_input(inst, file);    // instruction_address, instruction_encoding
    calc_code(inst);            // OPCode, Funct, InstrFormat
    calc_control(inst);         // RegDist, Jump, Branch, MemRead, MemtoReg, MemWrite, ALUSrc, RegWrite
    get_registers(inst);        // Read_register_1, Read_register_2, Write_register, Sign_extend_output
    get_alu_control(inst);      // ALU_control_output
    calc_btarget_jump(inst);    // BTarget

    int i;
    for(i = 0; i < inst->size; i++){
        printf("#====================\n");
        printf("instruction_address=0x%08X\n", inst->address[i]);
        printf("instruction_encoding=0x%08X\n", inst->code[i]);
        printf("OPCode=%06s\n", itoa(inst->opcode[i],2));
        printf("Funct=%06s\n", itoa(inst->funct[i],2));
        printf("InstrFormat=%c-type\n", inst->instrformat[i]);
        printf("RegDst=%d\n", inst->regdst[i]);
        printf("Jump=%d\n", inst->jump[i]);
        printf("Branch=%d\n", inst->branch[i]);
        printf("MemRead=%d\n", inst->memread[i]);
        printf("MemtoReg=%d\n", inst->memtoreg[i]);
        printf("MemWrite=%d\n", inst->memwrite[i]);
        printf("ALUSrc=%d\n", inst->alusrc[i]);
        printf("RegWrite=%d\n", inst->regwrite[i]);
        printf("Read_register_1=%05s %d\n", itoa(inst->read_register_1[i],2), inst->read_register_1[i]);
        printf("Read_register_2=%05s %d\n", itoa(inst->read_register_2[i],2), inst->read_register_2[i]);
        printf("Write_register=%05s %d\n", itoa(inst->write_register[i],2), inst->write_register[i]);
        printf("Sign_extend_output=0x%08X\n", inst->sign_extend_output[i]);
        printf("ALU_control_output=%04s\n", itoa(inst->alu_control_output[i],2));
        printf("BTarget=0x%08X\n", inst->btarget[i]);
        printf("Jump_address=0x%08X\n", inst->jump_address[i]);
    }
    free(inst);
}
