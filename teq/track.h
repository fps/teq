#ifndef LIBTEQ_TRACK_HH
#define LIBTEQ_TRACK_HH

#include <memory>
#include <map>
#include <utility>
#include <iostream>

#include <teq/event.h>

namespace teq
{
	struct track
	{
		typedef uint64_t tick;
		
		typedef std::multimap<tick, midi_event_ptr> events_map;
		
		struct range
		{
			tick m_start;
			
			tick m_end;
			
			range(tick start, tick end) :
				m_start(start),
				m_end(end)
			{
			
			}
		};
		
		struct loop_range : range
		{
			bool m_enabled;
			
			loop_range(tick start = 0, tick end = 0, bool enabled = false) :
				range(start, end),
				m_enabled(enabled)
			{
				
			}
		};
		
		events_map m_events;
		
		~track()
		{
			// std::cout << "~track" << std::endl;
		}
		
		void clear()
		{
			m_events.clear();
		}

		void copy_range(tick begin, tick end, const track &the_track)
		{
			auto low_it = the_track.m_events.lower_bound(begin);
			
			while (low_it != m_events.end() && low_it->first < end)
			{
				m_events.insert(std::make_pair(low_it->first, low_it->second));
				
				++low_it;
			}
		}
		
		void clear_range(tick begin, tick end)
		{
			auto low_it = m_events.lower_bound(begin);
			
			while (low_it != m_events.end() && low_it->first < end)
			{
				m_events.erase(low_it);
				
				low_it = m_events.lower_bound(begin);
			}
		}
		
		void add_note_on(tick position, unsigned channel, unsigned note, unsigned velocity)
		{
			m_events.insert
			(
				std::make_pair
				(
					position, 
					std::shared_ptr<midi_note_on_event>
					(
						new midi_note_on_event(channel, note, velocity)
					)
				)
			);
		}
		
		void add_note_off(tick position, unsigned channel, unsigned note, unsigned velocity)
		{
			m_events.insert
			(
				std::make_pair
				(
					position, 
					std::shared_ptr<midi_note_off_event>
					(
						new midi_note_off_event(channel, note, velocity)
					)
				)
			);
		}

		void add_all_notes_off(tick position, unsigned channel)
		{
			m_events.insert
			(
				std::make_pair
				(
					position, 
					std::shared_ptr<midi_all_notes_off_event>
					(
						new midi_all_notes_off_event(channel)
					)
				)
			);
		}

		void add_cc(tick position, unsigned channel, unsigned cc, unsigned value)
		{
			m_events.insert
			(
				std::make_pair
				(
					position, 
					std::make_shared<midi_cc_event>
					(
						channel, cc, value
					)
				)
			);
		}
	};	
}

#endif
