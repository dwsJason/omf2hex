//
// hexfile.cpp
//
#include "hexfile.h"

//------------------------------------------------------------------------------

HexFile::HexFile( std::string filepath )
	: m_filepath( filepath )
	, m_pFile(nullptr)
	, m_currentBank(0)
{
	fopen_s(&m_pFile, filepath.c_str(), "w");
}

//------------------------------------------------------------------------------

HexFile::~HexFile()
{
	if (m_pFile)
	{
		EndOfFile();
	}
}

//------------------------------------------------------------------------------

//
// Does not support writing a chunk that crosses bank boundaries
//
void HexFile::SaveBytes(u8* pRAM, u32 address, u32 length)
{
	if (!m_pFile) return;

	SetBank(address);

	const int MAX_BYTES_PER_LINE = 34;

	u8* pData = pRAM + address;

	while (length > 0)
	{
		int current_line_length = (int)length;

		if (current_line_length > MAX_BYTES_PER_LINE)
		{
			current_line_length = MAX_BYTES_PER_LINE;
		}

		int checksum = current_line_length;
		checksum += ((address>>8) & 0xFF);
		checksum += ((address>>0) & 0xFF);

		fprintf(m_pFile, ":%02X%04X00", current_line_length, address & 0xFFFF);

		for (int idx = 0; idx < current_line_length; ++idx)
		{
			checksum += pData[ idx ];
			fprintf(m_pFile, "%02X", pData[ idx ]);
		}

		checksum *= -1;

		fprintf(m_pFile, "%02X\n", checksum & 0xFF);

		length -= current_line_length;
		address += current_line_length;
		pData += current_line_length;
	}
}

//------------------------------------------------------------------------------

void HexFile::SetBank( u32 address )
{
	u32 bank = (address >> 16) & 0xFF;

	if (bank != m_currentBank)
	{
		if (m_pFile)
		{
			int checksum = 0x02+0x04+(bank&0xFF)+((bank>>8)&0xFF);
			checksum *= -1;
			fprintf(m_pFile, ":02000004%04X%02X\n", bank, checksum&0xFF);
		}

		m_currentBank = bank;
	}
}

//------------------------------------------------------------------------------

void HexFile::EndOfFile()
{
	if (m_pFile)
	{
		fprintf(m_pFile, ":00000001FF");

		fclose(m_pFile);
		m_pFile = nullptr;
	}
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

