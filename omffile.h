//
// OMFFile Class
//

#ifndef OMFFILE_H_
#define OMFFILE_H_

#include <string>
#include <vector>

#include "bctypes.h"
#include "memstream.h"

// Forward references

class ORGFile;

//------------------------------------------------------------------------------


class OMFSection
{
public:
	OMFSection(MemoryStream sectionStream);
	~OMFSection() {}

//private:

	i32 m_bytecnt;		// size of segment in bytes including m_bytecnt
	i32 m_resspc;   	// number of bytes of zeros to add to end of segment
	i32 m_length;   	// memory required by segment when loaded (including m_resspc)

	i8  m_undefined0;   // reserved for future expansion

	/* 
		m_lablen 
	    A 1-byte field indicating the length, in bytes, of each name or label
	    record in the segment body. If LABLEN is 0, the length of each name or
	    label is specified in the first byte of the record (that is, the first
	    byte of the record specifies how many bytes follow). LABLEN also
	    specifies the length of the SEGNAME field of the segment header, or, if
	    LAB LEN is 0, the first byte of SEGNAME specifies how many bytes follow.
	    (The LOAD NAME field always has a length of 10 bytes.) Fixed-length
		labels are always left justified and padded with spaces. 
	*/
	i8  m_lablen;

	i8  m_numlen;  // A 1-byte field indicating the length, in bytes, of each
				   // number field in the segment body. This field is 4 for
				   // the Apple IIGS.

	i8  m_version;   // currently 2, for OMF 2.1
//	i8  m_revision;  // currently 1, for OMF 2.1 (didn't make it out of draft)

	/*  BANKSIZE
	    A 4-byte binary number indicating the maximum memory-bank size for the
	    segment. If the segment is in an object file, the linker ensures that
	    the segment is not larger than this value. (The linker returns an error
	    if the segment is too large.) If the segment is in a load file, the
	    loader ensures that the segment is loaded into a memory block that does
	    not cross this boundary. For Apple IIGS code segments, this field must
	    be $00010000, indicating a 64K bank size. A value of 0 in this field
	    indicates that the segment can cross bank boundaries. Apple IIGS data
		segments can use any number from $00 to $00010000 for BANKSIZE
	*/
	i32 m_banksize;  // Should be 0x010000

	/* 
	    Bit(s)	Values	Meaning
	      0-4			Segment Type
	    		$00     code
	    		$01		data
	    		$02		Jump-table segment
	    		$04		pathname segment
	    		$08		library dictionary segment
	    		$10		initialization segment
	    		$12		directpage / stack segment
	 
	     10-15			Segment Attributes
	       8    if=1	Bank-relative
	       9 	if=1	Skip Segment
	      10	if=1	Reload Segment
	      11	if=1	Absolute-bank segment
	      12	if=0	Can be loaded into special memory
	      13	if=1	Position Independent
	      14	if=1	Private
	      15	if=0	Static; otherwise Dynamic
	 
	*/
	i16 m_kind;
	i16 m_undefined1;

	i32 m_org; 		// absolute address where segment should be loaded (normally 0) on the GS
	i32 m_align;	// 0, $100, or $10000 (0 = no alignment needed)

	i8  m_numsex;   // 0 = little endian, 1 = big endian
	i8  m_undefined2;

	i16 m_segnum;   // segnum, starting with 1

	i32 m_entry;	// A 4-byte field indicating the offset into the segment
					//that corresponds to the entry point of the segment. 

	/*  DISPNAME
	    A 2-byte field indicating the displacement of the LOADNAME field within
	    the segment header. Currently, DISPNAME = 44. DISPNAME is provided to
	    allow for future additions to the segment header; any new fields will be
	    added between DISPDATA and LOADNAME. DISPNAME allows you to reference
		LOADNAME and SEGNAME no matter what the actual size of the header
	*/
	i16 m_dispname;

	/*  DISPDATA
	    A 2-byte field indicating the displacement from the start of the segment
	    header to the start of the segment body. DISPDATA is provided to allow
	    for future additions to the segment header; any new fields will be added
	    between DISPDATA and LOADNAME. DISPDATA allows you to reference the
		start of the segment body no matter what the actual size of the header
	*/
	i16 m_dispdata;

	/*  TEMPORG
	    A 4-byte field indicating the temprorary origin of the Object segment.
	    A nonzero value indicates that all references to globals within this
	    segment will be interpreted as if the Object segment started at that
	    location. However, the actuallood address of the Object segment is still
		determined by the ORG field
	*/
	//i32 m_temporg;

	/*
	    A 10-byte field specifying the name of the load segment that will
	    contain the code generated by-the linker for this segment. More than one
	    segment in an object file can be merged by the linker into a single
	    segment in the load file. This field is unused in a load segment. The
	    position of LOAD NAME may change in future revisions of the OMF;
		therefore, you should always use DISPNAME to reference LOADNAME
	*/
	std::string m_loadname;

	/*
	    A field that is LAB LEN bytes long, and that specifies the name of the
	    segment. The position of SEGNAME may change in future revisions of the
		OMF; therefore, you should always use DISPNAME to reference SEGNAME
	*/
	std::string m_segname;

	// Unmodified data, as loaded from disk
	//std::vector<u8> m_rawSegmentData;
	MemoryStream m_rawSegmentStream; // we'll reference data in the parent file

};


class OMFFile
{
public:
	OMFFile( std::string filepath );
	~OMFFile();

	void MapIntoMemory(const ORGFile& org_file );

private:
	std::string m_filepath;
	std::vector<OMFSection> m_sections;

	u8* m_pRawData;

	// 16 megabytes of 65816 flat memory space
	u8* m_pRAM;



};

#endif // OMFFILE_H_

