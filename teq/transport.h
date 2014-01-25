#ifndef LIBTEQ_TRANSPORT_HH
#define LIBTEQ_TRANSPORT_HH

namespace teq
{
	typedef long long int tick;

	enum transport_state { STOPPED, PLAYING };		
	
	enum transport_source { JACK_TRANSPORT, INTERNAL };
	
	struct transport_position
	{
		tick m_pattern;
		
		tick m_tick;
		
		transport_position
		(
			tick pattern = 0, 
			tick the_tick = 0
		) :
			m_pattern(pattern),
			m_tick(the_tick)
		{
			
		}
	};
} // namespace

#endif