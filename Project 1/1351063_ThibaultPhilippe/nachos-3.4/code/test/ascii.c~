#include "syscall.h"

int main() {
	int i;
	// We have only the visible ASCII character from code 32 to 126 
	PrintString("\nASCII printable code chart, from code 32 to 126 (in decimal)\n");
	for (i = 32; i < 127; ++i) { 
		PrintString("- Code ");
		PrintInt(i);
		PrintString(": ");
		PrintChar(i);
		PrintString("\n");
	}
	return 0;
}
