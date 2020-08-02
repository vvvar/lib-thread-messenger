#pragma once
#include <queue>
#include "./Message.hpp"

namespace messageq
{

using Message = message::Message;

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

}
