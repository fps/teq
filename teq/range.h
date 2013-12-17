#ifndef LIBTEQ_RANGE_HH
#define LIBTEQ_RANGE_HH

namespace teq
{
	struct range
	{
		transport_position m_start;
		
		transport_position m_end;
	};
	
	struct loop_range : range
	{
		bool m_enabled;
		
		loop_range
		(
			bool enabled = false
		) :
			m_enabled(enabled)
		{
			
		}
	};
} // namespace		
#endif