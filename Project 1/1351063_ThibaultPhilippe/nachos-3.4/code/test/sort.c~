/* sort.c 
 *    Test program to sort a large number of integers.
 *
 *    Intention is to stress virtual memory system.
 *
 *    Ideally, we could read the unsorted array off of the file system,
 *	and store the result back to the file system!
 */

#include "syscall.h"

int main() {
	int input[100];
	int n, i, j, temp;

	// Input from console
	PrintString("Enter n : ");
	n = ReadInt();
	
	// If n > 100 ==> we do not prepare for this case 
	if (n > 100) {
		PrintString("Input Error: 0 <= n <= 100 \n");
		return 0;	
	}	
	
	// Get the input array from console
	for (i = 0; i < n; ++i) {
		PrintString("element ");
		PrintInt(i + 1);
		PrintString(" : ");
		input[i] = ReadInt();
	}
	
	// Sorting the array with bubble sort
   	for (i = 0; i < n; ++i) {
		for (j = n - 1; j > i; --j) {
			if (input[i] > input[j]) {
				temp = input[i];
				input[i] = input[j];
				input[j] = temp;
			}
		}
	}

	// Write the sorted array to the console
	PrintString("Sorted array : ");
	for (i = 0; i < n ; ++i) {
		PrintInt(input[i]);
		PrintString(" ");	
	}
	PrintString("\n");
	return 0;
}
