// Jean Remy Bougeois-Cruz
// School: jrb14b@my.fsu.edu
// Personal: jrb14b@impinity.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	int address;
	char name [50];
}Label;

Label labList[100];
int labCount;

// Take an operands string and converts an $0 to $x0 to be compatible with my translation function.
char * convertZeroes(char * str)
{
	int i, rCount = 0;
	char * returnVal;
	char converted [50];
	memset(converted, 0, sizeof(converted));
	for(i = 0; i < strlen(str); i++)
	{
		if(str[i] == '$' && str[i + 1] == '0')
		{
			converted[rCount] = '$';
			converted[rCount+1] = 'x';
			converted[rCount+2] = '0';
			rCount +=3;
			i +=1;
		}
		else
		{
			converted[rCount] = str[i];
			rCount++;
		}
	}
	converted[rCount] = '\0';
	returnVal = malloc(strlen(converted));
	returnVal = converted;
	return returnVal;
}

// Helper function to get the op code of an instruction. Otherwise I can't group multiple instructions together in
// the translation function as strings cant be used in switch statements.
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
	else if (strcmp(instruction, "beq") == 0)
	{
		return 4;
	}
	else if (strcmp(instruction, "la") == 0)
	{
		return 666;
	}
	else
	{
		return 1;
	}

}

// Determines register number based on character before number ($t0-$t7 or $s0-$s7)
// Otherwise we know it's a $0
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
	unsigned int result = 0;				// Variable for the final instruction
	int immed, targaddr, i;					// For I-Type instructions
	unsigned short rs, rt, rd, shamt;		// Variables for instruction fields
	char label [50];                        // For labels during jump and branch
	char rsType, rtType, rdType;            // Char for letter part of register (t in $t7)

	switch(op) {
		/**
		 * ADD, NOR
		 * R - Type
		 * Instruction $rd, $rs, $rt
		 */
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
		/**
		 * ADDI, ORI
		 * I - Type
		 * Instruction $rt, $rs, immed
		 */
		case 8:
		case 13:
			sscanf(operands, "$%c%hu,$%c%hu,%d", &rtType, &rt, &rsType, &rs, &immed);

			rs = rs + regType(rsType);
			rt = rt + regType(rtType);

			immed = immed & 0xFFFF;

			result = result | op;
			result = result << 5;
			result = result | rs;
			result = result << 5;
			result = result | rt;
			result = result << 16;
			// ADD 0 EXTENTION FOR ORI
			result = result | immed;
			break;
		/**
		 * SLL
		 * R - Type
		 * Instruction $rd, $rt, shamt
		 */
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
		/**
		 * LUI
		 * I - Type
		 * Instruction $rt, immed
		 */
		case 15:
			sscanf(operands, "$%c%hu,%d", &rtType, &rt, &immed);
			rt = rt + regType(rtType);
			immed = immed & 0xFFFF;

			result = result | op;
			result = result << 10;  //no rs
			result = result | rt;
			result = result << 16;
			result = result | immed;
			break;
		/**
		 * SW, LW
		 * I - Type
		 * Instruction $rt, immed($rs)
		 */
		case 43:
		case 35:
			sscanf(operands, "$%c%hu,%d($%c%hu)", &rtType, &rt, &immed, &rsType, &rs);
			rs = rs + regType(rsType);
			rt = rt + regType(rtType);

			immed = immed & 0xFFFF;

			result = result | op;
			result = result << 5;
			result = result | rs;
			result = result << 5;
			result = result | rt;
			result = result << 16;
			result = result | immed;
			break;
		/**
		 * BNE, BEQ
		 * I - Type
		 * Instruction $rs, $rt, label
		 */
		case 4:
		case 5:
			sscanf(operands, "$%c%hu,$%c%hu,%s", &rsType, &rs, &rtType, &rt, label);
			rs = rs + regType(rsType);
			rt = rt + regType(rtType);

			for (i = 0; i < labCount; i++) {
				if (strcmp(labList[i].name, label) == 0) {
					immed = (labList[i].address - (PC)) / 4;
				}
			}
			immed -= 1;     // PC + 1 relative addressing

			immed = immed & 0xFFFF;

			result = result | op;
			result = result << 5;
			result = result | rs;
			result = result << 5;
			result = result | rt;
			result = result << 16;
			result = result | immed;
			break;
		/**
		 * J
		 * J - Type
		 * Instruction label
		 */
		case 2:
			for (i = 0; i < labCount; i++) {
				if (strcmp(labList[i].name, operands) == 0) {
					targaddr = labList[i].address << 4;
					targaddr = targaddr >> 6;
				}
			}

			result = result | op;
			result = result << 26;
			result = result | targaddr;
			break;
        /**
         * LA (LUI + ORI)
         * "I" - Type
         * Instruction label
         */
		case 666:
			sscanf(operands, "$%c%hu,%s", &rtType, &rt, label);
			int upper, lower;
			rt = rt + regType(rtType);
			targaddr = 0;
			for (i = 0; i < labCount; i++) {
				//strcpy(temper, labList[i].name);
				if (strcmp(labList[i].name, label) == 0) {
					targaddr = labList[i].address;
					break;
				}
			}

			upper = targaddr >> 16;

			lower = targaddr & 0xFFFF;

			//Lui
			result = result | 15;
			result = result << 10;  //no rs
			result = result | 1;    //$at
			result = result << 16;
			result = result | upper;
			printf("0x%.8X: 0x%.8X\n", PC, result);

			result = 0;
			//Ori
			result = result | 13;
			result = result << 5;
			result = result | 1;
			result = result << 5;
			result = result | rt;
			result = result << 16;
			result = result | lower;

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
	unsigned int opCode;
	int textSection = 0, PC = 0;
	labCount = 0;
	//First pass, gets address of labels
	while (fgets(fileLine, 100, stdin))
	{
		memset(tok1, 0, sizeof(tok1));
		memset(tok2, 0, sizeof(tok2));
		memset(tok3, 0, sizeof(tok3));
		sscanf(fileLine, "%s\t%s\t%s", tok1, tok2, tok3);
		if (textSection == 1) {
			// Adds label and address to the label list
			if (tok1[strlen(tok1) - 1] == ':') {
				tok1[strlen(tok1) - 1] = '\0';
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
				PC += 4;
			}


			if(strcmp(tok1, ".space") == 0)
			{
				char *ptr;
				PC += strtol(tok2, &ptr, 10);
			}
			else if(strcmp(tok2, ".space") == 0)
			{
				char *ptr;
				PC += strtol(tok3, &ptr, 10);
			}
			else if(strcmp(tok1, ".data") != 0)
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
				if(strstr(tok3, "$0") != NULL) {
					translateAndPrint(opCode, PC, convertZeroes(tok3));
				}
				else
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
				if(strstr(tok2, "$0") != NULL) {
					translateAndPrint(opCode, PC, convertZeroes(tok2));
				}
				else {
					translateAndPrint(opCode, PC, tok2);
				}
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