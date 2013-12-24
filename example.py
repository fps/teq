# Let's import the teq module. Make sure the dynamic linker is setup to find libteq.so. Also
# make sure that python finds teq.so (the python module).
import teq

# Let's import the little python library that makes interoperability much easier
from pyteq import *


# Create a teq object. This creates the jack client, too..
t = teq.teq()

# Set the loop range.
set_loop_range(t, 0, 0, 1, 0, True)

# Create some tracks. Tracks have a name that do no need to be unique.
print ("Adding a midi track...")
t.insert_midi_track("foo", 0)

print ("Adding a midi track...")
t.insert_midi_track("bar", 1)

print ("Adding a CV track...")
t.insert_cv_track("cv", 2)

print ("Adding a control track...")
t.insert_control_track("control", 3)

# Let's create a pattern. We can only create patterns using the factory function of 
# the teq instance. It knows how many sequences the pattern has to have and their types.
p = t.create_pattern(16)

print ("Inserting a CV event...")
p.set_cv_event(2, 0, teq.cv_event(teq.cv_event_type.INTERVAL, 1, 1))

print ("Inserting a control event...")
p.set_control_event(3, 0, teq.control_event(teq.control_event_type.GLOBAL_TEMPO, 32))

for n in range(0, 16):
	print ("Adding a midi note at tick ", n, " with note ", n, "...")
	p.set_midi_event(0, n, teq.midi_event(teq.midi_event_type.ON, n, 64))
	p.set_midi_event(1, n, teq.midi_event(teq.midi_event_type.CC, n, 64))

t.insert_pattern(0, p)

t.wait()

print ("Cleaning up some memory...")
t.gc()

t.set_global_tempo(4)

print ("Setting the transport position and starting playback...")
set_transport_position(t, 0, 0)

play(t)

# Wait for the user to press a key...
try:
	i = input("Press Enter to continue...")
except:
	pass
