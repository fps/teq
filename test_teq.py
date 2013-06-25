import teq
r = teq.track_range()
r.enabled = True
r.start = 0
r.end = 48000

t = teq.teq("foo", 100)
t.set_loop_range(r)

tr = libteq.track()

tr.add_note_on(0, 0, 64, 64)
tr.clear_range(0, 48000)

for n in range(0, 512):
	tr.add_note_on(n * 48000/512, 0, 64, 64)
	
t.set_track(tr)

try:
	i = input("Press Enter to continue...")
except:
	pass