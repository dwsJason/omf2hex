//
// minialloc, Dumb Mini Allocator Class
// 
// This is allocating segmented memory, so it's working with buckets of 64KB
// it isn't going to give you memory that crosses bank boundaries (unless the requested size if > 64KB)
// 
// This doesn't have to be efficient, because:
// 
// 1.  it's not realtime
// 2.  it doesn't have to free memory
//
#ifndef MINIALLOC_H_
#define MINIALLOC_H_

#include "bctypes.h"
#include <vector>

class MiniAllocator
{
public:

	struct Allocation
	{
		u32 address;  	// memory address
		u32 size;		// size of the allocation
	};

	MiniAllocator(u32 minAddress, u32 alignment);
	~MiniAllocator();

	Allocation* Alloc( u32 sizeBytes, u32 alignment=0 );

	Allocation* AddAllocation(u32 address, u32 sizeBytes);

private:

	// Block Map, contains a bool for each 256 byte page available in memory

	bool block_map[ 256 * 256 ]; // 256 pages * 256 banks

	u32 force_align(u32 address, u32 alignment);


	u32 m_minAddress;
	u32 m_alignment;

	std::vector<Allocation*> m_allocations;  // I don't need this, but since it's already written, keep it
};


#endif //MINIALLOC_H_
