#ifndef LIBTEQ_ENGINE_HH
#define LIBTEQ_ENGINE_HH

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <utility>
#include <stdexcept>
#include <mutex>
#include <condition_variable>
#include <algorithm>

#include <jack/jack.h>
#include <jack/midiport.h>

#include <lart/ringbuffer.h>

#include <teq/event.h>
#include <teq/midi_event.h>
#include <teq/song.h>

namespace teq
{
	extern "C" 
	{
		int jack_process(jack_nframes_t nframes, void *arg);
	}
	
	struct teq
	{
		typedef std::function<void()> command;
		
		typedef uint64_t tick;

		struct transport_position
		{
			tick m_pattern;
			
			tick m_tick;
			
			transport_position(tick pattern = 0, tick the_tick = 0) :
				m_pattern(pattern),
				m_tick(the_tick)
			{
				
			}
		};
		
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
		
		template <class T>
		struct heap
		{
			typedef std::shared_ptr<T> T_ptr;
			
			std::list<T_ptr> m_heap;
			
			T_ptr add(T_ptr ptr)
			{
				m_heap.push_back(ptr);
				return ptr;
			}
			
			T_ptr add_new(T &&t)
			{
				T_ptr ptr = std::make_shared<T>(t);
				return add(ptr);
			}
			
			void gc()
			{
				for (auto it = m_heap.begin(); it != m_heap.end();) {
					if (it->unique()) {
						std::cout << "Erasing..." << std::endl;
						it = m_heap.erase(it);
					} else {
						++it;
					}
				}	
			}
		};
		
		
		enum transport_state { STOPPED, PLAYING };
		
		enum transport_source { INTERNAL, JACK_TRANSPORT };
		
		enum track_type { MIDI, CV, CONTROL };
		
		
		heap<song> m_song_heap;
		
		heap<song::global_track_properties_list> m_global_track_properties_list_heap;
		
		heap<pattern> m_pattern_heap;
		
		heap<song::pattern_list> m_pattern_list_heap;
		
		heap<event> m_event_heap;
		
		
		lart::ringbuffer<command> m_commands;
		
		std::mutex m_ack_mutex;
		
		std::condition_variable m_ack_condition_variable;
		
		bool m_ack;

		
		std::string m_client_name;
		
		jack_client_t *m_jack_client;
		
		transport_state m_last_transport_state;
		
		
		song_ptr m_song;
		
		loop_range m_loop_range;
		
		float m_tempo;
		
		
		transport_state m_transport_state;
		
		transport_source m_transport_source;
		
		tick m_frames_since_last_tick;
		
		transport_position m_transport_position;
		
		
		bool m_send_all_notes_off_on_loop;
		
		bool m_send_all_notes_off_on_stop;
		
		
		teq(const std::string &client_name = "teq", unsigned command_buffer_size = 128) :
			m_commands(command_buffer_size),
			m_ack(false),
			m_client_name(client_name),
			m_transport_state(STOPPED),
			m_transport_source(INTERNAL),
			m_send_all_notes_off_on_loop(true),
			m_send_all_notes_off_on_stop(true)
		{
			init();
		}
		
		teq(const teq &other) :
			m_commands(other.m_commands.size),
			m_ack(false),
			m_client_name(other.m_client_name),
			m_transport_state(STOPPED),
			m_transport_source(INTERNAL),
			m_send_all_notes_off_on_loop(other.m_send_all_notes_off_on_loop),
			m_send_all_notes_off_on_stop(other.m_send_all_notes_off_on_stop)
		{
			init();
		}
	
		void init()
		{
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
			
			if (m_transport_source == transport_source::JACK_TRANSPORT)
			{
				m_last_transport_state = 
					jack_transport_query(m_jack_client, 0) == JackTransportRolling ? transport_state::PLAYING : transport_state::STOPPED;
			}
			else
			{
				m_last_transport_state = transport_state::STOPPED;
			}
			
			int activate_return_code = jack_activate(m_jack_client);
			
			if (0 != activate_return_code)
			{
				jack_client_close(m_jack_client);
				throw std::runtime_error("Failed to activate jack client");
			}
		}
		
		~teq()
		{
			jack_deactivate(m_jack_client);
			jack_client_close(m_jack_client);
		}
		
		void set_send_all_notes_off_on_loop(bool on)
		{
			write_command_and_wait
			(
				[this, on]()
				{
					m_send_all_notes_off_on_loop = on;
				}
			);
		}
		
		void set_send_all_notes_off_on_stop(bool on)
		{
			write_command_and_wait
			(
				[this, on]()
				{
					m_send_all_notes_off_on_stop = on;
				}
			);
		}
		
		bool track_name_exists(const std::string track_name)
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
		
		song_ptr copy_and_prepare_song_for_track_insert()
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
		
		bool insert_midi_track(const std::string &track_name, unsigned index)
		{
			if (true == track_name_exists(track_name))
			{
				return false;
			}
			
			if (index > number_of_tracks())
			{
				return false;
			}
			
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
				return false;
			}
			
			insert_track<midi_track, global_midi_track_properties>(new_song, index, (void *)port);

			update_song(new_song);
			
			return true;
		}
		
		bool insert_cv_track(const std::string &track_name, unsigned index)
		{
			if (true == track_name_exists(track_name))
			{
				return false;
			}
			
			if (index > number_of_tracks())
			{
				return false;
			}
			
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
				return false;
			}
			
			insert_track<cv_track, global_cv_track_properties>(new_song, index, (void*)port);

			update_song(new_song);
			
			return true;
		}
		
		bool insert_control_track(const std::string &track_name, unsigned index)
		{
			if (true == track_name_exists(track_name))
			{
				return false;
			}
			
			if (index > number_of_tracks())
			{
				return false;
			}
			
			song_ptr new_song = copy_and_prepare_song_for_track_insert();

			insert_track<control_track, global_control_track_properties>(new_song, index, (void *)nullptr);

			update_song(new_song);
			
			return true;
		}
		
		//! For internal use only!
		template <class TrackType, class TrackPropertiesType>
		void insert_track(song_ptr new_song, unsigned index, void *port)
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
		
		size_t number_of_tracks()
		{
			return m_song->m_tracks->size();
		}
		
		bool remove_track(unsigned index);
		
		bool move_track(unsigned from, unsigned to);
		
		bool insert_pattern(unsigned index, unsigned pattern_length)
		{
			if (index > m_song->m_patterns->size())
			{
				return false;
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
			
			return true;
		}
	
		bool remove_pattern(unsigned index);
		
		bool move_pattern(unsigned from, unsigned to);
		
		bool clear_midi_event(unsigned pattern_index, unsigned track_index, unsigned column_index, unsigned tick)
		{
			midi_event_ptr new_midi_event;
			
			write_command_and_wait
			(
				[this, new_midi_event, pattern_index, track_index, column_index, tick] () mutable
				{
					auto track_ptr = std::dynamic_pointer_cast<midi_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
					track_ptr->m_columns[column_index].m_events[tick] = new_midi_event;
				}
			);
			
			return true;
		}
		
		bool set_midi_event
		(
			unsigned pattern_index, 
			unsigned track_index, 
			unsigned column_index, 
			unsigned tick, 
			midi_event::type type, 
			unsigned value1, 
			unsigned value2
		)
		{
			if (pattern_index >= m_song->m_patterns->size())
			{
				return false;
			}
			
			midi_event_ptr new_midi_event(new midi_event);
			
			m_event_heap.add(new_midi_event);

			new_midi_event->m_type = type;
			
			new_midi_event->m_value1 = value1;
			
			new_midi_event->m_value2 = value2;			
			
			write_command_and_wait
			(
				[this, new_midi_event, pattern_index, track_index, column_index, tick] () mutable
				{
					auto track_ptr = std::dynamic_pointer_cast<midi_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
					track_ptr->m_columns[column_index].m_events[tick] = new_midi_event;
					new_midi_event.reset();
				}
			);
			
			return true;
		}
		
		bool clear_cv_event(unsigned pattern_index, unsigned track_index, unsigned tick)
		{
			cv_event_ptr new_cv_event;
			
			write_command_and_wait
			(
				[this, new_cv_event, pattern_index, track_index, tick] () mutable
				{
					auto track_ptr = std::dynamic_pointer_cast<cv_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
					track_ptr->m_cv_column.m_events[tick] = new_cv_event;
				}
			);
			
			return true;
		}
		
		bool set_cv_event
		(
			unsigned pattern_index, 
			unsigned track_index, 
			unsigned tick, 
			cv_event::type type, 
			float value1, 
			float value2
		)
		{
			if (pattern_index >= m_song->m_patterns->size())
			{
				return false;
			}
			
			cv_event_ptr new_cv_event(new cv_event);
			
			m_event_heap.add(new_cv_event);
			
			new_cv_event->m_type = type;
			
			new_cv_event->m_value1 = value1;
			
			new_cv_event->m_value2 = value2;
			
			write_command_and_wait
			(
				[this, new_cv_event, pattern_index, track_index, tick] () mutable
				{
					auto track_ptr = std::dynamic_pointer_cast<cv_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
					track_ptr->m_cv_column.m_events[tick] = new_cv_event;
					new_cv_event.reset();
				}
			);
			
			return true;
		}
		
		bool clear_control_event(unsigned pattern_index, unsigned track_index, unsigned tick)
		{
			control_event_ptr new_control_event;
			
			write_command_and_wait
			(
				[this, new_control_event, pattern_index, track_index, tick] () mutable
				{
					auto track_ptr = std::dynamic_pointer_cast<control_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
					track_ptr->m_control_column.m_events[tick] = new_control_event;
				}
			);	
			return true;
		}
		
		bool set_control_event
		(
			unsigned pattern_index, 
			unsigned track_index, 
			unsigned tick, 
			control_event::type type, 
			float value1, 
			float value2
		)
		{
			if (pattern_index >= m_song->m_patterns->size())
			{
				return false;
			}
			
			control_event_ptr new_control_event(new control_event);
			
			m_event_heap.add(new_control_event);
			
			new_control_event->m_type = type;
			
			new_control_event->m_value1 = value1;
			
			new_control_event->m_value2 = value2;
			
			write_command_and_wait
			(
				[this, new_control_event, pattern_index, track_index, tick] () mutable
				{
					auto track_ptr = std::dynamic_pointer_cast<control_track>((*m_song->m_patterns)[pattern_index].m_tracks[track_index]);
					track_ptr->m_control_column.m_events[tick] = new_control_event;
					new_control_event.reset();
				}
			);
			
			return true;
		}

		void set_loop_range(const loop_range &range)
		{
			write_command_and_wait
			(
				[this, range]()
				{
					this->m_loop_range = range;
				}
			);
		}
		
		void set_global_tempo(float tempo)
		{
			write_command_and_wait
			(
				[this, tempo]()
				{
					this->m_tempo = tempo;
				}
			);
		}	
		
		void set_transport_state(transport_state state)
		{
			write_command_and_wait
			(
				[this, state]()
				{
					this->m_transport_state = state;
				}
			);
		}
		
		
		void set_transport_position(transport_position position)
		{
			write_command_and_wait
			(
				[this, position]()
				{
					this->m_transport_position = position;
				}
			);
		}
		
		void set_transport_source(transport_source source)
		{
			write_command_and_wait
			(
				[this, source]()
				{
					this->m_transport_source = source;
				}
			);
		}
		
		void gc()
		{
			m_song_heap.gc();
			m_global_track_properties_list_heap.gc();
			m_pattern_heap.gc();
			m_pattern_list_heap.gc();
			m_event_heap.gc();
		}
		
		void write_command(command f)
		{
			if (false == m_commands.can_write())
			{
				throw std::runtime_error("Failed to write command");
			}
			
			m_commands.write(f);
		}
		
		void write_command_and_wait(command f)
		{
			std::unique_lock<std::mutex> lock(m_ack_mutex);
			m_ack = false;
			
			write_command(f);
			
			m_ack_condition_variable.wait(lock, [this]() { return this->m_ack; });
		}

		void update_song(song_ptr new_song)
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

#if 0
		inline jack_nframes_t effective_position(jack_nframes_t transport_frame, jack_nframes_t process_frame)
		{
			const jack_nframes_t frame = transport_frame + process_frame;
			
			if 
			(
				false == m_loop_range.m_enabled || 
				frame < m_loop_range.m_end
			)
			{
				return frame;
			}
			else
			{
				return m_loop_range.m_start + ((frame - m_loop_range.m_start) % (m_loop_range.m_end - m_loop_range.m_start));
			}
			
			// Above list of conditions is exhaustive. The compiler doesn't seem 
			// to see that. So we make him happy..
			throw std::logic_error("This should never happen");
		}
#endif

		void render_event(const midi::midi_event &e, void *port_buffer, jack_nframes_t time)
		{
			jack_midi_data_t *event_buffer = jack_midi_event_reserve(port_buffer, time, e.size());
			e.render(event_buffer);
		}
		
		int process(jack_nframes_t nframes)
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
		
		friend int jack_process(jack_nframes_t, void*);
	};
}


#endif
