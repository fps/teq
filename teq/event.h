#ifndef LIBTEQ_EVENTS_HH
#define LIBTEQ_EVENTS_HH

#include <memory>

namespace teq
{
	struct control_event
	{
		enum type { NONE, GLOBAL_TEMPO, RELATIVE_TEMPO };
		
		type m_type;
		
		//! Start value: TEMPO: global tempo, RELATIVE_TEMPO: factor relative to global tempo
		float m_value;
	
		control_event(type the_type = type::NONE, float value = 0) :
			m_type(the_type),
			m_value(value)
		{
			
		}
	};
	
	struct cv_event
	{
		enum type { NONE, INTERVAL };
		
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

		cv_event(type the_type = type::NONE, float value1 = 0, float value2 = 0) :
			m_type(the_type),
			m_value1(value1),
			m_value2(value2)
		{
			
		}

	};
	
	struct midi_event
	{
		enum type { NONE, ON, OFF, CC, PITCHBEND };
		
		type m_type;

		//! ON: midi, OFF: ignored, CC: controller, PITCHBEND: bend
		unsigned m_value1;
		
		//! ON: velocity, OFF, CC, PITCHBEND: ignored
		unsigned m_value2;

 		midi_event(type the_type = type::NONE, unsigned value1 = 0, unsigned value2 = 0) :
			m_type(the_type),
			m_value1(value1),
			m_value2(value2)
		{
			
		}

	};
	
	typedef std::shared_ptr<midi_event> midi_event_ptr;
}

#endif
