#ifndef LIBTEQ_PATTERN_HH
#define LIBTEQ_PATTERN_HH

#include <vector>

#include <teq/track.h>

namespace teq
{
	struct pattern
	{
		typedef std::vector<track_ptr> track_list;
		
		track_list m_tracks;
		
		unsigned m_length;
		
		pattern(unsigned length = 128) :
			m_length(length)
		{
			
		}
	};
	
	typedef std::shared_ptr<pattern> pattern_ptr;
} // namespace

#endif

