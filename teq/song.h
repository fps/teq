#ifndef LIBTEQ_SONG_HH
#define LIBTEQ_SONG_HH

#include <vector>
#include <list>
#include <memory>
#include <map>
#include <utility>

#include <teq/pattern.h>
#include <teq/track.h>
#include <teq/transport.h>

#include <teq/exception.h>

namespace teq
{
	/**
	 * This is the central data structure of teq. The update strategy is
	 * as follows:
	 * 
	 * 1] Make a copy of the current song. This is supposed to be a rather 
	 * leight weight operation since all elements are stored by
	 * reference (e.g. the list of patterns, etc..
	 *
	 * 2] Edit the copy of the song to make all necessary changes.
	 *
	 * 3] Use the update_song() method to replace the current song
	 * with the edited copy in a RT-safe way.
	 *
	 * NOTE: Since pattern data (the data itself - notes, cv_events, etc),
	 * not the structure,  is plain old data (POD) it can be 
	 * edited by passing the function to update the event as a lambda 
	 * into the process loop via the write_command_and_wait() method.
	 *
	 * NOTE: When an editing operation modifies more than a singe event,
	 * it is probably more efficient (timewise, since write_command_and_wait()
	 * waits after each command until the command has been executed) to 
	 * make a copy of the song, then a copy of the pattern in question,
	 * edit it ad then commit the resulting updated pattern in a single i
	 * operation.
	*/
	struct song
	{
		/**
		 * To determine hat pattern has a global tick position just lookup
		 * (*m_Transport_lookup_list)[position].
		 */
		typedef std::vector<transport_position> transport_lookup_list;
		typedef std::shared_ptr<transport_lookup_list> transport_lookup_list_ptr;

		transport_lookup_list_ptr m_transport_lookup_list;
		
		/**
		 * The patterns are the material used for arrangement
		 */
		typedef std::vector<pattern_ptr> pattern_list;
		typedef std::shared_ptr<pattern_list> pattern_list_ptr;

		pattern_list_ptr m_pattern_list;


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
		
		track_list_ptr m_track_list;
		

		std::string m_name;
	
		std::string m_description;
		
		song(pattern_list_ptr the_pattern_list, track_list_ptr the_track_list) :
			m_transport_lookup_list(new transport_lookup_list),
			m_pattern_list(the_pattern_list),
			m_track_list(the_track_list)
		{
			
		}

		void check_track_index(int index)
		{
			if (index < 0 || index >= (int)m_track_list->size())
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Track index out of bounds: " << index << ". Number of tracks: " << m_track_list->size())
			}
		}
		
		void check_pattern_index(int index)
		{
			if (index < 0 || index >= (int)m_pattern_list->size())
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Pattern index out of bounds: " << index << ". Number of patterns: " << m_pattern_list->size())
			}
		}
		
		void check_tick_index(int pattern_index, int tick_index)
		{
			check_pattern_index(pattern_index);
			
			if (tick_index < 0 || tick_index >=  (*m_pattern_list)[pattern_index]->m_length)
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Tick index out of bounds: " << tick_index << ". Pattern length: " << (*m_pattern_list)[pattern_index]->m_length)
			}
		}
		
		
	};
	
	typedef std::shared_ptr<song> song_ptr;
} // namespace

#endif
