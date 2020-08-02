#pragma once
#include <string>
#include <thread>
#include <sstream>
#include <iostream>

namespace logging
{

class Logger
{
private:
  bool        _display_thread_id;
  std::string _prefix;
  
  std::string getThredId() const
  {
    auto myid = std::this_thread::get_id();
    std::stringstream ss;
    ss << myid;
    return ss.str();
  }
  template<typename ... Args>
  std::string format_string(const std::string& format, Args ... args) const
  {
    size_t size = snprintf( nullptr, 0, format.c_str(), args.c_str()... ) + 1; // Extra space for '\0'
    if (size <= 0)
    {
      throw std::runtime_error( "Error during formatting." );
    }
    std::unique_ptr<char[]> buf( new char[ size ] );
    snprintf( buf.get(), size, format.c_str(), args.c_str()... );
    return std::string( buf.get(), buf.get() + size - 1 );
  }
  std::string prefix_string(const std::string& channel, const std::string& str) const
  {
    if (_display_thread_id)
    {
      return channel + " - " + "[" + getThredId() + "] " + "[" + _prefix + "] " + str + "\n";
    }
    else
    {
      return channel + " - " + "[" + _prefix + "] " + str + "\n";
    }
  }
  template<typename ... Args>
  std::string to_log_line(const std::string& channel, const std::string& format, Args ... args) const
  {
    const auto formatted = format_string(format, std::string(args)... );
    return prefix_string(channel, formatted);
  }
  
public:
  Logger(std::string prefix, bool display_thread_id):
    _prefix(prefix),
    _display_thread_id(display_thread_id)
  {}
  template<typename ... Args>
  void log(const std::string& format, Args ...args) const
  {
    std::cout << to_log_line("log", format, args...);
  }
  template<typename ... Args>
  void error(const std::string& format, Args ...args) const
  {
    std::cerr << to_log_line("err", format, args...);
  }
};

class LoggerFactory
{
private:
  static constexpr bool _debug_thread_info = true;
  LoggerFactory() {};
public:
  using LoggerPrefix = const std::string;
  static Logger createLogger(LoggerPrefix prefix = "default")
  {
    return Logger(prefix, _debug_thread_info);
  }
};

}
