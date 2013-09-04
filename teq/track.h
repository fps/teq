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
				
		track()
		{
			
		}
	};
	
	typedef std::shared_ptr<track> track_ptr;
	
	
	struct global_track_properties
	{
		virtual ~global_track_properties() { }
	};

	typedef std::shared_ptr<global_track_properties> global_track_properties_ptr;
	
	
	struct midi_track : track
	{
		typedef std::shared_ptr<event_column<note_event>> note_column_ptr;
		typedef std::shared_ptr<std::vector<note_column_ptr>> column_list_ptr;
		
		column_list_ptr m_column_list;
	};
	
	
	struct global_midi_track_properties : global_track_properties
	{
		std::array<bool, 16> m_channels;

		global_midi_track_properties()
		{
			m_channels[0] = true;
		}
	};
	
	typedef event_column<cv_event> cv_track;

	typedef event_column<control_event> control_track;
	
}

#endif
