import teq

# Some utility functions to make life easier in the long run
def set_loop_range(t, start_pattern, start_tick, end_pattern, end_tick, onoff):
	r = teq.loop_range()
	r.enabled = onoff
	r.start.pattern = start_pattern
	r.start.tick = start_tick
	r.end.pattern = end_pattern
	r.end.tick = end_tick
	t.set_loop_range(r)
	
def set_loop_enabled(t, onoff):
	r = t.get_loop_range()
	r.enabled = onoff
	t.set_loop_range(r)
	
def set_transport_position(t, pattern, tick):
	p = teq.transport_position()
	p.pattern = pattern
	p.tick = tick
	t.set_transport_position(p)

def play(t):
	t.set_transport_state(teq.transport_state.PLAYING)

def stop(t):
	t.set_transport_state(teq.transport_state.STOPPED)