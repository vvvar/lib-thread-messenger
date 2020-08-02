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
  
  Channel(ChannelName _name);
  template<typename T>
  void publish(SharedPtr<T> data_ptr)
  {
    _transport.send(data_ptr);
    _cv.notify_all();
  }
  template<typename T>
  SharedPtr<T> unpublish()
  {
    return _transport.receive<T>();
  }
  void waitUnitlMessage();
  
private:
  using Logger = logging::Logger;
  
  ChannelName             _name;
  transport::Transport    _transport;
  std::condition_variable _cv;
  std::mutex              _cv_mutex;
  Logger                  _logger;
};

}
