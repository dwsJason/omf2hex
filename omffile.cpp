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
	, m_pRawData( nullptr )
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

		m_pRawData = pData;

		fclose(pFile);

		// make sure we a virtual memory map to load our segments into
		// so that we can patch them up, give it a 16MB Memory Space
		m_pRAM = new u8[ 16 * 1024 * 1024 ];
		memset(m_pRAM, 0, 16 * 1024 * 1024);

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
	if (m_pRAM)
	{
		delete[] m_pRAM;
		m_pRAM = nullptr;
	}

	if (m_pRawData)
	{
		delete[] m_pRawData;
		m_pRawData = nullptr;
	}
}
//------------------------------------------------------------------------------

// OMF Section Sub Object

OMFSection::OMFSection(MemoryStream sectionStream)
	: m_rawSegmentStream(sectionStream)
{
	MemoryStream& ss = sectionStream;
	size_t stream_start = sectionStream.SeekCurrent(0);	// save where we are right now

	// Make a copy of the rawSegmentData
	//m_rawSegmentData.insert(m_rawSegmentData.begin(), ss.NumBytesAvailable(), 0);  // carve out space for it
	//memcpy(&m_rawSegmentData[0], ss.GetPointer(), ss.NumBytesAvailable() ); 	   // copy it
	// I'm aware that this giant chonk of data is now going to get copied multiple times
	// on the flip side, it's convenient to have a copy, so our parent file can
	//m_rawSegmentStream = ss;

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

//	printf("m_version %d\n", m_version);
//	printf("m_segnum  %d\n", m_segnum);

	// Seek to the name
	ss.SeekSet(stream_start + m_dispname);

	// Loadname 10 bytes, left justified
	u8 tempLoadName[10];
	ss.Read(tempLoadName);
	m_loadname = FixedLabelToString( tempLoadName, sizeof(tempLoadName) );
//	printf("m_loadname = %s\n", m_loadname.c_str());

	m_segname = ss.ReadPString();
	printf("m_segname = %s\n", m_segname.c_str());

	// Seek to the data
	ss.SeekSet(stream_start + m_dispdata);

	// Let's print out body information here, just to see if we can parse it
	// Body Record List
	if (m_kind & 0x8000)
	{
		printf("Dynamic Segment\n");
	}
	else
	{
		printf("Static Segment\n");
	}

	switch (m_kind & 0x1F)
	{
	case 0:
		printf("kind = code\n");
		break;
	case 1:
		printf("kind = data\n");
		break;
	case 2:
		printf("kind = jumptable\n");
		break;
	case 4:
		printf("kind = pathname\n");
		break;
	case 8:
		printf("kind = Library Dictionary Segment\n");
		break;
	case 0x10:
		printf("kind = initialization segment (display a splash screen anyone?)\n");
		break;
	case 0x12:
		printf("kind = Direct-page/stack segment\n");
		break;
	}

	if (m_kind & 0x100) printf("bank relative\n");
	if (m_kind & 0x200) printf("skip segment\n");
	if (m_kind & 0x400) printf("reload segment\n");
	if (m_kind & 0x800) printf("absolute bank segment\n");
	if (m_kind & 0x1000){
		printf("not allowed in special memory\n");
	} else
	{
		printf("special memory allowed\n");
	}
	if (m_kind & 0x2000) printf("position independent\n");
	if (m_kind & 0x4000) printf("private\n");

	bool bDone = false;

	while (!bDone)
	{
		u8 opcode = ss.Read<u8>();

		if (opcode >= 1 && opcode <= 0xDF)
		{
			printf("CONST\n");
			bDone = true;
		}
		else
		{
			switch (opcode)
			{
			case 0: //end
				printf("END\n");
				bDone = true;
				break;
			case 0xE0:
				printf("ALIGN\n");
				bDone = true;
				break;
			case 0xE1:
				printf("ORG\n");
				bDone = true;
				break;
			case 0xE2:
				{
					printf("RELOC\n");
					u8 num_bytes_to_relocate = ss.Read<u8>();
					i8 shift_count = ss.Read<i8>();
					u32 first_offset = ss.Read<u16>();
					u32 ref_offset = ss.Read<u16>();
				}
				break;
			case 0xE3:
				printf("INTERSEG\n");
				bDone = true;
				break;
			case 0xE4:
				printf("USING\n");
				bDone = true;
				break;
			case 0xE5:
				printf("STRONG\n");
				bDone = true;
				break;
			case 0xE6:
				printf("GLOBAL\n");
				bDone = true;
				break;
			case 0xE7:
				printf("GEQU\n");
				bDone = true;
				break;
			case 0xE8:
				printf("MEM\n");
				bDone = true;
				break;
			case 0xEB:
				printf("EXPR\n");
				bDone = true;
				break;
			case 0xEC:
				printf("ZEXPR\n");
				bDone = true;
				break;
			case 0xED:
				printf("BEXPR\n");
				bDone = true;
				break;
			case 0xEE:
				printf("RELEXPR\n");
				bDone = true;
				break;
			case 0xEF:
				printf("LOCAL\n");
				bDone = true;
				break;
			case 0xF0:
				printf("EQU\n");
				bDone = true;
				break;
			case 0xF1:
				printf("DS\n");
				bDone = true;
				break;
			case 0xF2:
				{
					u32 count = ss.Read<u32>();
					printf("LCONST, size = %d\n", count);
					ss.SeekCurrent(count); // Seek ahead to skip the data
				}
				break;
			case 0xF3:
				printf("LEXPR\n");
				bDone = true;
				break;
			case 0xF4:
				printf("ENTRY\n");
				bDone = true;
				break;
			case 0xF5:
				{
					u8 num_bytes_to_relocate = ss.Read<u8>();
					i8 shift_count = ss.Read<i8>();
					u16 first_offset = ss.Read<u16>();
					u16 ref_offset = ss.Read<u16>();
					printf("cRELOC\n");
				}
				break;
			case 0xF6:
				printf("cINTERSEG\n");
				bDone = true;
				break;
			case 0xF7:
				{
					u32 length = ss.Read<u32>();
					printf("SUPER, size = %d\n", length);
					ss.SeekCurrent(length); // Seek ahead to skip the data
				}
				break;
			case 0xFB:
				printf("General\n");
				bDone = true;
				break;
			case 0xFC:
			case 0xFD:
			case 0xFE:
			case 0xFF:
				printf("$%02x Experimental\n", opcode);
				bDone = true;
				break;

			default:
				printf("$%02x Unknown\n", opcode);
				bDone = true;
				break;
			}
		}
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

