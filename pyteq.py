import teq

# Some utility functions to make life easier in the long run
def set_loop_range(t, start_pattern, start_tick, end_pattern, end_tick, onoff):
	r = teq.loop_range(start_pattern, start_tick, end_pattern, end_tick, onoff)
	t.set_loop_range(r)

def set_loop_enabled(t, onoff):
	r = t.get_loop_range()
	r.enabled = onoff
	t.set_loop_range(r)
	
def set_transport_position(t, pattern, tick):
	t.set_transport_position(teq.transport_position(pattern, tick))

def play(t):
	t.set_transport_state(teq.transport_state.PLAYING)

def stop(t):
	t.set_transport_state(teq.transport_state.STOPPED)
	
def toggle_playback(t):
	state = t.get_transport_state()