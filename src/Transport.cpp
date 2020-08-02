#include <lib-thread-messenger/Transport.hpp>

namespace transport
{

bool Transport::isMessageQExists(MessageQName q_name)
{
  std::lock_guard<std::mutex> lock(_message_q_map_mutex);
  MessageQMap::const_iterator it = _message_q_map.find(q_name);
  return (it != _message_q_map.end());
}

void Transport::createMessageQ(MessageQName q_name)
{
  if (!isMessageQExists(q_name)) {
    std::lock_guard<std::mutex> lock(_message_q_map_mutex);
    _message_q_map[q_name] = std::make_shared<messageq::MessageQ>();
  }
}

void Transport::ThrowError(ErrorText error_text)
{
  throw std::runtime_error(error_text);
}

Transport::SharedPtr<messageq::MessageQ> Transport::getMessageQ(MessageQName q_name)
{
  createMessageQ(q_name);
  std::lock_guard<std::mutex> lock(_message_q_map_mutex);
  auto message_q = _message_q_map.find(q_name);
  return message_q->second;
}

}


