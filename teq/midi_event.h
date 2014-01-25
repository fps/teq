#ifndef LIBTEQ_MIDI_EVENTS_HH
#define LIBTEQ_MIDI_EVENTS_HH

#include <memory>

/* Binary constant generator macro
By Tom Torfs - donated to the public domain
*/

/* All macro's evaluate to compile-time constants */

/* turn a numeric literal into a hex constant
(avoids problems with leading zeroes)
8-bit constants max value 0x11111111, always fits in unsigned long
*/
#define HEX__(n) 0x##n##LU

/* 8-bit conversion function */
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

/* *** user macros *** */

/* for upto 8-bit binary constants */
#define B8(d) ((unsigned char)B8__(HEX__(d)))

/* for upto 16-bit binary constants, MSB first */
#define B16(dmsb,dlsb) (((unsigned short)B8(dmsb)<< \
+ B8(dlsb))

/* for upto 32-bit binary constants, MSB first */
#define B32(dmsb,db2,db3,dlsb) (((unsigned long)B8(dmsb)<<24) \
+ ((unsigned long)B8(db2)<<16) \
+ ((unsigned long)B8(db3)<< \
+ B8(dlsb))

namespace teq
{
	namespace midi
	{
		struct midi_event
		{
			virtual unsigned size() const = 0;
			
			virtual void render(unsigned char *buffer) const = 0;
			
			virtual ~midi_event() 
			{
				
			}
		};
		
		typedef std::shared_ptr<midi_event> midi_event_ptr;
		
		struct midi_channel_event : midi_event
		{
			protected:
				unsigned char m_channel;
				
			public:
				midi_channel_event(unsigned char channel) :
					m_channel(channel)
				{
					
				}
		};
		
		struct midi_note_on_event : midi_channel_event
		{
			protected:
				unsigned char m_note;
				
				unsigned char m_velocity;
				
			public:
				midi_note_on_event(unsigned char channel, unsigned char note, unsigned char velocity) :
					midi_channel_event(channel),
					m_note(note),
					m_velocity(velocity)
				{
					
				}
				
				virtual unsigned size() const
				{
					return 3;
				}
				
				virtual void render(unsigned char *buffer) const
				{
					buffer[0] = B8(10010000) | m_channel;
					buffer[1] = m_note;
					buffer[2] = m_velocity;
				}
		};
		
		struct midi_note_off_event : midi_channel_event
		{
			protected:
				unsigned char m_note;
				
				unsigned char m_velocity;
				
			public:
				midi_note_off_event(unsigned char channel, unsigned char note, unsigned char velocity) :
					midi_channel_event(channel),
					m_note(note),
					m_velocity(velocity)
				{
					
				}
				
				virtual unsigned size() const
				{
					return 3;
				}
				
				virtual void render(unsigned char *buffer) const
				{
					buffer[0] = B8(10000000) | m_channel;
					buffer[1] = m_note;
					buffer[2] = m_velocity;
				}
		};
		
		struct midi_cc_event : midi_channel_event
		{
			protected:
				unsigned char m_cc;
				
				unsigned char m_value;
				
			public:
				midi_cc_event(unsigned char channel, unsigned char cc, unsigned char value) :
					midi_channel_event(channel),
					m_cc(cc),
					m_value(value)
				{
					
				}
				
				virtual unsigned size() const
				{
					return 3;
				}
				
				virtual void render(unsigned char *buffer) const 
				{
					buffer[0] = B8(10110000) | m_channel;
					buffer[1] = m_cc;
					buffer[2] = m_value;
				}
		};
		
		struct midi_all_notes_off_event : midi_cc_event
		{
			midi_all_notes_off_event(unsigned char channel) :
				midi_cc_event(channel, 123, 0)
			{
				
			}
		};
	} // namespace
} // namespace

#endif
