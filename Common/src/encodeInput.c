/*  
*  FILE          : encodeInput.c
*  PROJECT       : PROG1970 - Assignment #2
*  PROGRAMMER    : Alex Kozak 
*  FIRST VERSION : 2018-06-18 
*  DESCRIPTION   : 
*    The functions in this file are used determine flags and required input/output files aswell as format, 
*    which will be used by either the encodeSREC or encodeASM files to convert the given input to the 
*    desired output format.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../inc/encodeSREC.h"
#include "../inc/encodeASM.h"

#define MAX_FILE_NAME_LENGTH 100
#define MAX_FLAGS 5

enum cmdSwitch{ERROR = -1, INPUT, OUTPUT, SREC, HELP, NONE};

// Prototypes
enum cmdSwitch runSwitch(char* arg);
void printUsage();
void interpretSwitches( enum cmdSwitch *currentSwitch, bool *isSREC, bool *isHelp, char* inputFileName, char* outputFileName, char **argv, int argc, enum cmdSwitch *usedSwitches);
void generateFiles(enum cmdSwitch currentSwitch, bool isSREC, bool isHelp, bool *isError, char* outputFileName, char* inputFileName, FILE **fInput, FILE **fOutput);
void runEncoder(FILE *fInput, FILE *fOutput, bool isSREC, bool isError);

void runEncoder(FILE *fInput, FILE *fOutput, bool isSREC, bool isError)
{
  // FUNCTION      : runEncoder 
  // DESCRIPTION   : 
  //   This function takes the fInput and fOutput files and determines
  //   which function and thereby output format the files should be given to
  // PARAMETERS    : 
  //   FILE *fInput   : The input file
  //   FILE *fOutput  : The output file
  //   bool isSREC    : A boolean representing if the output is in SREC format or not
  //   bool isError   : A boolean representing if the program should be run or not
  // RETURNS       : 
  //   VOID

  if (!isError)
  {
    if (isSREC)
    {
      convertToSREC(fInput, fOutput);
    }
    else
    {
      convertToASM(fInput, fOutput);
    }
    fclose(fOutput);
    fclose(fInput);
  }
}

void interpretSwitches( enum cmdSwitch *pCurrentSwitch, bool *isSREC, bool *isHelp, char* inputFileName, char* outputFileName, char **argv, int argc, enum cmdSwitch *usedSwitches)
{
  // FUNCTION      : interpretSwitches 
  // DESCRIPTION   : 
  //   This function takes all of the triggered switches and sets the required
  //   variables like file names or isSREC booleans to the correct values selected
  //   by the user in the function call
  // PARAMETERS    : 
  //   enum cmdSwitch *pCurrentSwitch  : A pointer to the current switch, used as a switch as well as an indicator that there was an error
  //   bool *isSREC                    : A pointer to the isSREC boolean, which is toggled by the '-srec' switch
  //   bool *isHelp                    : A pointer to the isHelp boolean, which is toggled by the '-h' switch
  //   char *inputFileName             : A pointer to the inputFileName string
  //   char *outputFileName            : A pointer to the outputFileName string
  //   char **argv                     : The strings provided by the user in the command line
  //   int argc                        : The number of arguments provided
  //   enum cmdSwitch *usedSwitches    : An array of cmdSwitch enums representing the switches called in the command call
  // RETURNS       : 
  //   VOID

  for (int i = 1; i < argc && *pCurrentSwitch != ERROR; i++)
  {
    *pCurrentSwitch = usedSwitches[i - 1];  
    char* c = argv[i];  
    switch(*pCurrentSwitch)
    {
      case(ERROR):
        break;
      case(INPUT):
        strncpy(inputFileName, &c[2], strlen(c) - 2); // save the inputfile name without the -i at the begining
        inputFileName[strlen(c) - 2] = '\0';
        break;
      case(OUTPUT):        
        strncpy(outputFileName, &c[2], strlen(c) - 2); // save the inputfile name without the -o at the begining
        outputFileName[strlen(c) - 2] = '\0';
        break;
      case(SREC):
        *isSREC = true; // toggle the isSREC flag
        break;
      case(HELP):
        *isHelp = true; // toggle the isHelp flag
        break;
    }
  }
}

void generateFiles(enum cmdSwitch currentSwitch, bool isSREC, bool isHelp, bool *isError, char *outputFileName, char *inputFileName, FILE **fInput, FILE **fOutput)
{  
  // FUNCTION      : generateFiles 
  // DESCRIPTION   : 
  //   This function takes the fInput and fOutput files and determines
  //   which function and thereby output format the files should be given to
  // PARAMETERS    : 
  //   enum cmdSwitch currentSwitch   : The enum representing the current switch, which will be ERROR if there was an error in the flag parsing
  //   bool isSREC                    : The boolean representing if the '-srec' flag was set
  //   bool isHelp                    : The boolean representing if the '-h' flag was set
  //   bool *isError                  : The boolean representing if the encoding shouldn't be run for some reason
  //   char *OutputFileName           : The pointer to the string representing the name of the output file
  //   char *InputFileName            : The pointer to the string representing the name of the input file
  //   FILE **fInput                  : The pointer to the file pointer representing the input file
  //   FILE **fOutput                 : The pointer to the file pointer representing the output file
  // RETURNS       : 
  //   VOID
  
  if (currentSwitch == ERROR)
  {
    printUsage();   // if there was an error
    *isError = true;
  }
  else if (isHelp)
  {
    printUsage();   // if the user requested help
    *isError = true;
  }
  else
  {
    if (!strcmp(inputFileName, "")) // if the user didn't give a inout filename, the default to stdin
    {
      *fInput = stdin;
    }
    else
    {
      *fInput = fopen(inputFileName, "r");  // if a filename was given, open that file
    }
    if (*fInput != NULL || *fInput == stdin)  // if the file was able to be opened, continue
    {
      if (!strcmp(outputFileName, ""))  // 
      {
        if (!strcmp(inputFileName, ""))
        {          
          *fOutput = stdout;
        }        
        else // if there was an input file given
        {
          if (isSREC) 
          {
            strcat(outputFileName, inputFileName);
            strcat(outputFileName, ".srec"); // output file is .srec
          }
          else
          {
            strcat(outputFileName, inputFileName);
            strcat(outputFileName, ".asm"); // output file is .asm
          }
          *fOutput = fopen(outputFileName, "w");
        }
      }
      else
      {
        *fOutput = fopen(outputFileName, "w");
      }
      if (*fOutput == NULL && *fOutput != stdout) // if the output file was unable to be opened
      {
        printf("Output file %s does not exist or was unable to be opened", outputFileName);
        *isError = true;
      }
    }
    else  // if the input wile was unable to be opened
    {
      printf("Input file %s does not exist or was unable to be opened", inputFileName);
      *isError = true;
    }
  }
}

void printUsage()
{
  // FUNCTION      : printUsage 
  // DESCRIPTION   : 
  //   This function displays the propper usage of the encodeinput function
  // PARAMETERS    : 
  //   VOID
  // RETURNS       : 
  //   VOID
  printf("Proper usage of encodeInput: \n\tencodeinput [-i<InputFileName>][-o<OutputFileName>][-h][-srec]\n");
}

enum cmdSwitch runSwitch(char* arg)
{
  // FUNCTION      : runSwitch 
  // DESCRIPTION   : 
  //   this function takes the string representation of the argument and parces the flag type
  // PARAMETERS    : 
  //   char* arg      : The string representation of the 
  // RETURNS       : 
  //   enum cmdSwitch : The resultant cmdSwitch representing the type of flag set in the calling of the function
  enum cmdSwitch returnSwitch;
  if (arg[0] == '-' && strlen(arg) > 1)
  {
    switch(arg[1])
    {
      case 'i': // input file name flag
        returnSwitch = INPUT;
        break;
      case 'o': // output file name flag
        returnSwitch = OUTPUT;
        break;
      case 's': // beginning of the srec tag
        if (strlen(arg) == strlen("-srec")) // confirming that it is actually '-srec'
        {
          if (!strcmp("-srec", arg))
          {
            returnSwitch = SREC;
          }
          else
          {
            returnSwitch = ERROR; // if it had began with s and is not '-srec' then there was an error
          }
        }
        break;
      case 'h': // help tag
        returnSwitch = HELP;
        break;
      default:  // if there is no tag match then there was an error
        returnSwitch = ERROR;
    }
  }
  else
  {
    returnSwitch = ERROR; // if the switch didnt begin with '-' or is less than 2 characters long
  }
  return returnSwitch;
}

void main(int argc, char *argv[])
{
  // declare variables
  enum cmdSwitch usedSwitches[MAX_FLAGS] = { NONE };
  enum cmdSwitch currentSwitch = NONE;
  FILE *fInput;
  FILE *fOutput;
  char inputFileName[MAX_FILE_NAME_LENGTH] = "";
  char outputFileName[MAX_FILE_NAME_LENGTH] = "";
  bool isSREC = false;
  bool isHelp = false;
  bool isError = false;

  for (int i = 1 ; i < argc && i < MAX_FLAGS; i++)
  {    
    usedSwitches[i - 1] = runSwitch(argv[i]); // generate switches for all given flags
  }  

  // run all functions to generate encoded output
  interpretSwitches( &currentSwitch, &isSREC, &isHelp, inputFileName, outputFileName, argv, argc, usedSwitches);  
  generateFiles(currentSwitch, isSREC, isHelp, &isError, outputFileName, inputFileName, &fInput, &fOutput);
  runEncoder(fInput, fOutput, isSREC, isError);
}