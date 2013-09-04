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
	
	class_<teq::teq::loop_range>("loop_range")
		.def_readwrite("enabled", &teq::teq::loop_range::m_enabled)
		.def_readwrite("start", &teq::teq::loop_range::m_start)
		.def_readwrite("end", &teq::teq::loop_range::m_end)
	;
	
	class_<teq::teq>("teq", init<optional<std::string, unsigned>>())
		.def("set_global_tempo", &teq::teq::set_global_tempo)
		.def("set_loop_range", &teq::teq::set_loop_range)
		.def("set_transport_state", &teq::teq::set_transport_state)
		.def("set_transport_source", &teq::teq::set_transport_source)
		.def("set_transport_position", &teq::teq::set_transport_position)
		.def("set_send_all_notes_off_on_loop", &teq::teq::set_send_all_notes_off_on_loop)
		.def("set_send_all_notes_off_on_stop", &teq::teq::set_send_all_notes_off_on_stop)
		.def("gc", &teq::teq::gc)
	;
}