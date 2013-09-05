#include <teq/teq.h>

namespace teq
{
	extern "C" 
	{
		int jack_process(jack_nframes_t nframes, void *arg)
		{
			return ((teq*)arg)->process(nframes);
		}
	}
}