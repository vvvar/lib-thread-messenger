#pragma once
#include <string>
#include <functional>
#include <algorithm>
#include <queue>
#include <map>
#include <list>
#include <memory>
#include <condition_variable>
#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <sstream>
#include <thread>
#include <utility>
#include <typeinfo>

namespace messenger
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

class MessageData
{
public:
  template<typename T>
  using MessageDataPtr     = std::shared_ptr<T>;
  using BaseMessageDataPtr = std::shared_ptr<MessageData>;
  
  /**
   * Casts MessageData pointer to any provided pointer type
   */
  template<typename MessageDataType>
  MessageDataPtr<MessageDataType> static CastTo(MessageDataPtr<MessageData> data)
  {
    return std::static_pointer_cast<MessageDataType>(data);
  }
};

class Message
{
public:
  using MessageName    = const char*;
  using MessageDataPtr = MessageData::BaseMessageDataPtr;
  using MessagePtr     = std::shared_ptr<Message>;
  
  Message(MessageName name, MessageDataPtr data) :
    _name(name),
    _data(data)
  {}
  /**
   * Makes pointer to Message with data pointer casted to base MessageData
   */
  template<typename MessageDataType>
  static MessagePtr MakeMessage(std::shared_ptr<MessageDataType> message_data_ptr)
  {
    auto abstract_message_data_ptr = std::static_pointer_cast<MessageData>(message_data_ptr);
    auto message_name = typeid(MessageDataType).name();
    return std::make_shared<Message>(message_name, abstract_message_data_ptr);
  }
  MessageDataPtr getData() const
  {
    return _data;
  }
  MessageName getName() const
  {
    return _name;
  }
private:
  MessageName    _name;
  MessageDataPtr _data;
};

class MessageQ
{
public:
  using MessageQPtr = std::shared_ptr<MessageQ>;
  using MessagePtr  = Message::MessagePtr;
  
  static MessageQPtr MakeMessageQ()
  {
    return std::make_shared<MessageQ>();
  }
  void push(MessagePtr message_ptr)
  {
    std::lock_guard<std::mutex> lock(_message_q_mutex);
    _message_q.push(message_ptr);
  }
  MessagePtr pop()
  {
    std::lock_guard<std::mutex> lock(_message_q_mutex);
    if (_message_q.empty()) {
      throw std::runtime_error("messages stack underflow");
    }
    auto message_ptr = _message_q.front();
    _message_q.pop();
    return message_ptr;
  }
private:
  std::queue<MessagePtr> _message_q;
  std::mutex             _message_q_mutex;
};

class Transport
{
public:
  using MessageQName = std::string;
  using MessageQPtr  = MessageQ::MessageQPtr;
  using MessageQMap  = std::map<MessageQName, MessageQPtr>;
  using MessageName  = Message::MessageName;
  using MessagePtr   = Message::MessagePtr;
  
  void send(MessagePtr message_ptr)
  {
    auto message_name = message_ptr->getName();
    if (!isMessageQExists(message_name)) {
      createMessageQ(message_name);
    }
    getMessageQ(message_name)->push(message_ptr);
  }
  MessagePtr receive(MessageName message_name)
  {
    if (!isMessageQExists(message_name)) {
      throw std::runtime_error("no messages with such name");
    }
    return getMessageQ(message_name)->pop();
  }
private:
  MessageQMap _message_q_map;
  std::mutex  _message_q_map_mutex;
  
  bool isMessageQExists(MessageQName message_q_name)
  {
    std::lock_guard<std::mutex> lock(_message_q_map_mutex);
    MessageQMap::const_iterator it = _message_q_map.find(message_q_name);
    return (it != _message_q_map.end());
  }
  MessageQPtr getMessageQ(MessageQName message_q_name)
  {
    createMessageQ(message_q_name);
    std::lock_guard<std::mutex> lock(_message_q_map_mutex);
    auto message_q = _message_q_map.find(message_q_name);
    return message_q->second;
  }
  void createMessageQ(MessageQName message_name)
  {
    if (!isMessageQExists(message_name)) {
      std::lock_guard<std::mutex> lock(_message_q_map_mutex);
      _message_q_map[message_name] = MessageQ::MakeMessageQ();
    }
  }
};

class Channel
{
public:
  using ChannelName = std::string;
  using ChannelPtr  = std::shared_ptr<Channel>;
  using MessageName = Message::MessageName;
  using MessagePtr  = Message::MessagePtr;
  
  Channel(ChannelName _name):
    _logger(LoggerFactory::createLogger("Channel - " + _name)),
    _name(_name)
  {}
  static ChannelPtr MakeChannel(ChannelName _name)
  {
    return std::make_shared<Channel>(_name);
  }
  void publish(MessagePtr message_ptr)
  {
    _logger.log("Publishing message %s...", message_ptr->getName());
    _transport.send(message_ptr);
    _cv.notify_all();
  }
  MessagePtr unpublish(MessageName message_name)
  {
    _logger.log("Un-publishing message %s...", message_name);
    return _transport.receive(message_name);
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
  ChannelName                    _name;
  Transport               _transport;
  std::condition_variable _cv;
  std::mutex              _cv_mutex;
  Logger                  _logger;
};

class Messenger
{
public:
  template<typename T>
  using MessageDataPtr = MessageData::MessageDataPtr<T>;
  using ChannelName    = Channel::ChannelName;
  
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
  using ChanelPtr   = Channel::ChannelPtr;
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
      auto channel_ptr = Channel::MakeChannel(channel_name);
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
