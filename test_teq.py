import teq

# Create a teq object. This creates the jack client, too..
t = teq.teq()

t.set_global_tempo(250.0)

# We want to set a loop range, so let's do it. 
r = teq.loop_range()

r.enabled = True

r.start.pattern = 0
r.start.tick = 0

r.end.pattern = 3
r.end.tick = 0

# Set the loop range.
t.set_loop_range(r)

# Create a track
print ("Adding a midi track...")
t.insert_midi_track("foo", 0)

print ("Inserting some patterns...")
t.insert_pattern(0, 128)
t.insert_pattern(1, 128)

print ("Adding a midi track...")
t.insert_midi_track("bar", 1)

print ("Inserting a pattern...")
t.insert_pattern(0, 128)

print ("Adding a CV track...")
t.insert_cv_track("cv", 2)

print ("Inserting a CV event...")
t.set_cv_event(0, 2, 0, teq.cv_event_type.ONE_SHOT, 0.3, 0.0)

print ("Adding a control track...")
t.insert_control_track("control", 3)

print ("Inserting a control event...")
t.set_control_event(0, 3, 0, teq.control_event_type.GLOBAL_TEMPO_ONE_SHOT, 260, 0)

for n in range(0, 128):
	print ("Adding a midi note at tick ", n, " with note ", n, "...")
	t.set_midi_event(0, 0, 0, n, teq.midi_event(teq.midi_event_type.ON, n, 64))

# Clean up some memory
t.gc()

# Set the transport position and play
p = teq.transport_position()
p.pattern = 0
p.tick = 0

t.set_transport_position(p)

t.set_transport_state(teq.transport_state.PLAYING)


# Wait for the user to press a key...
try:
	i = input("Press Enter to continue...")
except:
	pass
