#ifndef LIBTEQ_COLUMN_HH
#define LIBTEQ_COLUMN_HH

#include <memory>

namespace teq
{
	struct column
	{
		virtual ~column() { }
	};
	
	typedef std::shared_ptr<column> column_ptr;
	
	template <class EventType>
	struct event_column : column
	{
		typedef std::vector<EventType> event_list_ptr;
		
		event_list_ptr m_events;
	};
} // namespace

#endif

