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
		enum type { NONE, MIDI, CV, CONTROL };
		
		type m_type;

		const std::string m_name;
		
		virtual ~global_track_properties() { }
		
		global_track_properties(type the_type = type::NONE)  :
			m_type(the_type)
		{
			
		}
		
		virtual track_ptr create_track() = 0;
	};

	typedef std::shared_ptr<global_track_properties> global_track_properties_ptr;
	
	
	struct midi_track : track
	{
		typedef event_column<midi_event> midi_column;
		
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
			global_track_properties(global_track_properties::type::MIDI),
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
		std::vector<cv_event> m_events;
		
		virtual void set_length(unsigned length)
		{
			m_events.resize(length);
		}
	};
	
	struct global_cv_track_properties : global_track_properties
	{
		cv_event m_state;
		float m_time_since_event;
		
		virtual track_ptr create_track()
		{
			return track_ptr(new cv_track);
		}
		
		global_cv_track_properties() :
			global_track_properties(global_track_properties::type::CV),
			m_time_since_event(0)
		{
			
		}
	};
	
	struct control_track : track
	{
		std::vector<control_event> m_events;
		
		virtual void set_length(unsigned length)
		{
			m_events.resize(length);
		}
	};

	struct global_control_track_properties : global_track_properties
	{
		control_event m_state;
		float m_time_since_event;
		
		virtual track_ptr create_track()
		{
			return track_ptr(new control_track);
		}
		
		global_control_track_properties() :
			global_track_properties(global_track_properties::type::CONTROL),
			m_time_since_event(0)
		{
			
		}
	};	
} // namespace

#endif
