import teq

t = teq.teq()

r = teq.loop_range()

r.enabled = True
r.start = 0
r.end = 48000

t.set_loop_range(r)

tr = teq.track()

tr.add_note_on(0, 0, 64, 64)
tr.clear_range(0, 48000)

for n in range(0, 512):
	tr.add_note_on(int(n * 48000/512), 0, 64, 64)
	
for n in range(0, 50):
	print ("track", n)
	t.set_track("foo", tr)
	t.set_track("bar", tr)
	t.gc()

try:
	i = input("Press Enter to continue...")
except:
	pass