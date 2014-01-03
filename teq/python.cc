#include <teq/teq.h>

#include <boost/python.hpp>

BOOST_PYTHON_MODULE(teq)
{
	using namespace boost::python;
	
	class_<teq::transport_position>("transport_position")
		.def_readwrite("pattern", &teq::transport_position::m_pattern)
		.def_readwrite("tick", &teq::transport_position::m_tick)
	;
	
	enum_<teq::transport_state>("transport_state")
		.value("STOPPED", teq::transport_state::STOPPED)
		.value("PLAYING", teq::transport_state::PLAYING)
	;

	class_<teq::teq::state_info>("state_info")
		.def_readwrite("transport_state", &teq::teq::state_info::m_transport_state)
		.def_readwrite("transport_position", &teq::teq::state_info::m_transport_position)
		.def_readwrite("loop_range", &teq::teq::state_info::m_loop_range)
	;

	class_<teq::loop_range>("loop_range")
		.def_readwrite("enabled", &teq::loop_range::m_enabled)
		.def_readwrite("start", &teq::loop_range::m_start)
		.def_readwrite("end", &teq::loop_range::m_end)
	;
	

	enum_<teq::track::type>("track_type")
		.value("MIDI", teq::track::type::MIDI)
		.value("CV", teq::track::type::CV)
		.value("CONTROL", teq::track::type::CONTROL)
	;


	class_<teq::midi_event>("midi_event", init<optional<teq::midi_event::type, unsigned, unsigned>>())
		.def_readwrite("type", &teq::midi_event::m_type)
		.def_readwrite("value1", &teq::midi_event::m_value1)
		.def_readwrite("value2", &teq::midi_event::m_value2)
	;

	enum_<teq::midi_event::type>("midi_event_type")
		.value("NONE", teq::midi_event::type::NONE)
		.value("ON", teq::midi_event::type::ON)
		.value("OFF", teq::midi_event::type::OFF)
		.value("CC", teq::midi_event::type::CC)
		.value("PITCHBEND", teq::midi_event::PITCHBEND)
	;


	class_<teq::cv_event>("cv_event", init<optional<teq::cv_event::type, float, float>>())
		.def_readwrite("type", &teq::cv_event::m_type)
		.def_readwrite("value1", &teq::cv_event::m_value1)
		.def_readwrite("value2", &teq::cv_event::m_value2)
	;

	enum_<teq::cv_event::type>("cv_event_type")
		.value("NONE", teq::cv_event::type::NONE)
		.value("INTERVAL", teq::cv_event::type::INTERVAL)
	;
	

	class_<teq::control_event>("control_event", init<optional<teq::control_event::type, float>>())
		.def_readwrite("type", &teq::control_event::m_type)
		.def_readwrite("value1", &teq::control_event::m_value)
	;

	enum_<teq::control_event::type>("control_event_type")
		.value("NONE", teq::control_event::type::NONE)
		.value("GLOBAL_TEMPO", teq::control_event::type::GLOBAL_TEMPO)
		.value("RELATIVE_TEMPO", teq::control_event::type::RELATIVE_TEMPO)
	;

	class_<teq::pattern>("pattern")
		.def("set_midi_event", &teq::pattern::set_event<teq::midi_event>)
		.def("get_midi_event", &teq::pattern::get_event<teq::midi_event>)
		.def("set_control_event", &teq::pattern::set_event<teq::control_event>)
		.def("get_control_event", &teq::pattern::get_event<teq::control_event>)
		.def("set_cv_event", &teq::pattern::set_event<teq::cv_event>)
		.def("get_cv_event", &teq::pattern::get_event<teq::cv_event>)
		.def("length", &teq::pattern::length)
		.def_readwrite("name", &teq::pattern::m_name)
	;
	
	
	class_<teq::teq>("teq", init<optional<std::string, unsigned>>())
		.def("gc", &teq::teq::gc)
		.def("set_global_tempo", &teq::teq::set_global_tempo)
		.def("get_global_tempo", &teq::teq::set_global_tempo)
		.def("set_loop_range", &teq::teq::set_loop_range)
		.def("get_loop_range", &teq::teq::get_loop_range)
		.def("set_transport_state", &teq::teq::set_transport_state)
		.def("set_transport_position", &teq::teq::set_transport_position)
		.def("set_send_all_notes_off_on_loop", &teq::teq::set_send_all_notes_off_on_loop)
		.def("set_send_all_notes_off_on_stop", &teq::teq::set_send_all_notes_off_on_stop)
		.def("number_of_tracks", &teq::teq::number_of_tracks)
		.def("track_name", &teq::teq::track_name)
		.def("track_type", &teq::teq::track_type)
		.def("insert_midi_track", &teq::teq::insert_midi_track)
		.def("insert_cv_track", &teq::teq::insert_cv_track)
		.def("insert_control_track", &teq::teq::insert_control_track)
		.def("insert_pattern", &teq::teq::insert_pattern)
		.def("number_of_patterns", &teq::teq::number_of_patterns)
		.def("create_pattern", &teq::teq::create_pattern)
		.def("get_pattern", &teq::teq::get_pattern)
		.def("get_state_info", &teq::teq::get_state_info)
		.def("wait", &teq::teq::wait)
	;
}