#ifndef LIBTEQ_ENGINE_HH
#define LIBTEQ_ENGINE_HH

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <utility>
#include <stdexcept>

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
		
		typedef std::shared_ptr<lart::junk<track> > track_junk_ptr;
		
		teq(const std::string &client_name = "teq", unsigned command_buffer_size = 1024) :
			m_commands(command_buffer_size),
			m_client_name(client_name),
			m_track(m_heap.add(track())),
			m_send_all_notes_off_on_loop(true),
			m_send_all_notes_off_on_stop(true),
			m_last_effective_position(0)
		{
			jack_status_t status;
			m_jack_client = jack_client_open(client_name.c_str(), JackNullOption, &status);
			
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
				
		void set_track(const track &track)
		{
			auto new_track = m_heap.add(track);
			
			write_command
			(
				[this, new_track]() mutable
				{
					this->m_track = new_track; 
					new_track.reset();
				}
			);
		}
		
		void set_loop_range(const track::range &range)
		{
			write_command
			(
				[this, range]()
				{
					this->m_loop_range.m_enabled = range.m_enabled;
					this->m_loop_range.m_start = range.m_start;
					this->m_loop_range.m_end = range.m_end;
				}
			);
		}
		
		
	protected:
		
		lart::heap m_heap;
		
		lart::ringbuffer<command> m_commands;

		std::string m_client_name;
		
		track_junk_ptr m_track;
		
		track::range m_loop_range;
		
		jack_client_t *m_jack_client;
		
		jack_port_t *m_jack_port;
		
		jack_transport_state_t m_last_transport_state;
		
		bool m_send_all_notes_off_on_loop;
		
		bool m_send_all_notes_off_on_stop;
		
		jack_nframes_t m_last_effective_position;
		
	protected:
		
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
		
		void render_event(const midi_event &e, void *port_buffer)
		{
			jack_midi_data_t *event_buffer = jack_midi_event_reserve(port_buffer, 0, e.size());
			e.render(event_buffer);
		}
		
		int process(jack_nframes_t nframes)
		{
			while(m_commands.can_read())
			{
				m_commands.snoop()();
				m_commands.read_advance();
			}
			
			jack_position_t transport_position;
			
			void *port_buffer = jack_port_get_buffer(m_jack_port, nframes);

			const jack_transport_state_t transport_state = jack_transport_query(m_jack_client, &transport_position);
			
			if 
			(
				m_send_all_notes_off_on_stop &&
				m_last_transport_state == JackTransportRolling && 
				transport_state != JackTransportRolling
			)
			{
				midi_all_notes_off_event e(0);
				render_event(e, port_buffer);
			}
			
			m_last_transport_state = transport_state;
			
			if (JackTransportRolling != transport_state)
			{
				return 0;
			}

			
			jack_midi_clear_buffer(port_buffer);
			
			auto events_it = m_track->t.m_events.lower_bound(effective_position(transport_position.frame, 0));
			
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
					render_event(e, port_buffer);
				}
				
				m_last_effective_position = effective_frame;
				
				if 
				(
					m_loop_range.m_enabled && 
					effective_frame == m_loop_range.m_start && 
					frame != 0
				)
				{
					events_it = m_track->t.m_events.lower_bound(effective_frame);
				}
				
				while 
				(
					events_it != m_track->t.m_events.end() && 
					events_it->first == effective_frame
				)
				{
					render_event(*(events_it->second), port_buffer);
					
					++events_it;
				}
			}
			
			return 0;
		}
		
		friend int process_midi(jack_nframes_t, void*);
	};
}


#endif
