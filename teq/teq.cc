#include <teq/teq.h>

namespace teq
{
	extern "C" 
	{
		int jack_process(jack_nframes_t nframes, void *arg)
		{
			return ((teq*)arg)->process(nframes);
		}
	}

	void teq::init
	(
		const std::string &client_name,
		transport_state the_transport_state,
		bool send_all_notes_off_on_loop,
		bool send_all_notes_off_on_stop
	)
	{
		m_client_name = client_name;
		
		m_ack = false;
		
		m_transport_state = the_transport_state;
		
		m_global_tempo = 125.0;
		
		m_relative_tempo = 1.0;
		
		m_send_all_notes_off_on_loop = send_all_notes_off_on_loop;
		
		m_send_all_notes_off_on_stop = send_all_notes_off_on_stop;
		
		m_song = m_song_heap.add_new(song());
		
		jack_status_t status;
		m_jack_client = jack_client_open(m_client_name.c_str(), JackNullOption, &status);
		
		if (0 == m_jack_client)
		{
			throw std::runtime_error("Failed to open jack client");
		}
		
		int set_process_return_code = jack_set_process_callback(m_jack_client, jack_process, this);
		
		if (0 != set_process_return_code)
		{
			jack_client_close(m_jack_client);
			throw std::runtime_error("Failed to set jack process callback");
		}
		
		m_last_transport_state = transport_state::STOPPED;
		
		int activate_return_code = jack_activate(m_jack_client);
		
		if (0 != activate_return_code)
		{
			jack_client_close(m_jack_client);
			throw std::runtime_error("Failed to activate jack client");
		}
	}
	
	teq::~teq()
	{
		jack_deactivate(m_jack_client);
		jack_client_close(m_jack_client);
	}
	
	void teq::set_send_all_notes_off_on_loop(bool on)
	{
		write_command_and_wait
		(
			[this, on]()
			{
				m_send_all_notes_off_on_loop = on;
			}
		);
	}
	
	void teq::set_send_all_notes_off_on_stop(bool on)
	{
		write_command_and_wait
		(
			[this, on]()
			{
				m_send_all_notes_off_on_stop = on;
			}
		);
	}
	
	bool teq::track_name_exists(const std::string track_name)
	{
		for (auto it : *(m_song->m_tracks))
		{
			if (track_name == it.first->m_name)
			{
				return true;
			}
		}
		
		return false;
	}
	
	song_ptr teq::copy_and_prepare_song_for_track_insert()
	{
		song_ptr new_song = m_song_heap.add_new(song(*m_song));
		
		new_song->m_tracks =
			m_global_track_properties_list_heap
				.add_new(song::global_track_properties_list(*m_song->m_tracks));
		
		new_song->m_patterns = 
			m_pattern_list_heap
				.add_new(song::pattern_list(*m_song->m_patterns));
				
		return new_song;
	}
	
	void teq::check_track_name_and_index_for_insert(const std::string &track_name, unsigned index)
	{
		if (true == track_name_exists(track_name))
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Track name already exists: " << track_name)
		}
		
		if (index > number_of_tracks())
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Track index out of bounds: " << index << ". Number of tracks: " << number_of_tracks())
		}
	}
	
	void teq::check_track_index(unsigned index)
	{
		if (index >= number_of_tracks())
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Track index out of bounds: " << index << ". Number of tracks: " << number_of_tracks())
		}
	}
	
	void teq::check_pattern_index(unsigned index)
	{
		if (index >= m_song->m_patterns->size())
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Pattern index out of bounds: " << index << ". Number of patterns: " << number_of_patterns())
		}
	}
	
	void teq::check_column_index(unsigned track_index, unsigned column_index)
	{
		
	}
	
	void teq::check_tick_index(unsigned pattern_index, unsigned tick_index)
	{
		check_pattern_index(pattern_index);
		
		if (tick_index >=  (*(m_song->m_patterns))[pattern_index].m_length)
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Tick index out of bounds: " << tick_index << ". Pattern length: " << (*(m_song->m_patterns))[pattern_index].m_length)
		}
	}
	
	global_track_properties::type teq::track_type(unsigned index)
	{
		check_track_index(index);
		
		return (*m_song->m_tracks)[index].first->m_type;
	}
	
	void teq::insert_midi_track(const std::string &track_name, unsigned index)
	{
		check_track_name_and_index_for_insert(track_name, index);
		
		song_ptr new_song = copy_and_prepare_song_for_track_insert();
		
		jack_port_t *port = jack_port_register
		(
			m_jack_client, 
			track_name.c_str(), 
			JACK_DEFAULT_MIDI_TYPE, 
			JackPortIsOutput | JackPortIsTerminal,
			0
		);
		
		if (0 == port)
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Failed to register jack port")
		}
		
		insert_track<midi_track, global_midi_track_properties>(new_song, index, (void *)port);

		update_song(new_song);
	}
	
	void teq::insert_cv_track(const std::string &track_name, unsigned index)
	{
		check_track_name_and_index_for_insert(track_name, index);
		
		song_ptr new_song = copy_and_prepare_song_for_track_insert();
		
		jack_port_t *port = jack_port_register
		(
			m_jack_client, 
			track_name.c_str(), 
			JACK_DEFAULT_AUDIO_TYPE, 
			JackPortIsOutput | JackPortIsTerminal,
			0
		);
		
		if (0 == port)
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Failed to register jack port")
		}
		
		insert_track<cv_track, global_cv_track_properties>(new_song, index, (void*)port);

		update_song(new_song);
	}
	
	void teq::insert_control_track(const std::string &track_name, unsigned index)
	{
		check_track_name_and_index_for_insert(track_name, index);
		
		song_ptr new_song = copy_and_prepare_song_for_track_insert();

		insert_track<control_track, global_control_track_properties>(new_song, index, (void *)nullptr);

		update_song(new_song);
	}
	
	//! For internal use only!
	template <class TrackType, class TrackPropertiesType>
	void teq::insert_track(song_ptr new_song, unsigned index, void *port)
	{
		new_song->m_tracks->insert
		(
			new_song->m_tracks->begin() + index, 
			std::make_pair(global_track_properties_ptr(new TrackPropertiesType()), port)
		);
		
		for (auto &it : *new_song->m_patterns)
		{
			it.m_tracks.insert
			(
				it.m_tracks.begin() + index,
				track_ptr(new TrackType)
			);
			
			(*(it.m_tracks.begin() + index))->set_length(it.m_length);
		}
		
	}
	
	size_t teq::number_of_tracks()
	{
		return m_song->m_tracks->size();
	}
	
	size_t teq::number_of_patterns()
	{
		return m_song->m_patterns->size();
	}
	
	size_t teq::number_of_ticks(unsigned pattern_index)
	{
		check_pattern_index(pattern_index);
		
		return (*m_song->m_patterns)[pattern_index].m_length;
	}
	
	void teq::remove_track(unsigned index)
	{
		if (index >= number_of_tracks())
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Track index out of bounds: " << index << ". Number of tracks: " << number_of_tracks())
		}
	}
	
	void teq::move_track(unsigned from, unsigned to)
	{
	
	}
	
	void teq::insert_pattern(unsigned index, unsigned pattern_length)
	{
		if (index > m_song->m_patterns->size())
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Pattern index out of bounds: " << index << ". Number of patterns: " << number_of_patterns())
		}
		
		pattern new_pattern;
		
		new_pattern.m_length = pattern_length;
		
		for (auto &it : *m_song->m_tracks)
		{
			std::cout << "Creating track" << std::endl;
			track_ptr new_track = it.first->create_track();
			
			new_track->set_length(pattern_length);
			
			new_pattern.m_tracks.push_back(new_track);
		}
		
		song::pattern_list_ptr new_pattern_list = m_pattern_list_heap.add_new(song::pattern_list(*m_song->m_patterns));
		
		new_pattern_list->insert(new_pattern_list->begin() + index, new_pattern);

		std::cout << "Pattern list has # of entries: " << new_pattern_list->size() << std::endl;
		write_command_and_wait
		(
			[this, new_pattern_list] () mutable
			{
				m_song->m_patterns = new_pattern_list;
				new_pattern_list.reset();
			}
		);
	}

	void teq::remove_pattern(unsigned index)
	{
		
	}
	
	void teq::move_pattern(unsigned from, unsigned to)
	{
		
	}
	

	void teq::set_midi_event
	(
		unsigned pattern_index, 
		unsigned track_index, 
		unsigned column_index, 
		unsigned tick_index, 
		const midi_event &event
	)
	{
		check_pattern_index(pattern_index);
		
		check_track_index(track_index);
		
		check_column_index(track_index, column_index);
		
		check_tick_index(pattern_index, tick_index);
		
		write_command_and_wait
		(
			[this, event, pattern_index, track_index, column_index, tick_index] () mutable
			{
				auto track_ptr = std::dynamic_pointer_cast<midi_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
				track_ptr->m_columns[column_index].m_events[tick_index] = event;
			}
		);			
	}

#if 0
	void set_midi_event
	(
		unsigned pattern_index, 
		unsigned track_index, 
		unsigned column_index, 
		unsigned tick_index, 
		midi_event::type type, 
		unsigned value1, 
		unsigned value2
	)
	{
		check_pattern_index(pattern_index);
		
		check_track_index(track_index);
		
		check_column_index(track_index, column_index);
		
		check_tick_index(pattern_index, tick_index);
		
		
		midi_event_ptr new_midi_event(new midi_event);
		
		m_event_heap.add(new_midi_event);

		new_midi_event->m_type = type;
		
		new_midi_event->m_value1 = value1;
		
		new_midi_event->m_value2 = value2;			
		
		write_command_and_wait
		(
			[this, new_midi_event, pattern_index, track_index, column_index, tick_index] () mutable
			{
				auto track_ptr = std::dynamic_pointer_cast<midi_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
				track_ptr->m_columns[column_index].m_events[tick_index] = new_midi_event;
				new_midi_event.reset();
			}
		);
	}
#endif

	midi_event teq::get_midi_event
	(
		unsigned pattern_index, 
		unsigned track_index, 
		unsigned column_index, 
		unsigned tick_index
	)
	{
		check_pattern_index(pattern_index);
		
		check_track_index(track_index);
		
		check_tick_index(pattern_index, tick_index);
		
		check_column_index(track_index, column_index);
		
		return 
			(std::dynamic_pointer_cast<midi_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index])
				->m_columns[column_index].m_events[tick_index]);
	}
	
	cv_event teq::get_cv_event	
	(
		unsigned pattern_index, 
		unsigned track_index, 
		unsigned tick_index
	)
	{
		check_pattern_index(pattern_index);
		
		check_track_index(track_index);
		
		check_tick_index(pattern_index, tick_index);

		return 
			(std::dynamic_pointer_cast<cv_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index])
				->m_events[tick_index]);
	}

	
	control_event teq::get_control_event
	(
		unsigned pattern_index, 
		unsigned track_index, 
		unsigned tick_index
	)
	{
		check_pattern_index(pattern_index);
		
		check_track_index(track_index);
		
		check_tick_index(pattern_index, tick_index);
		
		return 
			(std::dynamic_pointer_cast<control_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index])
				->m_events[tick_index]);
	}

		
	void teq::set_cv_event
	(
		unsigned pattern_index, 
		unsigned track_index, 
		unsigned tick_index, 
		const cv_event &event
	)
	{
		check_pattern_index(pattern_index);
		
		check_track_index(track_index);
		
		check_tick_index(pattern_index, tick_index);
		
		write_command_and_wait
		(
			[this, event, pattern_index, track_index, tick_index] () mutable
			{
				auto track_ptr = std::dynamic_pointer_cast<cv_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
				track_ptr->m_events[tick_index] = event;
			}
		);
	}
	
	void teq::set_control_event
	(
		unsigned pattern_index, 
		unsigned track_index, 
		unsigned tick_index, 
		const control_event &event
	)
	{
		check_pattern_index(pattern_index);
		
		check_track_index(track_index);
			
		check_tick_index(pattern_index, tick_index);
		
		write_command_and_wait
		(
			[this, event, pattern_index, track_index, tick_index] () mutable
			{
				auto track_ptr = std::dynamic_pointer_cast<control_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
				track_ptr->m_events[tick_index] = event;
			}
		);
	}

	void teq::set_loop_range(const loop_range &range)
	{
		write_command_and_wait
		(
			[this, range]()
			{
				this->m_loop_range = range;
			}
		);
	}
	
	void teq::set_global_tempo(float tempo)
	{
		write_command_and_wait
		(
			[this, tempo]()
			{
				this->m_global_tempo = tempo;
			}
		);
	}	
	
	void teq::set_transport_state(transport_state state)
	{
		write_command_and_wait
		(
			[this, state]()
			{
				this->m_transport_state = state;
			}
		);
	}
	
	
	void teq::set_transport_position(transport_position position)
	{
		write_command_and_wait
		(
			[this, position]()
			{
				this->m_transport_position = position;
			}
		);
	}
	
	void teq::gc()
	{
		m_song_heap.gc();
		m_global_track_properties_list_heap.gc();
		m_pattern_heap.gc();
		m_pattern_list_heap.gc();
	}
	
	void teq::write_command(command f)
	{
		if (false == m_commands.can_write())
		{
			throw std::runtime_error("Failed to write command");
		}
		
		m_commands.write(f);
	}
	
	void teq::write_command_and_wait(command f)
	{
		std::unique_lock<std::mutex> lock(m_ack_mutex);
		m_ack = false;
		
		write_command(f);
		
		m_ack_condition_variable.wait(lock, [this]() { return this->m_ack; });
	}

	void teq::update_song(song_ptr new_song)
	{
		write_command_and_wait
		(
			[this, new_song] () mutable
			{
				m_song = new_song;
				new_song.reset();
			}
		);
	}

	void teq::render_event(const midi::midi_event &e, void *port_buffer, jack_nframes_t time)
	{
		jack_midi_data_t *event_buffer = jack_midi_event_reserve(port_buffer, time, e.size());
		e.render(event_buffer);
	}
	
	
	int teq::process(jack_nframes_t nframes)
	{
		try
		{
			std::unique_lock<std::mutex> lock(m_ack_mutex, std::try_to_lock);
			
			while(m_commands.can_read())
			{
				m_commands.snoop()();
				m_commands.read_advance();
			}
			
			m_ack = true;
			
			m_ack_condition_variable.notify_all();
		}
		catch(std::system_error &e)
		{
			// locking failed
		}
		
		if (m_transport_state == transport_state::STOPPED)
		{
			return 0;
		}
		
		const float sample_duration = 1.0 / jack_get_sample_rate(m_jack_client);
		
		const std::vector<pattern> &patterns = *m_song->m_patterns;
		
		for (jack_nframes_t frame_index = 0; frame_index < nframes; ++frame_index)
		{
			const float tick_duration = 1.0 / (m_relative_tempo * m_global_tempo);
			
			if (m_time_since_last_tick >= tick_duration)
			{
				m_time_since_last_tick -= tick_duration;
				
				++m_transport_position.m_tick;
				
				if (m_transport_position.m_pattern >= patterns.size())
				{
					return 0;
				}
				
				if (m_transport_position.m_tick >= patterns[m_transport_position.m_pattern].m_length)
				{
					m_transport_position.m_tick = 0;
					++m_transport_position.m_pattern;
				}
		
				if 
				(
					true == m_loop_range.m_enabled &&
					m_loop_range.m_end.m_pattern == m_transport_position.m_pattern &&
					m_loop_range.m_end.m_tick == m_transport_position.m_tick
				)
				{
					std::cout << "<" << std::endl;
					
					m_transport_position.m_pattern = m_loop_range.m_start.m_pattern;
					m_transport_position.m_tick = m_loop_range.m_start.m_tick;
				}
				
				if (m_transport_position.m_pattern >= patterns.size())
				{
					return 0;
				}
				
				const pattern &the_pattern = patterns[m_transport_position.m_pattern];
				const int current_tick = m_transport_position.m_tick;
				
				for (size_t track_index = 0; track_index < m_song->m_tracks->size(); ++track_index)
				{
					auto &track_properties = *(*m_song->m_tracks)[track_index].first;
					
					switch(track_properties.m_type)
					{
						case global_track_properties::type::MIDI:
						{
							
						}
						break;
							
						case global_track_properties::type::CV:
						{
							const auto &the_track = *std::static_pointer_cast<cv_track>(the_pattern.m_tracks[track_index]);
							auto &cv_properties = *((global_cv_track_properties*)&track_properties);
							
							const auto &the_event = the_track.m_events[current_tick];
							cv_properties.m_current_event = the_event;
						}
						break;

						case global_track_properties::type::CONTROL:
						{
							const auto &the_track = *std::static_pointer_cast<control_track>(the_pattern.m_tracks[track_index]);
							auto &control_properties = *((global_control_track_properties*)&track_properties);
							
							const auto &the_event = the_track.m_events[current_tick];
							control_properties.m_current_event = the_event;
						}
						break;

						default:
							break;
					}
					
					
				}
			}
			
			m_time_since_last_tick += sample_duration;
		}
#if 0
		const tracks_map &tracks = m_tracks->t;;

		jack_position_t transport_position;
		
		const jack_transport_state_t transport_state = jack_transport_query(m_jack_client, &transport_position);

		if 
		(
			m_send_all_notes_off_on_stop &&
			m_last_transport_state == JackTransportRolling && 
			transport_state != JackTransportRolling
		)
		{
			for (auto track_it = tracks.begin(); track_it != tracks.end(); ++track_it)
			{
				void *port_buffer = jack_port_get_buffer(track_it->second->second, nframes);
				midi_all_notes_off_event e(0);
				render_event(e, port_buffer, 0);
			}
		}
		
		m_last_transport_state = transport_state;
		
		if (JackTransportRolling != transport_state)
		{
			return 0;
		}

		for (auto track_it = tracks.begin(); track_it != tracks.end(); ++track_it)
		{
			void *port_buffer = jack_port_get_buffer(track_it->second->second, nframes);
			
			jack_midi_clear_buffer(port_buffer);
			
			const track::events_map &events = track_it->second->first.m_events;
			
			auto events_it = events.lower_bound(effective_position(transport_position.frame, 0));
			
			// std::cout << ":";
			for (jack_nframes_t frame = 0; frame < nframes; ++frame)
			{
				const jack_nframes_t effective_frame = effective_position(transport_position.frame, frame);
				
				const jack_nframes_t last_effective_frame = effective_position(transport_position.frame, frame - 1);
				
				if 
				(
					m_send_all_notes_off_on_loop && 
					effective_frame != last_effective_frame + 1
				)
				{
					events_it = events.lower_bound(effective_frame);
					midi_all_notes_off_event e(0);
					render_event(e, port_buffer, frame);
				}

				while 
				(
					events_it != events.end() && 
					events_it->first == effective_frame
				)
				{
					render_event(*(events_it->second), port_buffer, frame);
					
					++events_it;
				}
			}
		}
#endif	
		return 0;
	}
}