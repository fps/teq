import teq

# Create a teq object. This creates the jack client, too..
t = teq.teq()

t.set_global_tempo(250.0)

t.set_transport_position(0)

t.set_transport_source(teq.transport_source.INTERNAL)
t.set_transport_state(teq.transport_state.PLAYING)

# We want to set a loop range, so let's do it. Times are samplerate based.
r = teq.loop_range()

r.enabled = True
r.start = 0
r.end = 64

# Set the loop range.
t.set_loop_range(r)

# Create a track
t.insert_midi_track("foo", 0)
t.insert_pattern(0, 128)
t.insert_pattern(1, 128)

t.insert_midi_track("bar", 1)
t.insert_pattern(0, 128)

for n in range(0, 128):
	print ("note", n)
	t.set_midi_event(0, 0, 0, n, teq.midi_event_type.ON, n, 64)

# Clean up some memory
t.gc()

# Wait for the user to press a key...
try:
	i = input("Press Enter to continue...")
except:
	pass
