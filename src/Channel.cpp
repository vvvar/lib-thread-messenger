#include <lib-thread-messenger/Channel.hpp>
#include <mutex>

namespace channel
{

Channel::Channel(ChannelName _name):
  _logger(logging::LoggerFactory::createLogger("Channel - " + _name)),
  _name(_name)
{}

void Channel::waitUnitlMessage()
{
  _logger.log("Waiting for new messages...");
  std::unique_lock<std::mutex> lock(_cv_mutex);
  _cv.wait(lock);
}

}
