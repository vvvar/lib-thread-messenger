#pragma once
#include <map>
#include "./Message.hpp"
#include "./MessageQ.hpp"

namespace transport
{

using Message = message::Message;
using MessageQ = messageq::MessageQ;

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

}
