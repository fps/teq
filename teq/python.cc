#include <teq/teq.h>


BOOST_PYTHON_MODULE(libteq)
{
	using namespace boost::python;
	
	class_<teq::midi_note_on_event>("note_on", init<unsigned, unsigned, unsigned>())
	;
	
	class_<teq::midi_note_off_event>("note_off", init<unsigned, unsigned, unsigned>())
	;
	
	class_<teq::midi_cc_event>("cc", init<unsigned, unsigned, unsigned>())
	;
	
	class_<teq::midi_all_notes_off_event>("all_notes_off", init<unsigned>())
	;
	
	class_<teq::track>("track")
		.def("clear", &teq::track::clear)
		.def("clear_range", &teq::track::clear_range)
		.def("copy_range", &teq::track::copy_range)
		.def("add_note_on", &teq::track::add_note_on)
		.def("add_note_off", &teq::track::add_note_off)
		.def("add_all_notes_off", &teq::track::add_note_off)
		.def("add_cc", &teq::track::add_cc)
	;
	
	class_<teq::track::range>("track_range")
		.def_readwrite("enabled", &teq::track::range::m_enabled)
		.def_readwrite("start", &teq::track::range::m_start)
		.def_readwrite("end", &teq::track::range::m_end)
	;
	
	class_<teq::teq>("teq", init<std::string, unsigned>())
		.def("set_loop_range", &teq::teq::set_loop_range)
		.def("set_track", &teq::teq::set_track)
    ;
}