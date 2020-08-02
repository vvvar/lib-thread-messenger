#include <lib-thread-messenger/Messenger.hpp>

namespace messenger
{

Messenger::Messenger():
  _logger(logging::LoggerFactory::createLogger("Messenger"))
{}

void Messenger::registerChannel(ChannelName channel_name)
{
  _logger.log("Creating channel: %s...", channel_name);
  if (!isChannelExists(channel_name)) {
    auto channel_ptr = std::make_shared<channel::Channel>(channel_name);
    std::lock_guard<std::mutex> lock(_channels_mutex);
    _channels_map[channel_name] = channel_ptr;
  } else {
    _logger.error("Attempt to create channel %s that already exists!", channel_name);
  }
}

void Messenger::unregisterChannel(ChannelName channel_name)
{
  _logger.log("Removing channel: %s...", channel_name);
  if (isChannelExists(channel_name)) {
    std::lock_guard<std::mutex> lock(_channels_mutex);
    auto channel = _channels_map.find(channel_name);
    _channels_map.erase(channel);
  } else {
    _logger.error("Attempt to remove channel %s that does not exist!", channel_name);
  }
}

void Messenger::waitForMessageInChannel(ChannelName channel_name)
{
  if (isChannelExists(channel_name)) {
    _logger.log("Waiting for new data from channel %s...", channel_name);
    getChannel(channel_name)->waitUnitlMessage();
    _logger.log("Received data in the channel %s!", channel_name);
  } else {
    _logger.error("Attempt to wait for channel %s that does not exists!", channel_name);
  }
}

void Messenger::ThrowError(ErrorText error_text)
{
  throw std::runtime_error("attempt to receive message from empty channel");
}

bool Messenger::isChannelExists(ChannelName channel_name)
{
  std::lock_guard<std::mutex> lock(_channels_mutex);
  return _channels_map.find(channel_name) != _channels_map.end();
}

Messenger::SharedPtr<channel::Channel> Messenger::getChannel(ChannelName channel_name)
{
  std::lock_guard<std::mutex> lock(_channels_mutex);
  return _channels_map.at(channel_name);
}

}
