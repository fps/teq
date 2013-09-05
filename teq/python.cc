#include <teq/teq.h>

#include <boost/python.hpp>

BOOST_PYTHON_MODULE(teq)
{
	using namespace boost::python;
	
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
		.def("number_of_tracks", &teq::teq::number_of_tracks)
		.def("insert_midi_track", &teq::teq::insert_midi_track)
	;
	
	enum_<teq::teq::transport_state>("transport_state")
		.value("STOPPED", teq::teq::transport_state::STOPPED)
		.value("PLAYING", teq::teq::transport_state::PLAYING)
	;

	enum_<teq::teq::transport_source>("transport_source")
		.value("INTERNAL", teq::teq::transport_source::INTERNAL)
		.value("JACK_TRANSPORT", teq::teq::transport_source::JACK_TRANSPORT)
	;

	enum_<teq::teq::track_type>("track_type")
		.value("MIDI", teq::teq::track_type::MIDI)
		.value("CV", teq::teq::track_type::CV)
		.value("CONTROL", teq::teq::track_type::CONTROL)
	;
}