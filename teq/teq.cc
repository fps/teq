#include <teq/teq.h>
#include <cassert>

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
		transport_position the_transport_position,
		bool send_all_notes_off_on_loop,
		bool send_all_notes_off_on_stop
	)
	{
		m_client_name = client_name;
		
		m_ack = false;
		
		m_transport_position = the_transport_position;
		
		m_transport_state = the_transport_state;
		
		m_global_tempo = 8.0;
		
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
		
		m_time_since_last_tick = 0;
		
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
	void insert_track(const std::string &name, song_ptr new_song, unsigned index, jack_port_t *port)
	{
		new_song->m_tracks->insert
		(
			new_song->m_tracks->begin() + index, 
			std::make_pair(track_ptr(new TrackType(name)), port)
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
		
		insert_track<sequence_of<midi_event>, midi_track>(track_name, new_song, index, port);

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
		
		insert_track<sequence_of<cv_event>, cv_track>(track_name, new_song, index, port);

		update_song(new_song);
	}
	
	void teq::insert_control_track(const std::string &track_name, unsigned index)
	{
		check_track_name_and_index_for_insert(track_name, index);
		
		song_ptr new_song = copy_and_prepare_song();

		insert_track<sequence_of<control_event>, control_track>(track_name, new_song, index, nullptr);

		update_song(new_song);
	}
	
	size_t teq::number_of_tracks()
	{
		return m_song->m_tracks->size();
	}
	
	std::string teq::track_name(unsigned index)
	{
		return (*m_song->m_tracks)[index].first->m_name;
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
			// std::cout << "Creating track" << std::endl;
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

		// std::cout << "Pattern list has # of entries: " << new_pattern_list->size() << std::endl;
		write_command_and_wait
		(
			[this, new_pattern_list] () mutable
			{
				m_song->m_patterns = new_pattern_list;
				new_pattern_list.reset();
			}
		);
	}

	void teq::set_pattern(unsigned index, const pattern &the_pattern)
	{	
		if (index >= m_song->m_patterns->size())
		{
			LIBTEQ_THROW_RUNTIME_ERROR("Pattern index out of bounds: " << index << ". Number of patterns: " << number_of_patterns())
		}
		song::pattern_list_ptr new_pattern_list = m_pattern_list_heap.add_new(song::pattern_list(*m_song->m_patterns));
		
		(*new_pattern_list)[index] = the_pattern;

		// std::cout << "Pattern list has # of entries: " << new_pattern_list->size() << std::endl;
		write_command_and_wait
		(
			[this, new_pattern_list] () mutable
			{
				m_song->m_patterns = new_pattern_list;
				new_pattern_list.reset();
			}
		);
	}

	pattern teq::get_pattern(unsigned index)
	{
		m_song->check_pattern_index(index);
		return (*m_song->m_patterns)[index];
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
	
	teq::state_info teq::get_state_info()
	{
		if (true == m_state_info_buffer.can_read())
		{
			state_info info;
			
			while(true == m_state_info_buffer.can_read())
			{
				info = m_state_info_buffer.read();
			}
			return info;
		}
		else
		{
			LIBTEQ_THROW_RUNTIME_ERROR("No state info available right now.")
		}
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
		if (false == m_command_buffer.can_write())
		{
			throw std::runtime_error("Failed to write command");
		}
		
		m_command_buffer.write(f);
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
			
			while(m_command_buffer.can_read())
			{
				m_command_buffer.snoop()();
				m_command_buffer.read_advance();
			}
			
			m_ack = true;
			
			m_ack_condition_variable.notify_all();
		}
		catch(std::system_error &e)
		{
			// locking failed
		}		
	}
	
	void teq::update_transport()
	{
		
	}
	
	void teq::fetch_port_buffers(jack_nframes_t nframes)
	{
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
	}

	void teq::write_cv_ports(unsigned frame_index)
	{
		/**
			* Write out CV values. This has to happen for every single frame
			* since it's a continous signal as opposed to the event based 
			* midi and control signals
			*/
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
	}
	
	int teq::process(jack_nframes_t nframes)
	{
		//std::cout << ".";
		if (true == m_state_info_buffer.can_write())
		{
			//std::cout << ".";
			state_info info;
			
			info.m_transport_position = m_transport_position;
			info.m_transport_state = m_transport_state;
			info.m_loop_range = m_loop_range;
			
			m_state_info_buffer.write(info);
		}
		
		process_commands();
		
		if (m_transport_state == transport_state::STOPPED)
		{
			return 0;
		}
		
		const float sample_duration = 1.0 / jack_get_sample_rate(m_jack_client);
		
		fetch_port_buffers(nframes);
		
		const std::vector<pattern> &patterns = *m_song->m_patterns;
		
		for (jack_nframes_t frame_index = 0; frame_index < nframes; ++frame_index)
		{
			const float tick_duration = 1.0 / (m_relative_tempo * m_global_tempo);
			
			update_transport();
			
			/**
			 * Here come all the state transitions that depend on the ticks
			 * and not individual frames.
			 */
			if (m_time_since_last_tick >= tick_duration)
			{
				m_time_since_last_tick -= tick_duration;
				
				/** 
					Safeguard around being off the song..
				*/
				if (m_transport_position.m_pattern < patterns.size() && m_transport_position.m_tick < patterns[m_transport_position.m_pattern].m_length)
				{
					++m_transport_position.m_tick;
				
					/**
					* Wrap tick around if we meet the pattern boundary
					*/
					if (m_transport_position.m_tick >= patterns[m_transport_position.m_pattern].m_length)
					{
						//std::cout << "pattern end" << std::endl;
						m_transport_position.m_tick = 0;
						++m_transport_position.m_pattern;
					}

					/**
					* Wrap aound to loop start if we hit the loop endl
					*/
					if 
					(
						true == m_loop_range.m_enabled &&
						m_loop_range.m_end.m_pattern == m_transport_position.m_pattern &&
						m_loop_range.m_end.m_tick == m_transport_position.m_tick
					)
					{
						//std::cout << "loop end" << std::endl;
						m_transport_position.m_pattern = m_loop_range.m_start.m_pattern;
						m_transport_position.m_tick = m_loop_range.m_start.m_tick;
					}
					
					/**
					* And stop the transport and do nothing if we ran out of the
					* end of the song
					*/
					if (m_transport_position.m_pattern >= patterns.size())
					{
						m_transport_state = transport_state::STOPPED;
						// std::cout << "end" << std::endl;
						return 0;
					}
				}
				
				assert(m_transport_position.m_pattern < m_loop_range.m_end.m_pattern);
				assert(m_transport_position.m_pattern >= m_loop_range.m_start.m_pattern);
				assert(m_transport_position.m_tick < m_loop_range.m_end.m_tick);
				assert(m_transport_position.m_tick >= m_loop_range.m_start.m_tick);
				
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
									render_event(midi::midi_note_on_event(properties.m_channel, the_event.m_value1, the_event.m_value2), properties.m_port_buffer, frame_index);
									properties.m_last_note = the_event.m_value1;
									break;
									
								case midi_event::OFF:
									render_event(midi::midi_note_off_event(properties.m_channel, properties.m_last_note, 127), properties.m_port_buffer, frame_index);
									break;
									
								case midi_event::CC:
									render_event(midi::midi_cc_event(properties.m_channel, the_event.m_value1, the_event.m_value2), properties.m_port_buffer, frame_index);
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
			
			write_cv_ports(frame_index);
			
			m_time_since_last_tick += sample_duration;
		}

		return 0;
	}
}