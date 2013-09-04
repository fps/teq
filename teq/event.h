#ifndef LIBTEQ_EVENTS_HH
#define LIBTEQ_EVENTS_HH

#include <memory>

namespace teq
{
	struct control_event
	{
		enum type { TEMPO, RELATIVE_TEMPO };
		
		type m_type;
		
		//! Start value: TEMPO: global tempo, RELATIVE_TEMPO: factor relative to global tempo
		float m_value1;
		
		//! End value
		float m_value2;
	};
	
	struct cv_event
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
	
	struct note_event
	{
		enum type { ON, OFF, CC, PITCHBEND };
		
		type m_type;

		//! ON: note, OFF: ignored, CC: controller, PITCHBEND: bend
		unsigned m_value1;
		
		//! ON: velocity, OFF, CC, PITCHBEND: ignored
		unsigned m_value2;
	};
	
}

#endif
