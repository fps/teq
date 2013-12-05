#include <teq/teq.h>

#include <boost/python.hpp>

BOOST_PYTHON_MODULE(teq)
{
	using namespace boost::python;
	
	class_<teq::teq::transport_position>("transport_position")
		.def_readwrite("pattern", &teq::teq::transport_position::m_pattern)
		.def_readwrite("tick", &teq::teq::transport_position::m_tick)
	;
	
	class_<teq::teq::loop_range>("loop_range")
		.def_readwrite("enabled", &teq::teq::loop_range::m_enabled)
		.def_readwrite("start", &teq::teq::loop_range::m_start)
		.def_readwrite("end", &teq::teq::loop_range::m_end)
	;
	
	class_<teq::midi_event>("midi_event", init<optional<teq::midi_event::type, unsigned, unsigned>>())
		.def_readwrite("type", &teq::midi_event::m_type)
		.def_readwrite("value1", &teq::midi_event::m_value1)
		.def_readwrite("value2", &teq::midi_event::m_value2)
	;

	class_<teq::cv_event>("cv_event", init<optional<teq::cv_event::type, float, float>>())
		.def_readwrite("type", &teq::cv_event::m_type)
		.def_readwrite("value1", &teq::cv_event::m_value1)
		.def_readwrite("value2", &teq::cv_event::m_value2)
	;

	class_<teq::control_event>("control_event", init<optional<teq::control_event::type, float>>())
		.def_readwrite("type", &teq::control_event::m_type)
		.def_readwrite("value1", &teq::control_event::m_value)
	;

	class_<teq::teq>("teq", init<optional<std::string, unsigned>>())
		.def("gc", &teq::teq::gc)
		.def("set_global_tempo", &teq::teq::set_global_tempo)
		.def("set_loop_range", &teq::teq::set_loop_range)
		.def("set_transport_state", &teq::teq::set_transport_state)
		.def("set_transport_position", &teq::teq::set_transport_position)
		.def("set_send_all_notes_off_on_loop", &teq::teq::set_send_all_notes_off_on_loop)
		.def("set_send_all_notes_off_on_stop", &teq::teq::set_send_all_notes_off_on_stop)
		.def("number_of_tracks", &teq::teq::number_of_tracks)
		.def("insert_midi_track", &teq::teq::insert_midi_track)
		.def("insert_cv_track", &teq::teq::insert_cv_track)
		.def("insert_control_track", &teq::teq::insert_control_track)
		.def("insert_pattern", &teq::teq::insert_pattern)
		.def("set_midi_event", &teq::teq::set_event<teq::midi_event>)
		.def("get_midi_event", &teq::teq::get_event<teq::midi_event>)
		.def("set_control_event", &teq::teq::set_event<teq::control_event>)
		.def("get_control_event", &teq::teq::get_event<teq::control_event>)
		.def("set_cv_event", &teq::teq::set_event<teq::cv_event>)
		.def("get_cv_event", &teq::teq::get_event<teq::cv_event>)
	;
	
	enum_<teq::teq::transport_state>("transport_state")
		.value("STOPPED", teq::teq::transport_state::STOPPED)
		.value("PLAYING", teq::teq::transport_state::PLAYING)
	;

	enum_<teq::track::type>("track_type")
		.value("MIDI", teq::track::type::MIDI)
		.value("CV", teq::track::type::CV)
		.value("CONTROL", teq::track::type::CONTROL)
	;

	enum_<teq::midi_event::type>("midi_event_type")
		.value("NONE", teq::midi_event::type::NONE)
		.value("ON", teq::midi_event::type::ON)
		.value("OFF", teq::midi_event::type::OFF)
		.value("CC", teq::midi_event::type::CC)
		.value("PITCHBEND", teq::midi_event::PITCHBEND)
	;

	enum_<teq::cv_event::type>("cv_event_type")
		.value("NONE", teq::cv_event::type::NONE)
		.value("INTERVAL", teq::cv_event::type::INTERVAL)
	;
	
	enum_<teq::control_event::type>("control_event_type")
		.value("NONE", teq::control_event::type::NONE)
		.value("GLOBAL_TEMPO", teq::control_event::type::GLOBAL_TEMPO)
		.value("RELATIVE_TEMPO", teq::control_event::type::RELATIVE_TEMPO)
	;
}