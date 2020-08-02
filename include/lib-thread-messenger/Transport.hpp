#pragma once
#include <map>
#include "./MessageQ.hpp"

namespace transport
{

class Transport
{
public:
  template<typename T>
  using SharedPtr   = std::shared_ptr<T>;
  using MessageName = const char*;
  
  template<typename T>
  void send(MessageName message_name, SharedPtr<T> message_data_ptr)
  {
    if (!isMessageQExists(message_name)) {
      createMessageQ(message_name);
    }
    getMessageQ(message_name)->push(message_name, message_data_ptr);
  }
  template<typename T>
  SharedPtr<T> receive(MessageName message_name)
  {
    if (!isMessageQExists(message_name)) {
      throw std::runtime_error("no messages with such name");
    }
    return getMessageQ(message_name)->pop<T>();
  }
private:
  using MessageQName = std::string;
  using MessageQPtr  = SharedPtr<messageq::MessageQ>;
  using MessageQMap  = std::map<MessageQName, MessageQPtr>;
  
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
      _message_q_map[message_name] = std::make_shared<messageq::MessageQ>();
    }
  }
};

}
