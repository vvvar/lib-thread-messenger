#pragma once
#include "./Logging.hpp"
#include "./Message.hpp"
#include "./Channel.hpp"

namespace messenger
{

using Logger = logging::Logger;
using LoggerFactory = logging::LoggerFactory;
using Message = message::Message;
using MessageData = message::MessageData;

template<typename T>
using MessageDataPtr = MessageData::MessageDataPtr<T>;
using ChannelName    = channel::ChannelName;

class Messenger
{
public:
  Messenger():
    _logger(LoggerFactory::createLogger("Messenger"))
  {}
  template<typename MessageDataType>
  void send(ChannelName channel_name, MessageDataPtr<MessageDataType> message_data_ptr)
  {
    publishToChannel(channel_name, Message::MakeMessage(message_data_ptr));
  }
  template<typename MessageDataType>
  MessageDataPtr<MessageDataType> receive(ChannelName channel_name)
  {
    auto message_name = typeid(MessageDataType).name();
    auto message_ptr = unpublishFromChannel(channel_name, message_name);
    return MessageData::CastTo<MessageDataType>(message_ptr->getData());
  }
  void registerChannel(ChannelName channel_name)
  {
    createChannel(channel_name);
  }
  void unregisterChannel(ChannelName channel_name)
  {
    removeChannel(channel_name);
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
  using MessageName = Message::MessageName;
  using MessagePtr  = Message::MessagePtr;
  using ChanelPtr   = channel::ChannelPtr;
  using ChanelsMap  = std::map<ChannelName, ChanelPtr>;
  
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
  void createChannel(ChannelName channel_name)
  {
    _logger.log("Creating channel: %s...", channel_name);
    if (!isChannelExists(channel_name)) {
      auto channel_ptr = channel::MakeChannel(channel_name);
      std::lock_guard<std::mutex> lock(_channels_mutex);
      _channels_map[channel_name] = channel_ptr;
    } else {
      _logger.error("Attempt to create channel %s that already exists!", channel_name);
    }
  }
  void removeChannel(ChannelName channel_name)
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
  void publishToChannel(ChannelName channel_name, MessagePtr message_ptr)
  {
    _logger.log("Publishing message %s to the channel %s...", message_ptr->getName(), channel_name);
    if (isChannelExists(channel_name)) {
      std::lock_guard<std::mutex> lock(_channels_mutex);
      _channels_map.at(channel_name)->publish(message_ptr);
    } else {
      _logger.error("No such channel %s!", channel_name);
    }
  }
  MessagePtr unpublishFromChannel(ChannelName channel_name, MessageName message_name)
  {
    _logger.log("Unpublishing %s message from the channel %s...", message_name, channel_name);
    if (isChannelExists(channel_name)) {
      std::lock_guard<std::mutex> lock(_channels_mutex);
      return _channels_map.at(channel_name)->unpublish(message_name);
    } else {
      _logger.error("No such channel %s!", channel_name);
      throw std::runtime_error("attempt to receive message from empty channel");
    }
  }
};

}
