#pragma once
#include "./Messenger.hpp"

namespace libthreadmessenger
{

using MessengerPtr = std::shared_ptr<messenger::Messenger>;

MessengerPtr MakeMessenger()
{
   return std::make_shared<messenger::Messenger>();
}

}


