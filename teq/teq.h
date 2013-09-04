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

#include <jack/jack.h>
#include <jack/midiport.h>

#include <lart/heap.h>
#include <lart/ringbuffer.h>

#include <teq/event.h>
#include <teq/midi_event.h>
#include <teq/song.h>

namespace teq
{
	extern "C" 
	{
		int process_midi(jack_nframes_t nframes, void *arg);
	}
	
	struct teq
	{
		typedef std::function<void()> command;
		
		typedef uint64_t tick;

		struct range
		{
			tick m_start;
			
			tick m_end;
			
			range(tick start, tick end) :
				m_start(start),
				m_end(end)
			{
			
			}
		};
		
		struct loop_range : range
		{
			bool m_enabled;
			
			loop_range(tick start = 0, tick end = 0, bool enabled = false) :
				range(start, end),
				m_enabled(enabled)
			{
				
			}
		};
		
		enum transport_state { STOPPED, PLAYING };
		
		enum transport_source { INTERNAL, JACK_TRANSPORT };
		
		teq(const std::string &client_name = "teq", unsigned command_buffer_size = 128) :
			m_commands(command_buffer_size),
			m_ack(false),
			m_client_name(client_name),
			m_transport_state(STOPPED),
			m_transport_source(INTERNAL),
			m_transport_position(0),
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
			m_transport_position(0),
			m_send_all_notes_off_on_loop(other.m_send_all_notes_off_on_loop),
			m_send_all_notes_off_on_stop(other.m_send_all_notes_off_on_stop)
		{
			init();
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
		
#if 0
		void add_track(const std::string &track_name);
		void remove_track(const std::string &track_name);
		
		void insert_midi_column(const std::string track_name, unsigned index);
		void remove_midi_column(const std::string track_name, unsigned index);
		unsigned number_of_midi_columns(const std::string track_name);
		
		void insert_cv_column(const std::string track_name, unsigned index);
		void remove_cv_column(const std::string track_name, unsigned index);
		unsigned number_of_cv_columns(const std::string track_name);
		
		
		void set_track(const std::string &name, const track &track)
		{
			auto new_tracks = m_heap.add(tracks_map(m_tracks->t));
			
			auto it = m_tracks->t.find(name);
			
			if (it == m_tracks->t.end())
			{
				jack_port_t *port = jack_port_register
				(
					m_jack_client, 
					name.c_str(), 
					JACK_DEFAULT_MIDI_TYPE, 
					JackPortIsOutput | JackPortIsTerminal,
					0
				);
				
				if (0 == port)
				{
					return;
				}
				
				track_with_port new_entry(track, port);
				new_tracks->t[name] = std::make_shared<track_with_port>(new_entry);
			}
			else
			{
				new_tracks->t[name]->first = track;
			}
			
			update_tracks(new_tracks);
		}
		
		void remove_track(const std::string &name)
		{
			auto it = m_tracks->t.find(name);
			
			if (it == m_tracks->t.end())
			{	
				return;
			}

			auto new_tracks = m_heap.add(tracks_map(m_tracks->t));
			
			auto it2 = new_tracks->t.find(name);
			
			if (it2 == new_tracks->t.end())
			{
				return;
			}
			
			jack_port_t *port = it->second->second;
			
			new_tracks->t.erase(it2);

			update_tracks(new_tracks);
			
			jack_port_unregister(m_jack_client, port);
		}
#endif

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
		
		void set_transport_position(tick position)
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
			m_heap.cleanup();
		}
		
	protected:
		
		lart::heap m_heap;
		
		lart::ringbuffer<command> m_commands;
		
		std::mutex m_ack_mutex;
		
		std::condition_variable m_ack_condition_variable;
		
		bool m_ack;

		
		std::string m_client_name;
		
		jack_client_t *m_jack_client;
		
		jack_transport_state_t m_last_transport_state;
		
		song_ptr m_song;
		
#if 0		
		tracks_junk_ptr m_tracks;
#endif
		
		loop_range m_loop_range;
		
		float m_tempo;
		
		transport_state m_transport_state;
		
		transport_source m_transport_source;
		
		tick m_transport_position;
		
		bool m_send_all_notes_off_on_loop;
		
		bool m_send_all_notes_off_on_stop;
		
	protected:

		void init()
		{
			jack_status_t status;
			m_jack_client = jack_client_open(m_client_name.c_str(), JackNullOption, &status);
			
			if (0 == m_jack_client)
			{
				throw std::runtime_error("Failed to open jack client");
			}
						
			int set_process_return_code = jack_set_process_callback(m_jack_client, process_midi, this);
			
			if (0 != set_process_return_code)
			{
				jack_client_close(m_jack_client);
				throw std::runtime_error("Failed to set jack process callback");
			}
			
			m_last_transport_state = jack_transport_query(m_jack_client, 0);
			
			int activate_return_code = jack_activate(m_jack_client);
			
			if (0 != activate_return_code)
			{
				jack_client_close(m_jack_client);
				throw std::runtime_error("Failed to activate jack client");
			}			
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

#if 0
		void update_tracks(tracks_junk_ptr new_tracks)
		{
			write_command_and_wait
			(
				[this, new_tracks] () mutable
				{
					m_tracks = new_tracks;
					new_tracks.reset();
				}
			);
		}
#endif

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
		
		void render_event(const midi_event &e, void *port_buffer, jack_nframes_t time)
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
		
		friend int process_midi(jack_nframes_t, void*);
	};
}


#endif
