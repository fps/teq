#include <teq/teq.h>

namespace teq
{
	extern "C" 
	{
		int jack_process(jack_nframes_t nframes, void *arg)
		{
			return ((teq*)arg)->process(nframes);
		}
		
		int jack_transport(jack_transport_state_t state, jack_position_t *position, void *arg)
		{
			return ((teq*)arg)->transport(state, position);
		}
	}
}