#ifndef LIBTEQ_EXCEPTION_HH
#define LIBTEQ_EXCEPTION_HH

#define LIBTEQ_THROW_RUNTIME_ERROR(x) \
{ \
	std::stringstream exception_string_stream; \
	exception_string_stream << x; \
	throw std::runtime_error(exception_string_stream.str().c_str()); \
}

#endif
