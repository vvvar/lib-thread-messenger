#pragma once
#include "./Logging.hpp"
#include "./Channel.hpp"

namespace messenger
{

class Messenger
{
public:
  template<typename T>
  using SharedPtr = std::shared_ptr<T>;
  using ChannelName = std::string;
  
  Messenger();
  template<typename T>
  void send(ChannelName channel_name, SharedPtr<T> data_ptr)
  {
    if (isChannelExists(channel_name)) {
      getChannel(channel_name)->publish(data_ptr);
    } else {
      _logger.error("No such channel %s!", channel_name);
    }
  }
  template<typename T>
  SharedPtr<T> receive(ChannelName channel_name)
  {
    if (isChannelExists(channel_name)) {
      return getChannel(channel_name)->unpublish<T>();
    } else {
      _logger.error("No such channel %s!", channel_name);
      ThrowError("attempt to receive message from empty channel");
    }
  }
  void registerChannel(ChannelName channel_name);
  void unregisterChannel(ChannelName channel_name);
  void waitForMessageInChannel(ChannelName channel_name);
  
private:
  using ChanelsMap = std::map<ChannelName, SharedPtr<channel::Channel>>;
  using Logger     = logging::Logger;
  using ErrorText  = std::string;
  
  ChanelsMap _channels_map;
  std::mutex _channels_mutex;
  Logger     _logger;
  
  void ThrowError(ErrorText error_text);
  bool isChannelExists(ChannelName channel_name);
  SharedPtr<channel::Channel> getChannel(ChannelName channel_name);
};

}
