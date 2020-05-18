//
// ORGFile Class
//

#ifndef ORGFILE_H_
#define ORGFILE_H_

#include <string>
#include <vector>

#include "bctypes.h"
#include "memstream.h"

//------------------------------------------------------------------------------

class ORGFile
{
public:
	ORGFile( std::string filepath );
	~ORGFile();

private:
	std::string m_filepath;

	std::vector<std::string> m_sections;
	std::vector<u32> m_orgs;
};

#endif // ORGFILE_H_


