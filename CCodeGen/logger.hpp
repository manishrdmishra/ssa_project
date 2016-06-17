#ifndef LOG_HPP_
#define LOG_HPP_

#include <iostream>
#include <string>
#include <ctime>
#include <sstream>
#include <mutex>
#include <fstream>
#include <stdexcept>
#include <memory>

#define ONE_MB 1024*1024
enum LEVEL {
	ALL = 1, DEBUG, INFO, WARN, ERROR, FATAL, OFF
};
class log_policy_interface {
public:
	virtual ~log_policy_interface() {
	}
	;

	virtual void open_ostream(const std::string& name) = 0;
	virtual void close_ostream() = 0;
	virtual void write(const std::string& msg) = 0;
	virtual void set_file_size(long size_limit) = 0;

};

/*
 * Implementation which allow to write into a file
 */

class file_log_policy: public log_policy_interface {
public:
	file_log_policy() :
			file_out_stream(new std::ofstream) {
		number_of_files = 0;
		//file_base_name = nullptr;
		file_size_limit = 2 * ONE_MB;
	}
	~file_log_policy();
	void open_ostream(const std::string& name);
	void close_ostream();
	void write(const std::string& msg);
	void set_file_size(long size_limit) {
		file_size_limit = size_limit;

	}

private:
	std::unique_ptr<std::ofstream> file_out_stream;
	long file_size_limit;
	std::string file_base_name;
	int number_of_files;
};

void file_log_policy::open_ostream(const std::string& name) {
	if (number_of_files == 0) {
		file_base_name = name;
	}
	file_out_stream->open(name.c_str(),
			std::ios_base::binary | std::ios_base::out);
	if (!file_out_stream->is_open()) {
		throw(std::runtime_error("LOGGER: Unable to open an output stream"));
	}

}

void file_log_policy::close_ostream() {
	if (file_out_stream) {
		file_out_stream->close();
	}
}

void file_log_policy::write(const std::string& msg) {
	(*file_out_stream) << msg << std::endl;
	long pos = file_out_stream->tellp();
	if (pos > file_size_limit) {
		close_ostream();
		std::ostringstream convert; // stream used for the conversion
		number_of_files = number_of_files + 1;
		convert << number_of_files;
		std::string next_file_name = file_base_name + convert.str();
		open_ostream(next_file_name);
	}
}

file_log_policy::~file_log_policy() {
	if (file_out_stream) {
		close_ostream();
	}
}

/*
 * Implementation which allow to write on standard output
 */
class console_log_policy: public log_policy_interface {
public:
	console_log_policy() {
		console_out_stream = &std::cout;
	}
	~console_log_policy();
	void open_ostream(const std::string& name) {
	}
	void close_ostream() {
	}
	inline void write(const std::string& msg) {

		(*console_out_stream) << msg << std::endl;

	}
	void set_file_size(long size_limit) {
	}

private:
	std::ostream* console_out_stream;
};

console_log_policy::~console_log_policy() {
	if (console_out_stream) {

		close_ostream();
	}

}
template<typename log_policy>
class logger {

public:

	logger(const std::string& name, LEVEL new_level = LEVEL::DEBUG);
	~logger();

	template<LEVEL severity, typename ...Args>
	void print(Args ...args);
	inline void set_level(LEVEL new_level) {
		level = new_level;
	}
	inline LEVEL get_level() {
		return level;
	}
	void set_file_size(long size_limit) {
		policy->set_file_size(size_limit);
	}

private:
	unsigned log_line_number;
	std::string get_time();
	std::string get_logline_header();
	std::stringstream log_stream;
	log_policy_interface* policy;
	std::mutex write_mutex;
	LEVEL level;
	//Core printing functionality
	void print_impl();
	template<typename First, typename ...Rest>
	void print_impl(First parm1, Rest ...parm);

};
template<typename log_policy>
logger<log_policy>::logger(const std::string& name, LEVEL new_level) {
	log_line_number = 0;
	policy = new log_policy;
	if (!policy) {
		throw std::runtime_error(
				"LOGGER: Unable to create the logger instance");
	}
	policy->open_ostream(name);
	level = new_level;
}

template<typename log_policy>
logger<log_policy>::~logger() {
	if (policy) {
		policy->close_ostream();
		delete policy;
	}
}
template<typename log_policy>
template<typename First, typename ...Rest>
void logger<log_policy>::print_impl(First parm1, Rest ...parm) {
	log_stream << parm1;
	print_impl(parm...);
}
/*
 * if the severity of the message is higher than the set
 * level then only it will be logged.
 */
template<typename log_policy>
template<LEVEL severity, typename ...Args>
void logger<log_policy>::print(Args ...args) {
	if (severity >= level) {
		write_mutex.lock();
		switch (severity) {
		case LEVEL::ALL:
			log_stream << "<ALL> :";
			break;
		case LEVEL::INFO:
			log_stream << "<INFO> :";
			break;
		case LEVEL::FATAL:
			log_stream << "<FATAL> :";
			break;
		case LEVEL::DEBUG:
			log_stream << "<DEBUG> :";
			break;
		case LEVEL::WARN:
			log_stream << "<WARNING> :";
			break;
		case LEVEL::ERROR:
			log_stream << "<ERROR> :";
			break;
		};
		print_impl(args...);
		write_mutex.unlock();
	}
}

template<typename log_policy>
void logger<log_policy>::print_impl() {
	policy->write(get_logline_header() + log_stream.str());
	log_stream.str("");
}

template<typename log_policy>
std::string logger<log_policy>::get_time() {
	std::string time_str;
	time_t raw_time;

	time(&raw_time);
	time_str = ctime(&raw_time);

	//without the newline character
	return time_str.substr(0, time_str.size() - 1);
}

template<typename log_policy>
std::string logger<log_policy>::get_logline_header() {
	std::stringstream header;

	header.str("");
	header.fill('0');
	header.width(7);
	header << log_line_number++ << " < " << get_time() << " - ";

	header.fill('0');
	header.width(7);
	header << clock() << " > ~ ";

	return header.str();
}
/*File logging macro */
#define INITIALIZE_FLOG(file_name,level) logger<file_log_policy> flog_inst(file_name,level)
#define FLOG_SETLEVEL(level) flog_inst.set_level(level)
#define FLOG_SET_FILE_SIZE(size) flog_inst.set_file_size(size)
#ifdef FLOGGING
#define FLOG flog_inst.print< LEVEL::DEBUG>
#define FLOG_ERR flog_inst.print< LEVEL::ERROR >
#define FLOG_WARN flog_inst.print< LEVEL::WARN >
#define FLOG_ALL flog_inst.print< LEVEL::ALL >
#define FLOG_FATAL flog_inst.print< LEVEL::FATAL >
#define FLOG_INFO flog_inst.print< LEVEL::INFO >
#else
#define FLOG(...)
#define FLOG_ERR(...)
#define FLOG_WARN(...)
#define FLOG_ALL (...)
#define FLOG_FATAL(...)
#define FLOG_INFO(...)
#endif

/*Console logging macro */
#define INITIALIZE_CLOG(level) logger<console_log_policy> clog_inst(" ", level)
#define CLOG_SETLEVEL(level) clog_inst.set_level(level)
#ifdef CLOGGING
#define CLOG clog_inst.print<DEBUG>
#define CLOG_ERR clog_inst.print< LEVEL::ERROR >
#define CLOG_WARN clog_inst.print< LEVEL::WARN >
#define CLOG_ALL flog_inst.print< LEVEL::ALL >
#define CLOG_FATAL flog_inst.print< LEVEL::FATAL >
#define CLOG_INFO flog_inst.print< LEVEL::INFO >
#else
#define CLOG(...)
#define CLOG_ERR(...)
#define CLOG_WARN(...)
#define CLOG_ALL(...)
#define CLOG_FATAL(...)
#define CLOG_INFO(...)
#endif

#endif
