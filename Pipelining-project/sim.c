#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUMMEMORY 65536 /* maximum number of data words in memory */
#define NUMREGS 8 /* number of machine registers */

#define ADD 0
#define NAND 1
#define LW 2
#define SW 3
#define BEQ 4
#define JALR 5 /* JALR – not implemented in this project */
#define HALT 6
#define NOOP 7

#define NOOPINSTRUCTION 0x1c00000

typedef struct IFIDstruct{
	int instr;
	int pcplus1;
} IFIDType;

typedef struct IDEXstruct{
	int instr;
	int pcplus1;
	int readregA;
	int readregB;
	int offset;
} IDEXType;

typedef struct EXMEMstruct{
	int instr;
	int branchtarget;
	int aluresult;
	int readreg;
} EXMEMType;

typedef struct MEMWBstruct{
	int instr;
	int writedata;
} MEMWBType;

typedef struct WBENDstruct{
	int instr;
	int writedata;
} WBENDType;

typedef struct statestruct{
	int pc;
	int instrmem[NUMMEMORY];
	int datamem[NUMMEMORY];
	int reg[NUMREGS];
	int numMemory;
	IFIDType IFID;
	IDEXType IDEX;
	EXMEMType EXMEM;
	MEMWBType MEMWB;
	WBENDType WBEND;
	int cycles;       /* Number of cycles run so far */
	int fetched;     /* Total number of instructions fetched */
	int retired;      /* Total number of completed instructions */
	int branches;  /* Total number of branches executed */
	int mispreds;  /* Number of branch mispredictions*/
} statetype;

int field0(int instruction){
	return( (instruction>>19) & 0x7);
}

int field1(int instruction){
	return( (instruction>>16) & 0x7);
}

int field2(int instruction){
	return(instruction & 0xFFFF);
}

int opcode(int instruction){
	return(instruction>>22);
}

int signExtend(int num){
	// convert a 16-bit number into a 32-bit integer
	if (num & (1<<15) ) {
		num -= (1<<16);
	}
	return num;
}


void fetch(statetype* newstate, statetype* state) {
	newstate->pc = state->pc+1;				//increments pc
	newstate->IFID.pcplus1 = state->pc+1;			//stores the value of pc+1
	newstate->IFID.instr = state->instrmem[state->pc];	//retreives instruction from instrmem[] and stores it

}
void decode(statetype* newstate, statetype* state) {
	newstate->IDEX.pcplus1 = state->IFID.pcplus1;
	newstate->IDEX.instr = state->IFID.instr;
	newstate->IDEX.readregA = state->reg[field0(state->IFID.instr)];//contents of register from field 0 are stored
	newstate->IDEX.readregB = state->reg[field1(state->IFID.instr)];//contents of register from field 1 are stored
	newstate->IDEX.offset = signExtend(field2(state->IFID.instr));	//signExtend function used for field 2 to store as offset value

}
void execute(statetype* newstate, statetype* state) {//data hazards dealt with here onward
	int regA = state->IDEX.readregA;		//creates temp variable for regA
	int regB = state->IDEX.readregB;		//creates temp variable for regB
	newstate->EXMEM.instr = state->IDEX.instr;	//forwards instruction

	//*START OF DATA HAZARDS*//
	if((opcode(state->WBEND.instr) == ADD || opcode(state->WBEND.instr) == NAND || opcode(state->WBEND.instr) == LW) && opcode(state->IDEX.instr) != HALT && opcode(state->IDEX.instr) != NOOP) {
	//If opcode could allow for a data hazard
		if(opcode(state->WBEND.instr) == LW){//if WBEND instruction is a lw
			if(opcode(state->IDEX.instr) == ADD || opcode(state->IDEX.instr) == NAND){//and IDEX instruction is add or nand
				if(field0(state->WBEND.instr) == field0(state->IDEX.instr)){//check appropriate fields
					regA = state->WBEND.writedata;
				}
				if(field0(state->WBEND.instr) == field1(state->IDEX.instr)){
					regB = state->WBEND.writedata;
				}
			} else {//IDEX instruction is lw, sw or beq
				if(field0(state->WBEND.instr) == field0(state->IDEX.instr)){//check appropriate fields
					regA = state->WBEND.writedata;
				}
				if(field0(state->WBEND.instr) == field1(state->IDEX.instr)){
					regB = state->WBEND.writedata;
				}
			}
		}
		else {//if WBEND instruction is add or nand
			if(opcode(state->IDEX.instr) == ADD || opcode(state->IDEX.instr) == NAND){//and IDEX instruction is add or nand
				if(field2(state->WBEND.instr) == field0(state->IDEX.instr)){//check appropriate fields
					regA = state->WBEND.writedata;
				}
				if(field2(state->WBEND.instr) == field1(state->IDEX.instr)){
					regB = state->WBEND.writedata;
				}
			} else {//IDEX instruction is lw, sw or bew
				if(field2(state->WBEND.instr) == field0(state->IDEX.instr)){//check appropriate fields
					regA = state->WBEND.writedata;
				}
				if(field2(state->WBEND.instr) == field1(state->IDEX.instr)){
					regB = state->WBEND.writedata;
				}
			}
		}
	}
	if((opcode(state->MEMWB.instr) == ADD || opcode(state->MEMWB.instr) == NAND || opcode(state->MEMWB.instr) == LW) && opcode(state->IDEX.instr) != HALT && opcode(state->IDEX.instr) != NOOP) {
	//checks opcode and fields for the following instruction, check for hazards in the MEMWB buffer
		if(opcode(state->MEMWB.instr) == LW){
			if(opcode(state->IDEX.instr) == ADD || opcode(state->IDEX.instr) == NAND){
				if(field0(state->MEMWB.instr) == field0(state->IDEX.instr)){
					regA = state->MEMWB.writedata;
				}
				if(field0(state->MEMWB.instr) == field1(state->IDEX.instr)){
					regB = state->MEMWB.writedata;
				}
			} else {
				if(field0(state->MEMWB.instr) == field0(state->IDEX.instr)){
					regA = state->MEMWB.writedata;
				}
				if(field0(state->MEMWB.instr) == field1(state->IDEX.instr)){
					regB = state->MEMWB.writedata;
				}
			}
		}
		else {//if MEMWB instr is add or nand
			if(opcode(state->IDEX.instr) == ADD || opcode(state->IDEX.instr) == NAND){
				if(field2(state->MEMWB.instr) == field0(state->IDEX.instr)){
					regA = state->MEMWB.writedata;
				}
				if(field2(state->MEMWB.instr) == field1(state->IDEX.instr)){
					regB = state->MEMWB.writedata;
				}
			} else {
				if(field2(state->MEMWB.instr) == field0(state->IDEX.instr)){
					regA = state->MEMWB.writedata;
				}
				if(field2(state->MEMWB.instr) == field1(state->IDEX.instr)){
					regB = state->MEMWB.writedata;
				}
			}
		}
	}
	if((opcode(state->EXMEM.instr) == ADD || opcode(state->EXMEM.instr) == NAND || opcode(state->EXMEM.instr) == LW) && opcode(state->IDEX.instr) != HALT && opcode(state->IDEX.instr) != NOOP) {
	//checks opcode and fields for the following instruction, check for hazards in the EXMEM buffer
		if(opcode(state->EXMEM.instr) == LW){
			if(opcode(state->IDEX.instr) == ADD || opcode(state->IDEX.instr) == NAND){
				if(field0(state->EXMEM.instr) == field0(state->IDEX.instr)){
					regA = state->EXMEM.aluresult;
				}
				if(field0(state->EXMEM.instr) == field1(state->IDEX.instr)){
					regB = state->EXMEM.aluresult;
				}
			} else {
				if(field0(state->EXMEM.instr) == field0(state->IDEX.instr)){
					regA = state->EXMEM.aluresult;
				}
				if(field0(state->EXMEM.instr) == field1(state->IDEX.instr)){
					regB = state->EXMEM.aluresult;
				}
			}
		}
		else {//if EXMEM instr is add or nand
			if(opcode(state->IDEX.instr) == ADD || opcode(state->IDEX.instr) == NAND){
				if(field2(state->EXMEM.instr) == field0(state->IDEX.instr)){
					regA = state->EXMEM.aluresult;
				}
				if(field2(state->EXMEM.instr) == field1(state->IDEX.instr)){
					regB = state->EXMEM.aluresult;
				}
			} else {
				if(field2(state->EXMEM.instr) == field0(state->IDEX.instr)){
					regA = state->EXMEM.aluresult;
				}
				if(field2(state->EXMEM.instr) == field1(state->IDEX.instr)){
					regB = state->EXMEM.aluresult;
				}
			}
		}
	}//end of data hazards


	if(opcode(newstate->EXMEM.instr) == LW || opcode(newstate->EXMEM.instr) == SW) {//if opcode is lw or sw, the aluresult is offset + regB
		newstate->EXMEM.aluresult = (state->IDEX.offset) + regB;
		newstate->EXMEM.readreg = regA;
	} else if (opcode(newstate->EXMEM.instr) == ADD) {				//if opcode is add, aluresult is contents of regA + contents of regB
		newstate->EXMEM.aluresult = regA + regB;
	}else if (opcode(newstate->EXMEM.instr) == NAND) {				//if opcode is nand, aluresult is !(regA&regB)
		newstate->EXMEM.aluresult = ~(regA & regB);
	} else if(opcode(newstate->EXMEM.instr) == BEQ){				//if opcode is BEQ, branchtarget = pcplus1+offset and aluresult is regA - regB
		newstate->EXMEM.branchtarget = state->IDEX.pcplus1 + state->IDEX.offset;
		newstate->EXMEM.aluresult = regA - regB;
	} else if(opcode(newstate->EXMEM.instr) == HALT || opcode(newstate->EXMEM.instr) == NOOP){ // if opcode is HALT or NOOP reset  aluresult
		newstate->EXMEM.aluresult = regA + regB;
	}

	if(opcode(state->EXMEM.instr) == LW && opcode(state->IDEX.instr) != HALT && opcode(state->IDEX.instr) != NOOP) {
	//If the instruction is a LW and the following instruction is not a HALT or a NOOP
		if(field0(state->EXMEM.instr) == field1(state->IDEX.instr) || (field0(state->EXMEM.instr) == field0(state->IDEX.instr) && opcode(state->IDEX.instr) != LW)){
		//If load stall is needed based on the registers required
			newstate->pc = state->pc;     //decrement pc by 1 to refetch the same instruction that was just overwritten
			newstate->IFID = state->IFID; //Stall IFID buffer
			newstate->IDEX = state->IDEX; //Stall IDEX buffer

			newstate->EXMEM.instr = NOOPINSTRUCTION; //Insert a NOOP into EXMEM
			newstate->EXMEM.branchtarget = 0;	//Reset all variables in EXMEM
			newstate->EXMEM.aluresult = 0;
			newstate->EXMEM.readreg = 0;
			newstate->retired = state->retired-1;	//Decrememnts retired to account for extra noop
			newstate->fetched = state->fetched-1;	//Decrements fetched to account for the refetching of the new instruction that was overwritten
			newstate->IDEX.readregA = regA;
			newstate->IDEX.readregB = regB;
		}
	}
}
void memory(statetype* newstate, statetype* state) {
	newstate->MEMWB.instr = state->EXMEM.instr;				//Passes thr instruction along
	newstate->MEMWB.writedata = state->EXMEM.aluresult;			//Passes along the aluresult into writedata
	if(opcode(newstate->MEMWB.instr)==BEQ && newstate->MEMWB.writedata==0){ //if BEQ instruction and aluresult is 0, meaning contents of registers are the same
			newstate->mispreds = state->mispreds + 1;		//mispreds++
			newstate->retired = state->retired-3;			//retired - 3 to account for extra bubbles in pipeline
			newstate->pc = state->EXMEM.branchtarget;		//sets the pc to where the BEQ branches to
			//BRANCH, reset buffers in IFID, IDEX and EXMEM
			newstate->IFID.instr = NOOPINSTRUCTION;
			newstate->IDEX.instr = NOOPINSTRUCTION;
			newstate->EXMEM.instr = NOOPINSTRUCTION;
			newstate->IFID.pcplus1 = 0;
			newstate->IDEX.pcplus1 = 0;
			newstate->IDEX.readregA = 0;
			newstate->IDEX.readregB = 0;
			newstate->IDEX.offset = 0;
			newstate->EXMEM.branchtarget = 0;
			newstate->EXMEM.aluresult = 0;
			newstate->EXMEM.readreg = 0;
	} else if (opcode(newstate->MEMWB.instr)==SW) { //If the instruction is a SW, it sets the specified data memory to the appropriate value
		newstate->datamem[newstate->MEMWB.writedata] = state->EXMEM.readreg; //state->reg[field0(state->EXMEM.instr)];
	} else if(opcode(newstate->MEMWB.instr)==LW){ 	//If the instruciton is a LW, it sets the specified register to the appropriate value
		newstate->MEMWB.writedata =  state->datamem[state->EXMEM.aluresult];
	}
}
void write_back(statetype* newstate, statetype* state) {
	newstate->WBEND.instr = state->MEMWB.instr;	//Passes along the instruction
	newstate->WBEND.writedata = state->MEMWB.writedata; //Passes along the data to be written back in writedata
	if(opcode(newstate->WBEND.instr)==ADD || opcode(newstate->WBEND.instr)==NAND){ //If the instruction in the buffer is add or nand
		newstate->reg[field2(newstate->WBEND.instr)] = state->MEMWB.writedata; //Store the result in the appropriate value
	} else if(opcode(newstate->WBEND.instr) == LW){ //If the instruction is a LW
		newstate->reg[field0(newstate->WBEND.instr)] = state->MEMWB.writedata; //Sets a register to a given value 
	} else if(opcode(newstate->WBEND.instr) == BEQ){ //If the instruction is a BEQ
		newstate->branches = state->branches+1;  //Increments branches by 1
	}
}


void printInstruction(int instr){ //Supplied code for output
	char opcodeString[10];
	if (opcode(instr) == ADD) {
		strcpy(opcodeString, "add");
	} else if (opcode(instr) == NAND) {
		strcpy(opcodeString, "nand");
	} else if (opcode(instr) == LW) {
		strcpy(opcodeString, "lw");
	} else if (opcode(instr) == SW) {
		strcpy(opcodeString, "sw");
	} else if (opcode(instr) == BEQ) {
		strcpy(opcodeString, "beq");
	} else if (opcode(instr) == JALR) {
		strcpy(opcodeString, "jalr");
	} else if (opcode(instr) == HALT) {
		strcpy(opcodeString, "halt");
	} else if (opcode(instr) == NOOP) {
		strcpy(opcodeString, "noop");
	} else {
		strcpy(opcodeString, "data");
	}

	if(opcode(instr) == ADD || opcode(instr) == NAND){
		printf("%s %d %d %d\n", opcodeString, field2(instr), field0(instr), field1(instr));
	} else if(0 == strcmp(opcodeString, "data")){
		printf("%s %d\n", opcodeString, signExtend(field2(instr)));
	} else{
		printf("%s %d %d %d\n", opcodeString, field0(instr), field1(instr),
		signExtend(field2(instr)));
	}
}

void printstate(statetype* stateptr){ //Supplied code for printing each state
    int i;
    printf("\n@@@\nstate before cycle %d starts\n", stateptr->cycles);
    printf("\tpc %d\n", stateptr->pc);

    printf("\tdata memory:\n");
	for (i=0; i<stateptr->numMemory; i++) {
	    printf("\t\tdataMem[ %d ] %d\n", i, stateptr->datamem[i]);
	}
    printf("\tregisters:\n");
	for (i=0; i<NUMREGS; i++) {
	    printf("\t\treg[ %d ] %d\n", i, stateptr->reg[i]);
	}
    printf("\tIFID:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->IFID.instr);
	printf("\t\tpcPlus1 %d\n", stateptr->IFID.pcplus1);
    printf("\tIDEX:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->IDEX.instr);
	printf("\t\tpcPlus1 %d\n", stateptr->IDEX.pcplus1);
	printf("\t\treadRegA %d\n", stateptr->IDEX.readregA);
	printf("\t\treadRegB %d\n", stateptr->IDEX.readregB);
	printf("\t\toffset %d\n", stateptr->IDEX.offset);
    printf("\tEXMEM:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->EXMEM.instr);
	printf("\t\tbranchTarget %d\n", stateptr->EXMEM.branchtarget);
	printf("\t\taluResult %d\n", stateptr->EXMEM.aluresult);
	printf("\t\treadReg %d\n", stateptr->EXMEM.readreg);
    printf("\tMEMWB:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->MEMWB.instr);
	printf("\t\twriteData %d\n", stateptr->MEMWB.writedata);
    printf("\tWBEND:\n");
	printf("\t\tinstruction ");
	printInstruction(stateptr->WBEND.instr);
	printf("\t\twriteData %d\n", stateptr->WBEND.writedata);
}


void print_stats(statetype* state){ //Supplied code for printing the final stats
	printf("total of %d cycles executed\n", state->cycles);
	printf("total of %d instructions fetched\n", state->fetched);
	printf("total of %d instructions retired\n", state->retired);
	printf("total of %d branches executed\n", state->branches);
	printf("total of %d branch mispredictions\n", state->mispreds);
}

void run(statetype* newstate, statetype* state){ //Runs the buffers

//Initialize each buffer to NOOP
	// Primary loop
	while(1){
		printstate(state);

		/* check for halt */
		if(HALT == opcode(state->MEMWB.instr)) {
			printf("machine halted \n");
			print_stats(state);
			break;
		}
		*newstate = *state;
		newstate->cycles++;
		/*------------------ IF stage ----------------- */ // Intruction Fetch
		fetch(newstate, state);
		/*------------------ ID stage ----------------- */ // Intruction Decode
		decode(newstate, state);
		/*------------------ EX stage ----------------- */ //Execute
		execute(newstate, state);
		/*------------------ MEM stage ----------------- */ //Memory
		memory(newstate, state);
		/*------------------ WB stage ----------------- */ //Write back
		write_back(newstate, state);

		newstate->retired++;
		newstate->fetched++;

		newstate->cycles = state->cycles+1;
		*state = *newstate; 	/* this is the last statement before the end of the loop.
					It marks the end of the cycle and updates the current
					state with the values calculated in this cycle
					– AKA “Clock Tick”. */
	}
}

int main(int argc, char** argv){ //Main function that runs run and each printing function as well as reads in the file

	/** Get command line arguments **/
	char* fname;

	opterr = 0;

	int cin = 0;

	while((cin = getopt(argc, argv, "i:")) != -1){
		switch(cin)
		{
			case 'i':
				fname=(char*)malloc(strlen(optarg));
				fname[0] = '\0';

				strncpy(fname, optarg, strlen(optarg)+1);
				break;
			case '?':
				if(optopt == 'i'){
					printf("Option -%c requires an argument.\n", optopt);
				}
				else if(isprint(optopt)){
					printf("Unknown option `-%c'.\n", optopt);
				}
				else{
					printf("Unknown option character `\\x%x'.\n", optopt);
					return 1;
				}
				break;
			default:
				abort();
		}
	}

	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("Cannot open file '%s' : %s\n", fname, strerror(errno));
		return -1;
	}

	/* count the number of lines by counting newline characters */
	int line_count = 0;
	int c;
	while (EOF != (c=getc(fp))) {
		if ( c == '\n' ){
			line_count++;
		}
	}
	// reset fp to the beginning of the file
	rewind(fp);

	statetype* state = (statetype*)malloc(sizeof(statetype));
	statetype* newstate = (statetype*)malloc(sizeof(statetype));


	state->pc = 0;

	state-> fetched = -3;
	state-> cycles = 0;
	state-> retired = -3;
	state->	branches = 0;
	state-> mispreds = 0;
	//allocates memory for state

	memset(state->instrmem, 0, NUMMEMORY*sizeof(int));
	memset(state->datamem, 0, NUMMEMORY*sizeof(int));
	memset(state->reg, 0, NUMREGS*sizeof(int));

	//Initializes each variable in state to 0
	state->IFID.pcplus1= 0;
	state->IDEX.pcplus1= 0;
	state->IDEX.readregA= 0;
	state->IDEX.readregB= 0;
	state->IDEX.offset= 0;
	state->EXMEM.branchtarget= 0;
	state->EXMEM.aluresult= 0;
	state->EXMEM.readreg= 0;
	state->MEMWB.writedata= 0;
	state->WBEND.writedata= 0;

	//Initializes each instruction to NOOP
	state->IFID.instr = NOOPINSTRUCTION;
	state->IDEX.instr = NOOPINSTRUCTION;
	state->EXMEM.instr = NOOPINSTRUCTION;
	state->MEMWB.instr = NOOPINSTRUCTION;
	state->WBEND.instr = NOOPINSTRUCTION;

	//Initializes each variable in newstate to 0
	newstate->IFID.pcplus1= 0;
	newstate->IDEX.pcplus1= 0;
	newstate->IDEX.readregA= 0;
	newstate->IDEX.readregB= 0;
	newstate->IDEX.offset= 0;
	newstate->EXMEM.branchtarget= 0;
	newstate->EXMEM.aluresult= 0;
	newstate->EXMEM.readreg= 0;
	newstate->MEMWB.writedata= 0;
	newstate->WBEND.writedata= 0;

	//Initializes each instruction in state to NOOP
	newstate->IFID.instr = NOOPINSTRUCTION;
	newstate->IDEX.instr = NOOPINSTRUCTION;
	newstate->EXMEM.instr = NOOPINSTRUCTION;
	newstate->MEMWB.instr = NOOPINSTRUCTION;
	newstate->WBEND.instr = NOOPINSTRUCTION;

	state->numMemory = line_count;


	char line[256];

	int i = 0;
	while (fgets(line, sizeof(line), fp)) {
		/* note that fgets doesn't strip the terminating \n, checking its
		   presence would allow to handle lines longer that sizeof(line) */
		state->instrmem[i] = atoi(line);
		state->datamem[i] = atoi(line);
		i++;
	}
	fclose(fp);

	/** Run the simulation **/
	run(newstate, state);

	free(state);
	free(newstate);
	free(fname);

}
