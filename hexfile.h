//
// HexFile Class
//

#ifndef HEXFILE_H_
#define HEXFILE_H_

#include <string>
#include <stdio.h>
#include "bctypes.h"

//------------------------------------------------------------------------------

class HexFile
{
public:
	HexFile( std::string filepath );
	~HexFile();

	// Chunk of bytes not allowed to cross bank boundary
	void SaveBytes(u8* pRAM, u32 address, u32 length);
	void EndOfFile();

private:

	void SetBank( u32 address );

	std::string m_filepath;

	FILE* m_pFile;

	u32 m_currentBank; // Bank State information

};

#endif // HEXFILE_H_


