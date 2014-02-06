#ifndef LIBTEQ_RANGE_HH
#define LIBTEQ_RANGE_HH

#include <teq/transport.h>

namespace teq
{
	struct range
	{
		transport_position m_start;
		
		transport_position m_end;
		
		range
		(
			const transport_position &start = transport_position(), 
			const transport_position &end = transport_position()
		) :
			m_start(start),
			m_end(end)
		{
			
		}
	};
	
	struct loop_range : range
	{
		bool m_enabled;
		
		loop_range
		(
			const transport_position &start = transport_position(),
			const transport_position &end = transport_position(),
			bool enabled = false
		) :
			range(start, end),
			m_enabled(enabled)
		{
			
		}

		loop_range
		(
			tick start_pattern,
			tick start_tick,
			tick end_pattern,
			tick end_tick,
			bool enabled
		) :
			range(transport_position(start_pattern, start_tick),
			      transport_position(end_pattern, end_tick)),
			m_enabled(enabled)
		{
		}

	};
} // namespace		
#endif
