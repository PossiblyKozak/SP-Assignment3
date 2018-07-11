/*  
*  FILE          : encodeSREC.c 
*  PROJECT       : PROG1970 - Assignment #2
*  PROGRAMMER    : Alex Kozak 
*  FIRST VERSION : 2018-06-18 
*  DESCRIPTION   : 
*    The functions in this file are used encode a given input into SREC format.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_DATA 16
#define MAX_BUFFER 2

int printSRECLine(FILE* fp, int sNum, int address, void* data, int index);

void convertToSREC(FILE* inputFile, FILE *outputFile)
{
	// FUNCTION      : convertToSREC 
	// DESCRIPTION   : 
	//   This function takes the fInput and fOutput files and generates the 
	//	 SREC equivilent code from the fInput and puts it in the fOutput
	// PARAMETERS    : 
	//   FILE *inputFile   : The input file
	//   FILE *outputFile  : The output file
	// RETURNS       : 
	//   VOID

	FILE *tmp = fopen("TEMPORARY_DATA", "w");	// the temp file to store the final infor before printing to the output file
	int header[5] = {'A','L','E','X'};	// The name header
	int numLines = 0;	// The running total of lines printed for S5

	printSRECLine(tmp, 0, 0, header, 4);		// print the S0 line

	char buffer[MAX_BUFFER];
	if (inputFile)
	{
		int c[MAX_BUFFER + MAX_LINE_DATA] = {0};
		int index = 0;
		while(fgets(buffer, MAX_BUFFER, inputFile) != NULL)	// get input to be converted
		{
			c[index++] = buffer[0];
			while (index >= MAX_LINE_DATA)
			{
				numLines += printSRECLine(tmp, 1, numLines * MAX_LINE_DATA, c, index);	// print MAX_LINE_DATA characters at a time
				memmove(c, c + MAX_LINE_DATA, index - MAX_LINE_DATA);
				c[index - MAX_LINE_DATA] = '\0';
				index = 0;
			}
		}
		if (index > 0)
		{
			// print the remaining characters if there are any past a multiple of MAX_LINE_DATA
			numLines += printSRECLine(tmp, 1, numLines * MAX_LINE_DATA, c, index);	
		}
	}	
		
	printSRECLine(tmp, 5, numLines, NULL, 0);	// print the S5 line
	printSRECLine(tmp, 9, 0, NULL, 0);			// print the S9 line

	fclose(tmp);	// close the temp file to save the data

	tmp = fopen("TEMPORARY_DATA", "r");	// open the temp file for reading
	char buf[MAX_BUFFER];
	while(fgets(buf, MAX_BUFFER, tmp))	// push all information stored in the temp file to the actual output
	{
		fprintf(outputFile, "%s", buf);	
	}
	fclose(tmp);
	remove("TEMPORARY_DATA");
}

int printSRECLine(FILE* fOutput, int sNum, int address, void* data, int index)
{
	// FUNCTION      : printSRECLine 
	// DESCRIPTION   : 
	//   This function takes the fInput and fOutput files and generates the 
	//	 SREC equivilent code from the fInput and puts it in the fOutput
	// PARAMETERS    : 
	//   FILE *fOutput   : The output file
	//	 int sNum		 : The S-Number to be displayed
	//	 int address	 : The hex address of the command
	//	 void* data 	 : The array conatining the data to be printed to the output file
	//	 int index		 : The number of elements to be printed
	// RETURNS       : 
	//   VOID

	int returnValue = 0;
	if (fOutput != NULL)
	{
		returnValue = 1;		
		int count = 3;
		int checkValue = count + address;
		int dataCount = 0;

		if (data != NULL)
		{
			dataCount = index;	
			count += dataCount;
			checkValue += dataCount;
		}

		fprintf(fOutput, "S%d%02X%04X", sNum, count, address);	// print the initial head of the line

		if (data != NULL)
		{
			int *info = (int*)data;
			for (int i = 0; i < dataCount && i < MAX_LINE_DATA; i++)
			{
				fprintf(fOutput, "%02X", info[i] & 0xFF);	// print each character as a 2 digit hex value
				checkValue += info[i];
			}
		}
		fprintf(fOutput, "%02X\n", ~checkValue & 0xFF);	// print the checksum, but confirm they're only the last 2 digits.
	}
	return returnValue;
}