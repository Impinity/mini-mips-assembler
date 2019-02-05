#include <stdio.h>
#include <string.h>

typedef struct {
    int address;
    char name [50];
}Label;

Label labList[100];
int labCount;

unsigned int getOpCode(char * instruction)
{
    if (strcmp(instruction, "add") == 0)
    {
        return 32;
    }
    else if (strcmp(instruction, "addi") == 0)
    {
        return 8;
    }
    else if (strcmp(instruction, "nor") == 0)
    {
        return 39;
    }
    else if (strcmp(instruction, "ori") == 0)
    {
        return 13;
    }
    else if (strcmp(instruction, "sll") == 0)
    {
        return 0;
    }
    else if (strcmp(instruction, "lui") == 0)
    {
        return 15;
    }
    else if (strcmp(instruction, "sw") == 0)
    {
        return 43;
    }
    else if (strcmp(instruction, "lw") == 0)
    {
        return 35;
    }
    else if (strcmp(instruction, "bne") == 0)
    {
        return 5;
    }
    else if (strcmp(instruction, "j") == 0)
    {
        return 2;
    }
    else if (strcmp(instruction, "la") == 0)
    {
        return 666;
    }
    else
    {
        return -1;
    }

}

// Simple helper function that helps us determine register numbers
unsigned short regType(char type)
{
    switch (type) {
        case 't':
            return 8;
        case 's':
            return 16;
        default:
            break;
    }
    return 0;
}

void translateAndPrint(unsigned int op, int PC, char * operands) {
    unsigned int result = 0;                                //32 bits enough to store instruction
    int immed, targaddr;
    unsigned short rs, rt, rd, shamt;     //short is 2 bytes, enough for 5 and 6 bit values
    char label [50];
    char rsType, rtType, rdType;

    switch(op) {
        // Instruction $rd, $rs, $rt
        case 32:
        case 39:
            sscanf(operands, "$%c%hu,$%c%hu,$%c%hu", &rdType, &rd, &rsType, &rs, &rtType, &rt);

            rd = rd + regType(rdType);
            rs = rs + regType(rsType);
            rt = rt + regType(rtType);

            result = result << 5;   // No op on R-types
            result = result | rs;
            result = result << 5;
            result = result | rt;
            result = result << 5;
            result = result | rd;
            result = result << 11;
            result = result | op;
            break;
            // Instruction $rt, $rs, immed
        case 8:
        case 13:
            sscanf(operands, "$%c%hu,$%c%hu,%d", &rtType, &rt, &rsType, &rs, &immed);
            rs = rs + regType(rsType);
            rt = rt + regType(rtType);

            result = result | op;
            result = result << 5;
            result = result | rs;
            result = result << 5;
            result = result | rt;
            result = result << 16;
            result = result | immed;
            break;
            // Instruction $rd, $rt, shamt
        case 0:
            sscanf(operands, "$%c%hu,$%c%hu,%hu", &rdType, &rd, &rtType, &rt, &shamt);
            rd = rd + regType(rdType);
            rt = rt + regType(rtType);

            result = result << 10;   //No rs or opcode
            result = result | rt;
            result = result << 5;
            result = result | rd;
            result = result << 5;
            result = result | shamt;
            result = result << 6;
            result = result | op;
            break;
            // lui $rt, immed
        case 15:
            sscanf(operands, "$%c%hu,%d", &rtType, &rt, &immed);
            rt = rt + regType(rtType);

            result = result | op;
            result = result << 10;  //no rs
            result = result | rt;
            result = result << 16;
            result = result | immed;
            break;
            //Instruction $rt,immed($rs)
        case 43:
        case 35:
            sscanf(operands, "$%c%hu,%d($%c%hu)", &rtType, &rt, &immed, &rsType, &rs);
            rs = rs + regType(rsType);
            rt = rt + regType(rtType);

            result = result | op;
            result = result << 5;
            result = result | rs;
            result = result << 5;
            result = result | rt;
            result = result << 16;
            result = result | immed;
            break;
        // bne $rs,$rt,label
        case 5:
            sscanf(operands, "$%c%hu,$%c%hu,%s", &rsType, &rs, &rtType, &rt, label);
            rs = rs + regType(rsType);
            rt = rt + regType(rtType);

            for (int i = 0; i < labCount; i++) {
                if (strcmp(labList[i].name, label) == 0) {
                    immed = (PC - labList[i].address) / 4;
                }
            }
            result = result | op;
            result = result << 5;
            result = result | rs;
            result = result << 5;
            result = result | rt;
            result = result << 16;
            result = result | immed;
            break;
            // j label
        case 2:
            for (int i = 0; i < labCount; i++) {
                if (strcmp(labList[i].name, operands) == 0) {
                    targaddr = labList[i].address << 4;
                    targaddr = targaddr >> 6;
                }
            }

            result = result | op;
            result = result << 26;
            result = result | targaddr;
            break;
            //la $rx, label
        case 666:
            sscanf(operands, "$%c%hu,%s", &rtType, &rt, label);
            rt = rt + regType(rtType);
            targaddr = 0;
            for (int i = 0; i < labCount; i++) {
                if (strcmp(labList[i].name, label) == 0) {
                    targaddr = labList[i].address;
                    break;
                }
            }

            //Lui
            result = result | 15;
            result = result << 10;  //no rs
            result = result | 1;    //$at
            result = result << 16;
            result = result | (targaddr >> 16);
            printf("0x%.8X: 0x%.8X\n", PC, result);

            result = 0;
            //Ori
            result = result | 13;
            result = result << 5;
            result = result | 1;
            result = result << 5;
            result = result | rt;
            result = result << 16;
            targaddr = targaddr & 0xFFFF;
            result = result | targaddr;

            printf("0x%.8X: 0x%.8X\n", PC + 4, result);
            return;
        default:
            printf("Error\n");
            return;
    }
    printf("0x%.8X: 0x%.8X\n", PC, result);
}

int main() {
    char fileLine [100];                    // Represents one line of the file
    char tok1[50], tok2[25], tok3[25];
    int opCode, textSection = 0, PC = 0;
    freopen("C:\\Users\\jrb14\\CLionProjects\\CompOrgProj1\\test.txt","r",stdin);    // To be removed

    labCount = 0;
    //First pass, gets address of labels
    while (fgets(fileLine, 100, stdin))
    {
        memset(tok1, 0, sizeof(tok1));
        memset(tok2, 0, sizeof(tok2));
        memset(tok3, 0, sizeof(tok3));
        sscanf(fileLine, "%s\t%s\t%s", tok1, tok2, tok3);

        if (textSection == 1) {
            //Adding label and address to the label list
            if (tok1[strlen(tok1) - 1] == ':') {
                tok1[strlen(tok1) - 1] = '\0';
                //printf("name: %s\taddress: %d\n", tok1, PC);
                strcpy(labList[labCount].name, tok1);
                labList[labCount].address = PC;
                labCount++;
                // la is a pseudo instruction that is actually two therefore we increase PC once more in this scenario
                if(strcmp(tok2, "la") == 0)
                {
                    PC +=4;
                }
            }
            else if (strcmp(tok1, "la") == 0)
            {
                PC +=4;
            }
            if(strcmp(tok1, ".data") != 0)
                PC += 4;
        }
        if (strcmp(tok1,".text") == 0)
        {
            textSection = 1;
        }
    }

    rewind(stdin);

    textSection = 0;
    PC = 0;
    // Second pass, translates
    while (fgets(fileLine, 100, stdin))
    {
        opCode = 0;
        memset(tok1, 0, sizeof(tok1));
        memset(tok2, 0, sizeof(tok2));
        memset(tok3, 0, sizeof(tok3));
        int tokNum = sscanf(fileLine, "%s\t%s\t%s", tok1, tok2, tok3);
        if (strcmp(tok1,".data") == 0)
        {
            textSection = 0;
        }
        //Only instructions
        if(textSection == 1)
        {
            /* tok1 - label
             * tok2 - instruction
             * tok3 - operand(s)
             */
            if(tokNum == 3) {
                opCode = getOpCode(tok2);
                translateAndPrint(opCode, PC, tok3);
                if (opCode == 666)          //Case of la
                    PC+=4;
            }
            /* tok1 - instruction
             * tok2 - operand(s)
             */
            else if (tokNum == 2)
            {
                opCode = getOpCode(tok1);
                //printf("%s %s\n", tok1, tok2);
                translateAndPrint(opCode, PC, tok2);
                if (opCode == 666)          //Case of la
                    PC+=4;
            }
            if (opCode >= 0)
                PC += 4;
        }
        // Indicates the text section, all instructions
        if (strcmp(tok1,".text") == 0)
        {
            textSection = 1;
        }


    }
    return 0;
}