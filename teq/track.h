#ifndef LIBTEQ_TRACK_HH
#define LIBTEQ_TRACK_HH

#include <memory>
#include <map>
#include <utility>
#include <iostream>
#include <array>

#include <teq/event.h>
#include <teq/column.h>

namespace teq
{
	struct track
	{
		virtual ~track() { }
		
		virtual void set_length(unsigned) = 0;
		
		track()
		{
			
		}
	};
	
	typedef std::shared_ptr<track> track_ptr;
	
	
	struct global_track_properties
	{
		const std::string m_name;
		
		virtual ~global_track_properties() { }
	};

	typedef std::shared_ptr<global_track_properties> global_track_properties_ptr;
	
	
	struct midi_track : track
	{
		typedef event_column<midi_event_ptr> midi_column;
		
		typedef std::vector<midi_column> column_list;
		
		column_list m_columns;
		
		virtual void set_length(unsigned length)
		{
			for (auto &it: m_columns)
			{
				it.m_events.resize(length);
			}
		}
	};
	
	
	struct global_midi_track_properties : global_track_properties
	{
		std::array<bool, 16> m_channels;

		global_midi_track_properties()
		{
			m_channels[0] = true;
		}
	};
	
	struct cv_track : track
	{
		event_column<cv_event> m_cv_column;
	};
	
	struct global_cv_track_properties
	{
		
	};
	
	struct control_track : track
	{
		event_column<control_event> m_control_column;
	};

	struct global_control_track_properties
	{
		
	};	
} // namespace

#endif
