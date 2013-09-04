#ifndef LIBTEQ_PATTERN_HH
#define LIBTEQ_PATTERN_HH

#include <vector>

#include <teq/track.h>

namespace teq
{
	struct pattern
	{
		typedef std::shared_ptr<std::vector<track_ptr>> track_list_ptr;
		
		track_list_ptr m_track_list;
	};
	
	typedef std::shared_ptr<pattern> pattern_ptr;
} // namespace

#endif

