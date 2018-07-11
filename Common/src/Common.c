/*  
*  FILE          : encodeASM.c
*  PROJECT       : PROG1970 - Assignment #2
*  PROGRAMMER    : Alex Kozak 
*  FIRST VERSION : 2018-06-18 
*  DESCRIPTION   : 
*    The functions in this file are used encode a given input into ASM format.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_DATA 16
#define MAX_BUFFER 2

void lineToASM(int* stringInput, FILE *outputFile, int index);

void convertToASM(FILE* inputFile, FILE* outputFile)
{
	// FUNCTION      : convertToASM 
	// DESCRIPTION   : 
	//   This function takes the fInput and fOutput files and generates the 
	//	 asm equivilent code from the fInput and puts it in the fOutput
	// PARAMETERS    : 
	//   FILE *inputFile   : The input file
	//   FILE *outputFile  : The output file
	// RETURNS       : 
	//   VOID

	FILE *tmp = fopen("TEMPORARY_DATA", "w");	// temporary file so that the output can be given to the right place all at once

	char buffer[MAX_BUFFER];
	if (inputFile)
	{
		int c[MAX_BUFFER + MAX_LINE_DATA] = {0};
		int index = 0;
		while(fgets(buffer, MAX_BUFFER, inputFile) != NULL)
		{
			c[index++] = buffer[0];
			while (index >= MAX_LINE_DATA)
			{
				lineToASM(c, tmp, index);
				memmove(c, c + MAX_LINE_DATA, index - MAX_LINE_DATA);
				c[index - MAX_LINE_DATA] = '\0';
				index = 0;
			}
		}
		if (index > 0)
		{
			lineToASM(c, tmp, index);
		}
	}	
	fclose(tmp);	// close the temp file to save the information

	tmp = fopen("TEMPORARY_DATA", "r");	// open the temp file for reading purposes
	char buf[MAX_BUFFER];
	while(fgets(buf, MAX_BUFFER, tmp))	// print all the information to the desired user output
	{
		fprintf(outputFile, "%s", buf);	
	}
	fclose(tmp);	// close the temp file
	remove("TEMPORARY_DATA");	// delete the temp file
}

void lineToASM(int* stringInput, FILE *outputFile, int index)
{
	// FUNCTION      : lineToASM 
	// DESCRIPTION   : 
	//   This function takes a string and outputs it to the given output file converted to ASM format
	// PARAMETERS    : 
	//   int *stringInput  : The int[] to convert to ASM format, int to prevent the loss of null terminators
	//   FILE *outputFile  : The output file
	//	 int index 		   : The current number of elements to add to the file
	// RETURNS       : 
	//   VOID
	int dataCount = index;
	int currentIndex = 0;
	char* c;	

	while (currentIndex < dataCount)
	{
		if (currentIndex == 0)
		{
			fprintf(outputFile, "db.c\t");
		}
		else if (currentIndex % MAX_LINE_DATA == 0)
		{
			fprintf(outputFile, "\ndb.c\t");	
		}
		
		fprintf(outputFile, "$%02X ", stringInput[currentIndex] & 0xFF);	// print only the last 2 elements of the stringInput character
		currentIndex++;
	}
	fprintf(outputFile, "\n");
}
