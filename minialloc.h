//
// minialloc, Dumb Mini Allocator Class
// 
// This is allocating segmented memory, so it's working with buckets of 64KB
// it isn't going to give you memory that crosses bank boundaries
// 
// This doesn't have to be efficient, because:
// 
// 1.  it's not realtime
// 2.  it'll at most have to make 256 allocations
// 3.  it doesn't have to free memory
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

	Allocation* Alloc( u32 sizeBytes );

	Allocation* AddAllocation(u32 address, u32 sizeBytes);

	std::vector<Allocation*> FindOverlaps(Allocation* pReferenceAlloc, bool bIgnoreReserved=true);

private:

	bool overlaps(Allocation* allocation, u32 address, u32 sizeBytes);

	u32 m_minAddress;
	u32 m_alignment;

	Allocation* m_pReservedAllocation; // reserved to hold out min address

	std::vector<Allocation*> m_allocations;
};


#endif //MINIALLOC_H_
