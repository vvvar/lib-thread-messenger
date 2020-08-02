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
  using MessageName = const char*;
  
  Messenger():
    _logger(logging::LoggerFactory::createLogger("Messenger"))
  {}
  template<typename T>
  void send(ChannelName channel_name, MessageName message_name, SharedPtr<T> message_data_ptr)
  {
    _logger.log("Publishing message %s to the channel %s...", message_name, channel_name);
    if (isChannelExists(channel_name)) {
      std::lock_guard<std::mutex> lock(_channels_mutex);
      _channels_map.at(channel_name)->publish(message_name, message_data_ptr);
    } else {
      _logger.error("No such channel %s!", channel_name);
    }
  }
  template<typename T>
  SharedPtr<T> receive(ChannelName channel_name, MessageName message_name)
  {
    _logger.log("Unpublishing %s message from the channel %s...", message_name, channel_name);
    if (isChannelExists(channel_name)) {
      std::lock_guard<std::mutex> lock(_channels_mutex);
      return _channels_map.at(channel_name)->unpublish<T>(message_name);
    } else {
      _logger.error("No such channel %s!", channel_name);
      throw std::runtime_error("attempt to receive message from empty channel");
    }
  }
  void registerChannel(ChannelName channel_name)
  {
    _logger.log("Creating channel: %s...", channel_name);
    if (!isChannelExists(channel_name)) {
      auto channel_ptr = std::make_shared<channel::Channel>(channel_name);
      std::lock_guard<std::mutex> lock(_channels_mutex);
      _channels_map[channel_name] = channel_ptr;
    } else {
      _logger.error("Attempt to create channel %s that already exists!", channel_name);
    }
  }
  void unregisterChannel(ChannelName channel_name)
  {
    _logger.log("Removing channel: %s...", channel_name);
    if (isChannelExists(channel_name)) {
      std::lock_guard<std::mutex> lock(_channels_mutex);
      auto channel = _channels_map.find(channel_name);
      _channels_map.erase(channel);
    } else {
      _logger.error("Attempt to remove channel %s that does not exist!", channel_name);
    }
  }
  void waitForMessageInChannel(ChannelName channel_name)
  {
    if (isChannelExists(channel_name)) {
      _logger.log("Waiting for new messages from channel %s...", channel_name);
      getChannel(channel_name)->waitUnitlMessage();
      _logger.log("Received new messages in channel %s!", channel_name);
    } else {
      _logger.error("Attempt to wait for channel %s that does not exists!", channel_name);
    }
  }
  
private:
  using ChanelPtr  = SharedPtr<channel::Channel>;
  using ChanelsMap = std::map<ChannelName, ChanelPtr>;
  using Logger     = logging::Logger;
  
  ChanelsMap _channels_map;
  std::mutex _channels_mutex;
  Logger     _logger;
  
  bool isChannelExists(ChannelName channel_name)
  {
    std::lock_guard<std::mutex> lock(_channels_mutex);
    return _channels_map.find(channel_name) != _channels_map.end();
  }
  ChanelPtr getChannel(ChannelName channel_name)
  {
    std::lock_guard<std::mutex> lock(_channels_mutex);
    return _channels_map.at(channel_name);
  }
};

}
