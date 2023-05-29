// omf2hex.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <stdlib.h>

#include "omffile.h"
#include "orgfile.h"

//------------------------------------------------------------------------------
void helpText()
{
	printf("omf2hex - v0.6\n");
	printf("--------------\n");
	printf("Convert an OMF file into a flat binary, and export as an\n");
	printf("Intel HEX file\n");
	printf("\nomf2hex [options] <OMF_File> <HEX_File>\n");
	printf("  -v  verbose\n\n");

	exit(-1);
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	char* pInfilePath  = nullptr;
	char* pOutfilePath = nullptr;
	bool bVerbose = false;

	for (int idx = 1; idx < argc; ++idx )
	{
		char* arg = argv[ idx ];

		if ('-' == arg[0])
		{
			// Parse as an option
			if ('v'==arg[1])
			{
				bVerbose = true;
			}
		}
		else if (nullptr == pInfilePath)
		{
			// Assume the first non-option is an input file path
			pInfilePath = argv[ idx ];
		}
		else if (nullptr == pOutfilePath)
		{
			// Assume second non-option is an output file path
			pOutfilePath = argv[ idx ];
		}
		else
		{
			// Oh Crap, we have a non-option, but we don't know what to do with
			// it
			printf("ERROR: Invalid option, Arg %d = %s\n\n", idx, argv[ idx ]);
			helpText();
		}
	}

	if (pInfilePath)
	{
		// Load the ORG File
		ORGFile org_file( std::string(pInfilePath) + ".org" );

		// Cache the raw OMF File
		OMFFile omf_file( pInfilePath, bVerbose );

		omf_file.MapIntoMemory( org_file );

		omf_file.LoadIntoMemory();

		if (pOutfilePath)
		{
			omf_file.SaveAsHex( pOutfilePath );
		}

	}
	else
	{
		helpText();
	}


	return 0;
}

