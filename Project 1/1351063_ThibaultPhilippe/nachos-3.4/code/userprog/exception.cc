// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchcons.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

SynchConsole * synch_cons = NULL;
int MAX_LENGTH = 25; 
int MAX_STRING_LENGTH = 500;
char * digits = NULL;
char * characters = NULL;


// Input: - User space address (int) 
//  - Limit of buffer (int) 
// Output:- Buffer (char*) 
// Purpose: Copy buffer from User memory space to System memory space 

char* User2System(int virtAddr,int limit) 
{ 
	int i;// index 
	int oneChar; 
	char* kernelBuf = NULL; 
	kernelBuf = new char[limit+1];//need for terminal string 
	if (kernelBuf == NULL) 
		return kernelBuf; 
	memset(kernelBuf,0,limit+1); 
	//printf("\n Filename u2s:"); 
	for (i = 0 ; i < limit ;i++) { 
		machine->ReadMem(virtAddr+i,1,&oneChar); 
		kernelBuf[i] = (char)oneChar; 
		//printf("%c",kernelBuf[i]); 
		if (oneChar == 0) 
	 		break;  
	} 
	return kernelBuf; 
} 


// Input: - User space address (int) 
// - Limit of buffer (int) 
// - Buffer (char[]) 
// Output:- Number of bytes copied (int) 
// Purpose: Copy buffer from System memory space to User memory space 

int System2User(int virtAddr,int len,char* buffer) { 
	if (len < 0) 
		return -1; 
	if (len == 0)
		return len; 
	int i = 0; 
	int oneChar = 0 ; 
	do{ 
		oneChar= (int) buffer[i];   
		machine->WriteMem(virtAddr+i,1,oneChar);   
		i ++;  
	} while (i < len && oneChar != 0); 
	return i; 
} 


void
IncreaseProgramCounter() 
{
	//Read the current pc
	int current_pc = machine->ReadRegister(PCReg);
	//Read the next pc
	int next_pc = machine->ReadRegister(NextPCReg);

	//update previous PC
	machine->WriteRegister(PrevPCReg, current_pc);
	
	//update current PC
	machine->WriteRegister(PCReg, next_pc);
	
	//update next PC = current PC + 4
	machine->WriteRegister(NextPCReg, current_pc + 4);
}

void
ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);
	if (synch_cons == NULL) {
		synch_cons = new SynchConsole();
	}

	switch (which) {
		case SyscallException:
			switch (type) {
				case SC_Exit:
					break;
				case SC_Exec:
					break;
				case SC_Join:
					break;
				case SC_Fork:
					break;
				case SC_Yield:
					break;

					
				case SC_Sub:
					int op1;
					op1 = machine->ReadRegister(4);
					int op2; 
					op2 = machine->ReadRegister(5); 
					int result; 
					result = op1 - op2; 

					machine->WriteRegister(2, result); 
					
					// With increasing the PC, we dont need it anymore					
					// interrupt->Halt(); 
					// Increase the program counter after executed a system call in order to avoid loop 
					IncreaseProgramCounter();
					break; 

				case SC_Halt:
					DEBUG('a', "Shutdown !\n"); 
					interrupt->Halt(); 
					break;
				
				case SC_ReadInt: 
					{	
						digits = new char[MAX_LENGTH];
						if (!digits) {
							DEBUG('a', "\n Error : Not enough memory!\n");
							printf("\nError: Not enough memory \n");
							machine->WriteRegister(2, -1); // Error
							delete[] digits;
							break;
						}

						int len = synch_cons->Read(digits, MAX_LENGTH);
						if (len == -1) {
							DEBUG('a', "\n Error in ReadInt\n");
							printf("\nError found!\n");
							// Error
							machine->WriteRegister(2, -1); 
							delete[] digits;
							break;
						}
				
						int value = 0;
						int sign = 1;
						int i = 0;

						// Get the sign of the actual input value (represented as a string)
						if (digits[0] == '-') {
							sign = -1;
							i += 1;
						} 
						while (i < len) {
							char c = digits[i];
							if ('0' <= c && c <= '9') {	
								//We add one by one digit from the input to the value
								value = value * 10 + (c - '0');
							}
							else {
								DEBUG('a', "\n Error in ReadInt\n"); 
								printf("\nError : Illegal characters !\n");
								machine->WriteRegister(2, -1); 
								delete[] digits;
								break;
							}
							i += 1;
						}
						
						// Underflow issue
						if (value < 0) {
							DEBUG('a', "\n Warning in ReadInt\n"); 
							printf("\Warning : Underflow Integer !\n");
							break;						
						}
						
						// Return result
						machine->WriteRegister(2, sign * value);

						delete[] digits;

					}
					// Increase the program counter after executed a system call in order to avoid loop 
					IncreaseProgramCounter();
					break;

				case SC_PrintInt: 
					{	
						int value = machine->ReadRegister(4);
						char *digits = new char[MAX_LENGTH];
						if (!digits) {
							DEBUG('a', "\n Error : Not enough memory!\n");
							printf("\nError: Not enough memory \n");
							machine->WriteRegister(2, -1); // Error
							delete[] digits;
							break;
						}
					
						int sign = 1;
						
						// Pick up the sign of the input value
						if (value < 0) {
							sign = -1;
							value = -value;
						} 
					
						int i = 0;
						// Split each digit from Value and put it to the array until nothing to split.
						while (value > 0 || i == 0) {
							digits[++i] = '0' + value % 10;
							value /= 10;
						}
	
						// Reverse the array (because we push from left to right and from smaller order to higher order digits)
						for (int j = 1; 2 * j <= i; ++j) {
							char tmp = digits[j];
							digits[j] = digits[i - j + 1];
							digits[i - j + 1] = tmp;
						}
					
						// Write digit to console
						if (sign < 0) {
							digits[0] = '-';
							synch_cons->Write(digits, i + 1);
						} else {
							synch_cons->Write(digits + 1, i);
						}
					
						// Return
						machine->WriteRegister(2, 0);
						delete[] digits;
						break;

					}
					// Increase the program counter after executed a system call in order to avoid loop 
					IncreaseProgramCounter();
					break;

				case SC_ReadChar:
					{	
						characters = new char[MAX_LENGTH];
						if (!characters) {
							DEBUG('a', "\n Error : Not enough memory!\n");
							printf("\nError: Not enough memory \n");
							machine->WriteRegister(2, -1); // Error
							delete[] characters;
							break;
						}

						// Only allow a character and the \n
						int len = synch_cons->Read(characters, 2);
						if (len == -1) {
							DEBUG('a', "\n Error in ReadChar \n");
							printf("\nError found!\n");
							// Error
							machine->WriteRegister(2, -1); 
							delete[] characters;
							break;
						}
						else if (len > 1) {
							DEBUG('a', "\n Error in ReadChar \n");
							printf("\n Error : Cannot Put more than one character \n");
							// Error
							machine->WriteRegister(2, -1); 
							delete[] characters;
							break;					
						}
						
						// Return result
						machine->WriteRegister(2, characters[0]);
						delete[] characters;

					}
					// Increase the program counter after executed a system call in order to avoid loop 
					IncreaseProgramCounter();
					break;

				case SC_PrintChar:
					{	
						// Just need 1 slot for 1 character
						char * character = new char[1];
						
						// Not be able to allocate more memory
						if (!character) {
							DEBUG('a', "\nError: Not enough memory!\n");
							printf("\nError: Not enough memory \n");
							machine->WriteRegister(2, -1); // Error
							delete[] character;
							break;
						}
						
						// Print to the console
						character[0] = machine->ReadRegister(4);
						synch_cons->Write(character, 1);
					
						// Return character
						machine->WriteRegister(2, 0);
						delete[] character;

					}
					// Increase the program counter after executed a system call in order to avoid loop 
					IncreaseProgramCounter();
					break;

				case SC_ReadString:
					{	
						char * input_buffer = new char[MAX_STRING_LENGTH];
						if (!input_buffer) {
							DEBUG('a', "\n Error : Not enough memory!\n");
							printf("\nError: Not enough memory \n");
							machine->WriteRegister(2, -1); // Error
							delete [] input_buffer;
							break;
						}

						// Keep the actual address of the buffer, which is given from user as a paremeter of this system call
						int input_address = machine->ReadRegister(4);

						// The maximum length which is given by user, represent the maximum length of the input buffer 
						int max_len = machine->ReadRegister(5);

						// The actual length of the input string (count by the length of the string user typed on the Console)
						int len = synch_cons->Read(input_buffer, max_len);

						// Number of bytes copied
						int res = System2User(input_address, len, input_buffer);

						machine->WriteRegister(2, res);
						delete[] input_buffer;

					}
					// Increase the program counter after executed a system call in order to avoid loop 
					IncreaseProgramCounter();
					break;

				case SC_PrintString: 
					{	
						// Keep the actual address of the buffer, which is given from user as a paremeter of this system call
						int input_address = machine->ReadRegister(4);

						// Get the input buffer
						char * input_buffer = User2System(input_address, MAX_STRING_LENGTH + 1);
						if (!input_buffer) {
							DEBUG('a', "\n Not enough memory in system");
							printf("\n Not enough memory in system");
							machine->WriteRegister(2, -1); // Error
							delete[] input_buffer;
							break;
						}
					
						int i = 0;
						// Move to the end of the string
						while (input_buffer[i] != '\0') {
							++i;
						}

						// Print to console
						synch_cons->Write(input_buffer, i);
					
						// Return
						machine->WriteRegister(2, i);
						delete[] input_buffer;					
					}
					// Increase the program counter after executed a system call in order to avoid loop 
					IncreaseProgramCounter();
					break;	
				
				default:
					printf("\n Unexpected User Mode Exception (%d %d)", which, type);
					interrupt->Halt();
					break;			
			}
			IncreaseProgramCounter();
			break; 

		case NoException:
			return;

		case PageFaultException:
			DEBUG('a', "\n No valid translation found");
			interrupt->Halt();
			break;
		case ReadOnlyException:
			DEBUG('a', "\n Write attempted to page marked 'read-only'");
			interrupt->Halt();
			break;
		case BusErrorException:
			DEBUG('a', "\n Translation resulted in an invalid physical address");
			interrupt->Halt();
			break;
		case AddressErrorException:
			DEBUG('a', "\n Unaligned reference or one that was beyond the end of the address space");
			interrupt->Halt();
			break;
		case OverflowException:
			DEBUG('a', "\n Integer overflow in add or sub.");
			interrupt->Halt();
			break;
		case IllegalInstrException:
			DEBUG('a', "\n Unimplemented or reserved instr.");
			interrupt->Halt();
			break;
		case NumExceptionTypes:
			DEBUG('a', "\n NumExceptionTypes");
			interrupt->Halt();
			break;

		default: 
			printf("Unexpected user mode exception %d %d\n", which, type);
		 	interrupt->Halt();
			break; 
	}
}


