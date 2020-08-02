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
      ThrowError("no messages with such name");
    }
    return getMessageQ(q_name)->pop<T>();
  }
  
private:
  using MessageQName = std::string;
  using MessageQMap  = std::map<MessageQName, SharedPtr<messageq::MessageQ>>;
  using ErrorText    = std::string;
  
  MessageQMap _message_q_map;
  std::mutex  _message_q_map_mutex;
  
  bool isMessageQExists(MessageQName q_name);
  void createMessageQ(MessageQName q_name);
  void ThrowError(ErrorText error_text);
  SharedPtr<messageq::MessageQ> getMessageQ(MessageQName q_name);
};

}
