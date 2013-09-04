#ifndef LIBTEQ_SONG_HH
#define LIBTEQ_SONG_HH

#include <vector>
#include <list>
#include <memory>

#include <teq/pattern.h>
#include <teq/track.h>

namespace teq
{
	struct song
	{
		typedef std::shared_ptr<std::vector<pattern>> pattern_list_ptr;
		
		typedef std::shared_ptr<std::vector<global_track_properties_ptr>> global_track_properties_list_ptr;
		
		std::string m_name;
	
		std::string m_description;
		
		pattern_list_ptr m_patterns;
	};
	
	typedef std::shared_ptr<song> song_ptr;
} // namespace

#endif
