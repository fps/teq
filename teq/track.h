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
		
		
		virtual track_ptr create_track() = 0;
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
	
	typedef std::shared_ptr<midi_track> midi_track_ptr;
	
	
	struct global_midi_track_properties : global_track_properties
	{
		unsigned m_number_of_columns;
		
		std::array<bool, 16> m_channels;

		global_midi_track_properties() : 
			m_number_of_columns(1)
		{
			m_channels[0] = true;
		}
		
		virtual track_ptr create_track()
		{
			midi_track_ptr new_midi_track(new midi_track);
			
			for (unsigned index = 0; index < m_number_of_columns; ++index)
			{
				new_midi_track->m_columns.push_back(midi_track::midi_column());
			}
			
			return new_midi_track;
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
