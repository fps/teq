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
		/**
		 * The patterns are the material used for arrangement
		 */
		typedef std::vector<pattern> pattern_list;

		typedef std::shared_ptr<pattern_list> pattern_list_ptr;
		

		/**
		 * This violates separation of concern, but since we don't plan
		 * to support backends other than jack, it's ok.
		 * 
		 * A track is tied to a port, so here's where we store the port
		 * handle.
		 */
		typedef std::pair<track_ptr, jack_port_t *> track_properties_and_payload;
		
		typedef std::vector<track_properties_and_payload> track_list;
		
		typedef std::shared_ptr<track_list> track_list_ptr;
		

		/**
		 * The arrangement consists of indices into the pattern list
		 */
		typedef std::shared_ptr<std::vector<unsigned>> arrangement_ptr;
		
		
		std::string m_name;
	
		std::string m_description;
		
		pattern_list_ptr m_patterns;

		arrangement_ptr m_arrangement;
		
		track_list_ptr m_tracks;
		
		song() :
			m_patterns(std::make_shared<pattern_list>()),
			m_tracks(std::make_shared<track_list>())
		{
			
		}

		void check_track_index(int index)
		{
			if (index < 0 || index >= (int)m_tracks->size())
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Track index out of bounds: " << index << ". Number of tracks: " << m_tracks->size())
			}
		}
		
		void check_pattern_index(int index)
		{
			if (index < 0 || index >= (int)m_patterns->size())
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Pattern index out of bounds: " << index << ". Number of patterns: " << m_patterns->size())
			}
		}
		
		void check_tick_index(int pattern_index, int tick_index)
		{
			check_pattern_index(pattern_index);
			
			if (tick_index < 0 || tick_index >=  (*m_patterns)[pattern_index].m_length)
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Tick index out of bounds: " << tick_index << ". Pattern length: " << (*m_patterns)[pattern_index].m_length)
			}
		}
	};
	
	typedef std::shared_ptr<song> song_ptr;
} // namespace

#endif
