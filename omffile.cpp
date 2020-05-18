//
// omffile.cpp
//
#include "omffile.h"

#include "bctypes.h"
#include "memstream.h"  // Jason's memory stream thing

#include <stdio.h>
//------------------------------------------------------------------------------
OMFFile::OMFFile( std::string filepath )
	: m_filepath( filepath )
{
	FILE* pFile = nullptr;
	errno_t err = fopen_s(&pFile, filepath.c_str(), "rb");

	if (0==err)
	{
		fseek(pFile, 0, SEEK_END);
		size_t length = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);

		printf("OMFFile %s - %lld bytes\n", filepath.c_str(), length);

		printf("\nReading %s\n", filepath.c_str());

		u8* pData = new u8[ length ];

		fread(pData, sizeof(u8), length / sizeof(u8), pFile);

		// Now we have a buffer, with the whole OMF File
		// Read in the Sections
		{
			MemoryStream memStream(pData, length);

			while (memStream.NumBytesAvailable() >= sizeof(i32))
			{
				// Read in the next segment
				i32 segment_size = memStream.Read<i32>(); // peek at the segment size
				memStream.SeekCurrent(-((int)sizeof(i32)));	  // bring seek back to start of the current segment

				OMFSection section = OMFSection(MemoryStream(memStream.GetPointer(), segment_size)); // Serialize the Segment

				m_sections.push_back(section);	// Save a copy of our finished work

				memStream.SeekCurrent(segment_size); // Skip Ahead to next Section
			}
		}

		delete[] pData;

		fclose(pFile);

		printf("\nRead Completed\n");
	}
	else
	{
		printf("OMFFile could not open %s\n", filepath.c_str());
	}
}

//------------------------------------------------------------------------------


OMFFile::~OMFFile()
{
}
//------------------------------------------------------------------------------

// OMF Section Sub Object

OMFSection::OMFSection(MemoryStream sectionStream)
{
	MemoryStream& ss = sectionStream;

	ss.Read( m_bytecnt );
	ss.Read( m_resspc );
	ss.Read( m_length );
	ss.Read( m_undefined0 );

	ss.Read( m_lablen );
	ss.Read( m_numlen );
	ss.Read( m_version );
	ss.Read( m_revision );

	ss.Read( m_banksize );

	ss.Read( m_kind );
	ss.Read( m_undefined1 );

	ss.Read( m_org );
	ss.Read( m_align );

	ss.Read( m_numsex );
	ss.Read( m_undefined2 );

	ss.Read( m_segnum );

	ss.Read( m_entry );

	ss.Read( m_dispname );

	ss.Read( m_dispdata );

	ss.Read( m_temporg );
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

