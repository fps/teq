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
		enum type { GLOBAL_TEMPO_ONE_SHOT, RELATIVE_TEMPO_ONE_SHOT, GLOBAL_TEMPO_INTERVAL, RELATIVE_TEMPO_INTERVAL };
		
		type m_type;
		
		//! Start value: TEMPO: global tempo, RELATIVE_TEMPO: factor relative to global tempo
		float m_value1;
		
		//! End value
		float m_value2;
		
		control_event(type the_type = type::GLOBAL_TEMPO_ONE_SHOT, float value1 = 0, float value2 = 0) :
			m_type(the_type),
			m_value1(value1),
			m_value2(value2)
		{
			
		}
	};
	
	typedef std::shared_ptr<control_event> control_event_ptr;

	struct cv_event : event
	{
		enum type { ONE_SHOT, INTERVAL };
		
		type m_type;
		
		//! Start value
		float m_value1;
		
		//! End value
		float m_value2;
		
		void set_value(float value)
		{
			m_value1 = value;
			m_value2 = value;
		}

		cv_event(type the_type = type::ONE_SHOT, float value1 = 0, float value2 = 0) :
			m_type(the_type),
			m_value1(value1),
			m_value2(value2)
		{
			
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

 		midi_event(type the_type = type::ON, unsigned value1 = 0, unsigned value2 = 0) :
			m_type(the_type),
			m_value1(value1),
			m_value2(value2)
		{
			
		}

	};
	
	typedef std::shared_ptr<midi_event> midi_event_ptr;
}

#endif
