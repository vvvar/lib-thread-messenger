#pragma once

namespace message
{

class MessageData {};

class Message
{
public:
  using MessageName = const char*;
  using MessageDataPtr = std::shared_ptr<MessageData>;
  Message(MessageName name, MessageDataPtr data):
    _name(name),
    _data(data)
  {}
  MessageName getName() const
  {
    return _name;
  }
  MessageDataPtr getData() const
  {
    return _data;
  }
private:
  MessageName    _name;
  MessageDataPtr _data;
};

}
