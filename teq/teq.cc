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
	
	song_ptr teq::copy_and_prepare_song()
	{
		song_ptr new_song = m_song_heap.add_new(song(*m_song));
		
		new_song->m_tracks =
			m_track_list_heap
				.add_new(song::track_list(*m_song->m_tracks));
		
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
	
	track::type teq::track_type(unsigned index)
	{
		m_song->check_track_index(index);
		
		return (*m_song->m_tracks)[index].first->m_type;
	}
	
	//! For internal use only!
	template <class SequenceType, class TrackType>
	void insert_track(song_ptr new_song, unsigned index, jack_port_t *port)
	{
		new_song->m_tracks->insert
		(
			new_song->m_tracks->begin() + index, 
			std::make_pair(track_ptr(new TrackType()), port)
		);
		
		for (auto &it : *new_song->m_patterns)
		{
			it.m_sequences.insert
			(
				it.m_sequences.begin() + index,
				sequence_ptr(new SequenceType)
			);
			
			(*(it.m_sequences.begin() + index))->set_length(it.m_length);
		}
	}
	
	void teq::insert_midi_track(const std::string &track_name, unsigned index)
	{
		check_track_name_and_index_for_insert(track_name, index);
		
		song_ptr new_song = copy_and_prepare_song();
		
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
		
		insert_track<sequence_of<midi_event>, midi_track>(new_song, index, port);

		update_song(new_song);
	}
	
	void teq::insert_cv_track(const std::string &track_name, unsigned index)
	{
		check_track_name_and_index_for_insert(track_name, index);
		
		song_ptr new_song = copy_and_prepare_song();
		
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
		
		insert_track<sequence_of<cv_event>, cv_track>(new_song, index, port);

		update_song(new_song);
	}
	
	void teq::insert_control_track(const std::string &track_name, unsigned index)
	{
		check_track_name_and_index_for_insert(track_name, index);
		
		song_ptr new_song = copy_and_prepare_song();

		insert_track<sequence_of<control_event>, control_track>(new_song, index, nullptr);

		update_song(new_song);
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
		m_song->check_pattern_index(pattern_index);
		
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
		throw std::logic_error("Not implemented yet");
	}
	
	pattern teq::create_pattern(unsigned pattern_length)
	{
		pattern new_pattern;
		
		new_pattern.m_length = pattern_length;
		
		for (auto &it : *m_song->m_tracks)
		{
			std::cout << "Creating track" << std::endl;
			sequence_ptr new_sequence = it.first->create_sequence();
			
			new_sequence->set_length(pattern_length);
			
			new_pattern.m_sequences.push_back(new_sequence);
		}
		
		return new_pattern;
	}
	
	void teq::insert_pattern(unsigned index, const pattern &the_pattern)
	{	
		if (index > m_song->m_patterns->size())
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Pattern index out of bounds: " << index << ". Number of patterns: " << number_of_patterns())
		}
		song::pattern_list_ptr new_pattern_list = m_pattern_list_heap.add_new(song::pattern_list(*m_song->m_patterns));
		
		new_pattern_list->insert(new_pattern_list->begin() + index, the_pattern);

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
		throw std::logic_error("Not implemented yet");
	}
	
	void teq::move_pattern(unsigned from, unsigned to)
	{
		throw std::logic_error("Not implemented yet");		
	}
	

	loop_range teq::get_loop_range()
	{
		return m_loop_range;
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
	
	float teq::get_global_tempo()
	{
		return m_global_tempo;
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
		m_track_list_heap.gc();
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

	
	void teq::wait()
	{
		write_command_and_wait([](){});
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
	
	void teq::process_commands()
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
	}
	
	int teq::process(jack_nframes_t nframes)
	{
		process_commands();
		
		if (m_transport_state == transport_state::STOPPED)
		{
			return 0;
		}
		
		const float sample_duration = 1.0 / jack_get_sample_rate(m_jack_client);
		
		for (size_t track_index = 0; track_index < m_song->m_tracks->size(); ++track_index)
		{
			auto &track_properties = *(*m_song->m_tracks)[track_index].first;
			jack_port_t *port = (*m_song->m_tracks)[track_index].second;
			
			switch(track_properties.m_type)
			{
				case track::type::CV:
				{
					auto &properties = *((cv_track*)&track_properties);
					
					properties.m_port_buffer = jack_port_get_buffer(port, nframes);
				}
				break;

				case track::type::MIDI:
				{
					auto &properties = *((midi_track*)&track_properties);
					
					properties.m_port_buffer = jack_port_get_buffer(port, nframes);
					
					jack_midi_clear_buffer(properties.m_port_buffer);
				}
				break;

				default:
					break;
			}
		}
		
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
					std::cout << "<" << std::endl;
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
						case track::type::MIDI:
						{
							const auto &the_sequence = *std::static_pointer_cast<sequence_of<midi_event>>(the_pattern.m_sequences[track_index]);
							auto &properties = *((midi_track*)&track_properties);

							const auto &the_event = the_sequence.m_events[current_tick];

							switch(the_event.m_type)
							{
								case midi_event::NONE:
									break;
									
								case midi_event::ON:
									render_event(midi::midi_note_on_event(0, the_event.m_value1, the_event.m_value2), properties.m_port_buffer, frame_index);
									break;
									
								case midi_event::OFF:
									render_event(midi::midi_note_off_event(0, the_event.m_value1, the_event.m_value2), properties.m_port_buffer, frame_index);
									break;
									
								case midi_event::CC:
									render_event(midi::midi_cc_event(0, the_event.m_value1, the_event.m_value2), properties.m_port_buffer, frame_index);
									break;
									
								case midi_event::PITCHBEND:
									break;
							}
						}
						break;
							
						case track::type::CV:
						{
							const auto &the_sequence = *std::static_pointer_cast<sequence_of<cv_event>>(the_pattern.m_sequences[track_index]);
							auto &cv_properties = *((cv_track*)&track_properties);
							const auto &the_previous_event = cv_properties.m_current_event;
							
							if (cv_event::type::INTERVAL == the_previous_event.m_type)
							{
								cv_properties.m_current_value = the_previous_event.m_value2;
							}
							
							const auto &the_event = the_sequence.m_events[current_tick];
							cv_properties.m_current_event = the_event;
						}
						break;

						case track::type::CONTROL:
						{
							const auto &the_sequence = *std::static_pointer_cast<sequence_of<control_event>>(the_pattern.m_sequences[track_index]);
							const auto &the_event = the_sequence.m_events[current_tick];
							
							switch (the_event.m_type)
							{
								case control_event::type::GLOBAL_TEMPO:
									m_global_tempo = the_event.m_value;
									break;
									
								case control_event::type::RELATIVE_TEMPO:
									m_relative_tempo = the_event.m_value;
									break;
									
								default: 
									break;
							}
						}
						break;

						default:
							break;
					}
					
					
				}
			}
			
			for (size_t track_index = 0; track_index < m_song->m_tracks->size(); ++track_index)
			{
				auto &track_properties = *(*m_song->m_tracks)[track_index].first;
				
				switch(track_properties.m_type)
				{
					case track::type::CV:
					{
						auto &cv_properties = *((cv_track*)&track_properties);
						
						((float*)(cv_properties.m_port_buffer))[frame_index] = cv_properties.m_current_value;
					}
					break;

					default:
						break;
				}
			}
			m_time_since_last_tick += sample_duration;
		}

		return 0;
	}
}