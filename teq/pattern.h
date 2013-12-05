#ifndef LIBTEQ_PATTERN_HH
#define LIBTEQ_PATTERN_HH

#include <vector>

#include <teq/track.h>

namespace teq
{
	struct pattern
	{
		typedef std::vector<sequence_ptr> sequence_list;
		
		sequence_list m_sequences;
		
		unsigned m_length;
		
		pattern(unsigned length = 128) :
			m_length(length)
		{
			
		}
	};
	
	typedef std::shared_ptr<pattern> pattern_ptr;
} // namespace

#endif

