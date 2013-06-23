#ifndef LIBTEQ_TRACK_HH
#define LIBTEQ_TRACK_HH

#include <memory>
#include <map>
#include <utility>

#include <teq/event.h>

namespace teq
{
	struct track
	{
		virtual ~track() { }
	};
	
	struct midi_track : track
	{
		typedef uint64_t tick;
		
		struct range
		{
			tick m_start;
			
			tick m_end;
			
			bool m_enabled;
			
			range(tick start = 0, tick end = 0, bool enabled = false):
				m_start(start),
				m_end(end),
				m_enabled(enabled)
			{
				
			}
		};
		
		std::map<tick, midi_event_ptr> m_events;
	};	
}

#endif
