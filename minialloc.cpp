//
// Dumb mini allocator
//
#include "minialloc.h"

//------------------------------------------------------------------------------

MiniAllocator::MiniAllocator(u32 minAddress, u32 alignment)
	: m_minAddress( minAddress )
	, m_alignment( alignment )
{

	memset(block_map, 0, sizeof(bool) * 256 * 256); // 256 banks, each with 256 pages

	// Never Auto Allocate out of bank 0 (you can manually ORG into there)
	if (!m_minAddress)
	{
		m_minAddress = 0x10000;
	}

	if (m_minAddress)
	{
		AddAllocation(0, m_minAddress);
	}
}

MiniAllocator::~MiniAllocator()
{
	// Free the allocation pointers
	for (int idx = 0; idx < m_allocations.size(); ++idx)
	{
		delete m_allocations[idx];
		m_allocations[idx] = nullptr;
	}
}
//------------------------------------------------------------------------------

MiniAllocator::Allocation* MiniAllocator::Alloc( u32 sizeBytes, u32 alignment )
{
	MiniAllocator::Allocation* pResult = nullptr;

	// This is dumb, just start crawling the allocation table
	for (int idx = 0; idx < 256*256; ++idx)
	{
		if (false == block_map[idx])
		{
			int endPage = (256*256)-1;
			// we have a free spot, how large is it though?
			for (int endIdx = idx; endIdx < 256*256; ++ endIdx)
			{
				if (true == block_map[endIdx])
				{
					endPage = endIdx - 1;
				}

				u32 candidateSize = (endPage-idx+1) * 256;
				int candidate_startAddress = idx * 256;

				int aligned_startAddress = force_align(candidate_startAddress, alignment);

				candidateSize -= (aligned_startAddress - candidate_startAddress);
				candidate_startAddress = aligned_startAddress;

				// Can we fit in the space?
				if (sizeBytes <= candidateSize)
				{
					if (sizeBytes < 0x10000)
					{
						// no bank wrap allowed
						// Are we going to wrap a bank
						int endAddress = candidate_startAddress+sizeBytes-1;

						if ((endAddress & 0xFF0000) == (candidate_startAddress & 0xFF0000))
						{
							// we have an acceptable allocation
							pResult = AddAllocation(candidate_startAddress, sizeBytes);
							return pResult;
						}
					}
					else
					{
						// we have an acceptable allocation
						pResult = AddAllocation(candidate_startAddress, sizeBytes);
						return pResult;
					}
				}
			}

		}
	}

	return pResult;
}

//------------------------------------------------------------------------------

MiniAllocator::Allocation* MiniAllocator::AddAllocation(u32 address, u32 sizeBytes)
{
	// Mark Allocation in the block_map

	int start_page = address / 256;
	int num_pages = (sizeBytes+255)/256;

	for (int pageNo = start_page; pageNo < (start_page+num_pages); ++pageNo)
	{
		block_map[pageNo] = true;
	}

	Allocation* pAlloc = new Allocation();

	pAlloc->address = address;
	pAlloc->size    = sizeBytes;

	// Attempt to insert this in numerical order

	size_t location_index = 0;

	for (int idx = 0; idx < m_allocations.size(); ++idx)
	{
		if (address <= m_allocations[idx]->address)
		{
			location_index = idx;
			break;
		}

		if (address > m_allocations[idx]->address)
		{
			location_index = idx+1;
		}

	}

	m_allocations.insert(m_allocations.begin() + location_index, 1, pAlloc);

	return pAlloc;
}

//------------------------------------------------------------------------------

u32 MiniAllocator::force_align(u32 address, u32 alignment)
{
	if (alignment)
	{
		address += (alignment-1);
		address &= ~(alignment-1);
	}

	return address;
}

//------------------------------------------------------------------------------

