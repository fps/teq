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
#include <sstream>

#include <jack/jack.h>
#include <jack/midiport.h>

#include <teq/ringbuffer.h>
#include <teq/event.h>
#include <teq/midi_event.h>
#include <teq/song.h>
#include <teq/range.h>
#include <teq/transport.h>
#include <teq/heap.h>

namespace teq
{
	extern "C" 
	{
		int jack_process(jack_nframes_t nframes, void *arg);
	}
	
	struct teq
	{
		struct state_info
		{
			transport_state m_transport_state;
			transport_position m_transport_position;
			loop_range m_loop_range;
			jack_nframes_t m_frame_time;
			bool m_is_tick;
		};
		
	protected:
		
		typedef std::function<void()> command;
		
		heap<song> m_song_heap;
		
		heap<song::track_list> m_track_list_heap;
		
		heap<song::pattern_list> m_pattern_list_heap;
		
		
		
		lart::ringbuffer<command> m_command_buffer;
		
		lart::ringbuffer<state_info> m_state_info_buffer;
		
		std::mutex m_ack_mutex;
		
		std::condition_variable m_ack_condition_variable;
		
		bool m_ack;

		
		std::string m_client_name;
		
		jack_client_t *m_jack_client;
		
		transport_state m_last_transport_state;
		
		
		song_ptr m_song;
		
		loop_range m_loop_range;
		
		float m_global_tempo;
		
		float m_relative_tempo;
		
		
		transport_state m_transport_state;
		
		transport_position m_transport_position;
		
		float m_time_since_last_tick;
		
		
		bool m_send_all_notes_off_on_loop;
		
		bool m_send_all_notes_off_on_stop;
		
	public:
		
		teq(const std::string client_name = "teq", int command_buffer_size = 1024, int state_info_buffer_size = 1024) :
			m_command_buffer(command_buffer_size),
 			m_state_info_buffer(state_info_buffer_size),
			m_ack(false)
		{
			init
			(
				client_name,
				transport_state::STOPPED,
				transport_position(),
				true,
				true
			);
		}
		
		teq(const teq &other) :
			m_command_buffer(other.m_command_buffer.size),
			m_state_info_buffer(other.m_state_info_buffer.size),
			m_ack(false)
		{
			init
			(
				other.m_client_name,
				other.m_transport_state,
				other.m_transport_position,
				other.m_send_all_notes_off_on_loop,
				other.m_send_all_notes_off_on_stop
			);
		}
	
		void init
		(
			const std::string client_name,
			transport_state the_transport_state,
			transport_position the_transport_position,
			bool send_all_notes_off_on_loop,
			bool send_all_notes_off_on_stop
		);
		
		~teq();

		void deactivate();
		
		void set_send_all_notes_off_on_loop(bool on);
		
		void set_send_all_notes_off_on_stop(bool on);
		
		bool track_name_exists(const std::string track_name);
		
		song_ptr copy_and_prepare_song();
		
		void check_track_name_and_index_for_insert(const std::string track_name, int index);
		
		
		int number_of_tracks();
		
		track::type track_type(int index);
		
		std::string track_name(int index);
		
		void insert_midi_track(const std::string track_name, int index);
		
		void insert_cv_track(const std::string track_name, int index);
		
		void insert_control_track(const std::string track_name, int index);
		
		void remove_track(int index);
		
		void move_track(int from, int to);
		

		int number_of_patterns();
		
		int number_of_ticks(int pattern_index);
		
		void insert_pattern(int index, const pattern the_pattern);
	
		void remove_pattern(int index);
		
		void move_pattern(int from, int to);
		
		void set_pattern(int index, const pattern the_pattern);
		
		pattern get_pattern(int pattern_index);

		/**
		 * Use this function to create new patterns. 
		 * 
		 * NOTE: If you change the number or ordering of tracks
		 * in the song after creating a pattern adding this pattern
		 * to the song will lead to undefined behaviour.
		 */
		pattern create_pattern(int length);
		
		
		void set_loop_range(const loop_range range);
		
		loop_range get_loop_range();
		
		void set_global_tempo(float tempo);
		
		float get_global_tempo();
		
		void set_transport_state(transport_state state);
		
		void set_transport_position(transport_position position);
		
		/**
		 * @short Get a recent state_info
		 * 
		 * NOTE: This function throws if it is called before 
		 * a new state_info is available. So make sure
		 * you catch that exception
		 */
		state_info get_state_info();
		
		void gc();
		
		void wait();
		
	protected:
		
		void write_command(command f);
		
		void write_command_and_wait(command f);
		
		void update_song(song_ptr new_song);

		void render_event(const midi::midi_event &e, void *port_buffer, jack_nframes_t time);
		
		void process_commands();
		
		void write_cv_ports(int frame_index);
		
		void update_transport();
		
		void fetch_port_buffers(jack_nframes_t nframes);
		
		int process(jack_nframes_t nframes);
		
		friend int jack_process(jack_nframes_t, void*);
	};
}


#endif
