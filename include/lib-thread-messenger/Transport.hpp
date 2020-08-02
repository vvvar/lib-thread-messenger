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
  void send(SharedPtr<T> data_ptr)
  {
    auto q_name = typeid(T).name();
    if (!isMessageQExists(q_name)) {
      createMessageQ(q_name);
    }
    getMessageQ(q_name)->push(q_name, data_ptr);
  }
  template<typename T>
  SharedPtr<T> receive()
  {
    auto q_name = typeid(T).name();
    if (!isMessageQExists(q_name)) {
      throw std::runtime_error("no messages with such name");
    }
    return getMessageQ(q_name)->pop<T>();
  }
private:
  using MessageQName = std::string;
  using MessageQPtr  = SharedPtr<messageq::MessageQ>;
  using MessageQMap  = std::map<MessageQName, MessageQPtr>;
  
  MessageQMap _message_q_map;
  std::mutex  _message_q_map_mutex;
  
  bool isMessageQExists(MessageQName q_name)
  {
    std::lock_guard<std::mutex> lock(_message_q_map_mutex);
    MessageQMap::const_iterator it = _message_q_map.find(q_name);
    return (it != _message_q_map.end());
  }
  MessageQPtr getMessageQ(MessageQName q_name)
  {
    createMessageQ(q_name);
    std::lock_guard<std::mutex> lock(_message_q_map_mutex);
    auto message_q = _message_q_map.find(q_name);
    return message_q->second;
  }
  void createMessageQ(MessageQName q_name)
  {
    if (!isMessageQExists(q_name)) {
      std::lock_guard<std::mutex> lock(_message_q_map_mutex);
      _message_q_map[q_name] = std::make_shared<messageq::MessageQ>();
    }
  }
};

}
