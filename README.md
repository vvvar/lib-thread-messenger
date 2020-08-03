# lib-thread-messenger
> Simple C++ data messaging library.

# Table of Contents
1. [Motivation](#motivation)
2. [Overview](#overview)
3. [Installation](#installation)
4. [Usage example](#usage-example)

## Motivation

During the development of my home pet-project, I was challenged to establish simple communication between threads. The communication consisted in sending of pointers to the objects between any number of threads. After researching, I wasn't able to find any simple enough solution for my task that can be easily integrated using CMake into my project, so I've docided to create my own one, especially since it's very simple.

## Overview

This library provides you with `Messenger` class that is capable of sending and receiving pointers to any kind of objects. Primary use case is data from one thread to another using single source of truth.

## Installation

Download and add to your CMake project:
```SH
add_subdirectory(lib-thread-messenger)
```

Another option for the CMake might be downloading it directly from the GitHub:
```SH
include(FetchContent)
FetchContent_Declare(
  lib-thread-messenger
  GIT_REPOSITORY git@github.com:vvvar/lib-thread-messenger.git
)
FetchContent_MakeAvailable(lib-thread-messenger)

target_link_libraries(${PROJECT_NAME}
	PRIVATE
		lib-thread-messenger
)

```

## Usage example

Let's imagine that we're challenged with a task to create simple application that shall ask user for name from console-based UI and print it. Sound's childish, isn't it? But let's now imagine that user data should be handled in one thread and processed in another. Now we should somehow pass data between UI thread and worker thread.

First, include main header file:
```CPP
#include <lib-thread-messenger/ThreadMessenger.hpp>
```

Let's definine the data. All classes that we would like to transfer between threads should inherit from `MessageData` that can be found in the `message` namespace. Here is how it will lok like:
```CPP
class UserData: public MessageData
{
public:
  using UserName = std::string;
  UserData(UserName user_name):
    MessageData(),
    _user_name(user_name)
  {}
  UserName getUserName()
  {
    return _user_name;
  }
private:
  UserName _user_name;
};
```

The main class that we will work with to send/receive data is `Messenger` and can be created using builder function:
```CPP
auto messenger_ptr = libthreadmessenger::MakeMessenger();
```
This will build instance of `Messenger` class wrapped in shared pointer.

In order to not notify each thread after every data transfer, we can use channels. Basic idea is to use one channel per thread, but you're free to use any amount of channels in any thread. To register channel, use `registerChannel(ChannelName)` method:
```CPP
messenger_ptr->registerChannel("worker");
```

Then, let's create two threads, first is Worker Thread that will be responsible for processing(simple print in our case) of data and UI Thread that will prompt data in console and will send it to the Worker Thread.
```CPP
void Worker_Thread(std::shared_ptr<Messenger> messenger_ptr)
{
  messenger_ptr->registerChannel("worker");
  messenger_ptr->waitForMessageInChannel("worker");
  auto user_data_ptr = messenger_ptr->receive<UserData>("worker");
  std::cout << "Worker got user data: " << user_data_ptr->getUserName() << ".\n";
}

void UI_Thread(std::shared_ptr<Messenger> messenger_ptr)
{
  std::cout << "Enter your name: ";
  std::string input;
  std::getline (std::cin, input);
  std::cin.clear();
  auto user_data_ptr = std::make_shared<UserData>(input);
  messenger_ptr->send<UserData>("worker", user_data_ptr);
}
```

In the Worker Thread, we would like to wait until data will come, as far as we're using _worker_ channel, let's wait until data will come there:
```CPP
// freeze thread until new data in channel
messenger_ptr->waitForMessageInChannel("worker");
```

Use `receive<DataType>(ChannelName)` method to receive desired data:
```CPP
auto user_data_ptr = messenger_ptr->receive<UserData>("worker");
```

And `send<DataType>(ChannelName, DataPtr)` to send it:
```CPP
messenger_ptr->send<UserData>("worker", user_data_ptr);
```

Now, let's run everything:
```CPP
int main()
{
  auto messenger_ptr = libthreadmessenger::MakeMessenger();
  
  messenger_ptr->registerChannel("worker");
  
  std::thread t1(Worker_Thread, messenger_ptr);
  std::thread t2(UI_Thread, messenger_ptr);

  t2.detach();
  t1.join();
  
  return 0;
}
```

```bash
Enter your name: jjj
Worker got user data: jjj.
Program ended with exit code: 0
```

Complete example as a working project can be found in the [Examples project](https://github.com/vvvar/lib-thread-messenger-examples).