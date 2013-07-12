import teq

# Create a teq object. This creates the jack client, too..
t = teq.teq()

# We want to set a loop range, so let's do it. Times are samplerate based.
r = teq.loop_range()

r.enabled = True
r.start = 0
r.end = 48000

# Set the loop range.
t.set_loop_range(r)

# Create a track
tr = teq.track()

# Add a note and clear the range immediately again.
tr.add_note_on(0, 0, 64, 64)
tr.clear_range(0, 48000)


# A 512th notes in the first second of the track.
for n in range(0, 512):
	tr.add_note_on(int(n * 48000/512), 0, 64, 64)

# Set the track 50 times just to test that it works
for n in range(0, 50):
	print ("track", n)
	t.set_track("foo", tr)
	t.set_track("bar", tr)
	t.gc()

# Wait for the user to press a key...
try:
	i = input("Press Enter to continue...")
except:
	pass
