#pragma once
#include "./Messenger.hpp"

std::shared_ptr<messenger::Messenger> MakeMessenger()
{
   return std::make_shared<messenger::Messenger>();
}
