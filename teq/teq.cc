#include <teq/teq.h>

namespace teq
{
	extern "C" 
	{
		int process_midi(jack_nframes_t nframes, void *arg)
		{
			return ((teq*)arg)->process(nframes);
		}
	}
}