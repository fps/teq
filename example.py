# Let's import the teq module. Make sure the dynamic linker is setup to find libteq.so. Also
# make sure that python finds teq.so (the python module).
import teq

# Let's import the little python library that makes interoperability much easier
import pyteq


# Create a teq object. This creates the jack client, too..
t = teq.teq()

t.set_global_tempo(8)

# Set the loop range.
set_loop_range(t, 0, 0, 1, 0, True)

# Create a track
print ("Adding a midi track...")
t.insert_midi_track("foo", 0)

print ("Adding a midi track...")
t.insert_midi_track("bar", 1)

print ("Inserting a pattern...")
t.insert_pattern(0, 16)

print ("Adding a CV track...")
t.insert_cv_track("cv", 2)

print ("Inserting a CV event...")
#t.set_cv_event(0, 2, 0, teq.cv_event(teq.cv_event_type.INTERVAL, 1, 1))

print ("Adding a control track...")
#t.insert_control_track("control", 3)

print ("Inserting a control event...")
#t.set_control_event(0, 3, 0, teq.control_event(teq.control_event_type.GLOBAL_TEMPO, 16))

#for n in range(0, 16):
#	print ("Adding a midi note at tick ", n, " with note ", n, "...")
#t.set_midi_event(0, 0, n, teq.midi_event(teq.midi_event_type.ON, n, 64))
#	t.set_midi_event(0, 1, n, teq.midi_event(teq.midi_event_type.CC, n, 64))

t.wait()

print ("Cleaning up some memory...")
t.gc()

print ("Setting the transport position and starting playback...")
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
