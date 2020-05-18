//
// omffile.cpp
//
#include "omffile.h"

#include "bctypes.h"
#include "memstream.h"  // Jason's memory stream thing

#include <stdio.h>

// -----------------------------------------------------------------------------

// Static Local Helper functions
//
// Fixed Label, probably doesn't have a zero termination
//
static std::string FixedLabelToString( u8* pLabel, size_t numBytes )
{
	std::string result;
	result.insert(0, (char*)pLabel, numBytes);
	return result;
}
// -----------------------------------------------------------------------------

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

		printf("\nOMFFile %s - %lld bytes\n", filepath.c_str(), length);

		printf("Reading %s\n", filepath.c_str());

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

	size_t stream_start = sectionStream.SeekCurrent(0);	// save where we are right now

	ss.Read( m_bytecnt );
	ss.Read( m_resspc );
	ss.Read( m_length );
	ss.Read( m_undefined0 );

	ss.Read( m_lablen );
	ss.Read( m_numlen );
	ss.Read( m_version );
//	ss.Read( m_revision );

	ss.Read( m_banksize );

	ss.Read( m_kind );
	ss.Read( m_undefined1 );

	ss.Read( m_org );
	ss.Read( m_align );

	ss.Read( m_numsex );
	ss.Read( m_undefined2 );

	ss.Read( m_segnum );

	ss.Read( m_entry );

	ss.Read( m_dispname ); // displacement to name

	ss.Read( m_dispdata ); // displacement to data

//	ss.Read( m_temporg );

	printf("m_version %d\n", m_version);
	printf("m_segnum  %d\n", m_segnum);

	// Seek to the name
	ss.SeekSet(stream_start + m_dispname);

	// Loadname 10 bytes, left justified
	u8 tempLoadName[10];
	ss.Read(tempLoadName);
	m_loadname = FixedLabelToString( tempLoadName, sizeof(tempLoadName) );
	printf("m_loadname = %s\n", m_loadname.c_str());

	m_segname = ss.ReadPString();
	printf("m_segname = %s\n", m_segname.c_str());

	// Seek to the data
	ss.SeekSet(stream_start + m_dispdata);

}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

