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

#define LIBTEQ_THROW_RUNTIME_ERROR(x) \
{ \
	std::stringstream exception_string_stream; \
	exception_string_stream << x; \
	throw std::runtime_error(exception_string_stream.str().c_str()); \
}

namespace teq
{
	extern "C" 
	{
		int jack_process
		(
			jack_nframes_t nframes, 
			void *arg
		);
	}
	
	struct teq
	{
		typedef std::function<void()> command;
		
		typedef uint64_t tick;

		struct transport_position
		{
			tick m_pattern;
			
			tick m_tick;
			
			transport_position
			(
				tick pattern = 0, 
				tick the_tick = 0
			) :
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
		
		heap<song> m_song_heap;
		
		heap<song::track_list> m_track_list_heap;
		
		heap<pattern> m_pattern_heap;
		
		heap<song::pattern_list> m_pattern_list_heap;
		
		
		lart::ringbuffer<command> m_commands;
		
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
		
		
		teq(const std::string &client_name = "teq", unsigned command_buffer_size = 128) :
			m_commands(command_buffer_size),
			m_ack(false)
		{
			init
			(
				client_name,
				transport_state::STOPPED,
				true,
				true
			);
		}
		
		teq(const teq &other) :
			m_commands(other.m_commands.size),
			m_ack(false)
		{
			init
			(
				other.m_client_name,
				other.m_transport_state,
				other.m_send_all_notes_off_on_loop,
				other.m_send_all_notes_off_on_stop
			);
		}
	
		void init
		(
			const std::string &client_name,
			transport_state the_transport_state,
			bool send_all_notes_off_on_loop,
			bool send_all_notes_off_on_stop
		);
		
		~teq();
		
		void set_send_all_notes_off_on_loop
		(
			bool on
		);
		
		void set_send_all_notes_off_on_stop
		(
			bool on
		);
		
		bool track_name_exists
		(
			const std::string track_name
		);
		
		song_ptr copy_and_prepare_song_for_track_insert();
		
		void check_track_name_and_index_for_insert
		(
			const std::string &track_name, 
			unsigned index
		);
		
		void check_track_index
		(
			unsigned index
		);
		
		void check_pattern_index
		(
			unsigned index
		);
		

		void check_column_index
		(
			unsigned track_index, 
			unsigned column_index
		);
		
		void check_tick_index
		(
			unsigned pattern_index, 
			unsigned tick_index
		);
		
		track::type track_type
		(
			unsigned index
		);
		
		void insert_midi_track
		(
			const std::string &track_name, 
			unsigned index
		);
		
		void insert_cv_track
		(
			const std::string &track_name, 
			unsigned index
		);
		
		void insert_control_track
		(
			const std::string &track_name, 
			unsigned index
		);
		
		size_t number_of_tracks();
		
		size_t number_of_patterns();
		
		size_t number_of_ticks
		(
			unsigned pattern_index
		);
		
		void remove_track
		(
			unsigned index
		);
		
		void move_track
		(
			unsigned from, 
			unsigned to
		);
		
		void insert_pattern
		(
			unsigned index, 
			unsigned pattern_length
		);
	
		void remove_pattern
		(
			unsigned index
		);
		
		void move_pattern
		(
			unsigned from, 
			unsigned to
		);

		template<class EventType>
		void set_event
		(
			unsigned pattern_index,
			unsigned track_index,
			unsigned tick_index,
			const EventType &event
		)
		{
			check_pattern_index(pattern_index);
			
			check_track_index(track_index);
			
			check_tick_index(pattern_index, tick_index);
			
			write_command_and_wait
			(
				[this, event, pattern_index, track_index, tick_index] () mutable
				{
					auto sequence_ptr = std::dynamic_pointer_cast<sequence_of<EventType>>((*m_song->m_patterns)[pattern_index].m_sequences[track_index]);
					sequence_ptr->m_events[tick_index] = event;
				}
			);	
		}
		
		template<class EventType>
		EventType get_event
		(
			unsigned pattern_index,
			unsigned track_index,
			unsigned tick_index
		)
		{
			return EventType();
		}
		
		void set_loop_range
		(
			const loop_range &range
		);
		
		void set_global_tempo
		(
			float tempo
		);
		
		void set_transport_state
		(
			transport_state state
		);
		
		
		void set_transport_position
		(
			transport_position position
		);
		
		void gc();
		
		void write_command
		(
			command f
		);
		
		void write_command_and_wait
		(
			command f
		);
		
		void update_song
		(
			song_ptr new_song\
		);


		void render_event
		(
			const midi::midi_event &e, 
			void *port_buffer, 
			jack_nframes_t time
		);
		
		void process_commands();
		
		int process
		(
			jack_nframes_t nframes
		);
		
		friend int jack_process
		(
			jack_nframes_t, 
			void*
		);
	};
}


#endif
