#pragma once
#include "./Transport.hpp"
#include "./Logging.hpp"

namespace channel
{

class Channel
{
public:
  template<typename T>
  using SharedPtr   = std::shared_ptr<T>;
  using ChannelName = std::string;
  using MessageName = const char*;
  
  Channel(ChannelName _name):
    _logger(logging::LoggerFactory::createLogger("Channel - " + _name)),
    _name(_name)
  {}
  template<typename T>
  void publish(MessageName message_name, SharedPtr<T> message_data_ptr)
  {
    _logger.log("Publishing message %s...", message_name);
    _transport.send(message_name, message_data_ptr);
    _cv.notify_all();
  }
  template<typename T>
  SharedPtr<T> unpublish(MessageName message_name)
  {
    _logger.log("Un-publishing message %s...", message_name);
    return _transport.receive<T>(message_name);
  }
  void waitUnitlMessage()
  {
    _logger.log("Waiting for new messages...");
    std::unique_lock<std::mutex> lock(_cv_mutex);
    _cv.wait(lock);
  }
  ChannelName getName()
  {
    return _name;
  }
private:
  using Logger = logging::Logger;
  
  ChannelName             _name;
  transport::Transport    _transport;
  std::condition_variable _cv;
  std::mutex              _cv_mutex;
  Logger                  _logger;
};

}
