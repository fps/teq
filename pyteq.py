# Some utility functions to make life easier in the long run
def set_loop_range(t, start_pattern, start_tick, end_pattern, end_tick, onoff):
	r = teq.loop_range()
	r.enabled = onoff
	r.start_pattern = start_pattern
	r.start_tick = start_tick
	r.end_pattern = end_pattern
	r.end_tick = end_tick
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
