#pragma once

namespace message
{

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

}
