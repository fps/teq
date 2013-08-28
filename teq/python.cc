#include <teq/teq.h>

#include <boost/python.hpp>

BOOST_PYTHON_MODULE(teq)
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
	
	class_<teq::track::loop_range>("loop_range")
		.def_readwrite("enabled", &teq::track::loop_range::m_enabled)
		.def_readwrite("start", &teq::track::loop_range::m_start)
		.def_readwrite("end", &teq::track::loop_range::m_end)
	;
	
	class_<teq::teq>("teq", init<optional<std::string, unsigned>>())
		.def("set_loop_range", &teq::teq::set_loop_range)
		.def("set_track", &teq::teq::set_track)
		.def("remove_track", &teq::teq::remove_track)
		.def("set_send_all_notes_off_on_loop", &teq::teq::set_send_all_notes_off_on_loop)
		.def("set_send_all_notes_off_on_stop", &teq::teq::set_send_all_notes_off_on_stop)
		.def("gc", &teq::teq::gc)
    ;
}