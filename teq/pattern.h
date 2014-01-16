#ifndef LIBTEQ_PATTERN_HH
#define LIBTEQ_PATTERN_HH

#include <vector>

#include <teq/track.h>
#include <teq/exception.h>

namespace teq
{
	struct pattern
	{
		/**
		 * Patterns should only ever be created by the teq::create_pattern method. See that
		 * method for more information on how to safely create and edit patterns
		 */
		pattern(unsigned length = 128) :
			m_length(length),
			m_muted(false)
		{
			
		}
		
		pattern(const pattern &other) :
			m_length(other.m_length),
			m_name(other.m_name),
			m_muted(other.m_muted)
		{
			for (auto &s : other.m_sequences)
			{
				m_sequences.push_back(s->clone());
			}
		}

		typedef std::vector<sequence_ptr> sequence_list;
		
		sequence_list m_sequences;
		
		unsigned m_length;
		
		std::string m_name;
		
		bool m_muted;
		
		void mute_sequence(unsigned index, bool muted)
		{
			check_track_index(index);
			
			m_sequences[index]->m_muted = muted;
		}
		
		unsigned length()
		{
			return m_length;
		}
		
		void check_tick_index(unsigned the_tick)
		{
			if (the_tick >= length()) 
			{
				LIBTEQ_THROW_RUNTIME_ERROR("tick out of range: " << the_tick << " >= " << length())
			}
		}
		
		void check_track_index(unsigned the_track)
		{
			if (the_track >= m_sequences.size())
			{
				LIBTEQ_THROW_RUNTIME_ERROR("track out of range: " << the_track << " >= " << m_sequences.size())
			}
		}
		
		template<class EventType>
		void set_event
		(
			unsigned track_index,
			unsigned tick_index,
			const EventType &event
		)
		{
			check_track_index(track_index);
			
			check_tick_index(tick_index);
			
			auto sequence_ptr = std::dynamic_pointer_cast<sequence_of<EventType>>(m_sequences[track_index]);
			
			if (!sequence_ptr)
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Cast to sequence type failed. Did you try to set a wrong event type?")
			}
			
			sequence_ptr->m_events[tick_index] = event;
		}
	
		
		template<class EventType>
		EventType get_event
		(
			unsigned track_index,
			unsigned tick_index
		)
		{
			check_track_index(track_index);
			
			check_tick_index(tick_index);
			
			auto sequence_ptr = std::dynamic_pointer_cast<sequence_of<EventType>>(m_sequences[track_index]);
			
			if (!sequence_ptr)
			{
				LIBTEQ_THROW_RUNTIME_ERROR("Cast to sequence type failed. Did you try to set a wrong event type?")
			}
			
			return sequence_ptr->m_events[tick_index];
		}
		

	};
	
	typedef std::shared_ptr<pattern> pattern_ptr;
} // namespace

#endif

