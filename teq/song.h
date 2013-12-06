#ifndef LIBTEQ_SONG_HH
#define LIBTEQ_SONG_HH

#include <vector>
#include <list>
#include <memory>

#include <teq/pattern.h>
#include <teq/track.h>

#include <teq/exception.h>

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

		void check_track_index(unsigned index)
		{
			if (index >= m_tracks->size())
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Track index out of bounds: " << index << ". Number of tracks: " << m_tracks->size())
			}
		}
		
		void check_pattern_index(unsigned index)
		{
			if (index >= m_patterns->size())
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Pattern index out of bounds: " << index << ". Number of patterns: " << m_patterns->size())
			}
		}
		
		void check_tick_index(unsigned pattern_index, unsigned tick_index)
		{
			check_pattern_index(pattern_index);
			
			if (tick_index >=  (*m_patterns)[pattern_index].m_length)
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Tick index out of bounds: " << tick_index << ". Pattern length: " << (*m_patterns)[pattern_index].m_length)
			}
		}
	};
	
	typedef std::shared_ptr<song> song_ptr;
} // namespace

#endif
