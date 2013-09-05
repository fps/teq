#ifndef LIBTEQ_EVENTS_HH
#define LIBTEQ_EVENTS_HH

#include <memory>

namespace teq
{
	struct event
	{
		virtual ~event() { }
	};
	
	typedef std::shared_ptr<event> event_ptr;
	
	struct control_event : event
	{
		enum type { TEMPO, RELATIVE_TEMPO };
		
		type m_type;
		
		//! Start value: TEMPO: global tempo, RELATIVE_TEMPO: factor relative to global tempo
		float m_value1;
		
		//! End value
		float m_value2;
	};
	
	typedef std::shared_ptr<control_event> control_event_ptr;

	struct cv_event : event
	{
		//! Start value
		float m_value1;
		
		//! End value
		float m_value2;
		
		void set_value(float value)
		{
			m_value1 = value;
			m_value2 = value;
		}
	};
	
	typedef std::shared_ptr<cv_event> cv_event_ptr;

	struct midi_event : event
	{
		enum type { ON, OFF, CC, PITCHBEND };
		
		type m_type;

		//! ON: midi, OFF: ignored, CC: controller, PITCHBEND: bend
		unsigned m_value1;
		
		//! ON: velocity, OFF, CC, PITCHBEND: ignored
		unsigned m_value2;
	};
	
	typedef std::shared_ptr<midi_event> midi_event_ptr;
}

#endif
