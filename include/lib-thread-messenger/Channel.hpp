#pragma once
#include "./Message.hpp"
#include "./Transport.hpp"
#include "./Logging.hpp"

namespace channel
{

class Channel;

using ChannelName = std::string;
using ChannelPtr  = std::shared_ptr<Channel>;

ChannelPtr MakeChannel(ChannelName _name)
{
  return std::make_shared<Channel>(_name);
}

class Channel
{
public:
  using Message = message::Message;
  using Logger = logging::Logger;
  using LoggerFactory = logging::LoggerFactory;
  using Transport = transport::Transport;
  using MessageName = Message::MessageName;
  using MessagePtr  = Message::MessagePtr;
  
  Channel(ChannelName _name):
    _logger(LoggerFactory::createLogger("Channel - " + _name)),
    _name(_name)
  {}
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
  ChannelName              _name;
  Transport               _transport;
  std::condition_variable _cv;
  std::mutex              _cv_mutex;
  Logger                  _logger;
};

}
