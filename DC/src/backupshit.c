switch (random_number)
  {
  	case EVERYTHING_OKAY:
  	{
  		strcpy (msg.message, "Everything is OKAY");
  	}
  	case PRESSURE_FAIL:
  	{
  		strcpy (msg.message, "Hydraulic Pressure Failure");
  	}
  	case BUTTON_FAIL:
  	{
  		strcpy (msg.message, "Safety Button Failure");
  	}
  	case NO_RAW_MATERIAL:
  	{
  		strcpy (msg.message, "No Raw Material in the Process");
  	}
  	case TEMP_OUT_OF_RANGE:
  	{
  		strcpy (msg.message, "Operating Temperature Out of Range");
  	}
  	case OPERATOR_ERROR
  	{
  		strcpy (msg.message, "Operator Error");
  	}
  	case OFF_LINE:
  	{
  		strcpy (msg.message, "Machine is Off-line");
  	}
  	default:
  	{
  		printf("Something went wrong with the number generator\n");
  		return ERROR_SENDING_MESSAGE;
  	}

#define ERROR_SENDING_MESSAGE 1
#define EVERYTHING_OKAY 0
#define PRESSURE_FAIL 1
#define BUTTON_FAIL 2
#define NO_RAW_MATERIAL 3
#define TEMP_OUT_OF_RANGE 4
#define OPERATOR_ERROR 5
#define OFF_LINE 6