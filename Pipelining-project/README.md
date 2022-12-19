<i> Authored by Tara Sothy and Matt Westerhaus </i>

This project was part of the CISC 340 Architecture class, this is a piplined simulator which can take machine code made by an assembler as input and execute the given assembly code. More details are provided below.

# **To run a file through the ref_asm**

1)./ref_asm -i input_file -o asm_output_file 
2a) ./sim -i asm_output_file OR 2b) ./ref_sim -i asm_output_file
If run through sim, the program would run through our pipelined simulator. If run through ref_sim, the program would run through the provided pipelined simulator.

# **Simulator**
Our simulator is a program designed to implement a pipelined processor that uses machine code made by the assembler to execute instructions.
It prints out the state of the program each cycle and prints the final number of executed, retired and fetched instructions as well as the 
number of performed cycles, branches and branch mispredictions. This program exits once a halt instruction is read from the MEMWB buffer.

# **Test Suite** 
The test suite is a directory that contains one supplied "class" file that is to be converted into machine code by ref_asm as well as several other tests 
that we wrote in order to thouroughly test the functionality and accuracy of our pipelined simulator this included dealing with data and control hazards.
Files were made to specifically test the following operations: ADD, NAND, SW, LW, BEQ, HALT, and NOOP which exludes JALR as instructed in the project specifications.

# **Test details**
> add_nand: Testing the accuracy of 2 consecutive i-type instructions.
> beq_simple: This tests if a branch is taken accurately.
> class: Supplied testing file which includes an instruction of each type and tests data hazards such as lw+add+beq consecutively which are dependent on each other. 
> beq: Tests a loop that loops ten times and includes data and control hazards
> nested_beq: Intense BEQ testing that includes a nested for loop and tests data and control hazards.
> data_hazards: Tests all data hazards that occur within one cycle of each other.
> data_hazards1: Tests all data hazards that occur in the second cycle following an instruction. 
> data_hazards2: Tests all data hazards that occur in the third cycle following an instruction. 
> halt: Tests the halt instruction, specifically that the final stats are accurate.
> lw_add: In response to problems with the add operation so this ensured add was working correctly.


# **To run tests** 
> To run add_nand.mc:		 ./sim -i tests/add_nand.mc 
> To run beq.mc:		 ./sim -i tests/beq.mc 
> To run beq_simple.mc:		 ./sim -i tests/beq_simple.mc 
> To run class.mc:		 ./sim -i tests/class.mc 
> To run data_hazards.mc:	 ./sim -i tests/data_hazards.mc 
> To run data_hazards1.mc:	 ./sim -i tests/data_hazards1.mc 
> To run data_hazards2.mc:	 ./sim -i tests/data_hazards2.mc 
> To run halt.mc:		 ./sim -i tests/halt.mc 
> To run lw_add.mc:		 ./sim -i tests/lw_add.mc 
> To run nested_beq.mc:		 ./sim -i tests/nested_beq.mc 
 

# **ref_sim/sim** 
The file ref_sim is a given executable which can take machine code and perform the instructions encoded by it as a single-cycle processor. The sim file
 works similarily but was created by us and was modified to be a multi-cycle processor.

# **ref_asm** 
A given reference executable which can take assembly code and convert it into machine code.

# **Makefile**
Makes the simulator file into an executable that can be run. To run the Makefile: use the command "make". This also rcognizes when an exacutable file has 
been made previously then cleans and overwrites it using the commmand "clean". To manually clean the directory: use the command "make clean"
