#ifndef LIBTEQ_TRACK_HH
#define LIBTEQ_TRACK_HH

#include <memory>
#include <map>
#include <utility>
#include <iostream>
#include <array>

#include <teq/event.h>

namespace teq
{
	struct sequence
	{
		virtual ~sequence() { }
		
		virtual void set_length(unsigned length) = 0;
	};
	
	typedef std::shared_ptr<sequence> sequence_ptr;
	
	template<class EventType>
	struct sequence_of : sequence
	{
		std::vector<EventType> m_events;
		
		virtual void set_length(unsigned length)
		{
			m_events.resize(length);
		}
	};

	
	struct track
	{
		enum type { NONE, MIDI, CV, CONTROL };
		
		type m_type;

		const std::string m_name;
		
		virtual ~track() { }
		
		track(type the_type = type::NONE)  :
			m_type(the_type)
		{
			
		}
		
		virtual sequence_ptr create_sequence() = 0;
	};

	typedef std::shared_ptr<track> track_ptr;
		
	struct midi_track : track
	{		
		std::array<bool, 16> m_channels;
		
		std::vector<midi_event> m_current_events;

		void *m_port_buffer;
		
		midi_track() : 
			track(track::type::MIDI),
			m_current_events(1)
		{
			m_channels[0] = true;
		}
		
		virtual sequence_ptr create_sequence()
		{
			return sequence_ptr(new sequence_of<midi_event>);
		}
	};
	
	
	struct cv_track : track
	{
		cv_event m_current_event;
		
		float m_current_value;
		
		void *m_port_buffer;
		
		virtual sequence_ptr create_sequence()
		{
			return sequence_ptr(new sequence_of<cv_event>);
		}
		
		cv_track() :
			track(track::type::CV),
			m_current_value(0)
		{
			
		}
	};
	
	struct control_track : track
	{
		virtual sequence_ptr create_sequence()
		{
			return sequence_ptr(new sequence_of<control_event>);
		}
		
		control_track() :
			track(track::type::CONTROL)
		{
			
		}
	};	
} // namespace

#endif
