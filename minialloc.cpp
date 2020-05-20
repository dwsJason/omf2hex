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
		m_pReservedAllocation = AddAllocation(0, m_minAddress);
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

MiniAllocator::Allocation* MiniAllocator::Alloc( u32 sizeBytes, u32 alignment )
{
	MiniAllocator::Allocation* pResult = nullptr;
	int address = 0;
	i32 prev_available_address = 0;

	// Just bail if the size is bigger than 64KB
	if (sizeBytes > 0x10000)
	{
		return nullptr;
	}

	// Forced alignment for the allocation
	if (m_alignment > alignment)
	{
		alignment = m_alignment;
	}

	// Loop through the current list of m_allocations, looking for gaps
	// return address in the first gap where we can fit our allocation
	for (int idx = 0; idx < m_allocations.size(); ++idx)
	{
		i32 possible_size = m_allocations[idx]->address - prev_available_address;

		if (possible_size > (i32)sizeBytes)
		{
			// See if it will fit here
			u32 new_end_address = prev_available_address + sizeBytes - 1;

			if ((new_end_address & 0xFF0000) == (prev_available_address & 0xFF0000))
			{
				// This spot looks good, but wait!, we have to make sure it's not
				// in the reserved memory

				int reserved_address_end = m_pReservedAllocation->address + m_pReservedAllocation->size;

				if ((i32)new_end_address < reserved_address_end)
				{
					prev_available_address = reserved_address_end;
					continue;
				}

				pResult = AddAllocation(prev_available_address, sizeBytes);
				break;
			}
			else
			{
				// Try this one again
				prev_available_address += 0x10000;
				prev_available_address &= ~0xFFFF;
				idx -= 1;
				continue;
			}
		}
		else
		{
			prev_available_address = m_allocations[idx]->address + m_allocations[idx]->size;
			prev_available_address = force_align(prev_available_address, alignment);
		}
	}

	if (!pResult)
	{
		// Oh this is so dumb
		u32 new_end_address = prev_available_address + sizeBytes - 1;

		if ((new_end_address & 0xFF0000) == (prev_available_address & 0xFF0000))
		{
			// This spot looks good
		}
		else
		{
			// Go to the next bank
			prev_available_address += 0x10000;
			prev_available_address &= ~0xFFFF;
		}

		pResult = AddAllocation(prev_available_address, sizeBytes);
	}

	return pResult;
}

//------------------------------------------------------------------------------

MiniAllocator::Allocation* MiniAllocator::AddAllocation(u32 address, u32 sizeBytes)
{
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

