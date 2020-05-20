//
// Dumb mini allocator
//
#include "minialloc.h"

//------------------------------------------------------------------------------

MiniAllocator::MiniAllocator(u32 minAddress, u32 alignment)
	: m_minAddress( minAddress )
	, m_alignment( alignment )
{
	// Never Auto Allocate out of bank 0 (you can manually ORG into there)
	if (!m_minAddress)
	{
		m_minAddress = 0x10000;
	}

	if (m_minAddress)
	{
		m_pReservedAllocation = AddAllocation(0, minAddress);
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

	m_pReservedAllocation = nullptr;
}
//------------------------------------------------------------------------------

MiniAllocator::Allocation* MiniAllocator::Alloc( u32 sizeBytes )
{
	u32 address = 0;

	// Loop through the current list of m_allocations, looking for gaps
	// return address in the first gap where we can fit our allocation


}

//------------------------------------------------------------------------------

MiniAllocator::Allocation* MiniAllocator::AddAllocation(u32 address, u32 sizeBytes)
{
	Allocation* pAlloc = new Allocation();

	pAlloc->address = address;
	pAlloc->size    = sizeBytes;

	m_allocations.push_back(pAlloc);

	return pAlloc;
}


//------------------------------------------------------------------------------

bool MiniAllocator::overlaps(Allocation* pAllocation, u32 address, u32 sizeBytes)
{
	if (pAllocation->address >= (address + sizeBytes))
	{
		// No overlap
		return false;
	}

	if (pAllocation->address+pAllocation->size <= address)
	{
		// No overlap
		return false;
	}

	// must overlap
	return true;
}

//------------------------------------------------------------------------------

std::vector<MiniAllocator::Allocation*> MiniAllocator::FindOverlaps(Allocation* pReferenceAlloc, bool bIgnoreReserved)
{
	std::vector<Allocation*> result;

	for (int idx = 0; idx < m_allocations.size(); ++idx)
	{
		if (m_allocations[ idx ])
		{
			if (bIgnoreReserved && (m_pReservedAllocation == m_allocations[idx]))
			{
				// Ignore the reserved Allocation
				continue;
			}

			if (pReferenceAlloc == m_allocations[idx])
			{
				// Ignore self
				continue;
			}

			if (overlaps(pReferenceAlloc, m_allocations[idx]->address, m_allocations[idx]->size))
			{
				result.push_back(m_allocations[ idx ]);
			}
		}
	}

	return result;
}

//------------------------------------------------------------------------------

