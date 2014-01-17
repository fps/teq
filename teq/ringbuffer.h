#ifndef RINGBUFFER_HH
#define RINGBUFFER_HH

#include <jack/ringbuffer.h>
#include <memory>

namespace lart
{

	/**
		T needs to be a default constructable type.. And it has to have valid
		copy constructor/assignment operator

		Note that this class needs to construct n objects of type T (with n == size)  so 
		that the places in the ringbuffer become assignable

		Note that the objects remain in the ringbuffer until they are overwritten again
		when the ringbuffer returns to the current position the next time around. I.e. a
		read() does not assign a T() to the read object as that could cause destructors
		to be called, etc..

		Note that read() creates a copy of the T, so the T(const T&) should be non blocking
	*/
	template <class T> 
	struct ringbuffer {
		unsigned int size;

		jack_ringbuffer_t *jack_ringbuffer;
		
		char *m_transfer_buffer;

		ringbuffer(unsigned int size) : size(size) {
			m_transfer_buffer = new char[sizeof(T)];
			
			jack_ringbuffer = jack_ringbuffer_create(sizeof(T) * size);
		}

		~ringbuffer() {
			delete m_transfer_buffer;
			
			jack_ringbuffer_free(jack_ringbuffer);
		}

		bool can_write() {
			if (jack_ringbuffer_write_space(jack_ringbuffer) >= sizeof(T)) {
				return true;
			}

			return false;
		}

		void write(const T &t) {
			*((T*)m_transfer_buffer) = t;
			jack_ringbuffer_write(jack_ringbuffer, m_transfer_buffer, sizeof(T));
		}

		bool can_read() {
			if (jack_ringbuffer_read_space(jack_ringbuffer) >= sizeof(T)) {
				return true;
			}

			return false;
		}

		void read_advance()
		{
			jack_ringbuffer_read_advance(jack_ringbuffer, sizeof(T));
		}
		
		T read() {
			jack_ringbuffer_read(jack_ringbuffer, m_transfer_buffer, sizeof(T));
			return *((T*)m_transfer_buffer);
		}

		T snoop() {
			jack_ringbuffer_peek(jack_ringbuffer, m_transfer_buffer, sizeof(T));
			return *((T*)m_transfer_buffer);
		}
	};
}

#endif
