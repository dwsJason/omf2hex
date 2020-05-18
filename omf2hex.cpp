// omf2hex.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdlib.h>

#include "omffile.h"

//------------------------------------------------------------------------------
void helpText()
{
	printf("omf2hex - v0.1\n");
	printf("--------------\n");
	printf("Convert an OMF file into a flat binary, and export as an\n");
	printf("Intel HEX file\n");
	printf("\nomf2hex <OMF_File> <HEX_File>\n");

	exit(-1);
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	char* pInfilePath  = nullptr;
	char* pOutfilePath = nullptr;

	if (argc >= 2)
	{
		pInfilePath = argv[1];
	}
	if (argc >= 3)
	{
		pOutfilePath = argv[2];
	}

	if (pInfilePath)
	{
	}
	else
	{
		helpText();
	}


	return 0;
}

