#pragma once
#include <queue>
#include "./Message.hpp"

namespace messageq
{

class MessageQ
{
public:
  template<typename T>
  using SharedPtr   = std::shared_ptr<T>;
  using MessageName = const char*;
  
  template<typename T>
  void push(MessageName message_name, SharedPtr<T> message_data_ptr)
  {
    auto message_ptr = std::make_shared<message::Message>(message_name, message_data_ptr);
    std::lock_guard<std::mutex> lock(_message_q_mutex);
    _message_q.push(message_ptr);
  }
  template<typename T>
  SharedPtr<T> pop()
  {
    std::lock_guard<std::mutex> lock(_message_q_mutex);
    if (_message_q.empty()) {
      throw std::runtime_error("messages stack underflow");
    }
    auto message_ptr = _message_q.front();
    _message_q.pop();
    return std::static_pointer_cast<T>(message_ptr->getData());
  }
private:
  std::queue<SharedPtr<message::Message>> _message_q;
  std::mutex                              _message_q_mutex;
};

}
