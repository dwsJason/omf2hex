//
// orgfile.cpp
//
#include "orgfile.h"
#include <stdio.h>

//------------------------------------------------------------------------------
// Static Helpers

static bool contains(char x, const char* pSeparators)
{
	while (*pSeparators)
	{
		if (x == *pSeparators)
		{
			return true;
		}
		pSeparators++;
	}

	return false;
}

static std::vector<std::string> split(const std::string& s, const char* separators)
{
	std::vector<std::string> output;
	std::string::size_type prev_pos = 0, pos = 0;

	for (int index = 0; index < s.length(); ++index)
	{
		pos = index;

		// if the index is a separator
		if (contains(s[index], separators))
		{
			// if we've skipped a token, collect it
			if (prev_pos != index)
			{
				output.push_back(s.substr(prev_pos, index-prev_pos));

				// skip white space here
				while (index < s.length())
				{
					if (contains(s[index], separators))
					{
						++index;
					}
					else
					{
						prev_pos = index;
						pos = index;
						break;
					}
				}
			}
			else
			{
				prev_pos++;
			}
		}
	}

    output.push_back(s.substr(prev_pos, pos-prev_pos+1)); // Last word

    return output;
}

// make this string lowercase
static void tolower(std::string& s)
{
	for (int index = 0; index < s.length(); ++index)
	{
		s[index] = tolower( s[index] );
	}
}

//------------------------------------------------------------------------------
ORGFile::ORGFile( std::string filepath )
	: m_filepath( filepath )
{
	FILE* pFile = nullptr;
	errno_t err = fopen_s(&pFile, filepath.c_str(), "rb");

	if (0==err)
	{
		fseek(pFile, 0, SEEK_END);
		size_t length = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		printf("\nORGFile %s - %lld bytes\n", filepath.c_str(), length);

		printf("Reading %s\n", filepath.c_str());

		u8* pData = new u8[ length ];

		fread(pData, sizeof(u8), length / sizeof(u8), pFile);

		// Now we have a buffer, with the whole OMF File
		// Read in the Sections
		{
			MemoryStream memStream(pData, length);

			while (memStream.NumBytesAvailable())
			{
				std::string lineData = memStream.ReadLine();

				// I like stuff that's not case sensitive
				tolower( lineData );

				std::vector<std::string> tokens = split(lineData, " \t");

				if (tokens.size() >= 3)
				{
					if ("org" == tokens[1])
					{
						std::string hexString = tokens[2];

						if ('$' == hexString[0])
						{
							u32 orgAddress;
							int c = sscanf_s(hexString.c_str() + 1, "%x", &orgAddress);

							if (1 == c)
							{
								// This looks like a parsable line
								//printf("%s org $%06X\n", tokens[0].c_str(), orgAddress);
								
								m_sections.push_back(tokens[0]);
								m_orgs.push_back(orgAddress);
							}
						}
					}
				}
			}
		}

		delete[] pData;

		fclose(pFile);

		printf("\nRead Completed\n");
	}
	else
	{
		printf("ORGFile could not open %s\n", filepath.c_str());
		printf("This file is used to set ORG addresses for each\n");
		printf("section in your OMF file, for the flat binary conversion\n");
		printf("\nFormat of contents in org file\n");
		printf("segmentname org $ADDRESS\n\n");
		printf("~GLOBALS org $20000\n");
		printf("ASMCODE org $30000\n");
		printf("vectors org $FFF0\n");
		printf("\n\nWhen this file doesn't exist, the converter will auto ORG\n");
		printf("starting in bank $01\n");
		printf("Special Auto Allocation Commands\n");
		printf(".alignment org $100   ; Minimum alignment to allocate\n");
		printf(".autopack org $20000  ; Allocator doesn't allocate below this address\n\n");
	}
}

//------------------------------------------------------------------------------

ORGFile::~ORGFile()
{
}

//------------------------------------------------------------------------------

u32 ORGFile::GetAddress(std::string sectionName)
{
	u32 result_address = 0;
	// If the address is in there, return it
	// First remove trailing spaces from the sectionName

	const size_t strEnd = sectionName.find_last_not_of(" ");

	if (strEnd != std::string::npos)
	{
		if ((strEnd+1) != sectionName.length())
		{
			sectionName.resize(strEnd+1);
		}
	}

	// Make sure not case sensitive
	tolower(sectionName);

	for (int labelIndex = 0; labelIndex < m_sections.size(); ++labelIndex)
	{
		if (sectionName == m_sections[labelIndex])
		{
			result_address = m_orgs[labelIndex];
			break;
		}
	}

	return result_address;
}

//------------------------------------------------------------------------------
