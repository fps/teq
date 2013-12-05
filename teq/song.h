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
		typedef std::vector<pattern> pattern_list;

		typedef std::shared_ptr<pattern_list> pattern_list_ptr;
		
		typedef std::pair<track_ptr, jack_port_t *> track_properties_and_payload;
		
		typedef std::vector<track_properties_and_payload> track_list;
		
		typedef std::shared_ptr<track_list> track_list_ptr;
		
		std::string m_name;
	
		std::string m_description;
		
		pattern_list_ptr m_patterns;
		
		track_list_ptr m_tracks;
		
		song() :
			m_patterns(new std::vector<pattern>),
			m_tracks(new std::vector<track_properties_and_payload>)
		{
			
		}
	};
	
	typedef std::shared_ptr<song> song_ptr;
} // namespace

#endif
