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

#include <teq/track.h>
#include <lart/heap.h>
#include <lart/ringbuffer.h>

namespace teq
{
	extern "C" 
	{
		int process_midi(jack_nframes_t nframes, void *arg);
	}
	
	struct teq
	{
		typedef std::function<void()> command;
		
		typedef std::pair<track, jack_port_t *> track_with_port;
		
		typedef std::map<std::string, std::shared_ptr<track_with_port>> tracks_map;
		
		typedef std::shared_ptr<lart::junk<tracks_map>> tracks_junk_ptr;
		
		typedef std::shared_ptr<lart::junk<track> > track_junk_ptr;
		
		teq(const std::string &client_name = "teq", unsigned command_buffer_size = 128) :
			m_commands(command_buffer_size),
			m_ack(false),
			m_client_name(client_name),
			m_track(m_heap.add(track())),
			m_tracks(m_heap.add(tracks_map())),
			m_send_all_notes_off_on_loop(true),
			m_send_all_notes_off_on_stop(true),
			m_last_effective_position(0)
		{
			init();
		}
		
		teq(const teq &other) :
			m_commands(other.m_commands.size),
			m_ack(false),
			m_client_name(other.m_client_name),
			m_send_all_notes_off_on_loop(other.m_send_all_notes_off_on_loop),
			m_send_all_notes_off_on_stop(other.m_send_all_notes_off_on_stop),
			m_last_effective_position(0)
		{
			init();
			
			// TODO: set_track();
		}
	
		~teq()
		{
			jack_deactivate(m_jack_client);
			jack_client_close(m_jack_client);
		}
		
#if 0
		void set_track(const track &track)
		{
			auto new_track = m_heap.add(track);
			
			write_command_and_wait
			(
				[this, new_track]() mutable
				{
					this->m_track = new_track; 
					new_track.reset();
				}
			);
		}
#endif

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
		
		void set_loop_range(const track::loop_range &range)
		{
			write_command_and_wait
			(
				[this, range]()
				{
					this->m_loop_range = range;
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
		
		track_junk_ptr m_track;
		
		tracks_junk_ptr m_tracks;
		
		track::loop_range m_loop_range;
		
		jack_client_t *m_jack_client;
		
		jack_port_t *m_jack_port;
		
		jack_transport_state_t m_last_transport_state;
		
		bool m_send_all_notes_off_on_loop;
		
		bool m_send_all_notes_off_on_stop;
		
		jack_nframes_t m_last_effective_position;
		
	protected:

		void init()
		{
			jack_status_t status;
			m_jack_client = jack_client_open(m_client_name.c_str(), JackNullOption, &status);
			
			if (0 == m_jack_client)
			{
				throw std::runtime_error("Failed to open jack client");
			}
			
			m_jack_port = jack_port_register(m_jack_client, "out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
			
			if (0 == m_jack_port)
			{
				jack_client_close(m_jack_client);
				throw std::runtime_error("Failed to register jack output port");
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
					
					if 
					(
						m_send_all_notes_off_on_loop && 
						effective_frame != m_last_effective_position + 1
					)
					{
						midi_all_notes_off_event e(0);
						render_event(e, port_buffer, frame);
					}
					
					m_last_effective_position = effective_frame;
					
					if 
					(
						m_loop_range.m_enabled && 
						effective_frame == m_loop_range.m_start && 
						frame != 0
					)
					{
						events_it = events.lower_bound(effective_frame);
					}
					
					while 
					(
						events_it != events.end() && 
						events_it->first == effective_frame
					)
					{
						render_event(*(events_it->second), port_buffer, effective_frame);
						
						++events_it;
					}
				}
			}
			
			return 0;
		}
		
		friend int process_midi(jack_nframes_t, void*);
	};
}


#endif
