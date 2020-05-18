//
// Jason's Memory Stream Helper thing
// Serialize a byte stream into real types
// 
// Currently only support Little Endian
//

#ifndef MEMSTREAM_H_
#define MEMSTREAM_H_


// Prototypes
void *memcpy(void *dest, const void *src, size_t n);

class MemoryStream
{
public:
	MemoryStream(unsigned char* pStreamStart, size_t streamSize)
		: m_pStreamStart( pStreamStart )
		, m_streamSize(streamSize)
		, m_pStreamCurrent( pStreamStart )
	{}

	~MemoryStream()
	{
		m_pStreamStart = nullptr;
		m_streamSize = 0;
		m_pStreamCurrent = nullptr;
	}

	// Really Dumb Reader Template
	template <class T>
	T Read()
	{
		T result;
	
		memcpy(&result, m_pStreamCurrent, sizeof(T));
		m_pStreamCurrent += sizeof(T);
	
		return result;
	}

	template <class T>
	void Read(T& result)
	{
		memcpy(&result, m_pStreamCurrent, sizeof(T));
		m_pStreamCurrent += sizeof(T);
	}

	size_t SeekCurrent(int delta)
	{
		m_pStreamCurrent += delta;
		return m_pStreamCurrent - m_pStreamStart;
	}

	unsigned char* GetPointer() { return m_pStreamCurrent; }

	size_t NumBytesAvailable() {

		unsigned char* pStreamEnd = m_pStreamStart + m_streamSize;

		if (pStreamEnd - m_pStreamCurrent >= 0)
		{
			return pStreamEnd - m_pStreamCurrent;
		}
		else
		{
			return 0;
		}
	}



private:

	unsigned char* m_pStreamStart;
	size_t m_streamSize;

	unsigned char* m_pStreamCurrent;
	
};


#endif // MEMSTREAM_H_


