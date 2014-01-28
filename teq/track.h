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
	class sequence;
	
	typedef std::shared_ptr<sequence> sequence_ptr;

	struct sequence
	{
		virtual ~sequence() { }
		
		virtual void set_length(unsigned length) = 0;

		bool m_muted;
		
		virtual sequence_ptr clone() = 0;
		
		sequence() :
			m_muted(false)
		{
			
		}
		
	};
	
	
	template<class EventType>
	struct sequence_of : sequence
	{
		std::vector<EventType> m_events;
		
		virtual void set_length(unsigned length)
		{
			m_events.resize(length);
		}
		
		virtual sequence_ptr clone()
		{
			std::shared_ptr<sequence_of<EventType>> s(new sequence_of<EventType>());
			s->m_events = m_events;
			return s;
		}
	};

	
	struct track
	{
		enum type { NONE, MIDI, CV, CONTROL };
		
		type m_type;

		std::string m_name;
		
		bool m_muted;
		
		virtual ~track() { }
		
		track(const std::string &name, type the_type = type::NONE)  :
			m_type(the_type),
			m_name(name),
			m_muted(false)
		{
			
		}
		
		virtual sequence_ptr create_sequence() = 0;
	};

	typedef std::shared_ptr<track> track_ptr;
		
	struct midi_track : track
	{
		bool m_note_off_on_new_note_on;
		
		midi_event m_last_note_on_event;

		unsigned char m_channel;
		
		void *m_port_buffer;
		
		midi_track(const std::string &name) : 
			track(name, track::type::MIDI),
			m_note_off_on_new_note_on(true),
			m_channel(0)
		{

			
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
		
		cv_track(const std::string &name) :
			track(name, track::type::CV),
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
		
		control_track(const std::string &name) :
			track(name, track::type::CONTROL)
		{
			
		}
	};	
} // namespace

#endif
